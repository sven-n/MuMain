#include "BenchStats.h"

#include <algorithm>
#include <cmath>

namespace Core::Benchmark::Stats
{
namespace
{
    float MsToFps(float ms)
    {
        return (ms > 0.0f) ? (1000.0f / ms) : 0.0f;
    }

    // Nearest-rank percentile on an already sorted, non-empty vector.
    float PercentileOfSorted(const std::vector<float>& sorted, float fraction)
    {
        const size_t maxIndex = sorted.size() - 1;
        const size_t index = (size_t)std::lround(fraction * (float)maxIndex);
        return sorted[std::min(index, maxIndex)];
    }

    float MeanOf(const std::vector<float>& values)
    {
        if (values.empty()) return 0.0f;
        double sum = 0.0;
        for (float v : values) sum += v;
        return (float)(sum / (double)values.size());
    }

    float StdDevOf(const std::vector<float>& values, float mean)
    {
        if (values.size() < 2) return 0.0f;
        double sumSquares = 0.0;
        for (float v : values)
        {
            const double d = (double)v - (double)mean;
            sumSquares += d * d;
        }
        return (float)std::sqrt(sumSquares / (double)(values.size() - 1));
    }

    // Mean of the worst (slowest) share of frames, at least one frame.
    float WorstFractionMean(const std::vector<float>& sorted, float fraction)
    {
        const size_t count = std::max<size_t>(1, (size_t)(sorted.size() * fraction));
        double sum = 0.0;
        for (size_t i = sorted.size() - count; i < sorted.size(); i++) sum += sorted[i];
        return (float)(sum / (double)count);
    }

    void FillPacing(const std::vector<float>& frameTimesMs, TimingStats& out)
    {
        if (frameTimesMs.size() < 2) return;

        double deltaSum = 0.0;
        for (size_t i = 1; i < frameTimesMs.size(); i++)
            deltaSum += std::fabs((double)frameTimesMs[i] - (double)frameTimesMs[i - 1]);
        out.pacingMeanAbsDeltaMs = (float)(deltaSum / (double)(frameTimesMs.size() - 1));

        const float slowThresholdMs = out.medianMs * kSlowFrameMedianMultiple;
        int currentRun = 0;
        for (float ms : frameTimesMs)
        {
            if (ms <= slowThresholdMs)
            {
                currentRun = 0;
                continue;
            }
            out.slowFrameCount++;
            currentRun++;
            out.longestSlowFrameRun = std::max(out.longestSlowFrameRun, currentRun);
        }
    }

    void FillPercentileCurve(const std::vector<float>& sorted, TimingStats& out)
    {
        out.percentileCurveMs.reserve(kPercentileCurvePoints);
        for (int i = 0; i < kPercentileCurvePoints; i++)
            out.percentileCurveMs.push_back(PercentileOfSorted(sorted, (float)i / (float)(kPercentileCurvePoints - 1)));
    }
}

TimingStats Summarize(const std::vector<float>& frameTimesMs)
{
    TimingStats out;
    if (frameTimesMs.empty()) return out;

    std::vector<float> sorted = frameTimesMs;
    std::sort(sorted.begin(), sorted.end());

    out.frameCount = (int)sorted.size();
    out.meanMs = MeanOf(sorted);
    out.medianMs = PercentileOfSorted(sorted, 0.5f);
    out.p95Ms = PercentileOfSorted(sorted, 0.95f);
    out.p99Ms = PercentileOfSorted(sorted, 0.99f);
    out.minMs = sorted.front();
    out.maxMs = sorted.back();
    out.stdDevMs = StdDevOf(sorted, out.meanMs);
    out.meanFps = MsToFps(out.meanMs);
    out.onePercentLowMs = WorstFractionMean(sorted, kLowPercentileFraction);
    out.onePercentLowFps = MsToFps(out.onePercentLowMs);

    FillPacing(frameTimesMs, out);
    FillPercentileCurve(sorted, out);
    return out;
}

namespace
{
    bool IsValid(const RepeatSamples& repeat)
    {
        return repeat.invalid == InvalidReason::None && !repeat.frames.empty();
    }

