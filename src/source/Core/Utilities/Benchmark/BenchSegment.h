#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "BenchTypes.h"

// The catalog of benchmark segments and the selection logic over it. Pure data: applying a
// segment to the live scene is BenchScene's job, running one is BenchRecorder's.
//
// A segment is one named, self-contained scenario measured on its own, so a run says *where* a
// change had an effect instead of only whether the frame got faster. This first catalog varies
// the effect surfaces and render settings of whatever scene the client is currently showing -- it
// needs no scripted content, and the differences between the rows are the per-surface cost at
// that spot. Scripted scenes (spawned characters, camera paths, weather) are a separate addition.

namespace Core::Benchmark
{
// Tags exist for selection and for reading the report; a segment can carry several.
enum SegmentTag : uint32_t
{
    TagBaseline = 1u << 0,
    TagEffects = 1u << 1,
    TagSprites = 1u << 2,
    TagParticles = 1u << 3,
    TagJoints = 1u << 4,
    TagModels = 1u << 5,
    TagWildlife = 1u << 6,
    TagWings = 1u << 7,
    TagShadows = 1u << 8,
    TagLook = 1u << 9,
    TagTextures = 1u << 10,
    TagAntialiasing = 1u << 11,
    TagOrder = 1u << 12,
};

// What the segment switches off while it is measured: the `$effects ...` console toggles
// (MainScene.h) one-for-one, the Enhanced look's features and the anti-aliasing and
// anisotropy levels. Deliberately data, so the catalog stays linkable from a unit test; none of
// it is saved.
struct SceneConfig
{
    bool disableEffects = false;
    // Mirrors the options window's "render all effects" switch, which `$effects off` turns
    // off alongside SetDisableEffects. It is a separate flag because it is a separate gate:
    // RenderSprites() and RenderParticles() early-return on this option and never look at
    // SetDisableEffects at all, so a segment that sets only the latter leaves the two
    // heaviest effect paths running.
    bool disableAllEffectsOption = false;
    bool disableSprites = false;
    bool disableParticles = false;
    bool disableSkillEffectModels = false;
    bool disableJoints = false;
    bool disableBoids = false;
    bool disableWingShadow = false;
    bool disableWingExtraLayers = false;
    bool disableBodyShadows = false;

    // The Enhanced look's features, switched off as session overrides. They change nothing
    // while the look is Classic, which the segment-inert finding then says.
    bool lookTextureFilteringOff = false;
    bool lookLightingOff = false;
    bool lookDynamicLightsOff = false;
    bool lookLightMoodOff = false;
    bool lookPostProcessingOff = false;

    // -1 keeps what the run found.
    int antialiasingSamples = -1;
    int anisotropy = -1;

    // Opaque map object draws in the order they were recorded, not sorted by state.
    bool disableDrawReordering = false;

    bool operator==(const SceneConfig&) const = default;
};

struct Segment
{
    const char* name;
    const char* description;
    uint32_t tags;
    SceneConfig config;
};
} // namespace Core::Benchmark

namespace Core::Benchmark::Segments
{
const std::vector<Segment>& All();

const Segment* Find(const std::string& name);

// Glob match supporting '*' (any run of characters, including none). Case-sensitive; segment
// names are lowercase by convention.
bool MatchesPattern(const std::string& name, const std::string& pattern);

// Segments whose name matches the glob, or whose tag list contains it when the pattern is
// written as "#tag". Returned in catalog order, which is the order they are measured in.
std::vector<const Segment*> Select(const std::string& pattern);

// Space-separated lowercase tag names, for the exports.
std::string TagsToString(uint32_t tags);
} // namespace Core::Benchmark::Segments
