#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/MapData/LookProfile.h"
#include "Render/Look/TuningControls.h"

#include <cmath>
#include <limits>
#include <set>
#include <string>

using Data::Maps::LookValue;
using Render::Look::Control;
using Render::Look::TuningControl;
using Render::Look::TuningControlId;
using Render::Look::TuningGroup;
using Render::Look::TuningUnit;
using Render::Look::TuningZero;

namespace
{
constexpr int Lorencia = 0;
constexpr int Icarus = 10;

// The normative table of the public-graphics-tuning specification: every control, its bounds,
// step, unit, zero meaning and placement.
struct SpecRow
{
    TuningControlId id;
    TuningGroup group;
    bool specialist;
    std::optional<LookValue> value;
    double min;
    double max;
    double step;
    TuningUnit unit;
    TuningZero zero;
};

const SpecRow Spec[] = {
    {TuningControlId::DiffuseScale, TuningGroup::Lighting, false, LookValue::DiffuseScale, 0, 2, 0.05, TuningUnit::None,
     TuningZero::Plain},
    {TuningControlId::RimStrength, TuningGroup::Lighting, false, LookValue::RimStrength, 0, 1, 0.05, TuningUnit::None,
     TuningZero::Plain},
    {TuningControlId::SpecularScale, TuningGroup::Lighting, false, LookValue::SpecularScale, 0, 2, 0.05,
     TuningUnit::None, TuningZero::Plain},
    {TuningControlId::DiffuseAmbient, TuningGroup::Lighting, true, LookValue::DiffuseAmbient, 0, 1, 0.05,
     TuningUnit::None, TuningZero::Plain},
    {TuningControlId::DiffuseFloor, TuningGroup::Lighting, true, LookValue::DiffuseFloor, 0, 1, 0.05, TuningUnit::None,
     TuningZero::Plain},
    {TuningControlId::HighlightKnee, TuningGroup::Lighting, true, LookValue::HighlightKnee, 0.5, 0.9, 0.01,
     TuningUnit::None, TuningZero::Plain},
    {TuningControlId::RimPower, TuningGroup::Lighting, true, LookValue::RimPower, 1, 8, 0.1, TuningUnit::None,
     TuningZero::Plain},
    {TuningControlId::SpecularPower, TuningGroup::Lighting, true, LookValue::SpecularPower, 4, 128, 1, TuningUnit::None,
     TuningZero::Plain},
    {TuningControlId::EffectGain, TuningGroup::DynamicLights, false, LookValue::DynamicEffectGain, 0, 4, 0.1,
     TuningUnit::Times, TuningZero::Plain},
    {TuningControlId::EffectRange, TuningGroup::DynamicLights, false, LookValue::DynamicEffectRange, 0.5, 4, 0.1,
     TuningUnit::Times, TuningZero::Plain},
    {TuningControlId::EffectFade, TuningGroup::DynamicLights, false, LookValue::DynamicEffectFade, 0, 2, 0.05,
     TuningUnit::Seconds, TuningZero::Immediate},
    {TuningControlId::EffectCap, TuningGroup::DynamicLights, false, LookValue::DynamicEffectCap, 0, 8, 0.1,
     TuningUnit::None, TuningZero::NoCap},
    {TuningControlId::Wrap, TuningGroup::DynamicLights, true, LookValue::DynamicWrap, 0, 0.5, 0.05, TuningUnit::None,
     TuningZero::Plain},
    {TuningControlId::MapSmoothing, TuningGroup::DynamicLights, true, LookValue::DynamicMapSmoothing, 0, 2, 0.05,
     TuningUnit::Seconds, TuningZero::Immediate},
    {TuningControlId::EffectRangeMax, TuningGroup::DynamicLights, true, LookValue::DynamicEffectRangeMax, 0, 16, 1,
     TuningUnit::Cells, TuningZero::NoCap},
    {TuningControlId::MoodStrength, TuningGroup::LightMood, false, std::nullopt, 0, 100, 1, TuningUnit::Percent,
     TuningZero::Plain},
    {TuningControlId::BloomStrength, TuningGroup::PostProcessing, false, LookValue::BloomStrength, 0, 2, 0.05,
     TuningUnit::None, TuningZero::Plain},
    {TuningControlId::BloomLevelFalloff, TuningGroup::PostProcessing, false, LookValue::BloomLevelFalloff, 0.25, 1,
     0.05, TuningUnit::None, TuningZero::Plain},
    {TuningControlId::GradingStrength, TuningGroup::PostProcessing, false, LookValue::GradingStrength, 0, 1, 0.01,
     TuningUnit::Percent, TuningZero::Plain},
};

Data::Maps::LookProfileData ShippedLikeProfile()
{
    Data::Maps::LookProfileData profile;
    profile.defaultOverrides[LookValue::BloomStrength] = 1.0;
    profile.defaultOverrides[LookValue::DynamicEffectGain] = 1.5;
    Data::Maps::MapLookOverrides icarus;
    icarus.world = Icarus;
    icarus.overrides[LookValue::BloomStrength] = 0.3;
    profile.maps.push_back(icarus);
    return profile;
}
} // namespace

