#include "stdafx.h"

#include "BenchRecorder.h"

#include "BenchEnvironment.h"
#include "BenchFindings.h"
#include "BenchReport.h"
#include "BenchScene.h"
#include "BenchStats.h"
#include "Core/Utilities/FrameProfiler.h"
#include "Core/Utilities/Log/ErrorReport.h"

#include <ctime>
#include <filesystem>
#include <fstream>

namespace Core::Benchmark
{
namespace
{
    // Runs are written under the client's working directory, one directory per run.
    constexpr const char* kRunsRootPath = "bench/runs";

    // Pass indices whose time is already contained in another pass. Summing them into a frame
    // total would count the same milliseconds twice.
    bool IsNestedPass(FrameProfiler::Pass pass)
    {
        return pass == FrameProfiler::Pass::Skinning || pass == FrameProfiler::Pass::CharWait;
    }

    std::string SanitizeForPath(const std::string& text)
    {
        std::string out;
        for (char c : text)
        {
            const bool safe = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                              (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_';
            if (safe) out += c;
            else if (c == ' ') out += '-';
        }
        return out;
    }

    std::string LocalTimestampForId()
    {
        const std::time_t now = std::time(nullptr);
        std::tm local = {};
#if defined(_WIN32)
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        char buffer[24];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &local);
        return buffer;
    }

    bool WriteTextFile(const std::filesystem::path& path, const std::string& contents)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) return false;

        file.write(contents.data(), (std::streamsize)contents.size());
        return file.good();
    }
}

static_assert((int)FrameProfiler::Pass::Count_ <= kMaxPasses,
              "FrameProfiler grew a pass -- raise Core::Benchmark::kMaxPasses to match");
static_assert((int)FrameProfiler::Counter::Count_ <= kMaxCounters,
              "FrameProfiler grew a counter -- raise Core::Benchmark::kMaxCounters to match");

Recorder& Recorder::Instance()
{
    static Recorder s_instance;
    return s_instance;
}

//=============================================================================
// Run planning
//=============================================================================

bool Recorder::BuildPlan(const std::string& pattern, std::string& outMessage)
{
    const std::vector<const Segment*> selected = Segments::Select(pattern);
    if (selected.empty())
    {
        outMessage = "no segment matches '" + pattern + "'";
        return false;
    }

    const Segment* baseline = Segments::Find(kBaselineSegmentName);
    if (!baseline)
    {
        outMessage = "the baseline segment is missing from the catalog";
        return false;
    }

    // The baseline always opens the run: without it a subset run has nothing to read its
    // segments against, and its numbers would only be comparable to another full run.
    m_plan.clear();
    m_plan.push_back(PlannedSegment{ baseline, kBaselineSegmentName });

    for (const Segment* segment : selected)
    {
        if (segment == baseline) continue;
        m_plan.push_back(PlannedSegment{ segment, segment->name });
    }

    // ... and closes it, so drift across the run is measured rather than assumed away. Pointless
    // when the baseline is the only thing being measured.
    if (m_plan.size() > 1)
        m_plan.push_back(PlannedSegment{ baseline, std::string(kBaselineSegmentName) + kDriftControlSuffix });

    return true;
}

bool Recorder::Start(const std::string& pattern, int repeats, int warmupFrames, int measureFrames,
                     std::string& outMessage)
{
    if (m_running)
    {
        outMessage = "a benchmark run is already in progress";
        return false;
    }

    if (repeats < 1 || warmupFrames < 0 || measureFrames < 1)
    {
        outMessage = "repeats and measured frames must be at least 1";
        return false;
    }

    if (!BuildPlan(pattern, outMessage)) return false;

    m_manifest = RunManifest{};
    m_manifest.label = m_label;
    m_manifest.pattern = pattern;
    m_manifest.repeats = repeats;
    m_manifest.warmupFrames = warmupFrames;
    m_manifest.measureFrames = measureFrames;
    for (const PlannedSegment& planned : m_plan) m_manifest.segmentNames.push_back(planned.resultName);

    m_environment = Environment::Capture();
    m_segments.clear();
    m_events.clear();
    m_segmentIndex = 0;
    m_running = true;

    const std::string suffix = SanitizeForPath(m_label);
    m_runId = LocalTimestampForId() + (suffix.empty() ? "" : "_" + suffix);

    // The counters have to be on for the whole run whatever the overlay was doing, and back to
    // what they were afterwards -- a run must not leave the client instrumented behind the
    // user's back.
    m_countersWereEnabled = FrameProfiler::g_CountersEnabled;
    FrameProfiler::g_CountersEnabled = true;
    m_configBeforeRun = Scene::CaptureCurrentConfig();
    m_lastFrameAt = std::chrono::steady_clock::now();

    BeginSegment();
    outMessage = "benchmark started: " + std::to_string(m_plan.size()) + " segments x " +
                 std::to_string(repeats) + " repeats";
    return true;
}

