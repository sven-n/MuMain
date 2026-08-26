#pragma once

// Per-frame CPU timing and renderer counters for the $details and $glstats overlays.
// Single render thread only. Reset after every frame's overlays have read the values.

#include <array>
#include <chrono>
#include <cstdint>

namespace FrameProfiler
{
enum class Pass : int
{
    Terrain,
    Objects,
    Characters,
    Items,
    Effects,
    Other,
    CharWait,
    MoveEffects,
    MoveParticles,
    Skinning,
    UI,
    Present,
    Sprites,
    Particles,
    Joints,
    Overlay,
    Count_
};

inline constexpr const char* kPassNames[static_cast<int>(Pass::Count_)] = {
    "Terrain", "Objects", "Chars", "Items", "Effects", "Other", "CharWait", "MoveFx", "MovePart",
    "Skinning", "UI", "Present", "Sprites", "Particles", "Joints", "Overlay",
};

enum class Counter : int
{
    DrawCalls,
    MergedDraws,
    Merged2DDraws,
    GlyphUploads,
    VertexBytes,
    TextureUploads,
    BatchDraws,
    BatchVertices,
    BatchBreakTexture,
    BatchBreakBlend,
    BatchBreakDepth,
    BatchBreakProgram,
    BatchBreakUniform,
    BatchBreakMatrix,
    BatchBreakDraw,
    BatchBreakOther,
    Count_
};

inline bool g_CountersEnabled = false;

inline float& AccumulatorMs(Pass pass)
{
    static float milliseconds[static_cast<int>(Pass::Count_)]{};
    return milliseconds[static_cast<int>(pass)];
}

inline std::uint32_t& CounterValue(Pass pass, Counter counter)
{
    static std::uint32_t values[static_cast<int>(Pass::Count_)][static_cast<int>(Counter::Count_)]{};
    return values[static_cast<int>(pass)][static_cast<int>(counter)];
}

inline std::uint32_t& CounterValue(Counter counter)
{
    static std::uint32_t totals[static_cast<int>(Counter::Count_)]{};
    return totals[static_cast<int>(counter)];
}

inline void ResetFrame()
{
    for (int index = 0; index < static_cast<int>(Pass::Count_); ++index)
    {
        AccumulatorMs(static_cast<Pass>(index)) = 0.0f;
    }
}

inline void ResetCounters()
{
    for (int pass = 0; pass < static_cast<int>(Pass::Count_); ++pass)
    {
        for (int counter = 0; counter < static_cast<int>(Counter::Count_); ++counter)
        {
            CounterValue(static_cast<Pass>(pass), static_cast<Counter>(counter)) = 0;
        }
    }

    for (int counter = 0; counter < static_cast<int>(Counter::Count_); ++counter)
    {
        CounterValue(static_cast<Counter>(counter)) = 0;
    }
}

namespace detail
{
inline constexpr std::size_t kMaxPassDepth = 8;

struct PassState
{
    std::array<Pass, kMaxPassDepth> stack{};
    std::size_t depth = 0;
    std::size_t overflow = 0;
    Pass current = Pass::Other;
};

inline PassState& State()
{
    static PassState state;
    return state;
}
}

inline Pass CurrentPass()
{
    return detail::State().current;
}

inline void PushPass(Pass pass)
{
    detail::PassState& state = detail::State();
    if (state.depth == state.stack.size())
    {
        // ponytail: eight nested scopes; raise kMaxPassDepth if profiling gains deeper nesting.
        ++state.overflow;
        return;
    }

    state.stack[state.depth++] = state.current;
    state.current = pass;
}

inline void PopPass()
{
    detail::PassState& state = detail::State();
    if (state.overflow > 0)
    {
        --state.overflow;
        return;
    }
    if (state.depth == 0)
    {
        state.current = Pass::Other;
        return;
    }

    state.current = state.stack[--state.depth];
}

inline void Count(Counter counter, std::uint32_t amount = 1)
{
    if (!g_CountersEnabled)
    {
        return;
    }

    CounterValue(CurrentPass(), counter) += amount;
    CounterValue(counter) += amount;
}

class Scope
{
public:
    explicit Scope(Pass pass)
        : m_pass(pass), m_startedAt(std::chrono::steady_clock::now())
    {
        PushPass(pass);
    }

    ~Scope()
    {
        const auto elapsed = std::chrono::steady_clock::now() - m_startedAt;
        const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
        AccumulatorMs(m_pass) += static_cast<float>(nanoseconds) / 1.0e6f;
        PopPass();
    }

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

private:
    Pass m_pass;
    std::chrono::steady_clock::time_point m_startedAt;
};
}

#define FRAME_PROFILE_CAT_(left, right) left##right
#define FRAME_PROFILE_CAT(left, right) FRAME_PROFILE_CAT_(left, right)
#define FRAME_PROFILE(passName)                                                                                         \
    FrameProfiler::Scope FRAME_PROFILE_CAT(_frameProfilerScope_, __LINE__)(FrameProfiler::Pass::passName)
