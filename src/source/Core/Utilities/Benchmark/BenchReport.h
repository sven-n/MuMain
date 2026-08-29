#pragma once

#include <string>
#include <vector>

#include "BenchFindings.h"
#include "BenchStats.h"
#include "BenchTypes.h"

// Serializers for a finished run. Pure string building, so the exact bytes written to disk are
// unit-testable.
//
// run.json is the canonical artifact -- the one a comparison tool reads. The Markdown and the
// CSVs are views rendered from the same data. The Markdown is never hand-edited: the moment a
// report becomes something a human maintains, the whole thing has decayed back into the
// screenshot workflow it exists to replace.
//
// Everything here formats floats at a fixed precision and emits rows in a fixed order, so two
// report.md files from two builds diff cleanly against each other.

namespace Core::Benchmark::Report
{
    inline constexpr const char* kRunJsonFileName = "run.json";
    inline constexpr const char* kReportFileName = "report.md";
    inline constexpr const char* kFramesCsvFileName = "frames.csv";
    inline constexpr const char* kPassesCsvFileName = "passes.csv";

    // Bumped whenever a field changes meaning or disappears, so a comparison tool can refuse a
    // run it would misread instead of quietly comparing the wrong columns.
    inline constexpr int kSchemaVersion = 1;

    std::string BuildRunJson(const RunData& run,
                             const std::vector<Stats::SegmentStats>& stats,
                             const std::vector<Findings::Finding>& findings);

    // One row per measured frame: identity, frame time, and per-pass CPU/GPU ms. No counter
    // columns -- counters are near-deterministic for a fixed workload, so their per-segment
    // totals in passes.csv carry the same information without 500 columns nobody reads.
    std::string BuildFramesCsv(const RunData& run);

    // One row per segment x repeat x pass, plus a TOTAL row per repeat: pass timings and every
    // GL counter, averaged per measured frame.
    std::string BuildPassesCsv(const RunData& run);

    std::string BuildReportMarkdown(const RunData& run,
                                    const std::vector<Stats::SegmentStats>& stats,
                                    const std::vector<Findings::Finding>& findings);
}