TEST_CASE("the 19 tuning controls have the specified bounds, steps, units and placement [render][look][tuning]")
{
    REQUIRE(Render::Look::TuningControlCount == 19);
    REQUIRE(std::size(Spec) == Render::Look::TuningControlCount);
    std::set<LookValue> covered;
    for (const SpecRow& row : Spec)
    {
        const TuningControl& control = Control(row.id);
        CAPTURE(static_cast<int>(row.id));
        CHECK(control.id == row.id);
        CHECK(control.group == row.group);
        CHECK(control.specialist == row.specialist);
        CHECK(control.lookValue == row.value);
        CHECK(control.min == doctest::Approx(row.min));
        CHECK(control.max == doctest::Approx(row.max));
        CHECK(control.step == doctest::Approx(row.step));
        CHECK(control.unit == row.unit);
        CHECK(control.zero == row.zero);
        if (row.value)
        {
            // The look profile's metadata is the authority for the range.
            const Data::Maps::LookValueInfo& info = Data::Maps::Info(*row.value);
            CHECK(control.min == info.min);
            CHECK(control.max == info.max);
            CHECK(Render::Look::FindTuningControl(*row.value) == row.id);
            covered.insert(*row.value);
        }
        // The range is a whole number of steps, so both ends are reachable on the grid.
        const double steps = (control.max - control.min) / control.step;
        CHECK(std::abs(steps - std::round(steps)) < 1e-9);
    }
    // Every look value has exactly one control.
    CHECK(covered.size() == Data::Maps::LookValueCount);
    // Colour grading may now be off entirely.
    CHECK(Data::Maps::Info(LookValue::GradingStrength).min == 0.0);
}

TEST_CASE("tuning values stay inside their inclusive bounds and on their steps [render][look][tuning]")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    for (const TuningControl& control : Render::Look::TuningControls())
    {
        CAPTURE(static_cast<int>(control.id));
        // Inclusive endpoints are valid, anything outside or non-finite is not.
        CHECK(Render::Look::ValidTuningValue(control, control.min) == control.min);
        CHECK(Render::Look::ValidTuningValue(control, control.max) == control.max);
        CHECK_FALSE(Render::Look::ValidTuningValue(control, control.min - control.step));
        CHECK_FALSE(Render::Look::ValidTuningValue(control, control.max + control.step));
        CHECK_FALSE(Render::Look::ValidTuningValue(control, nan));
        CHECK_FALSE(Render::Look::ValidTuningValue(control, infinity));

        // The pointer at either end of the track reaches the endpoint.
        CHECK(Render::Look::TuningValueFromFraction(control, 0.0) == doctest::Approx(control.min));
        CHECK(Render::Look::TuningValueFromFraction(control, 1.0) == doctest::Approx(control.max));
        CHECK(Render::Look::TuningValueFromFraction(control, -1.0) == doctest::Approx(control.min));
        CHECK(Render::Look::TuningValueFromFraction(control, 2.0) == doctest::Approx(control.max));
        CHECK(Render::Look::TuningValueFromFraction(control, nan) == control.min);
        CHECK(Render::Look::TuningFraction(control, control.min) == 0.0);
        CHECK(Render::Look::TuningFraction(control, control.max) == 1.0);

        // A key steps by one step and stops at the ends.
        CHECK(Render::Look::StepTuningValue(control, control.min, 1) == doctest::Approx(control.min + control.step));
        CHECK(Render::Look::StepTuningValue(control, control.min, -1) == doctest::Approx(control.min));
        CHECK(Render::Look::StepTuningValue(control, control.max, 1) == doctest::Approx(control.max));
        CHECK(Render::Look::StepTuningValue(control, control.max, -1) == doctest::Approx(control.max - control.step));
        CHECK(Render::Look::SnapToStep(control, infinity) == control.min);
        CHECK(Render::Look::SnapToStep(control, control.max * 10.0 + 10.0) == doctest::Approx(control.max));
    }
}