void Recorder::Abort()
{
    if (!m_running) return;

    m_currentRepeat.invalid = InvalidReason::Aborted;
    m_events.push_back("run aborted by the user during " + m_plan[m_segmentIndex].resultName);
    EndRepeat();
    Finish(RunStatus::Aborted);
}

//=============================================================================
// Segment / repeat lifecycle
//=============================================================================

void Recorder::BeginSegment()
{
    const PlannedSegment& planned = m_plan[m_segmentIndex];

    SegmentSamples segment;
    segment.name = planned.resultName;
    segment.description = planned.definition->description;
    segment.tags = Segments::TagsToString(planned.definition->tags);
    m_segments.push_back(segment);

    m_repeatIndex = 0;
    BeginRepeat();
}

void Recorder::BeginRepeat()
{
    // Re-applied per repeat, not per segment: it is the guard against a stray console command or
    // another system having changed a toggle underneath the run.
    Scene::ApplyConfig(m_plan[m_segmentIndex].definition->config);

    m_currentRepeat = RepeatSamples{};
    m_currentRepeat.repeatIndex = m_repeatIndex;
    m_currentRepeat.frames.reserve((size_t)m_manifest.measureFrames);
    m_warmupRemaining = m_manifest.warmupFrames;
    m_skipNextFrame = true;
}

void Recorder::EndRepeat()
{
    m_currentRepeat.warmupFramesDiscarded = m_manifest.warmupFrames - m_warmupRemaining;

    const bool tooFewFrames = (int)m_currentRepeat.frames.size() < m_manifest.measureFrames;
    if (tooFewFrames && m_currentRepeat.invalid == InvalidReason::None)
        m_currentRepeat.invalid = InvalidReason::TooFewFrames;

    m_segments.back().repeats.push_back(m_currentRepeat);
    m_currentRepeat = RepeatSamples{};
}

void Recorder::EndSegment()
{
    m_segmentIndex++;
    if (m_segmentIndex >= (int)m_plan.size())
    {
        Finish(RunStatus::Completed);
        return;
    }

    BeginSegment();
}

//=============================================================================
// Per-frame recording
//=============================================================================

float Recorder::ElapsedFrameMs()
{
    const auto now = std::chrono::steady_clock::now();
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_lastFrameAt).count();
    m_lastFrameAt = now;
    return (float)ns / 1.0e6f;
}

// GPU ms comes out of FrameProfiler's query ring, which reports work submitted two frames ago --
// the same reading the $glstats overlay shows. Over a 300-frame repeat the two-frame offset is
// immaterial; it is worth knowing about when reading a single row of frames.csv.
void Recorder::RecordFrame(float frameMs)
{
    FrameSample sample;
    sample.frameMs = frameMs;
    for (int p = 0; p < (int)FrameProfiler::Pass::Count_; p++)
    {
        const FrameProfiler::Pass pass = (FrameProfiler::Pass)p;
        sample.cpuMs[p] = FrameProfiler::AccumulatorMs(pass);
        sample.gpuMs[p] = FrameProfiler::GpuMs(pass);

        for (int c = 0; c < (int)FrameProfiler::Counter::Count_; c++)
            m_currentRepeat.counters.perPass[p][c] += FrameProfiler::CounterValue(pass, (FrameProfiler::Counter)c);
    }

    for (int c = 0; c < (int)FrameProfiler::Counter::Count_; c++)
        m_currentRepeat.counters.total[c] += FrameProfiler::CounterValue((FrameProfiler::Counter)c);

    m_currentRepeat.frames.push_back(sample);
}

