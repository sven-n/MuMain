#include "doctest.h"

#include "Core/Utilities/Benchmark/BenchSegment.h"

using namespace Core::Benchmark;

TEST_CASE("the catalog starts with the baseline segment")
{
    REQUIRE(!Segments::All().empty());
    CHECK(std::string(Segments::All().front().name) == kBaselineSegmentName);
    CHECK(Segments::All().front().tags == TagBaseline);
}

TEST_CASE("the baseline segment disables nothing")
{
    const Segment* baseline = Segments::Find(kBaselineSegmentName);
    REQUIRE(baseline != nullptr);
    CHECK_FALSE(baseline->config.disableEffects);
    CHECK_FALSE(baseline->config.disableParticles);
    CHECK_FALSE(baseline->config.disableJoints);
}

TEST_CASE("every non-baseline segment actually disables something")
{
    // A segment whose config is all-false is a segment that measures the baseline twice. The
    // first catalog shipped with fx.all.off in exactly that state for the two heaviest paths.
    for (const Segment& segment : Segments::All())
    {
        if (std::string(segment.name) == kBaselineSegmentName) continue;

        const SceneConfig& c = segment.config;
        const bool disablesSomething =
            c.disableEffects || c.disableAllEffectsOption || c.disableSprites ||
            c.disableParticles || c.disableSkillEffectModels || c.disableJoints ||
            c.disableBoids || c.disableWingShadow || c.disableWingExtraLayers;

        INFO("segment: " << segment.name);
        CHECK(disablesSomething);
    }
}

TEST_CASE("fx.all.off closes every gate, not just the one named after it")
{
    // RenderSprites()/RenderParticles() early-return on the options-window switch and never look
    // at SetDisableEffects; RenderJoints() looks at neither. A segment promising "everything off"
    // has to set all three or it silently leaves the heaviest paths running -- which is what the
    // first run of this benchmark reported: 162 particle draws per frame under fx.all.off.
    const Segment* all = Segments::Find("fx.all.off");
    REQUIRE(all != nullptr);
    CHECK(all->config.disableEffects);
    CHECK(all->config.disableAllEffectsOption);
    CHECK(all->config.disableJoints);
}

TEST_CASE("every segment name is unique")
{
    const std::vector<Segment>& all = Segments::All();
    for (size_t i = 0; i < all.size(); i++)
        for (size_t j = i + 1; j < all.size(); j++)
            CHECK(std::string(all[i].name) != all[j].name);
}

TEST_CASE("glob matching")
{
    CHECK(Segments::MatchesPattern("fx.particles.off", "*"));
    CHECK(Segments::MatchesPattern("fx.particles.off", "fx.*"));
    CHECK(Segments::MatchesPattern("fx.particles.off", "*.off"));
    CHECK(Segments::MatchesPattern("fx.particles.off", "fx.*.off"));
    CHECK(Segments::MatchesPattern("fx.particles.off", "fx.particles.off"));
    CHECK_FALSE(Segments::MatchesPattern("fx.particles.off", "fx."));
    CHECK_FALSE(Segments::MatchesPattern("fx.particles.off", "scene.*"));
    CHECK_FALSE(Segments::MatchesPattern("scene.full", "fx.*"));
}

TEST_CASE("glob backtracking does not stop at the first candidate match")
{
    CHECK(Segments::MatchesPattern("aaab", "*ab"));
    CHECK(Segments::MatchesPattern("abcabc", "*abc"));
    CHECK_FALSE(Segments::MatchesPattern("abcabd", "*abc"));
}

TEST_CASE("selection by pattern returns catalog order")
{
    const std::vector<const Segment*> selected = Segments::Select("fx.*");
    REQUIRE(!selected.empty());
    for (const Segment* segment : selected)
        CHECK(std::string(segment->name).rfind("fx.", 0) == 0);

    CHECK(Segments::Select("*").size() == Segments::All().size());
    CHECK(Segments::Select("nothing.matches.this").empty());
}

TEST_CASE("selection by tag")
{
    // Both the surface's own segment and fx.all.off, which now genuinely disables particles too.
    const std::vector<const Segment*> particles = Segments::Select("#particles");
    REQUIRE(particles.size() == 2);
    CHECK(std::string(particles.front()->name) == "fx.all.off");
    CHECK(std::string(particles.back()->name) == "fx.particles.off");

    CHECK(Segments::Select("#effects").size() > 1);
    CHECK(Segments::Select("#nosuchtag").empty());
}

TEST_CASE("tags render as space-separated lowercase names")
{
    CHECK(Segments::TagsToString(TagBaseline) == "baseline");
    CHECK(Segments::TagsToString(TagEffects | TagParticles) == "effects particles");
    CHECK(Segments::TagsToString(0) == "");
}
