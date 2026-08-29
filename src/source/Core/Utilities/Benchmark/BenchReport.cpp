#include "BenchReport.h"

#include <algorithm>
#include <cstdio>

namespace Core::Benchmark::Report
{
namespace
{
    // Fixed precision everywhere: two report.md files from two builds have to diff cleanly, and
    // a float printed at whatever width it happens to need defeats that.
    constexpr int kMsDecimals = 2;
    constexpr int kFpsDecimals = 1;
    constexpr int kPercentDecimals = 1;
    constexpr int kCounterDecimals = 1;

    std::string Fixed(double value, int decimals)
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
        return buffer;
    }

    std::string JsonEscape(const std::string& text)
    {
        std::string out;
        out.reserve(text.size() + 8);
        for (char c : text)
        {
            switch (c)
            {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if ((unsigned char)c < 0x20) out += ' ';
                else out += c;
            }
        }
        return out;
    }

    std::string JsonString(const std::string& key, const std::string& value, bool trailingComma = true)
    {
        return "\"" + key + "\": \"" + JsonEscape(value) + "\"" + (trailingComma ? "," : "");
    }

    std::string JsonNumber(const std::string& key, double value, int decimals, bool trailingComma = true)
    {
        return "\"" + key + "\": " + Fixed(value, decimals) + (trailingComma ? "," : "");
    }

    std::string JsonInt(const std::string& key, long long value, bool trailingComma = true)
    {
        return "\"" + key + "\": " + std::to_string(value) + (trailingComma ? "," : "");
    }

    std::string JsonBool(const std::string& key, bool value, bool trailingComma = true)
    {
        return "\"" + key + "\": " + (value ? "true" : "false") + (trailingComma ? "," : "");
    }

    // A CSV field is quoted only when it has to be, so the files stay readable in a terminal.
    std::string CsvField(const std::string& text)
    {
        const bool needsQuotes = text.find_first_of(",\"\n") != std::string::npos;
        if (!needsQuotes) return text;

        std::string out = "\"";
        for (char c : text)
        {
            if (c == '"') out += '"';
            out += c;
        }
        out += '"';
        return out;
    }

    void AppendRow(std::string& out, const std::string& label, const std::string& value)
    {
        out += "| " + label + " | " + value + " |\n";
    }

    std::string Yes(bool value)
    {
        return value ? "yes" : "no";
    }
}

//=============================================================================
// frames.csv
//=============================================================================

std::string BuildFramesCsv(const RunData& run)
{
    std::string out = "segment,repeat,frame,frame_ms";
    for (const std::string& pass : run.passNames) out += ",cpu_" + pass;
    for (const std::string& pass : run.passNames) out += ",gpu_" + pass;
    out += "\n";

    const int passCount = (int)run.passNames.size();
    for (const SegmentSamples& segment : run.segments)
    {
        for (const RepeatSamples& repeat : segment.repeats)
        {
            for (size_t frameIndex = 0; frameIndex < repeat.frames.size(); frameIndex++)
            {
                const FrameSample& frame = repeat.frames[frameIndex];
                out += CsvField(segment.name) + "," + std::to_string(repeat.repeatIndex) + "," +
                       std::to_string(frameIndex) + "," + Fixed(frame.frameMs, kMsDecimals);
                for (int p = 0; p < passCount; p++) out += "," + Fixed(frame.cpuMs[p], kMsDecimals);
                for (int p = 0; p < passCount; p++) out += "," + Fixed(frame.gpuMs[p], kMsDecimals);
                out += "\n";
            }
        }
    }
    return out;
}

//=============================================================================
// passes.csv
//=============================================================================

