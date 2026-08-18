#include "doctest.h"

#include "Core/Utilities/Benchmark/BenchFindings.h"
#include "Core/Utilities/Benchmark/BenchReport.h"
#include "Core/Utilities/Benchmark/BenchStats.h"

#include <string>
#include <vector>

using namespace Core::Benchmark;

namespace
{
    int CountOccurrences(const std::string& haystack, const std::string& needle)
    {
        int count = 0;
        for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
            count++;
        return count;
    }

    bool HasRule(const std::vector<Findings::Finding>& findings, const std::string& rule)
    {
        for (const Findings::Finding& finding : findings)
            if (finding.rule == rule) return true;
        return false;
    }

    RepeatSamples MakeRepeat(int index, int frameCount, float frameMs)
    {
        RepeatSamples repeat;
        repeat.repeatIndex = index;
        for (int i = 0; i < frameCount; i++)
        {
            FrameSample sample;
            sample.frameMs = frameMs;
            sample.cpuMs[0] = frameMs * 0.5f;   // Terrain
            sample.gpuMs[0] = frameMs * 0.25f;
            repeat.frames.push_back(sample);
        }
        repeat.counters.total[0] = (uint64_t)(100 * frameCount);
        repeat.counters.perPass[0][0] = (uint64_t)(60 * frameCount);
        return repeat;
    }

    // A minimal but complete run: two segments, two repeats each, one pass carrying half the
    // frame so the attribution remainder is a known 50%.
    RunData MakeRun()
    {
        RunData run;
        run.runId = "20260818_142233_test";
        run.manifest.label = "test";
        run.manifest.pattern = "*";
        run.manifest.repeats = 2;
        run.manifest.warmupFrames = 30;
        run.manifest.measureFrames = 10;
        run.manifest.segmentNames = { "scene.full", "fx.particles.off" };
        run.manifestHash = ComputeManifestHash(run.manifest);
        run.passNames = { "Terrain", "Skinning", "Present" };
        run.passIsNested = { false, true, false };
        run.counterNames = { "DrawCalls", "IRDraws", "IRVertices" };
        run.environment.buildConfig = "Release";
        run.environment.glRenderer = "Test Renderer";

        for (const std::string& name : run.manifest.segmentNames)
        {
            SegmentSamples segment;
            segment.name = name;
            segment.repeats.push_back(MakeRepeat(0, 10, 10.0f));
            segment.repeats.push_back(MakeRepeat(1, 10, 10.0f));
            run.segments.push_back(segment);
        }
        return run;
    }
}

TEST_CASE("manifest hash changes with the segment order but not with the label")
{
    RunManifest a;
    a.segmentNames = { "scene.full", "fx.particles.off" };
    a.repeats = 3;

    RunManifest reordered = a;
    reordered.segmentNames = { "fx.particles.off", "scene.full" };

    RunManifest relabelled = a;
    relabelled.label = "something else";

    CHECK(ComputeManifestHash(a) != ComputeManifestHash(reordered));
    CHECK(ComputeManifestHash(a) == ComputeManifestHash(relabelled));
    CHECK(ComputeManifestHash(a).size() == 16);
}

TEST_CASE("frames.csv has one header plus one row per measured frame")
{
    const RunData run = MakeRun();
    const std::string csv = Report::BuildFramesCsv(run);

    // 2 segments x 2 repeats x 10 frames, plus the header.
    CHECK(CountOccurrences(csv, "\n") == 41);
    CHECK(csv.rfind("segment,repeat,frame,frame_ms,cpu_Terrain", 0) == 0);
    CHECK(csv.find("cpu_Present") != std::string::npos);
    CHECK(csv.find("gpu_Terrain") != std::string::npos);
    // Counters belong in passes.csv, never per frame.
    CHECK(csv.find("DrawCalls") == std::string::npos);
}

TEST_CASE("passes.csv carries one row per pass plus a TOTAL row, per repeat")
{
    const RunData run = MakeRun();
    const std::string csv = Report::BuildPassesCsv(run);

    // 2 segments x 2 repeats x (3 passes + 1 total), plus the header.
    CHECK(CountOccurrences(csv, "\n") == 17);
    CHECK(CountOccurrences(csv, ",TOTAL,") == 4);
    CHECK(csv.find("DrawCalls") != std::string::npos);
    // 100 draw calls per frame, summed over 10 frames and reported per frame again.
    CHECK(csv.find(",TOTAL,10.00,0.00,100.0") != std::string::npos);
}

TEST_CASE("the report renders every section and is byte-identical for identical input")
{
    const RunData run = MakeRun();
    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    const std::vector<Findings::Finding> findings = Findings::Evaluate(run, stats);

    const std::string first = Report::BuildReportMarkdown(run, stats, findings);
    const std::string second = Report::BuildReportMarkdown(run, stats, findings);
    CHECK(first == second);

    CHECK(first.find("# Benchmark run 20260818_142233_test") != std::string::npos);
    CHECK(first.find("## Identity") != std::string::npos);
    CHECK(first.find("## Build") != std::string::npos);
    CHECK(first.find("## Machine") != std::string::npos);
    CHECK(first.find("## Graphics") != std::string::npos);
    CHECK(first.find("## Run configuration") != std::string::npos);
    CHECK(first.find("## Segment summary") != std::string::npos);
    CHECK(first.find("## Frame pacing") != std::string::npos);
    CHECK(first.find("## Findings") != std::string::npos);
    CHECK(first.find("## Pass breakdown") != std::string::npos);
    CHECK(first.find("## Health") != std::string::npos);
    CHECK(first.find("Test Renderer") != std::string::npos);
}

