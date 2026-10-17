#include "Render/Look/LookPreset.h"

#include "Render/Lighting/LightMoodScales.h"

#include <utility>

namespace Render::Look
{
namespace
{
// "on" or "off" after `prefix`, nothing else.
std::optional<bool> ParseSwitch(std::wstring_view command, std::wstring_view prefix)
{
    if (!command.starts_with(prefix))
    {
        return std::nullopt;
    }
    const std::wstring_view argument = command.substr(prefix.size());
    if (argument == L"on")
    {
        return true;
    }
    if (argument == L"off")
    {
        return false;
    }
    return std::nullopt;
}

void AppendSwitch(std::string& text, const char* name, const std::optional<bool>& value)
{
    if (value)
    {
        text += (text.empty() ? "" : ", ") + std::string(name) + (*value ? " on" : " off");
    }
}
} // namespace

LookFeatures PresetFeatures(LookPreset preset)
{
    const bool enhanced = preset == LookPreset::Enhanced;
    LookFeatures features;
    features.textureFiltering = enhanced;
    features.enhancedLighting = enhanced;
    features.dynamicLights = enhanced;
    features.lightMood = enhanced;
    features.postProcessing = enhanced;
    features.lightMoodStrength = Lighting::LightMood::MaxStrength;
    features.antialiasing = enhanced;
    return features;
}

bool FeatureOverrides::Any() const
{
    return textureFiltering || enhancedLighting || dynamicLights || lightMood || postProcessing || lightMoodStrength;
}

void FeatureOverrides::Merge(const FeatureOverrides& change)
{
    const auto merge = [](auto& target, const auto& source)
    {
        if (source)
        {
            target = source;
        }
    };
    merge(textureFiltering, change.textureFiltering);
    merge(enhancedLighting, change.enhancedLighting);
    merge(dynamicLights, change.dynamicLights);
    merge(lightMood, change.lightMood);
    merge(postProcessing, change.postProcessing);
    merge(lightMoodStrength, change.lightMoodStrength);
}

void FeatureOverrides::ApplyTo(LookFeatures& features) const
{
    features.textureFiltering = textureFiltering.value_or(features.textureFiltering);
    features.enhancedLighting = enhancedLighting.value_or(features.enhancedLighting);
    features.dynamicLights = dynamicLights.value_or(features.dynamicLights);
    features.lightMood = lightMood.value_or(features.lightMood);
    features.postProcessing = postProcessing.value_or(features.postProcessing);
    features.lightMoodStrength = lightMoodStrength.value_or(features.lightMoodStrength);
}

std::string FeatureOverrides::Describe() const
{
    std::string text;
    AppendSwitch(text, "texfilter", textureFiltering);
    AppendSwitch(text, "lighting", enhancedLighting);
    AppendSwitch(text, "dynlights", dynamicLights);
    AppendSwitch(text, "lightmood", lightMood);
    AppendSwitch(text, "post", postProcessing);
    if (lightMoodStrength)
    {
        text += (text.empty() ? "" : ", ") + std::string("lightmood strength ") + std::to_string(*lightMoodStrength);
    }
    return text.empty() ? "none" : text;
}

LookPresetController::LookPresetController(Hooks hooks, FeatureOverrides choices)
    : m_hooks(std::move(hooks)), m_choices(std::move(choices))
{
}

void LookPresetController::ApplyPreset(LookPreset preset)
{
    m_preset = preset;
    m_overrides = {};
    if (m_hooks.persist)
    {
        m_hooks.persist(preset);
    }
    if (m_hooks.apply)
    {
        m_hooks.apply(Effective());
    }
}

void LookPresetController::Override(const FeatureOverrides& change)
{
    m_overrides.Merge(change);
    if (m_hooks.apply)
    {
        m_hooks.apply(Effective());
    }
}

void LookPresetController::ReplaceOverrides(const FeatureOverrides& overrides)
{
    m_overrides = overrides;
    if (m_hooks.apply)
    {
        m_hooks.apply(Effective());
    }
}

void LookPresetController::SetChoices(const FeatureOverrides& choices)
{
    m_choices = choices;
    if (m_hooks.apply)
    {
        m_hooks.apply(Effective());
    }
}

LookFeatures LookPresetController::Effective() const
{
    LookFeatures features = PresetFeatures(m_preset);
    if (m_preset == LookPreset::Enhanced)
    {
        m_choices.ApplyTo(features);
    }
    m_overrides.ApplyTo(features);
    return features;
}

std::optional<LookPreset> ParseLookPresetCommand(std::wstring_view command)
{
    constexpr std::wstring_view Prefix = L"$look ";
    if (!command.starts_with(Prefix))
    {
        return std::nullopt;
    }
    return GameConfigValidation::ParseLook(command.substr(Prefix.size()));
}

std::optional<FeatureOverrides> ParseFeatureOverrideCommand(std::wstring_view command)
{
    struct SwitchCommand
    {
        std::wstring_view prefix;
        std::optional<bool> FeatureOverrides::* feature;
    };
    static constexpr SwitchCommand Switches[] = {
        {L"$texfilter ", &FeatureOverrides::textureFiltering},
        {L"$lighting ", &FeatureOverrides::enhancedLighting},
        {L"$dynlights ", &FeatureOverrides::dynamicLights},
        {L"$post ", &FeatureOverrides::postProcessing},
    };
    FeatureOverrides overrides;
    for (const SwitchCommand& entry : Switches)
    {
        const std::optional<bool> enabled = ParseSwitch(command, entry.prefix);
        if (enabled)
        {
            overrides.*entry.feature = enabled;
            return overrides;
        }
    }
    constexpr std::wstring_view MoodPrefix = L"$lightmood ";
    if (!command.starts_with(MoodPrefix))
    {
        return std::nullopt;
    }
    const std::optional<Lighting::LightMood::Command> mood =
        Lighting::LightMood::ParseCommandArgument(command.substr(MoodPrefix.size()));
    if (!mood)
    {
        return std::nullopt;
    }
    overrides.lightMood = mood->enabled;
    overrides.lightMoodStrength = mood->strength;
    return overrides;
}

} // namespace Render::Look