namespace
{
    void AppendPassRow(std::string& out, const RunData& run, const SegmentSamples& segment,
                       const RepeatSamples& repeat, int passIndex)
    {
        const int frames = (int)repeat.frames.size();
        if (frames == 0) return;

        double cpuSum = 0.0, gpuSum = 0.0;
        for (const FrameSample& frame : repeat.frames)
        {
            cpuSum += frame.cpuMs[passIndex];
            gpuSum += frame.gpuMs[passIndex];
        }

        out += CsvField(segment.name) + "," + std::to_string(repeat.repeatIndex) + "," +
               CsvField(run.passNames[passIndex]) + "," +
               Fixed(cpuSum / frames, kMsDecimals) + "," + Fixed(gpuSum / frames, kMsDecimals);
        for (size_t c = 0; c < run.counterNames.size(); c++)
            out += "," + Fixed((double)repeat.counters.perPass[passIndex][c] / frames, kCounterDecimals);
        out += "\n";
    }

    // The per-repeat summary row. Its cpu_ms column carries the mean *frame* time rather than a
    // pass time -- there is no such thing as a pass total once passes nest, and a column that
    // silently double-counted Skinning would be worse than none.
    void AppendTotalRow(std::string& out, const RunData& run, const SegmentSamples& segment,
                        const RepeatSamples& repeat)
    {
        const int frames = (int)repeat.frames.size();
        if (frames == 0) return;

        double frameMsSum = 0.0;
        for (const FrameSample& frame : repeat.frames) frameMsSum += frame.frameMs;

        out += CsvField(segment.name) + "," + std::to_string(repeat.repeatIndex) + ",TOTAL," +
               Fixed(frameMsSum / frames, kMsDecimals) + ",0.00";
        for (size_t c = 0; c < run.counterNames.size(); c++)
            out += "," + Fixed((double)repeat.counters.total[c] / frames, kCounterDecimals);
        out += "\n";
    }
}

std::string BuildPassesCsv(const RunData& run)
{
    std::string out = "segment,repeat,pass,cpu_ms,gpu_ms";
    for (const std::string& counter : run.counterNames) out += "," + counter;
    out += "\n";

    for (const SegmentSamples& segment : run.segments)
    {
        for (const RepeatSamples& repeat : segment.repeats)
        {
            for (size_t p = 0; p < run.passNames.size(); p++)
                AppendPassRow(out, run, segment, repeat, (int)p);
            AppendTotalRow(out, run, segment, repeat);
        }
    }
    return out;
}

//=============================================================================
// report.md
//=============================================================================

namespace
{
    void AppendMarkdownBanners(std::string& out, const RunData& run)
    {
        if (run.status == RunStatus::Aborted)
            out += "> **Aborted run.** The measurements below are partial.\n\n";

        if (run.environment.gitDirty)
            out += "> **Uncommitted changes.** This run cannot be reproduced from the commit alone.\n\n";

        if (run.environment.vsyncEffective)
            out += "> **Vsync was on.** Frame times are pinned to the display refresh.\n\n";
    }

    void AppendIdentitySection(std::string& out, const RunData& run)
    {
        out += "## Identity\n\n| Field | Value |\n|---|---|\n";
        AppendRow(out, "Run id", run.runId);
        AppendRow(out, "Label", run.manifest.label.empty() ? "(none)" : run.manifest.label);
        AppendRow(out, "Started (UTC)", run.environment.timestampUtc);
        AppendRow(out, "Commit", run.environment.gitDescribe.empty() ? "(unknown)" : run.environment.gitDescribe);
        AppendRow(out, "Working tree", run.environment.gitDirty ? "dirty" : "clean");
        AppendRow(out, "Status", ToString(run.status));
        AppendRow(out, "Manifest hash", run.manifestHash);
        out += "\n";
    }

    void AppendBuildSection(std::string& out, const EnvironmentInfo& env)
    {
        out += "## Build\n\n| Field | Value |\n|---|---|\n";
        AppendRow(out, "Configuration", env.buildConfig);
        AppendRow(out, "Editor", env.buildEditor);
        AppendRow(out, "Compiler", env.buildCompiler);
        AppendRow(out, "Architecture", env.buildArch);
        AppendRow(out, "Built", env.buildTimestamp);
        out += "\n";
    }

