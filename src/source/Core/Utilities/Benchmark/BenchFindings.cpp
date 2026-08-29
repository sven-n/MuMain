#include "BenchFindings.h"

#include <cmath>
#include <cstdio>

namespace Core::Benchmark::Findings
{
namespace
{
    int IndexOf(const std::vector<std::string>& names, const char* wanted)
    {
        for (size_t i = 0; i < names.size(); i++)
            if (names[i] == wanted) return (int)i;
        return -1;
    }

    void Add(std::vector<Finding>& out, Level level, const char* rule, const std::string& segment, std::string text)
    {
        out.push_back(Finding{ level, rule, segment, std::move(text) });
    }

    std::string Fixed(double value, int decimals)
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
        return buffer;
    }

    // The frame time that sits outside every FRAME_PROFILE scope. A large remainder means work is
    // running unmeasured -- the blind spot GLP-24 found after it had already invalidated a phase
    // of measurements, and the one check that would have caught it automatically.
    void CheckAttribution(const Stats::SegmentStats& segment, std::vector<Finding>& out)
    {
        if (std::fabs(segment.unattributedPercent) <= kAttributionGapPercent) return;

        Add(out, Level::Warning, "attribution-gap", segment.name,
            Fixed(segment.unattributedPercent, 1) + "% of the frame (" +
            Fixed(segment.unattributedMs, 2) + " ms) is outside every profiled pass");
    }

    // Run-to-run spread within one segment. A delta between two runs smaller than this is not a
    // result, it is the machine.
    void CheckRepeatSpread(const Stats::SegmentStats& segment, std::vector<Finding>& out)
    {
        if (segment.repeatSpreadPercent <= kNoisySegmentSpreadPercent) return;

        Add(out, Level::Warning, "noisy-segment", segment.name,
            "repeat medians spread " + Fixed(segment.repeatSpreadPercent, 1) + "% (" +
            Fixed(segment.repeatSpreadMs, 2) + " ms) -- treat smaller deltas as noise");
    }

    void CheckInvalidRepeats(const Stats::SegmentStats& segment, std::vector<Finding>& out)
    {
        if (segment.invalidRepeats == 0) return;

        Add(out, Level::Warning, "invalid-repeats", segment.name,
            std::to_string(segment.invalidRepeats) + " of " +
            std::to_string(segment.invalidRepeats + segment.validRepeats) +
            " repeats were discarded");
    }

    void CheckPresentStall(const RunData& run, const Stats::SegmentStats& segment, std::vector<Finding>& out)
    {
        const int presentIndex = IndexOf(run.passNames, "Present");
        if (presentIndex < 0 || segment.frame.meanMs <= 0.0f) return;

        const float presentPercent = 100.0f * segment.passCpuMs[presentIndex] / segment.frame.meanMs;
        if (presentPercent <= kPresentStallPercent) return;

        Add(out, Level::Info, "present-stall", segment.name,
            "Present is " + Fixed(presentPercent, 1) +
            "% of the frame -- the CPU is waiting on the GPU command queue");
    }

    // Fewer than two quads per IR draw in a pass that submits a lot of them: batching is merging
    // nothing there, so the cost is submission, not the work each draw does.
    void CheckSubmissionBound(const RunData& run, const Stats::SegmentStats& segment, std::vector<Finding>& out)
    {
        const int drawsIndex = IndexOf(run.counterNames, "IRDraws");
        const int verticesIndex = IndexOf(run.counterNames, "IRVertices");
        if (drawsIndex < 0 || verticesIndex < 0) return;

        for (size_t pass = 0; pass < run.passNames.size() && pass < (size_t)kMaxPasses; pass++)
        {
            const double draws = segment.passCounterPerFrame[pass][drawsIndex];
            if (draws < kSubmissionBoundMinDraws) continue;

            const double verticesPerDraw = segment.passCounterPerFrame[pass][verticesIndex] / draws;
            if (verticesPerDraw >= kSubmissionBoundVerticesPerDraw) continue;

            Add(out, Level::Info, "submission-bound", segment.name,
                run.passNames[pass] + ": " + Fixed(draws, 0) + " IR draws/frame at " +
                Fixed(verticesPerDraw, 1) + " vertices per draw -- batching is merging nothing");
        }
    }

    void CheckBufferOrphans(const RunData& run, const Stats::SegmentStats& segment, std::vector<Finding>& out)
    {
        const int orphansIndex = IndexOf(run.counterNames, "BufferOrphans");
        if (orphansIndex < 0) return;

        const double orphans = segment.counterPerFrame[orphansIndex];
        if (orphans <= kBufferOrphansPerFrameWarn) return;

        Add(out, Level::Info, "buffer-orphans", segment.name,
            Fixed(orphans, 1) + " buffer orphans per frame -- a streaming ring is wrapping repeatedly");
    }

