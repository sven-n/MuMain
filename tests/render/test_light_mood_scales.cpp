#include "doctest.h"

#include "Render/Lighting/LightMoodScales.h"

#include <algorithm>

using Data::Maps::LightMood;
using doctest::Approx;
namespace Mood = Render::Lighting::LightMood;

namespace
{
const LightMood Tuned{0.7, 0.7, 1.0, 1.0};
const LightMood Lifted{0.6, 0.85, 1.3, 0.5, 0.4};
} // namespace

TEST_CASE("strength 100 applies the full mood and strength 0 is Classic [render][lightmood]")
{
    const Mood::Scales full = Mood::Blend(Lifted, 100);
    CHECK(full.base == Approx(0.6));
    CHECK(full.figures == Approx(0.85));
    CHECK(full.dynamicGain == Approx(1.3));
    CHECK(full.background == Approx(0.5));
    CHECK(full.sun == Approx(0.4));
    CHECK(full.highlightKnee == Approx(Mood::HighlightShoulderStart));

    CHECK(Mood::Blend(Lifted, 0) == Mood::Scales{});
}

TEST_CASE("half strength blends linearly from Classic [render][lightmood]")
{
    // The spec's scenario: base 0.6 at strength 50 scales the static light by 0.8.
    const Mood::Scales half = Mood::Blend(Lifted, 50);
    CHECK(half.base == Approx(0.8));
    CHECK(half.figures == Approx(0.925));
    CHECK(half.dynamicGain == Approx(1.15));
    CHECK(half.background == Approx(0.75));
    CHECK(half.sun == Approx(0.7));
    CHECK(half.highlightKnee == Approx(0.9));
}

TEST_CASE("a strength outside 0 to 100 is clamped [render][lightmood]")
{
    CHECK(Mood::Blend(Lifted, -20) == Mood::Blend(Lifted, 0));
    CHECK(Mood::Blend(Lifted, 250) == Mood::Blend(Lifted, 100));
}

TEST_CASE("a figure's static light moves from base to figures, its dynamic light stays [render][lightmood]")
{
    const Mood::Scales scales = Mood::Blend(Lifted, 100);
    const float classicStatic[3] = {0.5f, 0.4f, 0.2f};
    const float dynamic[3] = {0.3f, 0.1f, 0.f};
    float darkenedStatic[3];
    float light[3];
    for (int i = 0; i < 3; ++i)
    {
        darkenedStatic[i] = classicStatic[i] * scales.base;
        light[i] = darkenedStatic[i] + dynamic[i];
    }

    Mood::CorrectFigureLight(scales, classicStatic, light);

    for (int i = 0; i < 3; ++i)
    {
        CHECK(light[i] == Approx(classicStatic[i] * 0.85f + dynamic[i]));
    }
}

TEST_CASE("figures at the base leave a figure's light as sampled [render][lightmood]")
{
    const Mood::Scales scales = Mood::Blend(Tuned, 100);
    const float darkenedStatic[3] = {0.35f, 0.35f, 0.35f};
    float light[3] = {0.6f, 0.5f, 0.4f};

    Mood::CorrectFigureLight(scales, darkenedStatic, light);

    CHECK(light[0] == 0.6f);
    CHECK(light[1] == 0.5f);
    CHECK(light[2] == 0.4f);
}

TEST_CASE("the own hero's fixed light takes the figure scale [render][lightmood]")
{
    const float abilityLight[3] = {1.f, 0.9f * 0.5f, 0.5f};
    float light[3];

    Mood::ScaleFixedFigureLight(Mood::Blend(Tuned, 100), abilityLight, light);
    CHECK(light[0] == Approx(0.7f));
    CHECK(light[1] == Approx(0.45f * 0.7f));
    CHECK(light[2] == Approx(0.35f));

    Mood::ScaleFixedFigureLight(Mood::Scales{}, abilityLight, light);
    CHECK(light[0] == abilityLight[0]);
    CHECK(light[1] == abilityLight[1]);
    CHECK(light[2] == abilityLight[2]);
}

