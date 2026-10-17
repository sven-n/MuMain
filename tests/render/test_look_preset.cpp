#include "stdafx.h"

#include "doctest.h"

#include "Render/Look/LookPreset.h"

#include <vector>

using Render::Look::FeatureOverrides;
using Render::Look::LookFeatures;
using Render::Look::LookPresetController;

namespace
{
const LookFeatures AllOn{true, true, true, true, true, 100, true};
const LookFeatures AllOff{false, false, false, false, false, 100, false};

// Records what the controller saves and applies.
struct Recorder
{
    std::vector<LookPreset> saved;
    std::vector<LookFeatures> applied;

    LookPresetController Controller()
    {
        return LookPresetController({[this](LookPreset preset) { saved.push_back(preset); },
                                     [this](const LookFeatures& features) { applied.push_back(features); }});
    }
};

FeatureOverrides PostOff()
{
    FeatureOverrides change;
    change.postProcessing = false;
    return change;
}
} // namespace

TEST_CASE("Enhanced switches the five features and anti-aliasing on at full mood strength, Classic switches them "
          "off [render][look][preset]")
{
    CHECK(Render::Look::PresetFeatures(LookPreset::Enhanced) == AllOn);
    CHECK(Render::Look::PresetFeatures(LookPreset::Classic) == AllOff);
}

TEST_CASE("applying a preset saves it and switches the features [render][look][preset]")
{
    Recorder recorder;
    LookPresetController controller = recorder.Controller();

    controller.ApplyPreset(LookPreset::Enhanced);
    REQUIRE(recorder.saved.size() == 1);
    CHECK(recorder.saved[0] == LookPreset::Enhanced);
    REQUIRE(recorder.applied.size() == 1);
    CHECK(recorder.applied[0] == AllOn);

    controller.ApplyPreset(LookPreset::Classic);
    CHECK(recorder.saved.back() == LookPreset::Classic);
    CHECK(recorder.applied.back() == AllOff);
}

TEST_CASE("a developer override applies for the session without saving or changing the preset [render][look][preset]")
{
    Recorder recorder;
    LookPresetController controller = recorder.Controller();
    controller.ApplyPreset(LookPreset::Enhanced);

    controller.Override(PostOff());
    CHECK(recorder.saved.size() == 1); // not persisted
    CHECK(controller.Preset() == LookPreset::Enhanced);
    LookFeatures expected = AllOn;
    expected.postProcessing = false;
    CHECK(recorder.applied.back() == expected);

    FeatureOverrides weakMood;
    weakMood.lightMoodStrength = 40;
    controller.Override(weakMood);
    expected.lightMoodStrength = 40;
    CHECK(recorder.applied.back() == expected);
    CHECK(controller.Overrides().Describe() == "post off, lightmood strength 40");
}

TEST_CASE("re-applying the current preset clears every override [render][look][preset]")
{
    Recorder recorder;
    LookPresetController controller = recorder.Controller();
    controller.ApplyPreset(LookPreset::Classic);
    FeatureOverrides lightingOn;
    lightingOn.enhancedLighting = true;
    controller.Override(lightingOn);
    CHECK(controller.Effective().enhancedLighting);

    controller.ApplyPreset(LookPreset::Classic);
    CHECK_FALSE(controller.Overrides().Any());
    CHECK(controller.Overrides().Describe() == "none");
    CHECK(recorder.applied.back() == AllOff);
}

TEST_CASE("the player's Enhanced choices act on Enhanced only, survive a preset and yield to an override "
          "[render][look][preset]")
{
    Recorder recorder;
    LookPresetController controller = recorder.Controller();
    controller.ApplyPreset(LookPreset::Enhanced);

    controller.SetChoices(PostOff());
    LookFeatures expected = AllOn;
    expected.postProcessing = false;
    CHECK(recorder.applied.back() == expected);
    CHECK(recorder.saved.size() == 1); // the caller saves choices

    controller.ApplyPreset(LookPreset::Classic);
    CHECK(recorder.applied.back() == AllOff);
    controller.ApplyPreset(LookPreset::Enhanced);
    CHECK(recorder.applied.back() == expected);

    FeatureOverrides postOn;
    postOn.postProcessing = true;
    controller.Override(postOn);
    CHECK(recorder.applied.back() == AllOn);
}

TEST_CASE("replacing the overrides drops the ones left out [render][look][preset]")
{
    Recorder recorder;
    LookPresetController controller = recorder.Controller();
    controller.ApplyPreset(LookPreset::Enhanced);
    controller.Override(PostOff());

    FeatureOverrides lightingOff;
    lightingOff.enhancedLighting = false;
    controller.ReplaceOverrides(lightingOff);
    LookFeatures expected = AllOn;
    expected.enhancedLighting = false;
    CHECK(recorder.applied.back() == expected);
    CHECK(controller.Overrides() == lightingOff);

    controller.ReplaceOverrides({});
    CHECK(recorder.applied.back() == AllOn);
}

TEST_CASE("$look classic|enhanced and the per-feature commands parse [render][look][preset]")
{
    using Render::Look::ParseFeatureOverrideCommand;
    using Render::Look::ParseLookPresetCommand;

    CHECK(ParseLookPresetCommand(L"$look classic") == LookPreset::Classic);
    CHECK(ParseLookPresetCommand(L"$look Enhanced") == LookPreset::Enhanced);
    CHECK_FALSE(ParseLookPresetCommand(L"$look show").has_value());
    CHECK_FALSE(ParseLookPresetCommand(L"$look").has_value());
    CHECK_FALSE(ParseLookPresetCommand(L"$lookclassic").has_value());

    CHECK(ParseFeatureOverrideCommand(L"$texfilter off")->textureFiltering == false);
    CHECK(ParseFeatureOverrideCommand(L"$lighting on")->enhancedLighting == true);
    CHECK(ParseFeatureOverrideCommand(L"$dynlights off")->dynamicLights == false);
    CHECK(ParseFeatureOverrideCommand(L"$post off")->postProcessing == false);
    CHECK(ParseFeatureOverrideCommand(L"$lightmood on")->lightMood == true);
    const auto strength = ParseFeatureOverrideCommand(L"$lightmood 40");
    REQUIRE(strength.has_value());
    CHECK(strength->lightMoodStrength == 40);
    CHECK_FALSE(strength->lightMood.has_value());

    // Not switches: the tuning aliases, diagnostics and anything else.
    CHECK_FALSE(ParseFeatureOverrideCommand(L"$lighting rim 0.4").has_value());
    CHECK_FALSE(ParseFeatureOverrideCommand(L"$dynlights stats").has_value());
    CHECK_FALSE(ParseFeatureOverrideCommand(L"$post view bloom").has_value());
    CHECK_FALSE(ParseFeatureOverrideCommand(L"$lightmood knee 0.9").has_value());
    CHECK_FALSE(ParseFeatureOverrideCommand(L"$texfilter maybe").has_value());
}