TEST_CASE("a value between steps moves to the next step in the key's direction [render][look][tuning]")
{
    const TuningControl& rim = Control(TuningControlId::RimStrength);
    // 0.37 lies between the steps 0.35 and 0.40.
    CHECK(Render::Look::StepTuningValue(rim, 0.37, 1) == doctest::Approx(0.40));
    CHECK(Render::Look::StepTuningValue(rim, 0.37, -1) == doctest::Approx(0.35));
    CHECK(Render::Look::StepTuningValue(rim, 0.37, 0) == doctest::Approx(0.35));
    // Steps are kept as config.ini would hold them: no binary fraction residue.
    const TuningControl& gain = Control(TuningControlId::EffectGain);
    double value = 0.0;
    for (int step = 0; step < 15; ++step)
    {
        value = Render::Look::StepTuningValue(gain, value, 1);
    }
    CHECK(value == 1.5);
}

TEST_CASE("a tuning value is shown with the step's decimals, and an inherited one as it is [render][look][tuning]")
{
    CHECK(Render::Look::FormatTuningValue(Control(TuningControlId::EffectGain), 1.5) == "1.5");
    CHECK(Render::Look::FormatTuningValue(Control(TuningControlId::DiffuseScale), 1.0) == "1.00");
    CHECK(Render::Look::FormatTuningValue(Control(TuningControlId::SpecularPower), 32.0) == "32");
    CHECK(Render::Look::FormatTuningValue(Control(TuningControlId::GradingStrength), 0.0) == "0");
    CHECK(Render::Look::FormatTuningValue(Control(TuningControlId::GradingStrength), 1.0) == "100");
    CHECK(Render::Look::FormatTuningValue(Control(TuningControlId::MoodStrength), 70.0) == "70");
    // The built-in rim strength 0.35 is on the 0.05 grid; 0.375 is not and keeps its digits.
    CHECK(Render::Look::FormatTuningValue(Control(TuningControlId::RimStrength), 0.35) == "0.35");
    CHECK(Render::Look::FormatTuningValue(Control(TuningControlId::RimStrength), 0.375) == "0.375");
}

TEST_CASE("player values apply after the map's and yield to a developer's session edit [render][look][tuning]")
{
    const Data::Maps::LookProfileData profile = ShippedLikeProfile();
    const Render::Look::DeveloperLookEdits none{};
    const Render::Look::TuningGroupSet all{true, true, true, true};
    Data::Maps::LookOverrides personal;

    // Untouched bloom follows each map.
    CHECK(Render::Look::ResolveLookValues(profile, Lorencia, personal, none, all)[LookValue::BloomStrength] == 1.0);
    CHECK(Render::Look::ResolveLookValues(profile, Icarus, personal, none, all)[LookValue::BloomStrength] == 0.3);

    // A player's 0.5 applies on every map; the other values keep their layers.
    personal[LookValue::BloomStrength] = 0.5;
    CHECK(Render::Look::ResolveLookValues(profile, Lorencia, personal, none, all)[LookValue::BloomStrength] == 0.5);
    CHECK(Render::Look::ResolveLookValues(profile, Icarus, personal, none, all)[LookValue::BloomStrength] == 0.5);
    CHECK(Render::Look::ResolveLookValues(profile, Icarus, personal, none, all)[LookValue::DynamicEffectGain] == 1.5);

    // A developer's edit of the same value wins while it lasts.
    Data::Maps::LookProfileData edited = profile;
    edited.defaultOverrides[LookValue::BloomStrength] = 1.8;
    Render::Look::DeveloperLookEdits developer;
    developer.Mark(LookValue::BloomStrength, std::nullopt);
    CHECK(Render::Look::ResolveLookValues(edited, Lorencia, personal, developer, all)[LookValue::BloomStrength] == 1.8);
    // Cleared, the player's value returns.
    CHECK(Render::Look::ResolveLookValues(profile, Lorencia, personal, none, all)[LookValue::BloomStrength] == 0.5);

    // Removing the player's value (Reset) gives each map its own value again.
    personal[LookValue::BloomStrength].reset();
    CHECK(Render::Look::ResolveLookValues(profile, Icarus, personal, none, all)[LookValue::BloomStrength] == 0.3);
    CHECK(Render::Look::ResolveLookValues(profile, Lorencia, personal, none, all)[LookValue::BloomStrength] == 1.0);
}

TEST_CASE("an inherited value between steps is not changed by resolving it [render][look][tuning]")
{
    Data::Maps::LookProfileData profile;
    profile.defaultOverrides[LookValue::RimStrength] = 0.37;
    const Data::Maps::LookValues values =
        Render::Look::ResolveLookValues(profile, Lorencia, {}, {}, {true, true, true, true});
    CHECK(values[LookValue::RimStrength] == 0.37);
    CHECK(values == profile.ForWorld(Lorencia));
}