    void AppendMachineSection(std::string& out, const EnvironmentInfo& env)
    {
        out += "## Machine\n\n| Field | Value |\n|---|---|\n";
        AppendRow(out, "OS", env.osVersion);
        AppendRow(out, "CPU", env.cpu);
        AppendRow(out, "Hardware threads", std::to_string(env.cpuThreads));
        AppendRow(out, "System memory", std::to_string(env.systemMemoryMB) + " MB");
        out += "\n";
    }

    void AppendGraphicsSection(std::string& out, const EnvironmentInfo& env)
    {
        out += "## Graphics\n\n| Field | Value |\n|---|---|\n";
        AppendRow(out, "GL vendor", env.glVendor);
        AppendRow(out, "GL renderer", env.glRenderer);
        AppendRow(out, "GL version", env.glVersion);
        AppendRow(out, "GLSL version", env.glslVersion);
        AppendRow(out, "Context obtained",
            std::to_string(env.contextMajor) + "." + std::to_string(env.contextMinor));
        AppendRow(out, "Caps: buffer storage", Yes(env.capsBufferStorage));
        AppendRow(out, "Caps: vertex attrib binding", Yes(env.capsVertexAttribBinding));
        AppendRow(out, "Caps: program binary", Yes(env.capsProgramBinary));
        AppendRow(out, "Caps: timer query", Yes(env.capsTimerQuery));
        AppendRow(out, "UBO offset alignment", std::to_string(env.capsUboOffsetAlignment));
        AppendRow(out, "Max uniform block size", std::to_string(env.capsMaxUniformBlockSize));
        AppendRow(out, "Resolution",
            std::to_string(env.windowWidth) + "x" + std::to_string(env.windowHeight));
        AppendRow(out, "Vsync requested / effective",
            Yes(env.vsyncRequested) + " / " + Yes(env.vsyncEffective));
        AppendRow(out, "Target FPS", Fixed(env.targetFps, kFpsDecimals));
        AppendRow(out, "Map", env.mapName.empty() ? "(unknown)" : env.mapName);
        out += "\n";
    }

    void AppendConfigSection(std::string& out, const RunManifest& manifest)
    {
        out += "## Run configuration\n\n| Field | Value |\n|---|---|\n";
        AppendRow(out, "Selection", manifest.pattern);
        AppendRow(out, "Repeats", std::to_string(manifest.repeats));
        AppendRow(out, "Warmup frames", std::to_string(manifest.warmupFrames));
        AppendRow(out, "Measured frames", std::to_string(manifest.measureFrames));

        std::string order;
        for (const std::string& name : manifest.segmentNames)
        {
            if (!order.empty()) order += " -> ";
            order += name;
        }
        AppendRow(out, "Segment order", order);
        out += "\n";
    }

    // The pass carrying the most CPU time, ignoring passes contained in another pass (which would
    // otherwise win by being counted twice) and the overlay's own measurement cost.
    std::string DominantPass(const RunData& run, const Stats::SegmentStats& segment)
    {
        std::string best = "-";
        float bestMs = 0.0f;
        for (size_t p = 0; p < run.passNames.size() && p < (size_t)kMaxPasses; p++)
        {
            const bool nested = (p < run.passIsNested.size()) && run.passIsNested[p];
            if (nested || run.passNames[p] == "Overlay") continue;
            if (segment.passCpuMs[p] <= bestMs) continue;

            bestMs = segment.passCpuMs[p];
            best = run.passNames[p];
        }
        return best + " " + Fixed(bestMs, kMsDecimals) + " ms";
    }

    void AppendSummarySection(std::string& out, const RunData& run,
                              const std::vector<Stats::SegmentStats>& stats)
    {
        out += "## Segment summary\n\n";
        out += "| Segment | Frames | Median ms | Mean FPS | 1% low FPS | p99 ms | Repeat spread | Dominant pass |\n";
        out += "|---|---:|---:|---:|---:|---:|---:|---|\n";

        for (const Stats::SegmentStats& segment : stats)
        {
            out += "| " + segment.name +
                   " | " + std::to_string(segment.measuredFrames) +
                   " | " + Fixed(segment.frame.medianMs, kMsDecimals) +
                   " | " + Fixed(segment.frame.meanFps, kFpsDecimals) +
                   " | " + Fixed(segment.frame.onePercentLowFps, kFpsDecimals) +
                   " | " + Fixed(segment.frame.p99Ms, kMsDecimals) +
                   " | " + Fixed(segment.repeatSpreadPercent, kPercentDecimals) + "% | " +
                   DominantPass(run, segment) + " |\n";
        }
        out += "\n";
    }