TEST_CASE("the sun scales the directional part of the model light only [render][lightmood]")
{
    // Classic's model light vector and a face pointing up: max(0.8 * dot(n, L) + 0.4, 0.2).
    const float up[3] = {0.f, 0.f, 1.f};
    const auto luminosity = [&up](const float light[3])
    { return std::max(0.8f * (up[0] * light[0] + up[1] * light[1] + up[2] * light[2]) + 0.4f, 0.2f); };

    float classic[3] = {1.3f, 0.f, 2.f};
    Mood::ScaleSunDirection(Mood::Scales{}, classic);
    CHECK(classic[0] == 1.3f);
    CHECK(classic[2] == 2.f);
    CHECK(luminosity(classic) == Approx(2.0f));

    float sun[3] = {1.3f, 0.f, 2.f};
    Mood::Scales scales;
    scales.sun = 0.6f;
    Mood::ScaleSunDirection(scales, sun);
    CHECK(sun[0] == Approx(0.78f));
    CHECK(luminosity(sun) == Approx(0.8f * 0.6f * 2.f + 0.4f));

    // No sun: every face gets the fill light.
    float none[3] = {1.3f, 0.f, 2.f};
    scales.sun = 0.f;
    Mood::ScaleSunDirection(scales, none);
    CHECK(luminosity(none) == Approx(0.4f));
}

TEST_CASE("a lit colour rolls off towards 1 above the knee [render][lightmood]")
{
    constexpr float knee = Mood::HighlightShoulderStart;

    // Up to the knee nothing changes.
    CHECK(Mood::RollOffHighlight(0.3f, knee) == 0.3f);
    CHECK(Mood::RollOffHighlight(knee, knee) == knee);

    // Above it the value keeps rising, stays below 1, and leaves the knee with slope 1.
    float previous = knee;
    for (const float value : {0.85f, 0.9f, 1.0f, 1.2f, 1.5f, 2.3f})
    {
        CAPTURE(value);
        const float rolled = Mood::RollOffHighlight(value, knee);
        CHECK(rolled > previous);
        CHECK(rolled < 1.f);
        CHECK(rolled < value);
        previous = rolled;
    }
    CHECK(Mood::RollOffHighlight(knee + 0.001f, knee) == Approx(knee + 0.001f).epsilon(0.0001));
    // A face that Classic burns out at 2.0 x a light of 0.68 keeps a step to one at 1.36 x 0.68.
    CHECK(Mood::RollOffHighlight(2.0f * 0.68f, knee) - Mood::RollOffHighlight(1.36f * 0.68f, knee) > 0.05f);

    // A knee of 1 is Classic: the value is left for the cut-off at 1.
    CHECK(Mood::RollOffHighlight(1.7f, 1.f) == 1.7f);
    CHECK(Mood::RollOffHighlight(0.5f, 1.f) == 0.5f);
}

TEST_CASE("only brightening dynamic light gets the gain [render][lightmood]")
{
    CHECK(Mood::ScaleDynamicLight(0.4f, 1.5f) == Approx(0.6f));
    CHECK(Mood::ScaleDynamicLight(-0.4f, 1.5f) == -0.4f);
    CHECK(Mood::ScaleDynamicLight(0.f, 1.5f) == 0.f);
    CHECK(Mood::ScaleDynamicLight(0.4f, 1.f) == 0.4f);
}

TEST_CASE("$lightmood takes on, off or a strength from 0 to 100 [render][lightmood]")
{
    const auto on = Mood::ParseCommandArgument(L" ON ");
    REQUIRE(on);
    CHECK(on->enabled == true);
    CHECK_FALSE(on->strength);

    const auto off = Mood::ParseCommandArgument(L"off");
    REQUIRE(off);
    CHECK(off->enabled == false);

    const auto strength = Mood::ParseCommandArgument(L"50");
    REQUIRE(strength);
    CHECK(strength->strength == 50);
    CHECK_FALSE(strength->enabled);
    CHECK(Mood::ParseCommandArgument(L"0")->strength == 0);
    CHECK(Mood::ParseCommandArgument(L"100")->strength == 100);

    CHECK_FALSE(Mood::ParseCommandArgument(L"101"));
    CHECK_FALSE(Mood::ParseCommandArgument(L"-5"));
    CHECK_FALSE(Mood::ParseCommandArgument(L"0.5"));
    CHECK_FALSE(Mood::ParseCommandArgument(L""));
    CHECK_FALSE(Mood::ParseCommandArgument(L"dark"));
}