void Recorder::Tick()
{
    if (!m_running) return;

    const float frameMs = ElapsedFrameMs();

    // The frame a configuration change lands in is not a measurement of either configuration.
    if (m_skipNextFrame)
    {
        m_skipNextFrame = false;
        return;
    }

    if (frameMs > kStallFrameMs && m_currentRepeat.invalid == InvalidReason::None)
    {
        m_currentRepeat.invalid = InvalidReason::Stalled;
        m_events.push_back(m_plan[m_segmentIndex].resultName + " repeat " +
                           std::to_string(m_repeatIndex) + ": a " + std::to_string((int)frameMs) +
                           " ms frame -- the client was not running normally");
    }

    if (m_warmupRemaining > 0)
    {
        m_warmupRemaining--;
        return;
    }

    RecordFrame(frameMs);
    if ((int)m_currentRepeat.frames.size() < m_manifest.measureFrames) return;

    EndRepeat();
    m_repeatIndex++;
    if (m_repeatIndex < m_manifest.repeats)
    {
        BeginRepeat();
        return;
    }

    EndSegment();
}

//=============================================================================
// Finishing and export
//=============================================================================

RunData Recorder::BuildRunData(RunStatus status) const
{
    RunData run;
    run.runId = m_runId;
    run.manifest = m_manifest;
    run.manifestHash = ComputeManifestHash(m_manifest);
    run.environment = m_environment;
    run.segments = m_segments;
    run.events = m_events;
    run.status = status;

    for (int p = 0; p < (int)FrameProfiler::Pass::Count_; p++)
    {
        run.passNames.push_back(FrameProfiler::kPassNames[p]);
        run.passIsNested.push_back(IsNestedPass((FrameProfiler::Pass)p));
    }
    for (int c = 0; c < (int)FrameProfiler::Counter::Count_; c++)
        run.counterNames.push_back(FrameProfiler::kCounterNames[c]);

    return run;
}

void Recorder::WriteExports(const RunData& run)
{
    const std::vector<Stats::SegmentStats> stats = Stats::SummarizeRun(run);
    const std::vector<Findings::Finding> findings = Findings::Evaluate(run, stats);

    const std::filesystem::path directory = std::filesystem::path(kRunsRootPath) / run.runId;
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error)
    {
        g_ErrorReport.Write(L"[Benchmark] could not create the run directory: %hs\r\n",
                            error.message().c_str());
        return;
    }

    const bool written =
        WriteTextFile(directory / Report::kRunJsonFileName, Report::BuildRunJson(run, stats, findings)) &&
        WriteTextFile(directory / Report::kReportFileName, Report::BuildReportMarkdown(run, stats, findings)) &&
        WriteTextFile(directory / Report::kFramesCsvFileName, Report::BuildFramesCsv(run)) &&
        WriteTextFile(directory / Report::kPassesCsvFileName, Report::BuildPassesCsv(run));

    if (!written)
    {
        g_ErrorReport.Write(L"[Benchmark] could not write every export for run %hs\r\n", run.runId.c_str());
        return;
    }

    m_lastOutputPath = directory.string();
}

void Recorder::Finish(RunStatus status)
{
    m_running = false;
    Scene::ApplyConfig(m_configBeforeRun);
    FrameProfiler::g_CountersEnabled = m_countersWereEnabled;

    WriteExports(BuildRunData(status));

    m_plan.clear();
    m_segments.clear();
}

std::string Recorder::StatusText() const
{
    if (!m_running) return std::string();

    const std::string phase = (m_warmupRemaining > 0)
        ? "warmup " + std::to_string(m_warmupRemaining)
        : std::to_string(m_currentRepeat.frames.size()) + "/" + std::to_string(m_manifest.measureFrames);

    return "BENCH " + m_plan[m_segmentIndex].resultName +
           " [" + std::to_string(m_segmentIndex + 1) + "/" + std::to_string(m_plan.size()) + "]" +
           " repeat " + std::to_string(m_repeatIndex + 1) + "/" + std::to_string(m_manifest.repeats) +
           " " + phase;
}
}
