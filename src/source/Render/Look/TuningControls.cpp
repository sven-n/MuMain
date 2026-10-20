#include "stdafx.h"

#include "Render/Look/TuningControls.h"

#include "Data/GameConfig/PersonalTuning.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace Render::Look
{
namespace
{
using Data::Maps::LookValue;

// Values within this of a step count as on it (steps are decimal fractions, not exact doubles).
constexpr double StepTolerance = 1e-9;
// The most decimals a value between steps is shown with.
constexpr int MaxShownDecimals = 3;

struct ControlSpec
{
    TuningControlId id;
    TuningGroup group;
    bool specialist;
    std::optional<LookValue> lookValue;
    double step;
    TuningUnit unit;
    double displayScale;
    TuningZero zero;
};

constexpr std::array<ControlSpec, TuningControlCount> Specs = {{
    {TuningControlId::DiffuseScale, TuningGroup::Lighting, false, LookValue::DiffuseScale, 0.05, TuningUnit::None, 1.0,
     TuningZero::Plain},
    {TuningControlId::RimStrength, TuningGroup::Lighting, false, LookValue::RimStrength, 0.05, TuningUnit::None, 1.0,
     TuningZero::Plain},
    {TuningControlId::SpecularScale, TuningGroup::Lighting, false, LookValue::SpecularScale, 0.05, TuningUnit::None,
     1.0, TuningZero::Plain},
    {TuningControlId::DiffuseAmbient, TuningGroup::Lighting, true, LookValue::DiffuseAmbient, 0.05, TuningUnit::None,
     1.0, TuningZero::Plain},
    {TuningControlId::DiffuseFloor, TuningGroup::Lighting, true, LookValue::DiffuseFloor, 0.05, TuningUnit::None, 1.0,
     TuningZero::Plain},
    {TuningControlId::HighlightKnee, TuningGroup::Lighting, true, LookValue::HighlightKnee, 0.01, TuningUnit::None, 1.0,
     TuningZero::Plain},
    {TuningControlId::RimPower, TuningGroup::Lighting, true, LookValue::RimPower, 0.1, TuningUnit::None, 1.0,
     TuningZero::Plain},
    {TuningControlId::SpecularPower, TuningGroup::Lighting, true, LookValue::SpecularPower, 1.0, TuningUnit::None, 1.0,
     TuningZero::Plain},
    {TuningControlId::EffectGain, TuningGroup::DynamicLights, false, LookValue::DynamicEffectGain, 0.1,
     TuningUnit::Times, 1.0, TuningZero::Plain},
    {TuningControlId::EffectRange, TuningGroup::DynamicLights, false, LookValue::DynamicEffectRange, 0.1,
     TuningUnit::Times, 1.0, TuningZero::Plain},
    {TuningControlId::EffectFade, TuningGroup::DynamicLights, false, LookValue::DynamicEffectFade, 0.05,
     TuningUnit::Seconds, 1.0, TuningZero::Immediate},
    {TuningControlId::EffectCap, TuningGroup::DynamicLights, false, LookValue::DynamicEffectCap, 0.1, TuningUnit::None,
     1.0, TuningZero::NoCap},
    {TuningControlId::Wrap, TuningGroup::DynamicLights, true, LookValue::DynamicWrap, 0.05, TuningUnit::None, 1.0,
     TuningZero::Plain},
    {TuningControlId::MapSmoothing, TuningGroup::DynamicLights, true, LookValue::DynamicMapSmoothing, 0.05,
     TuningUnit::Seconds, 1.0, TuningZero::Immediate},
    {TuningControlId::EffectRangeMax, TuningGroup::DynamicLights, true, LookValue::DynamicEffectRangeMax, 1.0,
     TuningUnit::Cells, 1.0, TuningZero::NoCap},
    {TuningControlId::MoodStrength, TuningGroup::LightMood, false, std::nullopt, 1.0, TuningUnit::Percent, 1.0,
     TuningZero::Plain},
    {TuningControlId::BloomStrength, TuningGroup::PostProcessing, false, LookValue::BloomStrength, 0.05,
     TuningUnit::None, 1.0, TuningZero::Plain},
    {TuningControlId::BloomLevelFalloff, TuningGroup::PostProcessing, false, LookValue::BloomLevelFalloff, 0.05,
     TuningUnit::None, 1.0, TuningZero::Plain},
    {TuningControlId::GradingStrength, TuningGroup::PostProcessing, false, LookValue::GradingStrength, 0.01,
     TuningUnit::Percent, 100.0, TuningZero::Plain},
}};

constexpr bool SpecsFollowIdOrder()
{
    for (std::size_t index = 0; index < Specs.size(); ++index)
    {
        if (static_cast<std::size_t>(Specs[index].id) != index)
        {
            return false;
        }
    }
    return true;
}
static_assert(SpecsFollowIdOrder());

std::array<TuningControl, TuningControlCount> BuildControls()
{
    std::array<TuningControl, TuningControlCount> controls{};
    for (std::size_t index = 0; index < Specs.size(); ++index)
    {
        const ControlSpec& spec = Specs[index];
        TuningControl& control = controls[index];
        control.id = spec.id;
        control.group = spec.group;
        control.specialist = spec.specialist;
        control.lookValue = spec.lookValue;
        control.step = spec.step;
        control.unit = spec.unit;
        control.displayScale = spec.displayScale;
        control.zero = spec.zero;
        if (spec.lookValue)
        {
            const Data::Maps::LookValueInfo& info = Data::Maps::Info(*spec.lookValue);
            control.min = info.min;
            control.max = info.max;
        }
        else
        {
            control.min = Data::Config::MinMoodStrengthPercent;
            control.max = Data::Config::MaxMoodStrengthPercent;
        }
    }
    return controls;
}

// The value's position in steps from the minimum.
double StepsFromMin(const TuningControl& control, double value)
{
    return (value - control.min) / control.step;
}

double OnGrid(const TuningControl& control, double steps)
{
    const double value = control.min + steps * control.step;
    return Data::Config::NormalizeTuningNumber(std::clamp(value, control.min, control.max));
}

int Decimals(double step)
{
    int decimals = 0;
    double scaled = step;
    while (decimals < MaxShownDecimals && std::abs(scaled - std::round(scaled)) > StepTolerance)
    {
        scaled *= 10.0;
        ++decimals;
    }
    return decimals;
}

std::string Fixed(double value, int decimals)
{
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << std::fixed << std::setprecision(decimals) << value;
    return text.str();
}
} // namespace

const std::array<TuningControl, TuningControlCount>& TuningControls()
{
    static const std::array<TuningControl, TuningControlCount> controls = BuildControls();
    return controls;
}

const TuningControl& Control(TuningControlId id)
{
    return TuningControls()[static_cast<std::size_t>(id)];
}

std::optional<TuningControlId> FindTuningControl(LookValue value)
{
    for (const TuningControl& control : TuningControls())
    {
        if (control.lookValue == value)
        {
            return control.id;
        }
    }
    return std::nullopt;
}

std::optional<double> ValidTuningValue(const TuningControl& control, double value)
{
    if (!std::isfinite(value) || value < control.min || value > control.max)
    {
        return std::nullopt;
    }
    return value;
}

double SnapToStep(const TuningControl& control, double value)
{
    if (!std::isfinite(value))
    {
        return control.min;
    }
    return OnGrid(control, std::round(StepsFromMin(control, std::clamp(value, control.min, control.max))));
}

double StepTuningValue(const TuningControl& control, double value, int steps)
{
    if (!std::isfinite(value))
    {
        return control.min;
    }
    const double position = StepsFromMin(control, std::clamp(value, control.min, control.max));
    const double nearest = std::round(position);
    const bool onStep = std::abs(position - nearest) <= StepTolerance;
    double base = nearest;
    if (!onStep && steps != 0)
    {
        // Between two steps: the first move reaches the next step in its direction.
        base = steps > 0 ? std::floor(position) : std::ceil(position);
    }
    return OnGrid(control, base + steps);
}

double TuningValueFromFraction(const TuningControl& control, double fraction)
{
    if (!std::isfinite(fraction))
    {
        return control.min;
    }
    const double clamped = std::clamp(fraction, 0.0, 1.0);
    return SnapToStep(control, control.min + clamped * (control.max - control.min));
}

double TuningFraction(const TuningControl& control, double value)
{
    if (!std::isfinite(value) || control.max <= control.min)
    {
        return 0.0;
    }
    return std::clamp((value - control.min) / (control.max - control.min), 0.0, 1.0);
}

std::string FormatTuningValue(const TuningControl& control, double value)
{
    const double shown = value * control.displayScale;
    const int stepDecimals = Decimals(control.step * control.displayScale);
    if (std::abs(std::round(StepsFromMin(control, value)) - StepsFromMin(control, value)) <= StepTolerance)
    {
        return Fixed(shown, stepDecimals);
    }
    std::string text = Fixed(shown, std::max(stepDecimals, MaxShownDecimals));
    // Between steps: as many decimals as the value needs, never fewer than the step has.
    const std::size_t point = text.find('.');
    while (point != std::string::npos && text.size() > point + 1 + static_cast<std::size_t>(stepDecimals) &&
           text.back() == '0')
    {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '.')
    {
        text.pop_back();
    }
    return text;
}

int TuningStepCount(const TuningControl& control)
{
    return static_cast<int>(std::lround((control.max - control.min) / control.step));
}

std::optional<double> TuningPointerValue(const TuningControl& control, double current, int level)
{
    const double value = OnGrid(control, static_cast<double>(std::clamp(level, 0, TuningStepCount(control))));
    if (value == current)
    {
        return std::nullopt;
    }
    return value;
}

void DeveloperLookEdits::Mark(LookValue value, std::optional<int> world)
{
    Flags& flags = world ? worlds[*world] : defaults;
    flags[static_cast<std::size_t>(value)] = true;
}

void DeveloperLookEdits::Clear(LookValue value, int world)
{
    const auto index = static_cast<std::size_t>(value);
    defaults[index] = false;
    const auto entry = worlds.find(world);
    if (entry != worlds.end())
    {
        entry->second[index] = false;
    }
}

bool DeveloperLookEdits::Covers(const Data::Maps::LookProfileData& profile, int world, LookValue value) const
{
    const auto index = static_cast<std::size_t>(value);
    const auto entry = worlds.find(world);
    if (entry != worlds.end() && entry->second[index])
    {
        return true;
    }
    if (!defaults[index])
    {
        return false;
    }
    // An edited default shows only where no map entry overrides it.
    return std::none_of(profile.maps.begin(), profile.maps.end(),
                        [world, value](const Data::Maps::MapLookOverrides& map)
                        { return map.world == world && map.overrides[value].has_value(); });
}

TuningGroupSet TuningGroupsApplying(const LookFeatures& features)
{
    TuningGroupSet applying{};
    applying[static_cast<std::size_t>(TuningGroup::Lighting)] = features.enhancedLighting;
    applying[static_cast<std::size_t>(TuningGroup::DynamicLights)] =
        features.enhancedLighting && features.dynamicLights;
    applying[static_cast<std::size_t>(TuningGroup::LightMood)] = features.lightMood;
    applying[static_cast<std::size_t>(TuningGroup::PostProcessing)] = features.postProcessing;
    return applying;
}

TuningGroupSet TuningGroupsInEffect(const LookFeatures& requested, bool enhancedLightingActive,
                                    bool postProcessingActive)
{
    LookFeatures accepted = requested;
    accepted.enhancedLighting = requested.enhancedLighting && enhancedLightingActive;
    accepted.postProcessing = requested.postProcessing && postProcessingActive;
    return TuningGroupsApplying(accepted);
}

void ApplyLookFeatures(const LookFeatures& requested, const LookFeatureAppliers& appliers)
{
    if (appliers.textureFiltering)
    {
        appliers.textureFiltering(requested.textureFiltering);
    }
    const bool lighting = appliers.enhancedLighting && appliers.enhancedLighting(requested.enhancedLighting);
    if (appliers.dynamicLights)
    {
        appliers.dynamicLights(requested.dynamicLights);
    }
    const bool postProcessing = appliers.postProcessing && appliers.postProcessing(requested.postProcessing);
    if (appliers.tuningGroups)
    {
        appliers.tuningGroups(TuningGroupsInEffect(requested, lighting, postProcessing));
    }
    if (appliers.lightMood)
    {
        appliers.lightMood(requested.lightMood, requested.lightMoodStrength);
    }
    if (appliers.antialiasing)
    {
        appliers.antialiasing(requested.antialiasing);
    }
}

bool PersonalValueApplies(const Data::Maps::LookProfileData& profile, int world,
                          const Data::Maps::LookOverrides& personal, const DeveloperLookEdits& developerEdits,
                          const TuningGroupSet& applying, LookValue value)
{
    if (!personal[value])
    {
        return false;
    }
    const std::optional<TuningControlId> control = FindTuningControl(value);
    if (!control || !applying[static_cast<std::size_t>(Control(*control).group)])
    {
        return false;
    }
    return !developerEdits.Covers(profile, world, value);
}

Data::Maps::LookValues ResolveLookValues(const Data::Maps::LookProfileData& profile, int world,
                                         const Data::Maps::LookOverrides& personal,
                                         const DeveloperLookEdits& developerEdits, const TuningGroupSet& applying)
{
    Data::Maps::LookValues values = profile.ForWorld(world);
    for (const Data::Maps::LookValueInfo& info : Data::Maps::LookValueInfos())
    {
        if (PersonalValueApplies(profile, world, personal, developerEdits, applying, info.value))
        {
            values[info.value] = *personal[info.value];
        }
    }
    return values;
}

} // namespace Render::Look
