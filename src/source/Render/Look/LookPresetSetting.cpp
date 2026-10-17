#include "stdafx.h"

#include "Render/Look/LookPresetSetting.h"

#include "App/Platform/Windows/Winmain.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameConfig/GameConfig.h"
#include "Render/Lighting/DynamicLightsSetting.h"
#include "Render/Lighting/EnhancedLightingSetting.h"
#include "Render/Lighting/LightMoodSetting.h"
#include "Render/PostProcessing/PostProcessingSetting.h"

namespace Render::Look
{
namespace
{
void SaveLook(LookPreset preset)
{
    GameConfig& config = GameConfig::GetInstance();
    config.SetLook(preset);
    config.Save();
}

// Enhanced lighting before dynamic lights, which only act while it is on.
void ApplyFeatures(const LookFeatures& features)
{
    MuSetTextureFilteringEnabled(features.textureFiltering);
    (void)Lighting::SetEnhancedLightingEnabled(features.enhancedLighting);
    Lighting::SetDynamicLightsEnabled(features.dynamicLights);
    Lighting::LightMood::SetMoodSwitch(features.lightMood, features.lightMoodStrength);
    (void)PostProcessing::SetPostProcessingEnabled(features.postProcessing);
    MuSetAntialiasingActive(features.antialiasing);
}

// Only a feature switched off is a choice; one left on follows the preset.
FeatureOverrides SavedChoices()
{
    const GameConfig& config = GameConfig::GetInstance();
    FeatureOverrides choices;
    const auto offOnly = [](bool enabled) { return enabled ? std::nullopt : std::optional<bool>(false); };
    choices.enhancedLighting = offOnly(config.GetEnhancedLighting());
    choices.dynamicLights = offOnly(config.GetDynamicLights());
    choices.lightMood = offOnly(config.GetLightMood());
    choices.postProcessing = offOnly(config.GetPostProcessing());
    return choices;
}

LookPresetController& Controller()
{
    static LookPresetController controller({SaveLook, ApplyFeatures}, SavedChoices());
    return controller;
}
} // namespace

void ApplyLookPreset(LookPreset preset)
{
    Controller().ApplyPreset(preset);
    mu::log::Get("render")->info("Look {}", preset == LookPreset::Enhanced ? "Enhanced" : "Classic");
}

LookPreset ActiveLookPreset()
{
    return Controller().Preset();
}

bool EnhancedFeatureChosen(EnhancedFeature feature)
{
    const GameConfig& config = GameConfig::GetInstance();
    switch (feature)
    {
    case EnhancedFeature::Lighting:
        return config.GetEnhancedLighting();
    case EnhancedFeature::DynamicLights:
        return config.GetDynamicLights();
    case EnhancedFeature::LightMood:
        return config.GetLightMood();
    case EnhancedFeature::PostProcessing:
        return config.GetPostProcessing();
    }
    return true;
}

void ChooseEnhancedFeature(EnhancedFeature feature, bool enabled)
{
    GameConfig& config = GameConfig::GetInstance();
    switch (feature)
    {
    case EnhancedFeature::Lighting:
        config.SetEnhancedLighting(enabled);
        break;
    case EnhancedFeature::DynamicLights:
        config.SetDynamicLights(enabled);
        break;
    case EnhancedFeature::LightMood:
        config.SetLightMood(enabled);
        break;
    case EnhancedFeature::PostProcessing:
        config.SetPostProcessing(enabled);
        break;
    }
    config.Save();
    Controller().SetChoices(SavedChoices());
}

bool HandleLookPresetCommand(std::wstring_view command)
{
    const std::optional<LookPreset> preset = ParseLookPresetCommand(command);
    if (!preset)
    {
        return false;
    }
    ApplyLookPreset(*preset);
    return true;
}

FeatureOverrides SessionOverrides()
{
    return Controller().Overrides();
}

void ReplaceSessionOverrides(const FeatureOverrides& overrides)
{
    Controller().ReplaceOverrides(overrides);
}

#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)
bool HandleFeatureOverrideCommand(std::wstring_view command)
{
    const std::optional<FeatureOverrides> change = ParseFeatureOverrideCommand(command);
    if (!change)
    {
        return false;
    }
    Controller().Override(*change);
    mu::log::Get("render")->info("Look overrides for this session: {}", Controller().Overrides().Describe());
    return true;
}

std::string DescribeFeatureOverrides()
{
    return Controller().Overrides().Describe();
}
#endif

} // namespace Render::Look
