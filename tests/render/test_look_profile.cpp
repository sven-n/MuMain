#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/MapData/LookProfile.h"
#include "Render/Lighting/DynamicLightHistory.h"
#include "Render/Lighting/DynamicLightSelection.h"
#include "Render/Lighting/LightMoodScales.h"
#include "Render/Look/LookTuningCommand.h"
#include "Render/PostProcessing/BloomComposite.h"
#include "Render/Renderer/SceneLighting.h"
#include "Render/Renderer/SdlGpuPostProcessing.h"

#include <cstddef>
#include <optional>
#include <vector>

using Data::Maps::Info;
using Data::Maps::LookValue;
using doctest::Approx;
namespace Mood = Render::Lighting::LightMood;
namespace Post = Render::PostProcessing;

namespace
{
double BuiltIn(LookValue value)
{
    return Info(value).builtIn;
}

float BuiltInFloat(LookValue value)
{
    return static_cast<float>(BuiltIn(value));
}
} // namespace

TEST_CASE("the look profile's built-in values are the renderer's values before it [render][look]")
{
    // Enhanced lighting.
    CHECK(BuiltInFloat(LookValue::DiffuseScale) == mu::kClassicDiffuseScale);
    CHECK(BuiltInFloat(LookValue::DiffuseAmbient) == mu::kClassicAmbient);
    CHECK(BuiltInFloat(LookValue::DiffuseFloor) == mu::kClassicDiffuseFloor);
    CHECK(BuiltInFloat(LookValue::HighlightKnee) == Mood::HighlightShoulderStart);
    CHECK(BuiltInFloat(LookValue::RimStrength) == mu::kDefaultRimStrength);
    CHECK(BuiltInFloat(LookValue::RimPower) == mu::kDefaultRimPower);
    CHECK(BuiltInFloat(LookValue::SpecularScale) == mu::kDefaultSpecularScale);
    CHECK(BuiltInFloat(LookValue::SpecularPower) == mu::kDefaultSpecularPower);

    // Dynamic lights and their history.
    CHECK(BuiltInFloat(LookValue::DynamicWrap) == mu::kDefaultDynamicWrap);
    const Render::Lighting::LightHistorySettings classicHistory;
    CHECK(BuiltInFloat(LookValue::DynamicMapSmoothing) == classicHistory.mapSmoothingSeconds);
    CHECK(BuiltInFloat(LookValue::DynamicEffectGain) == classicHistory.effectGain);
    CHECK(BuiltInFloat(LookValue::DynamicEffectRange) == classicHistory.effectRangeScale);
    CHECK(BuiltInFloat(LookValue::DynamicEffectRangeMax) == classicHistory.effectRangeMax);
    CHECK(BuiltInFloat(LookValue::DynamicEffectFade) == classicHistory.effectFadeSeconds);
    CHECK(BuiltInFloat(LookValue::DynamicEffectCap) == classicHistory.effectCap);

    // Post-processing: the former config.ini default bloom strength, the level weight, full grading.
    CHECK(BuiltIn(LookValue::BloomStrength) == Approx(0.6));
    CHECK(BuiltInFloat(LookValue::BloomLevelFalloff) == mu::SdlGpuPostProcessing::BloomLevelWeight);
    CHECK(BuiltInFloat(LookValue::BloomLevelFalloff) == mu::PostProcessingLook{}.bloomLevelFalloff);
    CHECK(BuiltIn(LookValue::GradingStrength) == 1.0);
}