TEST_CASE("a shadow keep darkens the shade and leaves bright places Classic [render][lightmood]")
{
    LightMood mood{0.6, 0.7, 1.0, 1.0, 1.0};
    mood.shadowKeep = 1.0;
    const Mood::Scales scales = Mood::Blend(mood, 100);
    CHECK(Mood::BrightWeight(scales, 0.2f) == Approx(0.f));
    CHECK(Mood::BrightWeight(scales, 1.0f) == Approx(1.f));
    CHECK(Mood::BrightWeight(scales, 0.65f) == Approx(0.5f));

    float factor[3];
    Mood::StaticFactor(scales, 0.f, factor);
    CHECK(factor[0] == Approx(0.6f));
    Mood::StaticFactor(scales, 1.f, factor);
    CHECK(factor[1] == Approx(1.f));

    // Where the place keeps Classic brightness the figures do too: no correction.
    const float darkenedStatic[3] = {0.8f, 0.8f, 0.8f};
    float light[3] = {0.8f, 0.8f, 0.8f};
    Mood::CorrectFigureLight(scales, darkenedStatic, light, 1.f);
    CHECK(light[0] == Approx(0.8f));

    CHECK(Mood::BrightWeight(Mood::Blend(LightMood{0.6, 0.7, 1.0, 1.0, 1.0}, 100), 1.0f) == Approx(0.f));
    CHECK(Mood::Blend(mood, 0) == Mood::Scales{});
}

TEST_CASE("a cool shade darkens red and green more than blue, in the shade only [render][lightmood]")
{
    LightMood mood{0.5, 0.5, 1.0, 1.0, 1.0};
    mood.shadeCool = 0.2;
    float factor[3];
    Mood::StaticFactor(Mood::Blend(mood, 100), 0.f, factor);
    CHECK(factor[0] == Approx(0.4f));
    CHECK(factor[1] == Approx(0.45f));
    CHECK(factor[2] == Approx(0.5f));
    Mood::StaticFactor(Mood::Blend(mood, 100), 1.f, factor);
    CHECK(factor[0] == Approx(1.f));
}

TEST_CASE("a bright scale dims the places at the shadow keep, never below the shade [render][lightmood]")
{
    LightMood mood{0.5, 0.6, 1.0, 1.0, 1.0};
    mood.shadowKeep = 0.8;
    mood.brightScale = 0.85;
    const Mood::Scales scales = Mood::Blend(mood, 100);
    float factor[3];
    Mood::StaticFactor(scales, 1.f, factor);
    CHECK(factor[0] == Approx(0.85f));
    Mood::StaticFactor(scales, 0.f, factor);
    CHECK(factor[0] == Approx(0.5f));
    Mood::StaticFactor(scales, 0.5f, factor);
    CHECK(factor[0] == Approx(0.675f));
    Mood::FigureFactor(scales, 1.f, factor);
    CHECK(factor[0] == Approx(0.85f));

    // A bright scale below the base or the figure scale does not darken bright places further.
    mood.brightScale = 0.4;
    Mood::StaticFactor(Mood::Blend(mood, 100), 1.f, factor);
    CHECK(factor[0] == Approx(0.5f));
    Mood::FigureFactor(Mood::Blend(mood, 100), 1.f, factor);
    CHECK(factor[0] == Approx(0.6f));

    // Half strength blends it towards Classic like the other scales.
    mood.brightScale = 0.8;
    CHECK(Mood::Blend(mood, 50).brightScale == Approx(0.9f));
}
