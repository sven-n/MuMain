#pragma once

// Per-frame CPU timing and renderer counters for the $details and $glstats overlays.
// Single render thread only. Publish after renderer EndFrame; overlays read the completed snapshot.
// Timings are inclusive CPU elapsed time, never GPU execution time.

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
    RendererBegin,
    RendererEnd,
    RendererSubmit,
    UIUpdate,
    RmlUiInput,
    UILayout,
    RenderFlush,
    Sprites,
    Particles,
    Joints,
    Overlay,
    RmlUiUpdate,
    RmlUiRender,
    Count_
};

inline constexpr const char* kPassNames[static_cast<int>(Pass::Count_)] = {
    "Terrain", "Objects", "Chars", "Items", "Effects", "Other", "CharWait", "MoveFx", "MovePart",
    "Skinning", "UI", "RenderBegin", "RenderEnd", "Submit", "UIUpdate", "RmlInput", "UILayout", "Flush",
    "Sprites", "Particles", "Joints", "Overlay",
    "RmlUpd", "RmlRend",
};

enum class Counter : int
{
    DrawCalls,
    MergedDraws,
    Merged2DDraws,
    GlyphUploads,
    GpuSkinningSubmissions,
    CpuSkinningIneligible,
    GpuSkinningFailures,
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

// Keep the display stable while this frame records more work (including its own overlay).
inline float g_completedMilliseconds[static_cast<int>(Pass::Count_)]{};
inline std::uint32_t g_completedCounters[static_cast<int>(Pass::Count_)][static_cast<int>(Counter::Count_)]{};
inline std::uint32_t g_completedTotals[static_cast<int>(Counter::Count_)]{};

inline float CompletedMs(Pass pass)
{
    return g_completedMilliseconds[static_cast<int>(pass)];
}

inline std::uint32_t CompletedCounter(Pass pass, Counter counter)
{
    return g_completedCounters[static_cast<int>(pass)][static_cast<int>(counter)];
}

inline std::uint32_t CompletedCounter(Counter counter)
{
    return g_completedTotals[static_cast<int>(counter)];
}

inline void CompleteFrame()
{
    for (int pass = 0; pass < static_cast<int>(Pass::Count_); ++pass)
    {
        g_completedMilliseconds[pass] = AccumulatorMs(static_cast<Pass>(pass));
        for (int counter = 0; counter < static_cast<int>(Counter::Count_); ++counter)
            g_completedCounters[pass][counter] = CounterValue(static_cast<Pass>(pass), static_cast<Counter>(counter));
    }
    for (int counter = 0; counter < static_cast<int>(Counter::Count_); ++counter)
        g_completedTotals[counter] = CounterValue(static_cast<Counter>(counter));
    ResetFrame();
    ResetCounters();
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
        AccumulatorMs(m_pass) += std::chrono::duration<float, std::milli>(elapsed).count();
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