    void AppendPacingSection(std::string& out, const std::vector<Stats::SegmentStats>& stats)
    {
        out += "## Frame pacing\n\n";
        out += "A low mean frame time with a high mean delta is a stuttering frame stream, not a smooth one.\n\n";
        out += "| Segment | Mean delta ms | Slow frames | Longest slow run | Max ms |\n|---|---:|---:|---:|---:|\n";

        for (const Stats::SegmentStats& segment : stats)
        {
            out += "| " + segment.name +
                   " | " + Fixed(segment.frame.pacingMeanAbsDeltaMs, kMsDecimals) +
                   " | " + std::to_string(segment.frame.slowFrameCount) +
                   " | " + std::to_string(segment.frame.longestSlowFrameRun) +
                   " | " + Fixed(segment.frame.maxMs, kMsDecimals) + " |\n";
        }
        out += "\n";
    }
}

namespace
{
    int CounterIndex(const RunData& run, const char* name)
    {
        for (size_t i = 0; i < run.counterNames.size(); i++)
            if (run.counterNames[i] == name) return (int)i;
        return -1;
    }

    std::string CounterCell(const Stats::SegmentStats& segment, int passIndex, int counterIndex)
    {
        if (counterIndex < 0) return "-";
        return Fixed(segment.passCounterPerFrame[passIndex][counterIndex], kCounterDecimals);
    }

    void AppendSegmentPassTable(std::string& out, const RunData& run,
                                const Stats::SegmentStats& segment)
    {
        const int drawIndex = CounterIndex(run, "DrawCalls");
        const int glIndex = CounterIndex(run, "GLCalls");

        out += "### " + segment.name + "\n\n";
        out += "| Pass | CPU ms | GPU ms | Draws | GL calls |\n|---|---:|---:|---:|---:|\n";

        for (size_t p = 0; p < run.passNames.size() && p < (size_t)kMaxPasses; p++)
        {
            const bool silent = segment.passCpuMs[p] == 0.0f && segment.passGpuMs[p] == 0.0f;
            if (silent) continue;

            out += "| " + run.passNames[p] +
                   " | " + Fixed(segment.passCpuMs[p], kMsDecimals) +
                   " | " + Fixed(segment.passGpuMs[p], kMsDecimals) +
                   " | " + CounterCell(segment, (int)p, drawIndex) +
                   " | " + CounterCell(segment, (int)p, glIndex) + " |\n";
        }

        out += "| **frame** | " + Fixed(segment.frame.meanMs, kMsDecimals) + " | | | |\n";
        out += "| *unattributed* | " + Fixed(segment.unattributedMs, kMsDecimals) +
               " (" + Fixed(segment.unattributedPercent, kPercentDecimals) + "%) | | | |\n\n";
    }

    void AppendBreakdownSection(std::string& out, const RunData& run,
                                const std::vector<Stats::SegmentStats>& stats)
    {
        out += "## Pass breakdown\n\n";
        out += "Per measured frame, averaged over the valid repeats. Nested passes (Skinning, CharWait) ";
        out += "are contained in another pass and are excluded from the unattributed remainder.\n\n";

        for (const Stats::SegmentStats& segment : stats)
            AppendSegmentPassTable(out, run, segment);
    }

    void AppendFindingsSection(std::string& out, const std::vector<Findings::Finding>& findings)
    {
        out += "## Findings\n\n";
        if (findings.empty())
        {
            out += "No rule fired.\n\n";
            return;
        }

        out += "Each row is one named rule with a fixed threshold. A finding is an observation about ";
        out += "this run, not a claim about a code change.\n\n";
        out += "| Level | Rule | Segment | Observation |\n|---|---|---|---|\n";
        for (const Findings::Finding& finding : findings)
        {
            out += std::string("| ") + Findings::ToString(finding.level) +
                   " | " + finding.rule +
                   " | " + (finding.segment.empty() ? "(run)" : finding.segment) +
                   " | " + finding.text + " |\n";
        }
        out += "\n";
    }

