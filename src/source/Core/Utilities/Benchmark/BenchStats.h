#pragma once

#include <string>
#include <vector>

#include "BenchTypes.h"

// Aggregation math for a benchmark run. Pure: no project dependencies, linkable from tests.
//
// The numbers here are deliberately more than a mean. A change can improve the average frame
// time and make pacing worse, and a mean alone will happily call that a win -- so the summary
// carries the distribution (percentile curve), the tail (1% low), and the pacing metrics
// side by side. See docs/benchmark.md.

namespace Core::Benchmark::Stats
{
    // A frame counts as "slow" at this multiple of the segment's own median frame time. Relative
    // rather than an absolute ms threshold so it means the same thing on a 30 FPS laptop and a
    // 240 FPS desktop.
    inline constexpr float kSlowFrameMedianMultiple = 2.0f;

    // Share of the frames that make up the "1% low" figure.
    inline constexpr float kLowPercentileFraction = 0.01f;

    struct TimingStats
    {
        int frameCount = 0;
        float meanMs = 0.0f;
        float medianMs = 0.0f;
        float p95Ms = 0.0f;
        float p99Ms = 0.0f;
        float minMs = 0.0f;
        float maxMs = 0.0f;
        float stdDevMs = 0.0f;
        float meanFps = 0.0f;
        // Mean of the worst 1% of frames -- the figure the $details overlay calls "1% Low".
        float onePercentLowMs = 0.0f;
        float onePercentLowFps = 0.0f;
        // Frame pacing: mean absolute difference between consecutive frames. Low mean frame time
        // with a high value here is a stuttering frame stream, not a smooth fast one.
        float pacingMeanAbsDeltaMs = 0.0f;
        int slowFrameCount = 0;
        int longestSlowFrameRun = 0;
        // Frame times at evenly spaced percentiles, ascending. kPercentileCurvePoints entries,
        // or empty when there were no frames.
        std::vector<float> percentileCurveMs;
    };

    TimingStats Summarize(const std::vector<float>& frameTimesMs);

    struct SegmentStats
    {
        std::string name;
        int validRepeats = 0;
        int invalidRepeats = 0;
        int measuredFrames = 0;

        // Pooled over every valid repeat's frames.
        TimingStats frame;

        // One median per valid repeat. The spread between them is the noise band a delta has to
        // clear before it means anything -- a run-to-run difference smaller than this is not a
        // result, which is exactly the trap GLP-29 fell into.
        std::vector<float> repeatMedianMs;
        float repeatSpreadMs = 0.0f;
        float repeatSpreadPercent = 0.0f;

        // Per measured frame, averaged over valid repeats.
        float passCpuMs[kMaxPasses] = {};
        float passGpuMs[kMaxPasses] = {};
        double counterPerFrame[kMaxCounters] = {};
        double passCounterPerFrame[kMaxPasses][kMaxCounters] = {};

        // Attribution check: pass CPU ms summed over the non-nested passes, against the measured
        // frame time. A large remainder means real work is running outside every FRAME_PROFILE
        // scope -- the blind spot GLP-24 found after it had already invalidated a phase of
        // measurements.
        float attributedCpuMs = 0.0f;
        float unattributedMs = 0.0f;
        float unattributedPercent = 0.0f;
    };

    // passIsNested marks passes contained in another pass, which must not be summed into the
    // attribution total. A short or empty vector is treated as "nothing is nested".
    SegmentStats Summarize(const SegmentSamples& segment, const std::vector<bool>& passIsNested);

    std::vector<SegmentStats> SummarizeRun(const RunData& run);

    // Index of the segment stats with the given name, or -1.
    int FindSegment(const std::vector<SegmentStats>& stats, const std::string& name);
}
