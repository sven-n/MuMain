#pragma once

#include "Data/GameConfig/GameConfigValidation.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>

// The look preset ([Render] Look, Classic or Enhanced). Enhanced switches on texture filtering,
// Enhanced lighting, dynamic model lights, the light mood at full strength, post-processing and
// multisample anti-aliasing (at the [Render] Antialiasing sample count); Classic switches all of
// them off, as the game always looked. The player can switch Enhanced lighting, dynamic lights,
// the light mood and post-processing off one by one (SetChoices). The anisotropy level is a
// setting of its own (it acts only with texture filtering).
//
// Developer builds (MU_ENABLE_CONTROL_SOCKET or _EDITOR) can override one feature, or the light
// mood strength, for the session with the per-feature `$` commands; an override is not saved and
// does not change the preset. Applying a preset (also the current one) clears every override.
namespace Render::Look
{

// The state of the five features the preset switches.
struct LookFeatures
{
    bool textureFiltering = false;
    bool enhancedLighting = false;
    bool dynamicLights = false;
    bool lightMood = false;
    bool postProcessing = false;
    // 0 (Classic) to 100 percent of the map's light mood.
    int lightMoodStrength = 100;
    // Multisampling at the [Render] Antialiasing sample count; off renders one sample per pixel.
    bool antialiasing = false;

    bool operator==(const LookFeatures&) const = default;
};

[[nodiscard]] LookFeatures PresetFeatures(LookPreset preset);

// Developer session overrides; an empty field follows the preset.
struct FeatureOverrides
{
    std::optional<bool> textureFiltering;
    std::optional<bool> enhancedLighting;
    std::optional<bool> dynamicLights;
    std::optional<bool> lightMood;
    std::optional<bool> postProcessing;
    std::optional<int> lightMoodStrength;

    [[nodiscard]] bool Any() const;
    // Sets the fields `change` has.
    void Merge(const FeatureOverrides& change);
    void ApplyTo(LookFeatures& features) const;
    // "postProcessing off, lightMoodStrength 40", or "none".
    [[nodiscard]] std::string Describe() const;
    bool operator==(const FeatureOverrides&) const = default;
};

// The preset and the overrides; how they are saved and applied is up to its hooks.
class LookPresetController
{
public:
    struct Hooks
    {
        // Saves the preset ([Render] Look).
        std::function<void(LookPreset)> persist;
        // Switches the features from the next frame on.
        std::function<void(const LookFeatures&)> apply;
    };

    // `choices`: the Enhanced features the player switched off (SetChoices).
    explicit LookPresetController(Hooks hooks, FeatureOverrides choices = {});

    // Saves the preset, clears every override and applies the preset's features: start-up, the
    // options window's checkbox and `$look classic|enhanced`.
    void ApplyPreset(LookPreset preset);
    // A developer override for the session: applied, not saved, the preset unchanged.
    void Override(const FeatureOverrides& change);
    // Every override at once, for the benchmark's segments and their restore.
    void ReplaceOverrides(const FeatureOverrides& overrides);
    // The player's Enhanced features, saved by the caller: they act on Enhanced only, under the
    // developer overrides, and applying a preset keeps them.
    void SetChoices(const FeatureOverrides& choices);

    [[nodiscard]] LookPreset Preset() const
    {
        return m_preset;
    }
    [[nodiscard]] const FeatureOverrides& Overrides() const
    {
        return m_overrides;
    }
    [[nodiscard]] const FeatureOverrides& Choices() const
    {
        return m_choices;
    }
    [[nodiscard]] LookFeatures Effective() const;

private:
    Hooks m_hooks;
    LookPreset m_preset = LookPreset::Enhanced;
    FeatureOverrides m_overrides;
    FeatureOverrides m_choices;
};

// `$look classic|enhanced` (any case); nothing for any other text.
[[nodiscard]] std::optional<LookPreset> ParseLookPresetCommand(std::wstring_view command);
// `$texfilter on|off`, `$lighting on|off`, `$dynlights on|off`, `$post on|off` and
// `$lightmood on|off|<0-100>`; nothing for any other text.
[[nodiscard]] std::optional<FeatureOverrides> ParseFeatureOverrideCommand(std::wstring_view command);

} // namespace Render::Look