    void AppendHealthSection(std::string& out, const RunData& run,
                             const std::vector<Stats::SegmentStats>& stats)
    {
        out += "## Health\n\n";
        out += "| Segment | Valid repeats | Discarded repeats | Measured frames |\n|---|---:|---:|---:|\n";
        for (const Stats::SegmentStats& segment : stats)
        {
            out += "| " + segment.name +
                   " | " + std::to_string(segment.validRepeats) +
                   " | " + std::to_string(segment.invalidRepeats) +
                   " | " + std::to_string(segment.measuredFrames) + " |\n";
        }
        out += "\n";

        if (run.events.empty()) return;

        out += "### Events\n\n";
        for (const std::string& event : run.events) out += "- " + event + "\n";
        out += "\n";
    }
}

std::string BuildReportMarkdown(const RunData& run,
                                const std::vector<Stats::SegmentStats>& stats,
                                const std::vector<Findings::Finding>& findings)
{
    std::string out = "# Benchmark run " + run.runId + "\n\n";

    AppendMarkdownBanners(out, run);
    AppendIdentitySection(out, run);
    AppendBuildSection(out, run.environment);
    AppendMachineSection(out, run.environment);
    AppendGraphicsSection(out, run.environment);
    AppendConfigSection(out, run.manifest);
    AppendSummarySection(out, run, stats);
    AppendPacingSection(out, stats);
    AppendFindingsSection(out, findings);
    AppendBreakdownSection(out, run, stats);
    AppendHealthSection(out, run, stats);

    return out;
}

//=============================================================================
// run.json -- the canonical artifact
//=============================================================================

namespace
{
    void AppendJsonEnvironment(std::string& out, const EnvironmentInfo& env)
    {
        out += "  \"environment\": {\n";
        out += "    " + JsonString("timestampUtc", env.timestampUtc) + "\n";
        out += "    " + JsonString("os", env.osVersion) + "\n";
        out += "    " + JsonString("cpu", env.cpu) + "\n";
        out += "    " + JsonInt("cpuThreads", env.cpuThreads) + "\n";
        out += "    " + JsonInt("systemMemoryMB", (long long)env.systemMemoryMB) + "\n";
        out += "    " + JsonString("glVendor", env.glVendor) + "\n";
        out += "    " + JsonString("glRenderer", env.glRenderer) + "\n";
        out += "    " + JsonString("glVersion", env.glVersion) + "\n";
        out += "    " + JsonString("glslVersion", env.glslVersion) + "\n";
        out += "    " + JsonInt("contextMajor", env.contextMajor) + "\n";
        out += "    " + JsonInt("contextMinor", env.contextMinor) + "\n";
        out += "    " + JsonBool("capsBufferStorage", env.capsBufferStorage) + "\n";
        out += "    " + JsonBool("capsVertexAttribBinding", env.capsVertexAttribBinding) + "\n";
        out += "    " + JsonBool("capsProgramBinary", env.capsProgramBinary) + "\n";
        out += "    " + JsonBool("capsTimerQuery", env.capsTimerQuery) + "\n";
        out += "    " + JsonInt("capsUboOffsetAlignment", env.capsUboOffsetAlignment) + "\n";
        out += "    " + JsonInt("capsMaxUniformBlockSize", env.capsMaxUniformBlockSize) + "\n";
        out += "    " + JsonString("buildConfig", env.buildConfig) + "\n";
        out += "    " + JsonString("buildEditor", env.buildEditor) + "\n";
        out += "    " + JsonString("buildCompiler", env.buildCompiler) + "\n";
        out += "    " + JsonString("buildArch", env.buildArch) + "\n";
        out += "    " + JsonString("buildTimestamp", env.buildTimestamp) + "\n";
        out += "    " + JsonString("gitDescribe", env.gitDescribe) + "\n";
        out += "    " + JsonBool("gitDirty", env.gitDirty) + "\n";
        out += "    " + JsonInt("windowWidth", env.windowWidth) + "\n";
        out += "    " + JsonInt("windowHeight", env.windowHeight) + "\n";
        out += "    " + JsonBool("vsyncRequested", env.vsyncRequested) + "\n";
        out += "    " + JsonBool("vsyncEffective", env.vsyncEffective) + "\n";
        out += "    " + JsonNumber("targetFps", env.targetFps, kFpsDecimals) + "\n";
        out += "    " + JsonString("map", env.mapName, false) + "\n";
        out += "  },\n";
    }