TEST_CASE("dynamic.wrap: the point-light diffuse from Lambert to half-Lambert [render][look]")
{
    const Data::Maps::LookValueInfo& wrap = Info(LookValue::DynamicWrap);
    CHECK(wrap.min == 0.0);
    CHECK(wrap.max == 0.5);

    // 0: Lambert.
    CHECK(mu::WrappedDiffuse(1.0f, 0.0f) == 1.0f);
    CHECK(mu::WrappedDiffuse(0.5f, 0.0f) == 0.5f);
    CHECK(mu::WrappedDiffuse(0.0f, 0.0f) == 0.0f);
    CHECK(mu::WrappedDiffuse(-0.5f, 0.0f) == 0.0f);
    // 0.5 (built-in): half-Lambert, the formula before the profile.
    CHECK(mu::WrappedDiffuse(1.0f, 0.5f) == 1.0f);
    CHECK(mu::WrappedDiffuse(0.0f, 0.5f) == 0.5f);
    CHECK(mu::WrappedDiffuse(-1.0f, 0.5f) == 0.0f);
    for (float normalDotLight = -1.0f; normalDotLight <= 1.0f; normalDotLight += 0.125f)
    {
        CAPTURE(normalDotLight);
        CHECK(mu::WrappedDiffuse(normalDotLight, 0.5f) == Approx(normalDotLight * 0.5f + 0.5f));
        // No wrap in the range gives a face more than full light, nor a face turned away any light.
        CHECK(mu::WrappedDiffuse(normalDotLight, 0.25f) <= 1.0f);
    }
    CHECK(mu::WrappedDiffuse(-1.0f, 0.25f) == 0.0f);
}

TEST_CASE("dynamic.wrap rides in the dynamic lights block after the light count [render][look]")
{
    CHECK(offsetof(mu::DynamicLightsUniform, lightCount) == 0);
    CHECK(offsetof(mu::DynamicLightsUniform, wrap) == 4);
    CHECK(offsetof(mu::DynamicLightsUniform, positionAndRadius) == 16);
    CHECK(sizeof(mu::DynamicLightsUniform) == 16 * (1 + 2 * mu::kMaxDynamicLights));

    // A selection carries the built-in wrap until the scene sets the profile's value.
    const std::vector<Render::Lighting::DynamicLight> none;
    const float reference[3]{};
    const Render::Lighting::DynamicLightSelection selection = Render::Lighting::SelectDynamicLights(none, reference);
    CHECK(selection.uniform.lightCount == 0u);
    CHECK(selection.uniform.wrap == mu::kDefaultDynamicWrap);
}

TEST_CASE("lighting.highlightKnee: the light mood's knee, blended with its strength [render][look]")
{
    const Data::Maps::LookValueInfo& knee = Info(LookValue::HighlightKnee);
    CHECK(knee.min == 0.5);
    CHECK(knee.max == 0.9);

    const Data::Maps::LightMood mood{0.7, 0.7, 1.0, 1.0, 0.6};
    CHECK(Mood::Blend(mood, 100).highlightKnee == Approx(Mood::HighlightShoulderStart));
    CHECK(Mood::Blend(mood, 100, 0.5f).highlightKnee == Approx(0.5));
    CHECK(Mood::Blend(mood, 100, 0.9f).highlightKnee == Approx(0.9));
    CHECK(Mood::Blend(mood, 50, 0.5f).highlightKnee == Approx(0.75));
    // Strength 0 is the Classic cut-off whatever the knee.
    CHECK(Mood::Blend(mood, 0, 0.5f).highlightKnee == 1.0f);

    // The CPU-lit mirror keeps the value up to the knee at both ends.
    CHECK(Mood::RollOffHighlight(0.5f, 0.5f) == 0.5f);
    CHECK(Mood::RollOffHighlight(0.9f, 0.5f) < 0.9f);
    CHECK(Mood::RollOffHighlight(0.85f, 0.9f) == 0.85f);

    // The Enhanced fragment shader gets it as diffuseCurve.w.
    mu::SceneLighting lighting;
    lighting.highlightKnee = 0.5f;
    CHECK(mu::PackSceneLightingUniform(lighting, {}).diffuseCurve[3] == 0.5f);
    lighting.highlightKnee = 0.9f;
    CHECK(mu::PackSceneLightingUniform(lighting, {}).diffuseCurve[3] == 0.9f);
}

