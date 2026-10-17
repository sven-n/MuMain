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

LookPresetController& Controller()
{
    static LookPresetController controller({SaveLook, ApplyFeatures});
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
