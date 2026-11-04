#pragma once

#include "Core/Math/ZzzMathLib.h"

#include <cstdint>

// The developer switch "fixedtick": in the world, the update runs in whole steps at the reference rate (ADR 0001) and
// each frame draws between the last two steps. Everything else in the frame still runs once per frame.
namespace Scenes::FixedTick
{
inline constexpr double TickMs = 40.0;
// A longer stall drops the time past this many ticks rather than catching up on it.
inline constexpr int MaxTicksPerFrame = 5;

class Clock
{
public:
    // The whole ticks `elapsedMs` completes.
    int Advance(double elapsedMs);
    // How far the frame is between the last tick and the next, in [0, 1).
    [[nodiscard]] float Alpha() const;

private:
    double m_pendingMs = 0.0;
};

// Where to draw between two ticks' positions; a jump no walk makes (a teleport, a respawned slot) isn't crossed.
void Interpolate(const vec3_t previous, const vec3_t current, float alpha, vec3_t out);
// An angle in degrees between two ticks, the short way round.
[[nodiscard]] float InterpolateAngle(float previous, float current, float alpha);
// An animation frame between two ticks; a new action or a loop holds the tick's frame until the next tick.
[[nodiscard]] float InterpolateFrame(float previous, float current, float alpha, bool sameAction);

[[nodiscard]] bool IsOn();
void SetOn(bool on);
// Whether this frame's world update runs in ticks.
[[nodiscard]] bool RunsWorld();
void BeginFrame(double nowMs);
[[nodiscard]] int TicksThisFrame();
[[nodiscard]] std::uint64_t TicksRun();

// One tick: the frame-rate-scaled code takes exactly one reference frame.
class TickScope
{
public:
    TickScope();
    ~TickScope();
    TickScope(const TickScope&) = delete;
    TickScope& operator=(const TickScope&) = delete;

private:
    float m_factor;
    int m_step;
};

// Keeps the positions, angles and animation frames the coming tick moves from.
void CaptureTransforms();
// For the frame's camera and draws, moved objects stand and pose between ticks; the draws animate them into bones of
// their own. Restore gives the update's values back before anything can change them.
void ApplyInterpolation();
void Restore();
} // namespace Scenes::FixedTick
