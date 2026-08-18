#pragma once

#include <string>
#include <vector>

#include "BenchStats.h"
#include "BenchTypes.h"

// Rule-based observations over a finished run. Pure, and deliberately narrow: every finding is
// produced by one named rule with a fixed threshold, and carries that rule's code so a reader
// can check what it actually tested. The tool does not write conclusions it cannot support --
// "this change improves particle performance" is a claim for a human to make after looking at
// two runs, not something a single run's numbers can establish.

namespace Core::Benchmark::Findings
{
    // Thresholds. Named because a magic number buried in a rule is a bug waiting to happen, and
    // because a reader has to be able to see what "warning" meant.
    inline constexpr float kAttributionGapPercent = 10.0f;   // frame time outside every pass scope
    inline constexpr float kDriftWarnPercent = 5.0f;         // baseline start vs. end of run
    inline constexpr float kNoisySegmentSpreadPercent = 10.0f;
    inline constexpr float kPresentStallPercent = 30.0f;     // Present as a share of frame time
    inline constexpr float kUboSkipSuspiciousRatio = 0.9f;   // skips per draw call
    inline constexpr float kBufferOrphansPerFrameWarn = 5.0f;
    inline constexpr int   kSubmissionBoundMinDraws = 100;
    // An IR quad is 6 vertices after decomposition, so this is "fewer than two quads per draw".
    inline constexpr float kSubmissionBoundVerticesPerDraw = 12.0f;

    enum class Level
    {
        Info,
        Warning,
    };

    const char* ToString(Level level);

    struct Finding
    {
        Level level = Level::Info;
        std::string rule;       // stable code, e.g. "attribution-gap"
        std::string segment;    // empty when the finding is about the run as a whole
        std::string text;
    };

    std::vector<Finding> Evaluate(const RunData& run, const std::vector<Stats::SegmentStats>& stats);
}
