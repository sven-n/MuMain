#include "doctest.h"

#include "Core/Utilities/FrameProfiler.h"

TEST_CASE("renderer counters follow nested frame passes [core][profiling]")
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
        }

        CHECK(FrameProfiler::CurrentPass() == Pass::Terrain);
    }

    CHECK(FrameProfiler::CurrentPass() == Pass::Other);
    CHECK(FrameProfiler::CounterValue(Pass::Terrain, Counter::DrawCalls) == 1);
    CHECK(FrameProfiler::CounterValue(Pass::Objects, Counter::MergedDraws) == 2);
    CHECK(FrameProfiler::CounterValue(Counter::MergedDraws) == 2);

    FrameProfiler::g_CountersEnabled = false;
    FrameProfiler::ResetCounters();
}

TEST_CASE("frame profile macro supports two scopes on one source line [core][profiling]")
{
    { FRAME_PROFILE(Terrain); } { FRAME_PROFILE(Objects); }
    CHECK(true);
}
