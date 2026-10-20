// DynamicLightHistory.h: the dynamic terrain lights followed from frame to frame.
//
// Classic adds every dynamic light for one frame only: the map's fires and candles pick a new
// random brightness each frame, and the lights that effects add while they are drawn (Aqua Beam,
// for one) come after the terrain of that frame and are gone before the next. With Enhanced
// dynamic lights the game records its light calls here instead, and once per frame, before the
// terrain is drawn, takes the frame's lights from the history:
//   - a map or character light (fire, candle, weapon or monster glow) eases towards its new
//     brightness over a time constant, so it breathes instead of flickering;
//   - an effect light (skills, spells) can be stronger and reach further than the others, and
//     fades out over a time constant after its effect stops adding it; only its brightening part
//     is boosted (a light may darken);
//   - a light recorded after the frame's lights were taken (while drawing) is part of the next
//     frame's lights.
// Every call is one light with its own position, colour and range, as in Classic: calls are never
// added together. A call continues the nearest light of the same source within kMatchDistance that
// no other call of the frame continues (one with the same range first); otherwise it starts a new
// one. A call with a darkening (negative) channel bypasses the history: it lights exactly its own
// frame, unboosted, as in Classic. A new world starts with no followed lights.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

#include "Render/Lighting/DynamicLightList.h"

namespace Render::Lighting
{

// Where a light call comes from: the map's own objects (fires, candles), characters (weapon and
// monster glows, the dark lights some monsters cast), or anything else (skills, spells).
enum class LightSource : std::uint8_t
{
    Map,
    Character,
    Effect,
};

// The source of the light calls made while it lives (light calls default to Effect).
class LightSourceScope
{
public:
    explicit LightSourceScope(LightSource source);
    ~LightSourceScope();
    LightSourceScope(const LightSourceScope&) = delete;
    LightSourceScope& operator=(const LightSourceScope&) = delete;

private:
    LightSource m_previous;
};

[[nodiscard]] LightSource CurrentLightSource();

struct LightHistorySettings
{
    // Time constant a map or character light eases towards its new brightness with; 0 is Classic.
    float mapSmoothingSeconds = 0.f;
    // Effect lights: brightness factor (on top of the light mood's gain), range factor, and the time
    // constant they fade out with once no longer added; 1, 1 and 0 are Classic.
    float effectGain = 1.f;
    float effectRangeScale = 1.f;
    // The most terrain cells the range factor may widen an effect light to (never below the light's
    // own range); 0 is no limit. A spell of many short lights (Aqua Beam, range 2) can reach further
    // than one of a few wide ones (Inferno's explosions, range 4) would at the same factor.
    float effectRangeMax = 0.f;
    float effectFadeSeconds = 0.f;
    // The most light effect lights may add up to at any of their positions (strongest channel,
    // with the effect gain, before the light mood's gain); 0 is no limit (Classic). Overlapping
    // effect lights (called AND fading) use one final-envelope scale per connected component,
    // bounded at every participating source centre. This is not a bound at arbitrary pixels or
    // after mood/static/map light. Every tracked effect participates; 0 bypasses the limiter.
    float effectCap = 0.f;

    bool operator==(const LightHistorySettings&) const = default;
};

// A light call: world position, colour (may be negative), range in terrain cells.
struct LightCall
{
    float x = 0.f;
    float y = 0.f;
    float color[3]{};
    int range = 0;
    LightSource source = LightSource::Effect;
};

// Refused history calls since the last clear, and which clear that was (ordered Classic fallback).
struct DropCount
{
    std::size_t total = 0;
    std::uint32_t generation = 0;
};

class DynamicLightHistory
{
public:
    // Calls within this many world units of a followed light continue it (a fire's flames move
    // by up to 8 units on each axis from frame to frame).
    static constexpr float kMatchDistance = 24.f;
    static constexpr std::size_t kCapacity = 1024;
    // All followed effect lights participate in the cap; no uncapped subset.
    // How far the lights no call continued may fill the frame's light list. The calls themselves
    // always go out in full, as in Classic (the list keeps its first entries); the lights no call
    // continued are displaceable there, so later calls (darkening lights added while the frame is
    // drawn) replace the weakest of them (DynamicLightList::MarkDisplaceable).
    static constexpr std::size_t kListBudget = DynamicLightList::kCapacity;