    void AccumulatePassMs(const SegmentSamples& segment, SegmentStats& out)
    {
        double cpuSum[kMaxPasses] = {};
        double gpuSum[kMaxPasses] = {};
        int frames = 0;

        for (const RepeatSamples& repeat : segment.repeats)
        {
            if (!IsValid(repeat)) continue;
            for (const FrameSample& frame : repeat.frames)
            {
                for (int p = 0; p < kMaxPasses; p++)
                {
                    cpuSum[p] += frame.cpuMs[p];
                    gpuSum[p] += frame.gpuMs[p];
                }
            }
            frames += (int)repeat.frames.size();
        }

        if (frames == 0) return;
        for (int p = 0; p < kMaxPasses; p++)
        {
            out.passCpuMs[p] = (float)(cpuSum[p] / frames);
            out.passGpuMs[p] = (float)(gpuSum[p] / frames);
        }
    }

    void AccumulateCounters(const SegmentSamples& segment, SegmentStats& out)
    {
        int frames = 0;
        for (const RepeatSamples& repeat : segment.repeats)
        {
            if (!IsValid(repeat)) continue;
            for (int c = 0; c < kMaxCounters; c++)
            {
                out.counterPerFrame[c] += (double)repeat.counters.total[c];
                for (int p = 0; p < kMaxPasses; p++)
                    out.passCounterPerFrame[p][c] += (double)repeat.counters.perPass[p][c];
            }
            frames += (int)repeat.frames.size();
        }

        if (frames == 0) return;
        for (int c = 0; c < kMaxCounters; c++)
        {
            out.counterPerFrame[c] /= frames;
            for (int p = 0; p < kMaxPasses; p++)
                out.passCounterPerFrame[p][c] /= frames;
        }
    }

    void FillAttribution(SegmentStats& out, const std::vector<bool>& passIsNested)
    {
        for (int p = 0; p < kMaxPasses; p++)
        {
            const bool nested = ((size_t)p < passIsNested.size()) && passIsNested[p];
            if (!nested) out.attributedCpuMs += out.passCpuMs[p];
        }

        out.unattributedMs = out.frame.meanMs - out.attributedCpuMs;
        if (out.frame.meanMs > 0.0f)
            out.unattributedPercent = 100.0f * out.unattributedMs / out.frame.meanMs;
    }

    void FillRepeatSpread(SegmentStats& out)
    {
        if (out.repeatMedianMs.size() < 2) return;

        const auto range = std::minmax_element(out.repeatMedianMs.begin(), out.repeatMedianMs.end());
        out.repeatSpreadMs = *range.second - *range.first;

        const float meanOfMedians = MeanOf(out.repeatMedianMs);
        if (meanOfMedians > 0.0f)
            out.repeatSpreadPercent = 100.0f * out.repeatSpreadMs / meanOfMedians;
    }
}

SegmentStats Summarize(const SegmentSamples& segment, const std::vector<bool>& passIsNested)
{
    SegmentStats out;
    out.name = segment.name;

    std::vector<float> pooledFrameTimes;
    for (const RepeatSamples& repeat : segment.repeats)
    {
        if (!IsValid(repeat))
        {
            out.invalidRepeats++;
            continue;
        }

        out.validRepeats++;
        std::vector<float> repeatFrameTimes;
        repeatFrameTimes.reserve(repeat.frames.size());
        for (const FrameSample& frame : repeat.frames) repeatFrameTimes.push_back(frame.frameMs);

        out.repeatMedianMs.push_back(Summarize(repeatFrameTimes).medianMs);
        pooledFrameTimes.insert(pooledFrameTimes.end(), repeatFrameTimes.begin(), repeatFrameTimes.end());
    }

    out.measuredFrames = (int)pooledFrameTimes.size();
    out.frame = Summarize(pooledFrameTimes);

    AccumulatePassMs(segment, out);
    AccumulateCounters(segment, out);
    FillAttribution(out, passIsNested);
    FillRepeatSpread(out);
    return out;
}

std::vector<SegmentStats> SummarizeRun(const RunData& run)
{
    std::vector<SegmentStats> stats;
    stats.reserve(run.segments.size());
    for (const SegmentSamples& segment : run.segments)
        stats.push_back(Summarize(segment, run.passIsNested));
    return stats;
}

int FindSegment(const std::vector<SegmentStats>& stats, const std::string& name)
{
    for (size_t i = 0; i < stats.size(); i++)
        if (stats[i].name == name) return (int)i;
    return -1;
}
}