TEST_CASE("a developer's edit covers only the scope it was made in [render][look][tuning]")
{
    Data::Maps::LookProfileData profile = ShippedLikeProfile();
    Render::Look::DeveloperLookEdits developer;
    // `$look set map bloom.strength 0.2` in Icarus.
    developer.Mark(LookValue::BloomStrength, Icarus);
    CHECK(developer.Covers(profile, Icarus, LookValue::BloomStrength));
    CHECK_FALSE(developer.Covers(profile, Lorencia, LookValue::BloomStrength));
    CHECK_FALSE(developer.Covers(profile, Icarus, LookValue::RimStrength));

    // A default edit shows where no map entry overrides it: not in Icarus, which has its own bloom.
    developer = {};
    developer.Mark(LookValue::BloomStrength, std::nullopt);
    CHECK(developer.Covers(profile, Lorencia, LookValue::BloomStrength));
    CHECK_FALSE(developer.Covers(profile, Icarus, LookValue::BloomStrength));

    // A named reset clears the default and the reset world's entry, not another world's.
    developer.Mark(LookValue::BloomStrength, Icarus);
    developer.Mark(LookValue::BloomStrength, Lorencia);
    developer.Clear(LookValue::BloomStrength, Lorencia);
    CHECK_FALSE(developer.Covers(profile, Lorencia, LookValue::BloomStrength));
    CHECK(developer.Covers(profile, Icarus, LookValue::BloomStrength));
}

TEST_CASE("player values apply only while their feature is in effect [render][look][tuning]")
{
    Render::Look::LookFeatures enhanced = Render::Look::PresetFeatures(LookPreset::Enhanced);
    CHECK(Render::Look::TuningGroupsApplying(enhanced) == Render::Look::TuningGroupSet{true, true, true, true});
    CHECK(Render::Look::TuningGroupsApplying(Render::Look::PresetFeatures(LookPreset::Classic)) ==
          Render::Look::TuningGroupSet{});
    // Dynamic lights act only with Enhanced lighting.
    Render::Look::LookFeatures noLighting = enhanced;
    noLighting.enhancedLighting = false;
    CHECK(Render::Look::TuningGroupsApplying(noLighting) == Render::Look::TuningGroupSet{false, false, true, true});

    const Data::Maps::LookProfileData profile = ShippedLikeProfile();
    Data::Maps::LookOverrides personal;
    personal[LookValue::HighlightKnee] = 0.6;
    personal[LookValue::BloomStrength] = 0.5;
    const Data::Maps::LookValues off = Render::Look::ResolveLookValues(profile, Lorencia, personal, {},
                                                                       Render::Look::TuningGroupsApplying(noLighting));
    // The knee (Enhanced lighting) is the map's again; bloom (its feature still on) stays the player's.
    CHECK(off[LookValue::HighlightKnee] == profile.ForWorld(Lorencia)[LookValue::HighlightKnee]);
    CHECK(off[LookValue::BloomStrength] == 0.5);
}

TEST_CASE("a pointer sets the step it is on, also from an inherited value between steps [render][look][tuning]")
{
    const TuningControl& bloom = Control(TuningControlId::BloomStrength);
    const int steps = Render::Look::TuningStepCount(bloom);
    REQUIRE(steps == 40);
    // Inherited 0.01 shares its nearest level (0) with the minimum: the minimum still applies.
    CHECK(Render::Look::TuningPointerValue(bloom, 0.01, 0) == 0.0);
    CHECK(Render::Look::TuningPointerValue(bloom, 0.01, steps) == doctest::Approx(2.0));
    // An inherited value just below the maximum.
    CHECK(Render::Look::TuningPointerValue(bloom, 1.99, steps) == 2.0);
    // 0.37 is nearest to step 7 (0.35): a pointer on step 7 still sets 0.35.
    CHECK(Render::Look::TuningPointerValue(Control(TuningControlId::RimStrength), 0.37, 7) == doctest::Approx(0.35));
    // On the value already: nothing to change.
    CHECK_FALSE(Render::Look::TuningPointerValue(bloom, 0.0, 0));
    CHECK_FALSE(Render::Look::TuningPointerValue(bloom, 2.0, steps));
    // Levels beyond the track are its ends.
    CHECK(Render::Look::TuningPointerValue(bloom, 1.0, steps + 5) == 2.0);
    CHECK(Render::Look::TuningPointerValue(bloom, 1.0, -3) == 0.0);
    for (const TuningControl& control : Render::Look::TuningControls())
    {
        CAPTURE(static_cast<int>(control.id));
        const double offGrid = control.min + control.step * 0.25;
        CHECK(Render::Look::TuningPointerValue(control, offGrid, 0) == doctest::Approx(control.min));
        const double belowMax = control.max - control.step * 0.25;
        CHECK(Render::Look::TuningPointerValue(control, belowMax, Render::Look::TuningStepCount(control)) ==
              doctest::Approx(control.max));
    }
}
