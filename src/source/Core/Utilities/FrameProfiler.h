#pragma once

// Tiny per-frame timing utility used by the $details overlay to break a frame
// down into a fixed list of named passes. Header-only and lock-free; intended
// for ad-hoc bottleneck hunting on a single thread (the render thread).
//
// Usage:
//   { FRAME_PROFILE(Terrain); RenderTerrain(false); }
// Then `FrameProfiler::AccumulatorMs(Pass::Terrain)` returns the elapsed ms.
// Call `FrameProfiler::ResetFrame()` once per frame after reading the values.
//
// GLP-01: extended with per-pass GL call/draw/buffer counters and GPU-side GL_TIMESTAMP
// timers, both behind `g_CountersEnabled` (toggled by the `$glstats` console command) so
// later perf tasks in this series can be judged against a measured baseline instead of a
// plausible mechanism. Read via RenderGLStats() in SceneManager.cpp.

#include <chrono>
#include <cstdint>
#include <SDL3/SDL.h>

#ifndef APIENTRY
#define APIENTRY
#endif

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
        CharWait, // subset of Characters — time spent blocked in WaitCharactersAnimation()
        MoveEffects, // MoveEffects() in UpdateGameEntities() — Move/update phase, not render
        MoveParticles, // MoveParticles() in UpdateGameEntities() — Move/update phase, not render
        Skinning, // BMD::Transform() self-time (DXP-20 baseline) — subset of Objects/Characters/Items
        UI, // DXP-23: RenderMainSceneUI() self-time — was previously unmeasured (fell outside every
            // FRAME_PROFILE scope), interface panels/party window/NewUI system/cursor
        Present, // DXP-23: PlatformSwapBuffers() self-time, split out of Other so a large reading
                 // unambiguously means "CPU stalling on the GPU command queue", not HUD render cost
        // GLP-24: split out of Other. RenderSprites()/RenderParticles()/RenderJoints() sit outside
        // the Effects scope (MainScene.cpp:509/525/526, and again in the water-map second pass at
        // :544/553/554), so all three landed in Other -- which the $glstats overlay never printed.
        // On the Intel HD 530 skill-cast capture they accounted for ~94% of the frame's draw-call
        // increase while being completely invisible.
        Sprites,   // RenderSprites() -- IR per-quad path
        Particles, // RenderParticles() -- IR per-quad path, the heaviest in effect-dense frames
        Joints,    // RenderJoints() -- beam/tail-trail effects (Wing of Ruin tail, lightning beams)
        Overlay,   // $details/$glstats/FPS-counter self-cost. These render text as roughly one IR
                   // quad per glyph and used to be tagged Other, contaminating the exact bucket
                   // under investigation -- an observer effect large enough to mislead.
        Count_
    };

    inline constexpr const char* kPassNames[(int)Pass::Count_] = {
        "Terrain", "Objects", "Chars", "Items", "Effects", "Other", "CharWait", "MoveFx", "MovePart",
        "Skinning", "UI", "Present", "Sprites", "Particles", "Joints", "Overlay"
    };

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

    // ---- GL call / draw / buffer counters ----
    // Master switch for both the counters below and the GPU timers further down, toggled by
    // `$glstats on/off`. A predictable-branch bool check, not #ifdef'd out, so it can measure a
    // Release build on the machine that actually shows the regression.
    inline bool g_CountersEnabled = false;

    enum class Counter : int
    {
        GLCalls,        // every wrapped GL entry point counted below, summed
        DrawCalls,      // glDrawArrays + glDrawElements
        BufferUpdates,  // glBufferData + glBufferSubData
        BufferOrphans,  // glBufferData(.., nullptr, ..) specifically -- subset of BufferUpdates
        ProgramBinds,   // glUseProgram calls that actually reached the driver
        TextureBinds,   // glBindTexture calls that actually reached the driver
        UniformWrites,  // glUniform* calls that actually reached the driver
        UboSkips,       // GLP-10: UpdateUniformBlock calls short-circuited because the content
                         // was byte-identical to that slot's last same-frame upload -- NOT
                         // included in GLCalls (no GL call happened), unlike every counter above
        Count_
    };

    // Per-[Pass][Counter] breakdown -- what tells you "terrain is 18,000 of the 23,000 GL
    // calls" in one reading. Attributed automatically via CurrentPass(), so hook sites never
    // need to know which pass they're running under.
    inline uint32_t& CounterValue(Pass p, Counter c)
    {
        static uint32_t s_counts[(int)Pass::Count_][(int)Counter::Count_] = {};
        return s_counts[(int)p][(int)c];
    }

    // Frame-total per counter, summed across passes -- same static-local shape as AccumulatorMs.
    inline uint32_t& CounterValue(Counter c)
    {
        static uint32_t s_totals[(int)Counter::Count_] = {};
        return s_totals[(int)c];
    }

    inline void ResetCounters()
    {
        for (int p = 0; p < (int)Pass::Count_; p++)
            for (int c = 0; c < (int)Counter::Count_; c++)
                CounterValue((Pass)p, (Counter)c) = 0;
        for (int c = 0; c < (int)Counter::Count_; c++)
            CounterValue((Counter)c) = 0;
    }

    namespace detail
    {
        // Small fixed-depth stack (single render thread, no allocation) so a nested scope
        // (Skinning inside Objects, CharWait inside Characters) restores the outer pass's
        // attribution on exit instead of leaking counts into whatever pass follows.
        inline constexpr int kMaxPassDepth = 8;

        inline Pass* PassStack()
        {
            static Pass s_stack[kMaxPassDepth];
            return s_stack;
        }

        inline int& PassStackDepth()
        {
            static int s_depth = 0;
            return s_depth;
        }
    }

    inline Pass& CurrentPass()
    {
        static Pass s_current = Pass::Other;
        return s_current;
    }

    inline void PushPass(Pass p)
    {
        int& depth = detail::PassStackDepth();
        if (depth < detail::kMaxPassDepth)
            detail::PassStack()[depth] = CurrentPass();
        depth++;
        CurrentPass() = p;
    }

    inline void PopPass()
    {
        int& depth = detail::PassStackDepth();
        if (depth <= 0) return;
        depth--;
        if (depth < detail::kMaxPassDepth)
            CurrentPass() = detail::PassStack()[depth];
    }

    // Hook-site helper: a specific wrapped GL entry point was actually issued this frame --
    // reached the driver, not just attempted (the existing BindState.cpp/shader dirty-check
    // caches are supposed to be suppressing most of these; this is how you find out whether
    // they actually do). Bumps both the specific bucket and the GLCalls total together --
    // GLCalls is never incremented on its own.
    inline void CountGLCall(Counter specific, uint32_t amount = 1)
    {
        if (!g_CountersEnabled) return;
        CounterValue(CurrentPass(), Counter::GLCalls) += amount;
        CounterValue(Counter::GLCalls) += amount;
        CounterValue(CurrentPass(), specific) += amount;
        CounterValue(specific) += amount;
    }

    // Like CountGLCall(), for entry points with no dedicated bucket of their own (e.g.
    // glBindVertexArray -- there's no VAOBinds counter, just the GLCalls total).
    inline void CountGLCall(uint32_t amount = 1)
    {
        if (!g_CountersEnabled) return;
        CounterValue(CurrentPass(), Counter::GLCalls) += amount;
        CounterValue(Counter::GLCalls) += amount;
    }

    // Additional tag alongside a CountGLCall(BufferUpdates) call, for the specific
    // glBufferData(.., nullptr, ..) orphan variant. Does not bump GLCalls again -- the paired
    // CountGLCall() already did, this is the same physical GL call.
    inline void TagBufferOrphan(uint32_t amount = 1)
    {
        if (!g_CountersEnabled) return;
        CounterValue(CurrentPass(), Counter::BufferOrphans) += amount;
        CounterValue(Counter::BufferOrphans) += amount;
    }

    // GLP-10: a call that was skipped entirely (no GL call issued) -- unlike CountGLCall(), does
    // NOT bump GLCalls, since nothing reached the driver. Currently only UboSkips uses this, kept
    // generic in case a future dirty-check needs the same "skipped, not just orphaned" shape.
    inline void CountSkip(Counter specific, uint32_t amount = 1)
    {
        if (!g_CountersEnabled) return;
        CounterValue(CurrentPass(), specific) += amount;
        CounterValue(specific) += amount;
    }

    // ---- GPU-side pass timers ----
    // GL_TIMESTAMP query pairs, not GL_TIME_ELAPSED -- GL_TIME_ELAPSED cannot nest, and
    // Skinning/CharWait genuinely nest inside Objects/Characters/Items (see the Pass enum
    // comments above). GL_TIMESTAMP sidesteps that: each query object independently marks
    // "when did the GPU reach this point," so nested or interleaved passes can't corrupt each
    // other's result.
    //
    // Restricted to the GL-call-bearing render passes. CharWait (CPU wait on the animation
    // thread pool), Skinning (CPU vertex/normal transform), MoveEffects/MoveParticles (update-
    // phase simulation, not render) and Present (SDL buffer swap, not a queued GL command) never
    // submit GL work themselves -- a GPU timestamp pair around them would just read whatever was
    // already in the command stream, not their own cost.
    // GLP-24 adds Sprites/Particles/Joints: each is one contiguous submission region, so a
    // timestamp pair around it is meaningful. Overlay and Other stay untimed -- Other is a
    // remainder bucket scattered across the whole frame, so a single pair around it would bracket
    // unrelated work, and Overlay's cost is an artifact of measuring rather than something to fix.
    inline constexpr Pass kGpuTimedPasses[] = {
        Pass::Terrain, Pass::Objects, Pass::Characters, Pass::Items, Pass::Effects, Pass::UI,
        Pass::Sprites, Pass::Particles, Pass::Joints
    };
    inline constexpr int kGpuTimedPassCount = sizeof(kGpuTimedPasses) / sizeof(kGpuTimedPasses[0]);

    inline int GpuTimedIndex(Pass p)
    {
        for (int i = 0; i < kGpuTimedPassCount; i++)
            if (kGpuTimedPasses[i] == p) return i;
        return -1;
    }

    inline float& GpuMs(Pass p)
    {
        static float s_gpuMs[(int)Pass::Count_] = {};
        return s_gpuMs[(int)p];
    }

    // GLP-24: true when the last completed reading for this pass had to drop entries because the
    // pass was entered more times in one frame than kMaxEntriesPerPass allows. The overlay flags
    // it, so a truncated sum is never read as a complete one.
    inline bool& GpuMsTruncated(Pass p)
    {
        static bool s_truncated[(int)Pass::Count_] = {};
        return s_truncated[(int)p];
    }

    namespace detail
    {
        // Self-contained function-pointer loading, matching the per-file convention used by
        // BindState.cpp/RHI_GL.cpp elsewhere in Render/ -- loads its own extension entry points
        // via SDL_GL_GetProcAddress rather than relying on glewInit() having run.
        typedef void (APIENTRY* PFNGLGENQUERIESPROC)(GLsizei n, GLuint* ids);
        typedef void (APIENTRY* PFNGLQUERYCOUNTERPROC)(GLuint id, GLenum target);
        typedef void (APIENTRY* PFNGLGETQUERYOBJECTIVPROC)(GLuint id, GLenum pname, GLint* params);
        typedef void (APIENTRY* PFNGLGETQUERYOBJECTUI64VPROC)(GLuint id, GLenum pname, GLuint64* params);

        struct GpuQueryFns
        {
            PFNGLGENQUERIESPROC          GenQueries          = nullptr;
            PFNGLQUERYCOUNTERPROC        QueryCounter        = nullptr;
            PFNGLGETQUERYOBJECTIVPROC    GetQueryObjectiv    = nullptr;
            PFNGLGETQUERYOBJECTUI64VPROC GetQueryObjectui64v = nullptr;
            bool                         loaded              = false;
        };

        inline GpuQueryFns& QueryFns()
        {
            static GpuQueryFns s_fns;
            return s_fns;
        }

        inline bool LoadGpuQueryFunctions()
        {
            GpuQueryFns& fns = QueryFns();
            if (fns.loaded) return true;
            fns.GenQueries          = (PFNGLGENQUERIESPROC)SDL_GL_GetProcAddress("glGenQueries");
            fns.QueryCounter        = (PFNGLQUERYCOUNTERPROC)SDL_GL_GetProcAddress("glQueryCounter");
            fns.GetQueryObjectiv    = (PFNGLGETQUERYOBJECTIVPROC)SDL_GL_GetProcAddress("glGetQueryObjectiv");
            fns.GetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC)SDL_GL_GetProcAddress("glGetQueryObjectui64v");
            fns.loaded = (fns.GenQueries != nullptr && fns.QueryCounter != nullptr &&
                          fns.GetQueryObjectiv != nullptr && fns.GetQueryObjectui64v != nullptr);
            return fns.loaded;
        }

        // Triple-buffered: this frame's queries write into ring[frame % kGpuRingSize]. Before a
        // slot is reused (every 3rd frame) its previous result is read first -- by then the GPU
        // is a full ring cycle past submission, nowhere close to still working on it, so
        // GetQueryObjectui64v(GL_QUERY_RESULT) never blocks. GL_QUERY_RESULT_AVAILABLE is still
        // polled and the update skipped if it isn't ready (a slow/busy driver), per the "never
        // block" requirement -- the stale previous reading just stays on screen one extra frame.
        //
        // GLP-29: the paragraph above describes the design; AdvanceGpuTimers() did not implement
        // it. It read ring[frame % kGpuRingSize] -- the slot THIS frame had just written -- giving
        // the GPU zero frames to retire the queries, and then reset the slot regardless. Passes
        // submitted late in the frame therefore almost never had results ready, were discarded
        // rather than retried, and their GpuMs stayed at its initial 0.00 forever. That is why
        // `Particles`, the last GPU-timed pass, reported 0.00 ms against ~2,970 draw calls while
        // earlier passes read plausibly. Now reads ring[(frame + 1) % kGpuRingSize] -- the slot
        // about to be overwritten next frame, holding results submitted two frames ago.
        inline constexpr int kGpuRingSize = 3;

        // GLP-24: a pass can be entered more than once per frame. Objects is entered twice
        // (RenderObjects() and RenderObjects_AfterCharacter()); Sprites/Particles/Joints are each
        // entered twice on water maps (MainScene.cpp's byWaterMap == 2 second pass). With one query
        // pair per pass, each new entry overwrote the previous entry's begin stamp, so GPU ms
        // reported only the last entry while CPU ms (AccumulatorMs) summed them all -- the two
        // columns on such a row were not measuring the same work. N pairs per pass, summed, fixes it.
        inline constexpr int kMaxEntriesPerPass = 4;

        struct GpuQuerySlot
        {
            GLuint begin[kGpuTimedPassCount][kMaxEntriesPerPass] = {};
            GLuint end[kGpuTimedPassCount][kMaxEntriesPerPass]   = {};
            // Completed pairs so far this frame. `open` marks a begin awaiting its end, so
            // GpuTimerEnd stamps the same index GpuTimerBegin used.
            int    count[kGpuTimedPassCount]     = {};
            bool   open[kGpuTimedPassCount]      = {};
            // An entry past the cap is dropped whole, never wrapped -- wrapping would pair a late
            // begin with an early end and yield a plausible-looking garbage delta.
            bool   truncated[kGpuTimedPassCount] = {};
        };

        inline GpuQuerySlot& GpuRing(int slot)
        {
            static GpuQuerySlot s_ring[kGpuRingSize];
            return s_ring[slot];
        }

        inline bool& GpuQueriesGenerated()
        {
            static bool s_generated = false;
            return s_generated;
        }

        inline void EnsureGpuQueriesGenerated()
        {
            if (GpuQueriesGenerated()) return;
            if (!LoadGpuQueryFunctions()) return;
            for (int s = 0; s < kGpuRingSize; s++)
            {
                GpuQuerySlot& slot = GpuRing(s);
                // 2D arrays are contiguous, so one GenQueries call fills the whole block.
                QueryFns().GenQueries(kGpuTimedPassCount * kMaxEntriesPerPass, &slot.begin[0][0]);
                QueryFns().GenQueries(kGpuTimedPassCount * kMaxEntriesPerPass, &slot.end[0][0]);
            }
            GpuQueriesGenerated() = true;
        }

        inline int& GpuFrameIndex()
        {
            static int s_frame = 0;
            return s_frame;
        }
    }

    inline void GpuTimerBegin(Pass p)
    {
        if (!g_CountersEnabled) return;
        const int idx = GpuTimedIndex(p);
        if (idx < 0) return;
        detail::EnsureGpuQueriesGenerated();
        if (!detail::GpuQueriesGenerated()) return;
        detail::GpuQuerySlot& slot = detail::GpuRing(detail::GpuFrameIndex() % detail::kGpuRingSize);
        if (slot.open[idx]) return; // unbalanced Begin without an End -- keep the first, ignore this
        if (slot.count[idx] >= detail::kMaxEntriesPerPass)
        {
            slot.truncated[idx] = true;
            return;
        }
        detail::QueryFns().QueryCounter(slot.begin[idx][slot.count[idx]], GL_TIMESTAMP);
        slot.open[idx] = true;
    }

    inline void GpuTimerEnd(Pass p)
    {
        if (!g_CountersEnabled || !detail::GpuQueriesGenerated()) return;
        const int idx = GpuTimedIndex(p);
        if (idx < 0) return;
        detail::GpuQuerySlot& slot = detail::GpuRing(detail::GpuFrameIndex() % detail::kGpuRingSize);
        // No open begin means its pair was dropped at the cap (or never issued) -- drop this end
        // too rather than stamping a query object nothing will read.
        if (!slot.open[idx]) return;
        detail::QueryFns().QueryCounter(slot.end[idx][slot.count[idx]], GL_TIMESTAMP);
        slot.count[idx]++;
        slot.open[idx] = false;
    }

    // Reads the ring slot about to be reused, then advances the ring. Call exactly once per
    // frame, after that frame's GpuTimerBegin/End calls -- unconditionally, even when
    // $glstats is off, so a toggle mid-session never leaves a slot half-read.
    inline void AdvanceGpuTimers()
    {
        if (detail::GpuQueriesGenerated())
        {
            // GLP-29: read the OLDEST slot -- the one next frame is about to overwrite -- not the
            // one this frame just wrote. Its queries were submitted two frames ago, so the GPU has
            // long since retired them and the results are actually available. Reading the current
            // slot gave the GPU no time at all, which stranded every late-submitted pass at 0.00.
            detail::GpuQuerySlot& slot = detail::GpuRing((detail::GpuFrameIndex() + 1) % detail::kGpuRingSize);
            for (int i = 0; i < kGpuTimedPassCount; i++)
            {
                const int pairs = slot.count[i];
                if (pairs > 0)
                {
                    // Timestamps retire in submission order, so if the last pair's end is ready
                    // every earlier one is too -- one availability poll covers the whole set.
                    GLint available = 0;
                    detail::QueryFns().GetQueryObjectiv(slot.end[i][pairs - 1], GL_QUERY_RESULT_AVAILABLE, &available);
                    if (available)
                    {
                        double totalNs = 0.0;
                        for (int e = 0; e < pairs; e++)
                        {
                            GLuint64 beginNs = 0, endNs = 0;
                            detail::QueryFns().GetQueryObjectui64v(slot.begin[i][e], GL_QUERY_RESULT, &beginNs);
                            detail::QueryFns().GetQueryObjectui64v(slot.end[i][e], GL_QUERY_RESULT, &endNs);
                            totalNs += (double)(endNs - beginNs);
                        }
                        GpuMs(kGpuTimedPasses[i]) = (float)(totalNs / 1.0e6);
                        GpuMsTruncated(kGpuTimedPasses[i]) = slot.truncated[i];
                    }
                    // Not ready: leave the previous reading on screen one more frame, as before.
                }

                // Reset unconditionally, including the not-ready case. This slot is the one the
                // NEXT frame writes into, so it must start at zero pairs; leaving stale pairs here
                // would get them summed with that frame's. Discarding an unready reading is safe
                // now that the slot is two frames old -- if it still isn't ready the GPU is more
                // than two frames behind, and one stale row on the overlay is the right outcome.
                slot.count[i]     = 0;
                slot.open[i]      = false;
                slot.truncated[i] = false;
            }
        }

        detail::GpuFrameIndex()++;
    }

    // RAII timer. Constructor stamps the start, destructor accumulates elapsed
    // ms into the named pass. Multiple Scopes for the same Pass within a frame
    // accumulate (so calling RenderObjects twice per frame sums correctly).
    //
    // autoGpuTimer (default true): also brackets a GL_TIMESTAMP pair around the whole scope.
    // GLP-16 GPU-ms investigation found this default placement misattributes CPU-only work to
    // "GPU time" for any pass whose scope spans real CPU work (gather/sort/etc.) BEFORE its GPU
    // commands are submitted -- GL_TIMESTAMP measures wall-clock time between two points in the
    // GPU's command stream, so if the GPU finishes the begin-marker and then has nothing queued
    // while the CPU keeps working, that idle wait reads as "GPU time" too. Pass false here and
    // call FrameProfiler::GpuTimerBegin/GpuTimerEnd manually, tightly around just the real draw
    // submission, for any pass where this matters (Terrain does this now -- see
    // ZzzLodTerrain.cpp's FlushTerrainBuckets()). CPU ms (AccumulatorMs) is unaffected either
    // way -- it always covers the whole scope, which is correct for CPU self-time.
    class Scope
    {
    public:
        explicit Scope(Pass p, bool autoGpuTimer = true)
            : m_pass(p), m_t0(std::chrono::steady_clock::now()), m_autoGpuTimer(autoGpuTimer)
        {
            PushPass(p);
            if (m_autoGpuTimer) GpuTimerBegin(p);
        }

        ~Scope()
        {
            if (m_autoGpuTimer) GpuTimerEnd(m_pass);
            const auto t1 = std::chrono::steady_clock::now();
            const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - m_t0).count();
            AccumulatorMs(m_pass) += (float)ns / 1.0e6f;
            PopPass();
        }

    private:
        Pass m_pass;
        std::chrono::steady_clock::time_point m_t0;
        bool m_autoGpuTimer;
    };
}

#define FRAME_PROFILE_CAT_(a, b) a##b
#define FRAME_PROFILE_CAT(a, b) FRAME_PROFILE_CAT_(a, b)
#define FRAME_PROFILE(passName) FrameProfiler::Scope FRAME_PROFILE_CAT(_frameProf_, __LINE__)(FrameProfiler::Pass::passName)
// Same as FRAME_PROFILE, but skips the automatic GL_TIMESTAMP pair -- use when the caller places
// FrameProfiler::GpuTimerBegin/GpuTimerEnd manually, tightly around the real GPU submission
// instead of the whole scope. CPU ms tracking (AccumulatorMs) is identical to FRAME_PROFILE.
#define FRAME_PROFILE_CPU_ONLY(passName) FrameProfiler::Scope FRAME_PROFILE_CAT(_frameProf_, __LINE__)(FrameProfiler::Pass::passName, false)