TEST_CASE("an aborted, dirty-tree run says so at the top of the report")
{
    RunData run = MakeRun();
    run.status = RunStatus::Aborted;
    run.environment.gitDirty = true;

    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    const std::string markdown = Report::BuildReportMarkdown(run, stats, Findings::Evaluate(run, stats));

    CHECK(markdown.find("**Aborted run.**") != std::string::npos);
    CHECK(markdown.find("**Uncommitted changes.**") != std::string::npos);
}

TEST_CASE("run.json is well-formed and carries the schema version")
{
    const RunData run = MakeRun();
    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    const std::string json = Report::BuildRunJson(run, stats, Findings::Evaluate(run, stats));

    CHECK(json.rfind("{\n", 0) == 0);
    CHECK(json.find("\"schemaVersion\": 1") != std::string::npos);
    CHECK(json.find("\"manifestHash\"") != std::string::npos);
    CHECK(json.find("\"percentileCurveMs\"") != std::string::npos);
    CHECK(CountOccurrences(json, "{") == CountOccurrences(json, "}"));
    CHECK(CountOccurrences(json, "[") == CountOccurrences(json, "]"));
}

TEST_CASE("strings that would break the exports are escaped, not passed through")
{
    RunData run = MakeRun();
    run.manifest.label = "quote\" and \\ backslash";
    run.environment.glRenderer = "line\nbreak";

    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    const std::string json = Report::BuildRunJson(run, stats, {});

    CHECK(json.find("quote\\\" and \\\\ backslash") != std::string::npos);
    CHECK(json.find("line\\nbreak") != std::string::npos);
}

TEST_CASE("attribution gap fires when frame time sits outside every pass")
{
    const RunData run = MakeRun();
    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);

    // Terrain is 5 ms of a 10 ms frame and Skinning is nested, so half the frame is unattributed.
    CHECK(stats.front().unattributedPercent == doctest::Approx(50.0f));
    CHECK(HasRule(Findings::Evaluate(run, stats), "attribution-gap"));
}

TEST_CASE("drift fires when the closing baseline differs from the opening one")
{
    RunData run = MakeRun();
    SegmentSamples driftControl;
    driftControl.name = std::string(kBaselineSegmentName) + kDriftControlSuffix;
    driftControl.repeats.push_back(MakeRepeat(0, 10, 12.0f));
    run.segments.push_back(driftControl);

    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    CHECK(HasRule(Findings::Evaluate(run, stats), "drift"));
}

TEST_CASE("drift stays quiet when the closing baseline matches")
{
    RunData run = MakeRun();
    SegmentSamples driftControl;
    driftControl.name = std::string(kBaselineSegmentName) + kDriftControlSuffix;
    driftControl.repeats.push_back(MakeRepeat(0, 10, 10.0f));
    run.segments.push_back(driftControl);

    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    CHECK_FALSE(HasRule(Findings::Evaluate(run, stats), "drift"));
}

TEST_CASE("submission-bound fires on many IR draws carrying almost no vertices")
{
    RunData run = MakeRun();
    for (RepeatSamples& repeat : run.segments.front().repeats)
    {
        repeat.counters.perPass[0][1] = 3000 * 10;  // IRDraws over 10 frames
        repeat.counters.perPass[0][2] = 3000 * 10 * 6;
    }

    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    CHECK(HasRule(Findings::Evaluate(run, stats), "submission-bound"));
}

TEST_CASE("submission-bound stays quiet when batches are actually merging")
{
    RunData run = MakeRun();
    for (RepeatSamples& repeat : run.segments.front().repeats)
    {
        repeat.counters.perPass[0][1] = 200 * 10;
        repeat.counters.perPass[0][2] = 200 * 10 * 600;
    }

    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    CHECK_FALSE(HasRule(Findings::Evaluate(run, stats), "submission-bound"));
}

TEST_CASE("a discarded repeat is reported rather than silently averaged in")
{
    RunData run = MakeRun();
    run.segments.front().repeats[1].invalid = InvalidReason::Stalled;

    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    CHECK(HasRule(Findings::Evaluate(run, stats), "invalid-repeats"));
}

TEST_CASE("vsync and a dirty tree are flagged for the run as a whole")
{
    RunData run = MakeRun();
    run.environment.vsyncEffective = true;
    run.environment.gitDirty = true;

    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    const std::vector<Findings::Finding> findings = Findings::Evaluate(run, stats);
    CHECK(HasRule(findings, "vsync-on"));
    CHECK(HasRule(findings, "dirty-tree"));
}