// The merged light-mood scenario "Burnt-out top keeps shading" at both ends of the knee's range:
// faces that Classic burns out stay apart and below full light. Classic lights a face with up to
// 2 x its per-object light (at most 1); on maps whose sun is 1 the mood leaves that factor as it
// is. A knee of 0.95 failed here: in float, every value from about 1.65 up is exactly 1.
TEST_CASE("lighting.highlightKnee: a burnt-out top keeps its shading at both range ends [render][look]")
{
    const Data::Maps::LookValueInfo& knee = Info(LookValue::HighlightKnee);
    for (const double end : {knee.min, knee.max})
    {
        const float k = static_cast<float>(end);
        CAPTURE(k);
        float previous = Mood::RollOffHighlight(1.05f, k);
        CHECK(previous < 1.f);
        for (const float value : {1.2f, 1.36f, 1.6f, 1.68f, 1.8f, 2.0f})
        {
            CAPTURE(value);
            const float rolled = Mood::RollOffHighlight(value, k);
            CHECK(rolled > previous); // the brighter face stays brighter
            CHECK(rolled < 1.f);      // and does not reach full light
            previous = rolled;
        }
    }
}

TEST_CASE("bloom.levelFalloff: the composite divides by the summed level weights [render][look]")
{
    const Data::Maps::LookValueInfo& falloff = Info(LookValue::BloomLevelFalloff);
    CHECK(falloff.min == 0.25);
    CHECK(falloff.max == 1.0);

    constexpr std::size_t Levels = mu::SdlGpuPostProcessing::BloomLevelCount;
    // Built-in 0.5: 1 + 0.5 + 0.25 + 0.125 + 0.0625, the constant before the profile.
    CHECK(Post::BloomWeightSum(Levels, 0.5f) == Approx(1.9375f));
    CHECK(Post::BloomWeightSum(Levels, 0.25f) == Approx(1.0f + 0.25f + 0.0625f + 0.015625f + 0.00390625f));
    CHECK(Post::BloomWeightSum(Levels, 1.0f) == Approx(static_cast<float>(Levels)));

    // The per-pass level weight sits after the texel size.
    CHECK(offsetof(mu::SdlGpuBloomUniform, levelWeight) == 8);
    CHECK(sizeof(mu::SdlGpuBloomUniform) == 16);
}

TEST_CASE(
    "grading.strength: blend of graded and ungraded colour, off without a table or in a diagnostic view [render][look]")
{
    const Data::Maps::LookValueInfo& grading = Info(LookValue::GradingStrength);
    CHECK(grading.min == 0.0);
    CHECK(grading.max == 1.0);

    // CPU mirror of the composite's lerp at the range ends and halfway; 0 is the ungraded colour.
    CHECK(Post::BlendGrading(0.8f, 0.4f, 1.0f) == Approx(0.4f));
    CHECK(Post::BlendGrading(0.8f, 0.4f, 0.1f) == Approx(0.76f));
    CHECK(Post::BlendGrading(0.8f, 0.4f, 0.5f) == Approx(0.6f));
    CHECK(Post::BlendGrading(0.8f, 0.4f, 0.0f) == 0.8f);
    // A table that never brightens stays not brighter at any strength.
    for (float strength = 0.0f; strength <= 1.0f; strength += 0.1f)
    {
        CHECK(Post::BlendGrading(0.8f, 0.4f, strength) <= 0.8f);
    }

    // The guards: no table, or a diagnostic view, is ungraded (0).
    CHECK(Post::CompositeGradingStrength(true, false, 0.8f) == 0.8f);
    CHECK(Post::CompositeGradingStrength(true, false, 1.0f) == 1.0f);
    CHECK(Post::CompositeGradingStrength(false, false, 0.8f) == 0.0f);
    CHECK(Post::CompositeGradingStrength(true, true, 0.8f) == 0.0f);
    // A strength of 0 with a table is ungraded too: the composite skips the table (strength > 0)
    // while the bloom keeps its own scale, so bloom stays on without any grading.
    CHECK(Post::CompositeGradingStrength(true, false, 0.0f) == 0.0f);

    // The strength replaces the former gradingEnabled flag in the last register lane.
    CHECK(offsetof(mu::SdlGpuCompositeUniform, bloomScale) == 8);
    CHECK(offsetof(mu::SdlGpuCompositeUniform, gradingStrength) == 12);
    CHECK(sizeof(mu::SdlGpuCompositeUniform) == 16);
}

