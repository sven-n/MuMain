#pragma once

// Tiny per-frame timing utility used by the $details overlay to break a frame
// down into a fixed list of named passes. Header-only and lock-free; intended
// for ad-hoc bottleneck hunting on a single thread (the render thread).
//
// Usage:
//   { FRAME_PROFILE(Terrain); RenderTerrain(false); }
// Then `FrameProfiler::AccumulatorMs(Pass::Terrain)` returns the elapsed ms.
// Call `FrameProfiler::ResetFrame()` once per frame after reading the values.

#include <chrono>

namespace FrameProfiler
{
    // Stable, indexed pass list. Add a slot to extend; keep Count_ last.
enum class Pass : int
{
    Terrain,
    Objects,
    Characters,
    Items,
    Effects,
    Other,
    CharWait,      // subset of Characters — time spent blocked in WaitCharactersAnimation()
    MoveEffects,   // MoveEffects() in UpdateGameEntities() — Move/update phase, not render
    MoveParticles, // MoveParticles() in UpdateGameEntities() — Move/update phase, not render
    Skinning,      // BMD::Transform() self-time (DXP-20 baseline) — subset of Objects/Characters/Items
    UI,            // DXP-23: RenderMainSceneUI() self-time — was previously unmeasured (fell outside every
        // FRAME_PROFILE scope), interface panels/party window/NewUI system/cursor
    Present, // DXP-23: PlatformSwapBuffers() self-time, split out of Other so a large reading
             // unambiguously means "CPU stalling on the GPU command queue", not HUD render cost
    Count_
};

inline constexpr const char* kPassNames[(int)Pass::Count_] = {"Terrain",  "Objects",  "Chars",    "Items",
                                                              "Effects",  "Other",    "CharWait", "MoveFx",
                                                              "MovePart", "Skinning", "UI",       "Present"};

inline float& AccumulatorMs(Pass p)
{
    static float s_ms[(int)Pass::Count_] = {};
    return s_ms[(int)p];
}

    inline void ResetFrame()
    {
        for (int i = 0; i < (int)Pass::Count_; i++)
            AccumulatorMs((Pass)i) = 0.f;
    }

    // RAII timer. Constructor stamps the start, destructor accumulates elapsed
    // ms into the named pass. Multiple Scopes for the same Pass within a frame
    // accumulate (so calling RenderObjects twice per frame sums correctly).
    class Scope
    {
    public:
        explicit Scope(Pass p)
            : m_pass(p), m_t0(std::chrono::steady_clock::now()) {}

        ~Scope()
        {
            const auto t1 = std::chrono::steady_clock::now();
            const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - m_t0).count();
            AccumulatorMs(m_pass) += (float)ns / 1.0e6f;
        }

    private:
        Pass m_pass;
        std::chrono::steady_clock::time_point m_t0;
    };
}

#define FRAME_PROFILE(passName) FrameProfiler::Scope _frameProf_##__LINE__(FrameProfiler::Pass::passName)