    // A new tick of the game. The calls of an earlier tick that no Take() collected (a tick without
    // a drawn frame) are dropped; calls recorded between the last Take() and the first tick after
    // it (while that frame was drawn) stay for the next Take().
    void BeginTick();
    // False at capacity or during this frame's Classic overflow phase. Before the frame is drawn
    // the caller MUST first apply DrainToClassic() in order, then the refused call; while drawing
    // it applies only the refused call (see LightGrid::RecordFrameLight).
    bool Record(const LightCall& call);
    // On the first refusal, drain the raw prefix before applying the refused tail. This frame
    // then uses ordered Classic light calls, with no history transforms or lingering lights.
    [[nodiscard]] std::span<const LightCall> DrainToClassic();
    // How many of the last Take()'s lights came from calls (the rest linger from earlier frames).
    [[nodiscard]] std::size_t CalledOut() const
    {
        return m_calledOut;
    }
    // The calls belong to `world`: when it differs from the last one, forgets every queued call,
    // the followed lights and an overflow phase (they all belong to the old world).
    void SyncWorld(int world);
    // Between Take() and the next tick: the frame is being drawn, its terrain light is final. A
    // darkening call made now takes the Classic path (this frame's later models only) instead.
    [[nodiscard]] bool IsDrawing() const
    {
        return m_drawing;
    }
    // The lights of this frame after `seconds` since the last Take(): every call recorded since,
    // in call order, then the lights no call continued, strongest first, while the total stays
    // within kListBudget. The effect gain and range factor are applied; the span is valid until the
    // next Record() or Take().
    [[nodiscard]] std::span<const LightCall> Take(float seconds, const LightHistorySettings& settings);
    void Clear();

    [[nodiscard]] std::size_t FollowedCount(LightSource source) const;
    // Calls the history could not take since Clear() (they took the Classic path). A new light
    // first replaces the weakest light no call of the frame continues.
    [[nodiscard]] std::size_t Overflow() const
    {
        return m_overflow;
    }
    // Changes with every Clear(), which also restarts Overflow().
    [[nodiscard]] std::uint32_t Generation() const
    {
        return m_generation;
    }
    [[nodiscard]] DropCount Drops() const
    {
        return {m_overflow, m_generation};
    }

private:
    struct Followed
    {
        float x = 0.f;
        float y = 0.f;
        int range = 0;
        LightSource source = LightSource::Effect;
        // The colour called this frame (0 if not called), and the colour shown.
        float target[3]{};
        float value[3]{};
        bool called = false;
        bool fresh = true;
    };

    // Calls recorded since the last Take(); `m_tickStart` is the first one of the current tick.
    std::array<LightCall, kCapacity> m_calls{};
    std::size_t m_callCount = 0;
    std::size_t m_tickStart = 0;
    bool m_tickOpen = false;
    std::array<Followed, kCapacity> m_followed{};
    std::size_t m_followedCount = 0;
    std::array<LightCall, kCapacity> m_out{};
    std::uint32_t m_generation = 0;
    std::size_t m_overflow = 0;

    static constexpr std::size_t kBypass = static_cast<std::size_t>(-1);
    static constexpr std::size_t kUnplaced = static_cast<std::size_t>(-2);
    static constexpr std::size_t kDropped = static_cast<std::size_t>(-3);

    std::array<std::size_t, kCapacity> m_callLight{};
    std::array<std::size_t, kCapacity> m_order{};
    std::array<std::size_t, kCapacity> m_capParent{};
    std::array<double, kCapacity> m_capTotal{};
    std::array<double, kCapacity> m_capPeak{};
    std::array<float, kCapacity> m_capRadius{};
    std::array<float, kCapacity> m_capStrength{};
    bool m_classicOverflow = false;
    bool m_drawing = false;
    int m_world = (std::numeric_limits<int>::min)();
    std::size_t m_calledOut = 0;

    // Distance (world units) of one terrain cell, for the effect cap's falloff.
    static constexpr float kCellSize = 100.f;
    void LimitEffectLights(const LightHistorySettings& settings);
    std::size_t CapRoot(std::size_t index);
    std::size_t PrepareEffectCap(const LightHistorySettings& settings);
    void MeasureEffectCap(std::size_t count);
    [[nodiscard]] Followed* Continued(const LightCall& call);
    [[nodiscard]] Followed* Slot();
    std::size_t Continue(Followed& light, const LightCall& call);
    [[nodiscard]] static LightCall Shown(const Followed& light, const LightHistorySettings& settings);
    [[nodiscard]] static int EffectRange(int range, const LightHistorySettings& settings);
};

// Reports dropped calls at most once per IntervalSeconds; a new generation of the count (the
// history was cleared) starts it over.
class DropReporter
{
public:
    static constexpr float IntervalSeconds = 60.f;
    // How many drops to report now (0: nothing new, or reported less than a minute ago).
    [[nodiscard]] std::size_t Due(DropCount count, float nowSeconds);

private:
    std::size_t m_reported = 0;
    std::uint32_t m_generation = 0;
    float m_lastSeconds = 0.f;
    bool m_hasReported = false;
};

// The history the game's terrain light calls record into (its settings come from the look
// profile, Render::Look::LightHistorySettings).
[[nodiscard]] DynamicLightHistory& FrameLightHistory();

} // namespace Render::Lighting
