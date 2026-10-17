#include "BenchSegment.h"

namespace Core::Benchmark::Segments
{
namespace
{
// Written out longhand rather than built from a loop: the catalog is the specification of
// what a run measures, and it should be readable as a list.
const std::vector<Segment>& Catalog()
{
    static const std::vector<Segment> s_segments = {
        {kBaselineSegmentName,
         "Everything on. The reference every other segment is read against, and the drift control.", TagBaseline,
         SceneConfig{}},

        {"fx.all.off",
         "Every effect surface off, exactly as `$effects off` does it. Upper bound on what "
         "effects cost here.",
         TagEffects | TagSprites | TagParticles | TagJoints | TagModels,
         []
         {
             SceneConfig c;
             c.disableEffects = true;
             c.disableAllEffectsOption = true;
             // Joints are gated by neither of the two switches above (ZzzEffectJoint.cpp
             // checks only its own flag), so "all off" has to say so explicitly or the
             // segment quietly leaves the beam/tail path running.
             c.disableJoints = true;
             return c;
         }()},

        {"fx.sprites.off", "RenderSprites() off. IR per-quad path.", TagEffects | TagSprites,
         []
         {
             SceneConfig c;
             c.disableSprites = true;
             return c;
         }()},

        {"fx.particles.off", "RenderParticles() off. The heaviest IR per-quad path in effect-dense frames.",
         TagEffects | TagParticles,
         []
         {
             SceneConfig c;
             c.disableParticles = true;
             return c;
         }()},

        {"fx.joints.off", "RenderJoints() off. Beam and tail-trail effects.", TagEffects | TagJoints,
         []
         {
             SceneConfig c;
             c.disableJoints = true;
             return c;
         }()},

        {"fx.skillmodels.off", "Skill effect models off, legacy impact effects still rendering.",
         TagEffects | TagModels,
         []
         {
             SceneConfig c;
             c.disableSkillEffectModels = true;
             return c;
         }()},

        {"fx.boids.off", "Ambient wildlife off. Always-on cost in town maps, independent of combat.", TagWildlife,
         []
         {
             SceneConfig c;
             c.disableBoids = true;
             return c;
         }()},

        {"fx.wingshadow.off", "The extra per-wing body shadow draw off.", TagWings,
         []
         {
             SceneConfig c;
             c.disableWingShadow = true;
             return c;
         }()},

        {"fx.winglayers.off", "Wing glow overlay passes off. Visibly changes the wing -- a measurement, not a fix.",
         TagWings,
         []
         {
             SceneConfig c;
             c.disableWingExtraLayers = true;
             return c;
         }()},

        {"fx.bodyshadows.off", "Every body shadow off: characters, their wings and pets, objects and wildlife.",
         TagShadows,
         []
         {
             SceneConfig c;
             c.disableBodyShadows = true;
             return c;
         }()},

        {"look.texfilter.off", "Enhanced texture filtering off: no mipmaps or anisotropy on world textures.",
         TagLook | TagTextures,
         []
         {
             SceneConfig c;
             c.lookTextureFilteringOff = true;
             return c;
         }()},

        {"look.lighting.off", "Enhanced per-pixel model lighting off, and the dynamic lights that need it.", TagLook,
         []
         {
             SceneConfig c;
             c.lookLightingOff = true;
             return c;
         }()},

        {"look.dynlights.off", "Dynamic model lights off: candles, fires and spells no longer light models.", TagLook,
         []
         {
             SceneConfig c;
             c.lookDynamicLightsOff = true;
             return c;
         }()},

        {"look.mood.off", "The light mood off: the map's baked light as Classic has it.", TagLook,
         []
         {
             SceneConfig c;
             c.lookLightMoodOff = true;
             return c;
         }()},

        {"look.post.off", "Post-processing off: no bloom, no colour grading.", TagLook,
         []
         {
             SceneConfig c;
             c.lookPostProcessingOff = true;
             return c;
         }()},

        {"aa.off", "Multisample anti-aliasing off. Changes nothing while the look is Classic.", TagAntialiasing,
         []
         {
             SceneConfig c;
             c.antialiasingSamples = 0;
             return c;
         }()},

        {"aniso.off", "Anisotropic filtering off (1x); mipmaps stay.", TagTextures,
         []
         {
             SceneConfig c;
             c.anisotropy = 1;
             return c;
         }()},

        {"render.sort.off",
         "Opaque map object draws submitted as recorded, not first and grouped by pipeline and texture.", TagOrder,
         []
         {
             SceneConfig c;
             c.disableDrawReordering = true;
             return c;
         }()},
    };
    return s_segments;
}

struct TagName
{
    SegmentTag tag;
    const char* name;
};

const TagName kTagNames[] = {
    {TagBaseline, "baseline"}, {TagEffects, "effects"}, {TagSprites, "sprites"},   {TagParticles, "particles"},
    {TagJoints, "joints"},     {TagModels, "models"},   {TagWildlife, "wildlife"}, {TagWings, "wings"},
    {TagShadows, "shadows"},   {TagLook, "look"},       {TagTextures, "textures"}, {TagAntialiasing, "antialiasing"},
    {TagOrder, "order"},
};

bool HasTagNamed(uint32_t tags, const std::string& wanted)
{
    for (const TagName& entry : kTagNames)
        if (wanted == entry.name)
            return (tags & entry.tag) != 0;
    return false;
}
} // namespace

const std::vector<Segment>& All()
{
    return Catalog();
}

const Segment* Find(const std::string& name)
{
    for (const Segment& segment : Catalog())
        if (name == segment.name)
            return &segment;
    return nullptr;
}

bool MatchesPattern(const std::string& name, const std::string& pattern)
{
    // Iterative glob with backtracking on the last '*' -- no recursion, no allocation.
    size_t nameIndex = 0, patternIndex = 0;
    size_t starIndex = std::string::npos, nameAtStar = 0;

    while (nameIndex < name.size())
    {
        const bool literalMatch =
            patternIndex < pattern.size() && (pattern[patternIndex] == '?' || pattern[patternIndex] == name[nameIndex]);
        if (literalMatch)
        {
            nameIndex++;
            patternIndex++;
            continue;
        }

        if (patternIndex < pattern.size() && pattern[patternIndex] == '*')
        {
            starIndex = patternIndex++;
            nameAtStar = nameIndex;
            continue;
        }

        if (starIndex == std::string::npos)
            return false;

        // Backtrack: let the last '*' swallow one more character.
        patternIndex = starIndex + 1;
        nameIndex = ++nameAtStar;
    }

    while (patternIndex < pattern.size() && pattern[patternIndex] == '*')
        patternIndex++;
    return patternIndex == pattern.size();
}

std::vector<const Segment*> Select(const std::string& pattern)
{
    std::vector<const Segment*> selected;
    const bool byTag = !pattern.empty() && pattern[0] == '#';
    const std::string tagName = byTag ? pattern.substr(1) : std::string();

    for (const Segment& segment : Catalog())
    {
        const bool matches = byTag ? HasTagNamed(segment.tags, tagName) : MatchesPattern(segment.name, pattern);
        if (matches)
            selected.push_back(&segment);
    }
    return selected;
}

std::string TagsToString(uint32_t tags)
{
    std::string out;
    for (const TagName& entry : kTagNames)
    {
        if ((tags & entry.tag) == 0)
            continue;
        if (!out.empty())
            out += ' ';
        out += entry.name;
    }
    return out;
}
} // namespace Core::Benchmark::Segments
