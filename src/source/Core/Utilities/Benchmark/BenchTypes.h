#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Data model for a benchmark run, shared by the recorder (which fills it in) and the
// stats/findings/report layers (which read it).
//
// Deliberately free of any project dependency -- no FrameProfiler.h, no GL, no SDL. The
// recorder copies FrameProfiler's per-frame values into these plain structs, which keeps
// everything downstream of the copy linkable from a unit test. See docs/benchmark.md.

namespace Core::Benchmark
{
    // Upper bound on the pass/counter dimensions copied out of FrameProfiler. Static-asserted
    // against the real enum sizes in BenchRecorder.cpp, which is the only file that knows both.
    inline constexpr int kMaxPasses = 32;
    inline constexpr int kMaxCounters = 32;

    // Name of the segment every run starts and ends with, so drift across the run is measurable
    // rather than assumed away -- a GPU is not the same GPU at minute 8 as at minute 0. Defined
    // here rather than in the catalog because the exports and the findings both key off it.
    inline constexpr const char* kBaselineSegmentName = "scene.full";

    // Suffix on the result name of the closing re-measurement of the baseline, so it is a
    // distinct row in the exports while pointing at the same segment definition.
    inline constexpr const char* kDriftControlSuffix = "@end";

    // Number of points the frame-time distribution is decimated to. Enough to plot a percentile
    // curve (where stutter shows as a tail hockey-stick) without storing every frame twice.
    inline constexpr int kPercentileCurvePoints = 101;

    // One measured frame. Warmup frames are counted but never stored.
    struct FrameSample
    {
        float frameMs = 0.0f;
        float cpuMs[kMaxPasses] = {};
        float gpuMs[kMaxPasses] = {};
    };

    // GL counters summed over a repeat's measured frames. Counters are near-deterministic for a
    // fixed workload, so per-segment sums carry the same information as per-frame values at a
    // fraction of the width -- this is why frames.csv has no counter columns.
    struct CounterTotals
    {
        uint64_t perPass[kMaxPasses][kMaxCounters] = {};
        uint64_t total[kMaxCounters] = {};
    };

    // Why a repeat's numbers must not be trusted. Anything other than None makes the repeat
    // invalid: it is still exported (so the reason is visible) but excluded from aggregation.
    enum class InvalidReason
    {
        None,
        Stalled,        // a frame far longer than the run's own scale: alt-tab, minimise, or a
                        // load hitch. The OS was not running the client normally, so the repeat
                        // measures the interruption rather than the scene.
        Aborted,        // the run was stopped before this repeat finished
        TooFewFrames,   // ended with fewer measured frames than the manifest asked for
    };

    const char* ToString(InvalidReason reason);

    struct RepeatSamples
    {
        int repeatIndex = 0;
        int warmupFramesDiscarded = 0;
        std::vector<FrameSample> frames;
        CounterTotals counters;
        InvalidReason invalid = InvalidReason::None;
    };

    struct SegmentSamples
    {
        std::string name;
        std::string description;
        std::string tags;           // space-separated, as exported
        std::vector<RepeatSamples> repeats;
    };

    // What the run was asked to do. Hashed into RunData::manifestHash so two runs can be checked
    // for comparability before their numbers are ever put side by side.
    struct RunManifest
    {
        std::string label;                  // free-text, from `$bench label`
        std::vector<std::string> segmentNames;  // in execution order
        int repeats = 0;
        int warmupFrames = 0;
        int measureFrames = 0;
        std::string pattern;                // the selection the user typed
    };

    // Everything about the machine and build that makes numbers comparable (or not). Captured
    // once at run start. Fields that could not be resolved stay empty rather than guessing.
    struct EnvironmentInfo
    {
        std::string timestampUtc;
        std::string osVersion;
        std::string cpu;
        int cpuThreads = 0;
        uint64_t systemMemoryMB = 0;

        std::string glVendor;
        std::string glRenderer;
        std::string glVersion;
        std::string glslVersion;
        int contextMajor = 0;       // the version the GLP-08 descending chain actually got
        int contextMinor = 0;
        bool capsBufferStorage = false;
        bool capsVertexAttribBinding = false;
        bool capsProgramBinary = false;
        bool capsTimerQuery = false;
        int capsUboOffsetAlignment = 0;
        int capsMaxUniformBlockSize = 0;

        std::string buildConfig;    // Debug / Release
        std::string buildEditor;    // Editor / NoEditor
        std::string buildCompiler;
        std::string buildArch;
        std::string buildTimestamp;
        std::string gitDescribe;    // empty when not baked in
        bool gitDirty = false;

        int windowWidth = 0;
        int windowHeight = 0;
        bool vsyncRequested = false;
        bool vsyncEffective = false;
        double targetFps = 0.0;
        std::string mapName;
    };

    enum class RunStatus
    {
        Completed,
        Aborted,
    };

    const char* ToString(RunStatus status);

    // Short stable fingerprint of everything that decides whether two runs measured the same
    // thing. Two runs with different hashes are not comparable, however similar their tables
    // look -- segment order alone changes the numbers, because a GPU is warmer at segment 9
    // than at segment 1.
    std::string ComputeManifestHash(const RunManifest& manifest);

    // A run's whole result: what was asked for, what machine ran it, and every sample taken.
    struct RunData
    {
        std::string runId;              // directory name, e.g. 20260818_142233_glp09
        std::string manifestHash;
        RunManifest manifest;
        EnvironmentInfo environment;
        std::vector<SegmentSamples> segments;
        std::vector<std::string> passNames;
        // True for a pass whose time is already contained in another pass (Skinning inside
        // Objects/Characters/Items, CharWait inside Characters). Summing those into a frame total
        // would double-count them, so the attribution check in BenchStats skips them. Filled by
        // the recorder, which is the only place that knows FrameProfiler's nesting semantics.
        std::vector<bool> passIsNested;
        std::vector<std::string> counterNames;
        std::vector<std::string> events;    // warnings raised during the run, in order
        RunStatus status = RunStatus::Completed;
    };
}
