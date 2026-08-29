#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "BenchSegment.h"
#include "BenchTypes.h"

// Drives a benchmark run and writes its exports. One instance, owned by the render thread.
//
// A run is a list of segments, each measured `repeats` times. Every repeat discards a warmup
// window before it starts recording -- the first frames of a segment pay for shader compiles,
// texture uploads and streaming-ring growth, which is real hitching worth knowing about but is
// not the steady state a comparison is about.
//
// The baseline segment is measured first and again last, whatever the selection was, so drift
// across the run is a measured number rather than an assumption. See docs/benchmark.md.

namespace Core::Benchmark
{
    class Recorder
    {
    public:
        // Defaults chosen so a full run is a couple of minutes and a single-segment run is
        // seconds. A repeat is a fixed frame count, never a fixed duration: a faster build must
        // render the same frames, not more of them.
        static constexpr int kDefaultRepeats = 3;
        static constexpr int kDefaultWarmupFrames = 60;
        static constexpr int kDefaultMeasureFrames = 300;

        // A frame this long did not measure the scene. Alt-tab, minimise and load hitches all
        // land well above it, and no rendered frame legitimately does.
        static constexpr float kStallFrameMs = 500.0f;

        static Recorder& Instance();

        // Starts a run over the segments matching `pattern`. Returns false and fills `outMessage`
        // when it cannot start (already running, or nothing matched).
        bool Start(const std::string& pattern, int repeats, int warmupFrames, int measureFrames,
                   std::string& outMessage);

        // Ends the run early. Whatever was measured is still exported, marked aborted.
        void Abort();

        // Called once per frame, after the frame's passes have run and before FrameProfiler is
        // reset. Cheap and branch-predictable when no run is active.
        void Tick();

        bool IsRunning() const { return m_running; }

        // One line for the overlay while a run is in progress.
        std::string StatusText() const;

        void SetLabel(const std::string& label) { m_label = label; }

        // Directory the last finished run was written to, empty until one finishes.
        const std::string& LastOutputPath() const { return m_lastOutputPath; }

    private:
        struct PlannedSegment
        {
            const Segment* definition = nullptr;
            std::string resultName;
        };

        bool BuildPlan(const std::string& pattern, std::string& outMessage);
        void BeginSegment();
        void BeginRepeat();
        void RecordFrame(float frameMs);
        void EndRepeat();
        void EndSegment();
        void Finish(RunStatus status);
        void WriteExports(const RunData& run);
        RunData BuildRunData(RunStatus status) const;
        float ElapsedFrameMs();

        bool m_running = false;
        bool m_countersWereEnabled = false;
        SceneConfig m_configBeforeRun;

        std::vector<PlannedSegment> m_plan;
        int m_segmentIndex = 0;
        int m_repeatIndex = 0;
        int m_warmupRemaining = 0;
        bool m_skipNextFrame = false;   // the frame that straddles a configuration change

        RunManifest m_manifest;
        EnvironmentInfo m_environment;
        std::vector<SegmentSamples> m_segments;
        std::vector<std::string> m_events;
        RepeatSamples m_currentRepeat;

        std::string m_label;
        std::string m_runId;
        std::string m_lastOutputPath;
        std::chrono::steady_clock::time_point m_lastFrameAt;
    };
}