    std::string JsonStringArray(const std::vector<std::string>& values)
    {
        std::string out = "[";
        for (size_t i = 0; i < values.size(); i++)
        {
            if (i > 0) out += ", ";
            out += "\"" + JsonEscape(values[i]) + "\"";
        }
        return out + "]";
    }

    void AppendJsonManifest(std::string& out, const RunManifest& manifest)
    {
        out += "  \"manifest\": {\n";
        out += "    " + JsonString("label", manifest.label) + "\n";
        out += "    " + JsonString("pattern", manifest.pattern) + "\n";
        out += "    " + JsonInt("repeats", manifest.repeats) + "\n";
        out += "    " + JsonInt("warmupFrames", manifest.warmupFrames) + "\n";
        out += "    " + JsonInt("measureFrames", manifest.measureFrames) + "\n";
        out += "    \"segments\": " + JsonStringArray(manifest.segmentNames) + "\n";
        out += "  },\n";
    }

    std::string JsonNumberArray(const std::vector<float>& values, int decimals)
    {
        std::string out = "[";
        for (size_t i = 0; i < values.size(); i++)
        {
            if (i > 0) out += ", ";
            out += Fixed(values[i], decimals);
        }
        return out + "]";
    }

    void AppendJsonTiming(std::string& out, const Stats::TimingStats& timing, const char* indent)
    {
        out += indent + JsonInt("frameCount", timing.frameCount) + "\n";
        out += indent + JsonNumber("meanMs", timing.meanMs, kMsDecimals) + "\n";
        out += indent + JsonNumber("medianMs", timing.medianMs, kMsDecimals) + "\n";
        out += indent + JsonNumber("p95Ms", timing.p95Ms, kMsDecimals) + "\n";
        out += indent + JsonNumber("p99Ms", timing.p99Ms, kMsDecimals) + "\n";
        out += indent + JsonNumber("minMs", timing.minMs, kMsDecimals) + "\n";
        out += indent + JsonNumber("maxMs", timing.maxMs, kMsDecimals) + "\n";
        out += indent + JsonNumber("stdDevMs", timing.stdDevMs, kMsDecimals) + "\n";
        out += indent + JsonNumber("meanFps", timing.meanFps, kFpsDecimals) + "\n";
        out += indent + JsonNumber("onePercentLowMs", timing.onePercentLowMs, kMsDecimals) + "\n";
        out += indent + JsonNumber("onePercentLowFps", timing.onePercentLowFps, kFpsDecimals) + "\n";
        out += indent + JsonNumber("pacingMeanAbsDeltaMs", timing.pacingMeanAbsDeltaMs, kMsDecimals) + "\n";
        out += indent + JsonInt("slowFrameCount", timing.slowFrameCount) + "\n";
        out += indent + JsonInt("longestSlowFrameRun", timing.longestSlowFrameRun) + "\n";
        out += std::string(indent) + "\"percentileCurveMs\": " + JsonNumberArray(timing.percentileCurveMs, kMsDecimals) + "\n";
    }

    void AppendJsonPasses(std::string& out, const RunData& run, const Stats::SegmentStats& segment)
    {
        const size_t passCount = std::min(run.passNames.size(), (size_t)kMaxPasses);
        out += "      \"passes\": [\n";
        for (size_t p = 0; p < passCount; p++)
        {
            out += "        { " + JsonString("name", run.passNames[p]) + " " +
                   JsonNumber("cpuMs", segment.passCpuMs[p], kMsDecimals) + " " +
                   JsonNumber("gpuMs", segment.passGpuMs[p], kMsDecimals, false) + " }";
            out += (p + 1 < passCount) ? ",\n" : "\n";
        }
        out += "      ],\n";
    }

