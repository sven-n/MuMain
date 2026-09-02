#include "doctest.h"

#include "Core/Utilities/FrameProfiler.h"

TEST_CASE("renderer counters follow nested frame passes [core][profiling][frame_profiler]")
{
    using Counter = FrameProfiler::Counter;
    using Pass = FrameProfiler::Pass;

    FrameProfiler::ResetCounters();
    FrameProfiler::g_CountersEnabled = true;

    {
        FRAME_PROFILE(Terrain);
        FrameProfiler::Count(Counter::DrawCalls);

        {
            FRAME_PROFILE(Objects);
            FrameProfiler::Count(Counter::MergedDraws, 2);
            FrameProfiler::Count(Counter::Merged2DDraws);
            FrameProfiler::Count(Counter::GlyphUploads, 3);
            FrameProfiler::Count(Counter::GpuSkinningSubmissions, 4);
            FrameProfiler::Count(Counter::CpuSkinningIneligible, 2);
            FrameProfiler::Count(Counter::GpuSkinningFailures);
        }

        CHECK(FrameProfiler::CurrentPass() == Pass::Terrain);
    }

    CHECK(FrameProfiler::CurrentPass() == Pass::Other);
    CHECK(FrameProfiler::CounterValue(Pass::Terrain, Counter::DrawCalls) == 1);
    CHECK(FrameProfiler::CounterValue(Pass::Objects, Counter::MergedDraws) == 2);
    CHECK(FrameProfiler::CounterValue(Pass::Objects, Counter::Merged2DDraws) == 1);
    CHECK(FrameProfiler::CounterValue(Pass::Objects, Counter::GlyphUploads) == 3);
    CHECK(FrameProfiler::CounterValue(Pass::Objects, Counter::GpuSkinningSubmissions) == 4);
    CHECK(FrameProfiler::CounterValue(Pass::Objects, Counter::CpuSkinningIneligible) == 2);
    CHECK(FrameProfiler::CounterValue(Pass::Objects, Counter::GpuSkinningFailures) == 1);
    CHECK(FrameProfiler::CounterValue(Counter::MergedDraws) == 2);
    CHECK(FrameProfiler::CounterValue(Counter::Merged2DDraws) == 1);
    CHECK(FrameProfiler::CounterValue(Counter::GlyphUploads) == 3);
    CHECK(FrameProfiler::CounterValue(Counter::GpuSkinningSubmissions) == 4);
    CHECK(FrameProfiler::CounterValue(Counter::CpuSkinningIneligible) == 2);
    CHECK(FrameProfiler::CounterValue(Counter::GpuSkinningFailures) == 1);

    FrameProfiler::g_CountersEnabled = false;
    FrameProfiler::ResetCounters();
}

TEST_CASE("frame profile macro supports two scopes on one source line [core][profiling][frame_profiler]")
{
    { FRAME_PROFILE(Terrain); } { FRAME_PROFILE(Objects); }
    CHECK(true);
}
