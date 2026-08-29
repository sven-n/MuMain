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
            { kBaselineSegmentName,
              "Everything on. The reference every other segment is read against, and the drift control.",
              TagBaseline, SceneConfig{} },

            { "fx.all.off",
              "Every effect surface off, exactly as `$effects off` does it. Upper bound on what "
              "effects cost here.",
              TagEffects | TagSprites | TagParticles | TagJoints | TagModels,
              [] {
                  SceneConfig c;
                  c.disableEffects = true;
                  c.disableAllEffectsOption = true;
                  // Joints are gated by neither of the two switches above (ZzzEffectJoint.cpp
                  // checks only its own flag), so "all off" has to say so explicitly or the
                  // segment quietly leaves the beam/tail path running.
                  c.disableJoints = true;
                  return c;
              }() },

            { "fx.sprites.off",
              "RenderSprites() off. IR per-quad path.",
              TagEffects | TagSprites, [] { SceneConfig c; c.disableSprites = true; return c; }() },

            { "fx.particles.off",
              "RenderParticles() off. The heaviest IR per-quad path in effect-dense frames.",
              TagEffects | TagParticles, [] { SceneConfig c; c.disableParticles = true; return c; }() },

            { "fx.joints.off",
              "RenderJoints() off. Beam and tail-trail effects.",
              TagEffects | TagJoints, [] { SceneConfig c; c.disableJoints = true; return c; }() },

            { "fx.skillmodels.off",
              "Skill effect models off, legacy impact effects still rendering.",
              TagEffects | TagModels, [] { SceneConfig c; c.disableSkillEffectModels = true; return c; }() },

            { "fx.boids.off",
              "Ambient wildlife off. Always-on cost in town maps, independent of combat.",
              TagWildlife, [] { SceneConfig c; c.disableBoids = true; return c; }() },

            { "fx.wingshadow.off",
              "The extra per-wing body shadow draw off.",
              TagWings, [] { SceneConfig c; c.disableWingShadow = true; return c; }() },

            { "fx.winglayers.off",
              "Wing glow overlay passes off. Visibly changes the wing -- a measurement, not a fix.",
              TagWings, [] { SceneConfig c; c.disableWingExtraLayers = true; return c; }() },
        };
        return s_segments;
    }

    struct TagName
    {
        SegmentTag tag;
        const char* name;
    };

    const TagName kTagNames[] = {
        { TagBaseline,  "baseline" },
        { TagEffects,   "effects" },
        { TagSprites,   "sprites" },
        { TagParticles, "particles" },
        { TagJoints,    "joints" },
        { TagModels,    "models" },
        { TagWildlife,  "wildlife" },
        { TagWings,     "wings" },
    };

    bool HasTagNamed(uint32_t tags, const std::string& wanted)
    {
        for (const TagName& entry : kTagNames)
            if (wanted == entry.name) return (tags & entry.tag) != 0;
        return false;
    }
}

const std::vector<Segment>& All()
{
    return Catalog();
}

const Segment* Find(const std::string& name)
{
    for (const Segment& segment : Catalog())
        if (name == segment.name) return &segment;
    return nullptr;
}

bool MatchesPattern(const std::string& name, const std::string& pattern)
{
    // Iterative glob with backtracking on the last '*' -- no recursion, no allocation.
    size_t nameIndex = 0, patternIndex = 0;
    size_t starIndex = std::string::npos, nameAtStar = 0;

    while (nameIndex < name.size())
    {
        const bool literalMatch = patternIndex < pattern.size() &&
            (pattern[patternIndex] == '?' || pattern[patternIndex] == name[nameIndex]);
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

        if (starIndex == std::string::npos) return false;

        // Backtrack: let the last '*' swallow one more character.
        patternIndex = starIndex + 1;
        nameIndex = ++nameAtStar;
    }

    while (patternIndex < pattern.size() && pattern[patternIndex] == '*') patternIndex++;
    return patternIndex == pattern.size();
}

std::vector<const Segment*> Select(const std::string& pattern)
{
    std::vector<const Segment*> selected;
    const bool byTag = !pattern.empty() && pattern[0] == '#';
    const std::string tagName = byTag ? pattern.substr(1) : std::string();

    for (const Segment& segment : Catalog())
    {
        const bool matches = byTag ? HasTagNamed(segment.tags, tagName)
                                   : MatchesPattern(segment.name, pattern);
        if (matches) selected.push_back(&segment);
    }
    return selected;
}

std::string TagsToString(uint32_t tags)
{
    std::string out;
    for (const TagName& entry : kTagNames)
    {
        if ((tags & entry.tag) == 0) continue;
        if (!out.empty()) out += ' ';
        out += entry.name;
    }
    return out;
}
}
