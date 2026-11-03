#pragma once

#include "Data/GameData/MapData/LookProfile.h"
#include "Render/Look/LookPreset.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>

// The player's tuning controls of the Enhanced look: one per look value a player may change, plus
// the light mood's strength. Each belongs to the feature it tunes (the options window shows it
// under that feature's switch), common or under "More settings". The look values' ranges are the
// look profile's (Data::Maps::LookValueInfos); a control only adds the step a slider or a key
// moves by and the unit it is shown in.
namespace Render::Look
{

// In the order of the options window's feature switches (Render::Look::EnhancedFeature).
enum class TuningGroup : std::uint8_t
{
    Lighting,
    DynamicLights,
    LightMood,
    PostProcessing,
    Count,
};
inline constexpr std::size_t TuningGroupCount = static_cast<std::size_t>(TuningGroup::Count);

enum class TuningUnit : std::uint8_t
{
    None,
    // A factor: 1.5×.
    Times,
    Seconds,
    // Terrain cells.
    Cells,
    Percent,
};

// What a value of zero means for a control, where it is not simply none of it.
enum class TuningZero : std::uint8_t
{
    Plain,
    // The limit is off: dynamic.effectCap and dynamic.effectRangeMax.
    NoCap,
    // No easing: the change is immediate.
    Immediate,
};

// In display order: per group, the common controls first.
enum class TuningControlId : std::uint8_t
{
    DiffuseScale,
    RimStrength,
    SpecularScale,
    DiffuseAmbient,
    DiffuseFloor,
    HighlightKnee,
    RimPower,
    SpecularPower,
    EffectGain,
    EffectRange,
    EffectFade,
    EffectCap,
    Wrap,
    MapSmoothing,
    EffectRangeMax,
    MoodStrength,
    BloomStrength,
    BloomLevelFalloff,
    GradingStrength,
    Count,
};
inline constexpr std::size_t TuningControlCount = static_cast<std::size_t>(TuningControlId::Count);

struct TuningControl
{
    TuningControlId id = TuningControlId::DiffuseScale;
    TuningGroup group = TuningGroup::Lighting;
    // Shown under the group's "More settings".
    bool specialist = false;
    // The look value it sets; none for the light mood's strength (whole percent).
    std::optional<Data::Maps::LookValue> lookValue;
    // The range (inclusive) and the step, in the value's own units.
    double min = 0.0;
    double max = 0.0;
    double step = 0.0;
    TuningUnit unit = TuningUnit::None;
    // The shown number is the value times this (grading.strength 0..1 is shown as 0..100 %).
    double displayScale = 1.0;
    TuningZero zero = TuningZero::Plain;
};

[[nodiscard]] const std::array<TuningControl, TuningControlCount>& TuningControls();
[[nodiscard]] const TuningControl& Control(TuningControlId id);
[[nodiscard]] std::optional<TuningControlId> FindTuningControl(Data::Maps::LookValue value);

// The value when it is a finite number inside the control's range; nothing otherwise.
[[nodiscard]] std::optional<double> ValidTuningValue(const TuningControl& control, double value);
// The nearest step (counted from the minimum) inside the range; a non-finite value gives the minimum.
[[nodiscard]] double SnapToStep(const TuningControl& control, double value);
// The value `steps` steps away (negative: down), on the step grid and inside the range. A value
// between two steps moves to the next one in that direction first.
[[nodiscard]] double StepTuningValue(const TuningControl& control, double value, int steps);
// Where a pointer at `fraction` (0 = minimum, 1 = maximum) of the track puts the value, on a step.
[[nodiscard]] double TuningValueFromFraction(const TuningControl& control, double fraction);
// The track as a gauge with one level per step: how many levels it has after the first.
[[nodiscard]] int TuningStepCount(const TuningControl& control);
// The value a pointer at step `level` (0..TuningStepCount()) sets, when it differs from `current`.
// `current` is compared exactly, not by its nearest level: a value between steps (an inherited one)
// changes to the step the pointer is on even when that step is the value's nearest one.
[[nodiscard]] std::optional<double> TuningPointerValue(const TuningControl& control, double current, int level);
// Where the value is on the track, 0 to 1.
[[nodiscard]] double TuningFraction(const TuningControl& control, double value);
// The shown number without its unit: as many decimals as the step has, more (up to three) for a
// value between steps, so an inherited value is shown as it is.
[[nodiscard]] std::string FormatTuningValue(const TuningControl& control, double value);

// Which look values a developer changed this session (`$look set`, `$look set map`, the
// DevEditor), in the default or in one world's entry: there they win over the player's values until
// reset.
struct DeveloperLookEdits
{
    using Flags = std::array<bool, Data::Maps::LookValueCount>;
    Flags defaults{};
    std::map<int, Flags> worlds;