TEST_CASE("the developer tuning commands name their look values [render][look]")
{
    using Render::Look::ParseLookTuningCommand;

    const auto single = [](const wchar_t* text, LookValue value, double expected)
    {
        CAPTURE(value);
        const std::optional<Render::Look::LookTuningCommand> command = ParseLookTuningCommand(text);
        REQUIRE(command.has_value());
        REQUIRE(command->count == 1);
        CHECK(command->assignments[0].value == value);
        CHECK(command->assignments[0].newValue == Approx(expected));
    };
    single(L"$lighting rim 0.4", LookValue::RimStrength, 0.4);
    single(L"$lighting specular 0", LookValue::SpecularScale, 0.0);
    single(L"$dynlights wrap 0.3", LookValue::DynamicWrap, 0.3);
    single(L"$dynlights smooth 0.4", LookValue::DynamicMapSmoothing, 0.4);
    single(L"$dynlights cap 1", LookValue::DynamicEffectCap, 1.0);
    single(L"$bloom falloff 0.6", LookValue::BloomLevelFalloff, 0.6);
    single(L"$bloom 0.9", LookValue::BloomStrength, 0.9);

    const std::optional<Render::Look::LookTuningCommand> diffuse =
        ParseLookTuningCommand(L"$lighting diffuse 1.0 0.3 0.15");
    REQUIRE(diffuse.has_value());
    REQUIRE(diffuse->count == 3);
    CHECK(diffuse->assignments[0].value == LookValue::DiffuseScale);
    CHECK(diffuse->assignments[1].value == LookValue::DiffuseAmbient);
    CHECK(diffuse->assignments[2].value == LookValue::DiffuseFloor);
    CHECK(diffuse->assignments[2].newValue == Approx(0.15));

    const std::optional<Render::Look::LookTuningCommand> spell = ParseLookTuningCommand(L"$dynlights spell 2.5 2 0.5");
    REQUIRE(spell.has_value());
    REQUIRE(spell->count == 3);
    CHECK(spell->assignments[0].value == LookValue::DynamicEffectGain);
    CHECK(spell->assignments[1].value == LookValue::DynamicEffectRange);
    CHECK(spell->assignments[2].value == LookValue::DynamicEffectFade);

    const std::optional<Render::Look::LookTuningCommand> reach = ParseLookTuningCommand(L"$dynlights reach 8");
    REQUIRE(reach.has_value());
    REQUIRE(reach->count == 1);
    CHECK(reach->assignments[0].value == LookValue::DynamicEffectRangeMax);
    CHECK(reach->assignments[0].newValue == Approx(8.0));

    // Not tuning commands: switches, too few or too many numbers, text after the number.
    CHECK_FALSE(ParseLookTuningCommand(L"$lighting on").has_value());
    CHECK_FALSE(ParseLookTuningCommand(L"$dynlights stats").has_value());
    CHECK_FALSE(ParseLookTuningCommand(L"$bloom").has_value());
    CHECK_FALSE(ParseLookTuningCommand(L"$bloom bright").has_value());
    CHECK_FALSE(ParseLookTuningCommand(L"$bloom 0.6x").has_value());
    CHECK_FALSE(ParseLookTuningCommand(L"$lighting diffuse 1 0.3").has_value());
    CHECK_FALSE(ParseLookTuningCommand(L"$dynlights wrap 0.3 0.4").has_value());
}