    void AppendJsonCounters(std::string& out, const RunData& run, const Stats::SegmentStats& segment)
    {
        const size_t counterCount = std::min(run.counterNames.size(), (size_t)kMaxCounters);
        out += "      \"countersPerFrame\": {\n";
        for (size_t c = 0; c < counterCount; c++)
        {
            const bool last = (c + 1 == counterCount);
            out += "        " + JsonNumber(run.counterNames[c], segment.counterPerFrame[c],
                                           kCounterDecimals, !last) + "\n";
        }
        out += "      }\n";
    }

    void AppendJsonSegment(std::string& out, const RunData& run,
                           const Stats::SegmentStats& segment, bool last)
    {
        out += "    {\n";
        out += "      " + JsonString("name", segment.name) + "\n";
        out += "      " + JsonInt("validRepeats", segment.validRepeats) + "\n";
        out += "      " + JsonInt("invalidRepeats", segment.invalidRepeats) + "\n";
        out += "      " + JsonNumber("repeatSpreadMs", segment.repeatSpreadMs, kMsDecimals) + "\n";
        out += "      " + JsonNumber("repeatSpreadPercent", segment.repeatSpreadPercent, kPercentDecimals) + "\n";
        out += "      \"repeatMedianMs\": " + JsonNumberArray(segment.repeatMedianMs, kMsDecimals) + ",\n";
        out += "      " + JsonNumber("attributedCpuMs", segment.attributedCpuMs, kMsDecimals) + "\n";
        out += "      " + JsonNumber("unattributedMs", segment.unattributedMs, kMsDecimals) + "\n";
        out += "      " + JsonNumber("unattributedPercent", segment.unattributedPercent, kPercentDecimals) + "\n";
        out += "      \"frame\": {\n";
        AppendJsonTiming(out, segment.frame, "        ");
        out += "      },\n";
        AppendJsonPasses(out, run, segment);
        AppendJsonCounters(out, run, segment);
        out += last ? "    }\n" : "    },\n";
    }

    void AppendJsonFindings(std::string& out, const std::vector<Findings::Finding>& findings)
    {
        out += "  \"findings\": [\n";
        for (size_t i = 0; i < findings.size(); i++)
        {
            out += "    { " + JsonString("level", Findings::ToString(findings[i].level)) + " " +
                   JsonString("rule", findings[i].rule) + " " +
                   JsonString("segment", findings[i].segment) + " " +
                   JsonString("text", findings[i].text, false) + " }";
            out += (i + 1 < findings.size()) ? ",\n" : "\n";
        }
        out += "  ]\n";
    }
}

std::string BuildRunJson(const RunData& run,
                         const std::vector<Stats::SegmentStats>& stats,
                         const std::vector<Findings::Finding>& findings)
{
    std::string out = "{\n";
    out += "  " + JsonInt("schemaVersion", kSchemaVersion) + "\n";
    out += "  " + JsonString("runId", run.runId) + "\n";
    out += "  " + JsonString("manifestHash", run.manifestHash) + "\n";
    out += "  " + JsonString("status", ToString(run.status)) + "\n";
    AppendJsonManifest(out, run.manifest);
    AppendJsonEnvironment(out, run.environment);
    out += "  \"passNames\": " + JsonStringArray(run.passNames) + ",\n";
    out += "  \"counterNames\": " + JsonStringArray(run.counterNames) + ",\n";
    out += "  \"events\": " + JsonStringArray(run.events) + ",\n";

    out += "  \"segments\": [\n";
    for (size_t i = 0; i < stats.size(); i++)
        AppendJsonSegment(out, run, stats[i], i + 1 == stats.size());
    out += "  ],\n";

    AppendJsonFindings(out, findings);
    out += "}\n";
    return out;
}
}