    // A `$look set` (no world) or `$look set map` on `world`.
    void Mark(Data::Maps::LookValue value, std::optional<int> world);
    // A `$look reset <name>` on `world`: the default and that world's entry, as the reset restores.
    void Clear(Data::Maps::LookValue value, int world);
    // True when the developer's value is what `profile` gives `world`: the world's edited entry, or an
    // edited default that no entry of the world overrides.
    [[nodiscard]] bool Covers(const Data::Maps::LookProfileData& profile, int world, Data::Maps::LookValue value) const;
    bool operator==(const DeveloperLookEdits&) const = default;
};

// The tuning groups whose player values apply, by Render::Look::TuningGroup. A group applies while
// its feature is in effect, so a feature switched off (or the Classic look) keeps the player's values
// without showing them; dynamic lights also need Enhanced lighting.
using TuningGroupSet = std::array<bool, TuningGroupCount>;
[[nodiscard]] TuningGroupSet TuningGroupsApplying(const LookFeatures& features);

// The groups whose player values apply once the renderer has taken `requested`: a feature the
// renderer refused (`enhancedLightingActive`, `postProcessingActive` false) applies none, as one the
// player switched off.
[[nodiscard]] TuningGroupSet TuningGroupsInEffect(const LookFeatures& requested, bool enhancedLightingActive,
                                                  bool postProcessingActive);

// Where a look's features go (Render/Look/LookPresetSetting.cpp gives the renderer's).
struct LookFeatureAppliers
{
    std::function<void(bool)> textureFiltering;
    // These two return whether the renderer draws the feature now (false when it refused it).
    std::function<bool(bool)> enhancedLighting;
    std::function<bool(bool)> postProcessing;
    std::function<void(bool)> dynamicLights;
    // Which groups' player values apply (Render::Look::SetPersonalTuningGroups).
    std::function<void(const TuningGroupSet&)> tuningGroups;
    // Rebuilds the map light, which reads the look values (the highlight knee).
    std::function<void(bool enabled, int strengthPercent)> lightMood;
    std::function<void(bool)> antialiasing;
};

// Applies `requested`: Enhanced lighting before dynamic lights, which only act while it is on; then
// the player's groups from what the renderer accepted, before the light mood is rebuilt from them.
void ApplyLookFeatures(const LookFeatures& requested, const LookFeatureAppliers& appliers);

// The effective values on `world`: built-in, the profile's default, the world's entry, then the
// player's values of the groups in `applying`, except where a developer's value applies.
[[nodiscard]] Data::Maps::LookValues ResolveLookValues(const Data::Maps::LookProfileData& profile, int world,
                                                       const Data::Maps::LookOverrides& personal,
                                                       const DeveloperLookEdits& developerEdits,
                                                       const TuningGroupSet& applying);
// True when the player's value of `value` applies on `world` (it exists, its group applies and no
// developer's value does).
[[nodiscard]] bool PersonalValueApplies(const Data::Maps::LookProfileData& profile, int world,
                                        const Data::Maps::LookOverrides& personal,
                                        const DeveloperLookEdits& developerEdits, const TuningGroupSet& applying,
                                        Data::Maps::LookValue value);

} // namespace Render::Look