    // Skips matching draw calls one-for-one is the red flag GLP-10 named for its own dirty check:
    // a dedupe that never misses is more likely broken than perfect.
    void CheckUboSkips(const RunData& run, const Stats::SegmentStats& segment, std::vector<Finding>& out)
    {
        const int skipsIndex = IndexOf(run.counterNames, "UboSkips");
        const int drawsIndex = IndexOf(run.counterNames, "DrawCalls");
        if (skipsIndex < 0 || drawsIndex < 0) return;

        const double draws = segment.counterPerFrame[drawsIndex];
        if (draws <= 0.0) return;

        const double ratio = segment.counterPerFrame[skipsIndex] / draws;
        if (ratio < kUboSkipSuspiciousRatio) return;

        Add(out, Level::Warning, "ubo-skip-suspicious", segment.name,
            Fixed(ratio, 2) + " uniform-block skips per draw call -- check the dirty check is not over-skipping");
    }

    // A segment that changed nothing the client submitted did not test what it claims to. Either
    // the scene had none of what it disables (no wings equipped, no particles on screen) or the
    // segment's configuration does not reach the path it names. Both are worth saying out loud:
    // otherwise the row reads as "this costs nothing", which is a very different claim.
    void CheckInertSegments(const RunData& run, const std::vector<Stats::SegmentStats>& stats,
                            std::vector<Finding>& out)
    {
        const int drawsIndex = IndexOf(run.counterNames, "DrawCalls");
        const int baselineIndex = Stats::FindSegment(stats, kBaselineSegmentName);
        if (drawsIndex < 0 || baselineIndex < 0) return;

        const double baselineDraws = stats[baselineIndex].counterPerFrame[drawsIndex];
        if (baselineDraws <= 0.0) return;

        for (const Stats::SegmentStats& segment : stats)
        {
            if (segment.name == kBaselineSegmentName) continue;
            if (segment.name == std::string(kBaselineSegmentName) + kDriftControlSuffix) continue;

            const double draws = segment.counterPerFrame[drawsIndex];
            const double differencePercent = 100.0 * std::fabs(draws - baselineDraws) / baselineDraws;
            if (differencePercent >= kInertSegmentDrawPercent) continue;

            Add(out, Level::Warning, "segment-inert", segment.name,
                "submitted " + Fixed(draws, 1) + " draws/frame against the baseline's " +
                Fixed(baselineDraws, 1) + " -- this segment disabled nothing measurable here, so "
                "its timing is not evidence that what it targets is cheap");
        }
    }

    // The opening and closing measurements of the baseline segment. They ran minutes apart on the
    // same scene, so any difference between them is the machine drifting under the run.
    void CheckDrift(const std::vector<Stats::SegmentStats>& stats, std::vector<Finding>& out)
    {
        const int startIndex = Stats::FindSegment(stats, kBaselineSegmentName);
        const int endIndex = Stats::FindSegment(stats, std::string(kBaselineSegmentName) + kDriftControlSuffix);
        if (startIndex < 0 || endIndex < 0) return;

        const float startMs = stats[startIndex].frame.medianMs;
        const float endMs = stats[endIndex].frame.medianMs;
        if (startMs <= 0.0f) return;

        const float driftPercent = 100.0f * (endMs - startMs) / startMs;
        if (std::fabs(driftPercent) <= kDriftWarnPercent) return;

        Add(out, Level::Warning, "drift", "",
            "the baseline segment moved " + Fixed(driftPercent, 1) +
            "% between the start and the end of the run -- every segment in between is suspect");
    }

    void CheckRunStatus(const RunData& run, std::vector<Finding>& out)
    {
        if (run.status == RunStatus::Aborted)
            Add(out, Level::Warning, "aborted", "", "the run did not finish; partial data only");

        if (run.environment.gitDirty)
            Add(out, Level::Warning, "dirty-tree", "",
                "the working tree had uncommitted changes -- this run cannot be reproduced from the commit alone");

        if (run.environment.vsyncEffective)
            Add(out, Level::Warning, "vsync-on", "",
                "vsync was active -- frame times are pinned to the display refresh, not to what the client can do");
    }
}

const char* ToString(Level level)
{
    switch (level)
    {
    case Level::Info:    return "info";
    case Level::Warning: return "warning";
    }
    return "unknown";
}

std::vector<Finding> Evaluate(const RunData& run, const std::vector<Stats::SegmentStats>& stats)
{
    std::vector<Finding> findings;

    CheckRunStatus(run, findings);
    CheckDrift(stats, findings);
    CheckInertSegments(run, stats, findings);

    for (const Stats::SegmentStats& segment : stats)
    {
        CheckInvalidRepeats(segment, findings);
        CheckAttribution(segment, findings);
        CheckRepeatSpread(segment, findings);
        CheckPresentStall(run, segment, findings);
        CheckSubmissionBound(run, segment, findings);
        CheckBufferOrphans(run, segment, findings);
        CheckUboSkips(run, segment, findings);
    }

    return findings;
}
}
