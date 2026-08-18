#include "doctest.h"

#include "Core/Utilities/Benchmark/BenchStats.h"

#include <vector>

using namespace Core::Benchmark;

namespace
{
    // A repeat with the given frame times, and nothing else set.
    RepeatSamples MakeRepeat(int index, const std::vector<float>& frameTimesMs)
    {
        RepeatSamples repeat;
        repeat.repeatIndex = index;
        for (float ms : frameTimesMs)
        {
            FrameSample sample;
            sample.frameMs = ms;
            repeat.frames.push_back(sample);
        }
        return repeat;
    }

    std::vector<float> ConstantFrames(int count, float ms)
    {
        return std::vector<float>((size_t)count, ms);
    }
}

TEST_CASE("empty input produces a zeroed summary, not a division by zero")
{
    const Stats::TimingStats stats = Stats::Summarize({});
    CHECK(stats.frameCount == 0);
    CHECK(stats.meanMs == 0.0f);
    CHECK(stats.meanFps == 0.0f);
    CHECK(stats.percentileCurveMs.empty());
}

TEST_CASE("constant frame stream: every percentile is the same value")
{
    const Stats::TimingStats stats = Stats::Summarize(ConstantFrames(100, 10.0f));
    CHECK(stats.frameCount == 100);
    CHECK(stats.meanMs == doctest::Approx(10.0f));
    CHECK(stats.medianMs == doctest::Approx(10.0f));
    CHECK(stats.p99Ms == doctest::Approx(10.0f));
    CHECK(stats.meanFps == doctest::Approx(100.0f));
    CHECK(stats.stdDevMs == doctest::Approx(0.0f));
    CHECK(stats.pacingMeanAbsDeltaMs == doctest::Approx(0.0f));
    CHECK(stats.slowFrameCount == 0);
    CHECK((int)stats.percentileCurveMs.size() == kPercentileCurvePoints);
}

TEST_CASE("1% low averages the worst frames, not the mean")
{
    // 99 fast frames and one very slow one: the mean barely moves, the 1% low collapses.
    std::vector<float> frames = ConstantFrames(99, 10.0f);
    frames.push_back(100.0f);

    const Stats::TimingStats stats = Stats::Summarize(frames);
    CHECK(stats.meanMs == doctest::Approx(10.9f));
    CHECK(stats.onePercentLowMs == doctest::Approx(100.0f));
    CHECK(stats.onePercentLowFps == doctest::Approx(10.0f));
    CHECK(stats.maxMs == doctest::Approx(100.0f));
}

TEST_CASE("pacing separates a smooth stream from an alternating one with the same mean")
{
    const Stats::TimingStats smooth = Stats::Summarize(ConstantFrames(100, 10.0f));

    std::vector<float> alternating;
    for (int i = 0; i < 100; i++) alternating.push_back((i % 2 == 0) ? 5.0f : 15.0f);
    const Stats::TimingStats jittery = Stats::Summarize(alternating);

    CHECK(smooth.meanMs == doctest::Approx(jittery.meanMs));
    CHECK(smooth.pacingMeanAbsDeltaMs == doctest::Approx(0.0f));
    CHECK(jittery.pacingMeanAbsDeltaMs == doctest::Approx(10.0f));
}

TEST_CASE("slow frames are counted relative to the median, and consecutive ones are tracked")
{
    std::vector<float> frames = ConstantFrames(10, 10.0f);
    frames.push_back(30.0f);
    frames.push_back(30.0f);
    frames.push_back(30.0f);
    frames.push_back(10.0f);

    const Stats::TimingStats stats = Stats::Summarize(frames);
    CHECK(stats.slowFrameCount == 3);
    CHECK(stats.longestSlowFrameRun == 3);
}

TEST_CASE("invalid repeats are excluded from the segment aggregate")
{
    SegmentSamples segment;
    segment.name = "scene.full";
    segment.repeats.push_back(MakeRepeat(0, ConstantFrames(10, 10.0f)));

    RepeatSamples discarded = MakeRepeat(1, ConstantFrames(10, 100.0f));
    discarded.invalid = InvalidReason::Stalled;
    segment.repeats.push_back(discarded);

    const Stats::SegmentStats stats = Stats::Summarize(segment, {});
    CHECK(stats.validRepeats == 1);
    CHECK(stats.invalidRepeats == 1);
    CHECK(stats.measuredFrames == 10);
    CHECK(stats.frame.medianMs == doctest::Approx(10.0f));
}

TEST_CASE("repeat spread reports the gap between repeat medians")
{
    SegmentSamples segment;
    segment.name = "scene.full";
    segment.repeats.push_back(MakeRepeat(0, ConstantFrames(10, 10.0f)));
    segment.repeats.push_back(MakeRepeat(1, ConstantFrames(10, 12.0f)));

    const Stats::SegmentStats stats = Stats::Summarize(segment, {});
    CHECK(stats.repeatMedianMs.size() == 2);
    CHECK(stats.repeatSpreadMs == doctest::Approx(2.0f));
    CHECK(stats.repeatSpreadPercent == doctest::Approx(100.0f * 2.0f / 11.0f));
}

TEST_CASE("nested passes are excluded from the attribution total")
{
    SegmentSamples segment;
    segment.name = "scene.full";

    RepeatSamples repeat;
    repeat.repeatIndex = 0;
    FrameSample sample;
    sample.frameMs = 10.0f;
    sample.cpuMs[0] = 6.0f;   // a top-level pass
    sample.cpuMs[1] = 3.0f;   // nested inside pass 0 -- already counted in its 6 ms
    repeat.frames.push_back(sample);
    segment.repeats.push_back(repeat);

    const std::vector<bool> nested = { false, true };
    const Stats::SegmentStats stats = Stats::Summarize(segment, nested);
    CHECK(stats.attributedCpuMs == doctest::Approx(6.0f));
    CHECK(stats.unattributedMs == doctest::Approx(4.0f));
    CHECK(stats.unattributedPercent == doctest::Approx(40.0f));
}

TEST_CASE("counters are reported per measured frame")
{
    SegmentSamples segment;
    segment.name = "scene.full";

    RepeatSamples repeat = MakeRepeat(0, ConstantFrames(4, 10.0f));
    repeat.counters.total[0] = 400;
    repeat.counters.perPass[2][0] = 200;
    segment.repeats.push_back(repeat);

    const Stats::SegmentStats stats = Stats::Summarize(segment, {});
    CHECK(stats.counterPerFrame[0] == doctest::Approx(100.0));
    CHECK(stats.passCounterPerFrame[2][0] == doctest::Approx(50.0));
}
