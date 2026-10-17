#include "stdafx.h"

#include "BenchScene.h"

#include "Render/Look/LookPresetSetting.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Scenes/MainScene.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Options/OptionWindow.h"

namespace Core::Benchmark::Scene
{
namespace
{
// What the run found, which a segment's -1 levels and the restore go back to.
Render::Look::FeatureOverrides s_overridesBeforeRun;
int s_antialiasingBeforeRun = 0;
int s_anisotropyBeforeRun = 0;
int s_anisotropyApplied = 0;
} // namespace

SceneConfig CaptureCurrentConfig()
{
    SceneConfig config;
    config.disableEffects = IsEffectsDisabledDebug();
    config.disableAllEffectsOption = g_pOption && !g_pOption->GetRenderAllEffects();
    config.disableSprites = IsSpritesDisabledDebug();
    config.disableParticles = IsParticlesDisabledDebug();
    config.disableSkillEffectModels = IsSkillEffectModelsDisabledDebug();
    config.disableJoints = IsJointsDisabledDebug();
    config.disableBoids = IsBoidsDisabledDebug();
    config.disableWingShadow = IsWingShadowDisabledDebug();
    config.disableWingExtraLayers = IsWingExtraLayersDisabledDebug();
    config.disableBodyShadows = IsBodyShadowsDisabledDebug();

    s_overridesBeforeRun = Render::Look::SessionOverrides();
    s_antialiasingBeforeRun = mu::GetRenderer().GetAntialiasingSamples();
    s_anisotropyBeforeRun = Bitmaps.RequestedAnisotropy();
    s_anisotropyApplied = s_anisotropyBeforeRun;
    return config;
}

void ApplyConfig(const SceneConfig& config)
{
    SetDisableEffects(config.disableEffects);
    if (g_pOption)
        g_pOption->SetRenderAllEffects(!config.disableAllEffectsOption);
    SetDisableSprites(config.disableSprites);
    SetDisableParticles(config.disableParticles);
    SetDisableSkillEffectModels(config.disableSkillEffectModels);
    SetDisableJoints(config.disableJoints);
    SetDisableBoids(config.disableBoids);
    SetDisableWingShadow(config.disableWingShadow);
    SetDisableWingExtraLayers(config.disableWingExtraLayers);
    SetDisableBodyShadows(config.disableBodyShadows);

    // Only an "off" is set, so what the run found otherwise stays in place.
    Render::Look::FeatureOverrides overrides = s_overridesBeforeRun;
    const auto switchOff = [](std::optional<bool>& feature, bool off)
    {
        if (off)
            feature = false;
    };
    switchOff(overrides.textureFiltering, config.lookTextureFilteringOff);
    switchOff(overrides.enhancedLighting, config.lookLightingOff);
    switchOff(overrides.dynamicLights, config.lookDynamicLightsOff || config.lookLightingOff);
    switchOff(overrides.lightMood, config.lookLightMoodOff);
    switchOff(overrides.postProcessing, config.lookPostProcessingOff);
    if (!(overrides == Render::Look::SessionOverrides()))
        Render::Look::ReplaceSessionOverrides(overrides);

    // The renderer rebuilds its targets only when the count changes, at the next BeginFrame.
    mu::GetRenderer().SetAntialiasingSamples(config.antialiasingSamples < 0 ? s_antialiasingBeforeRun
                                                                            : config.antialiasingSamples);

    // A changed level re-creates every mipmapped texture's sampler, so it is set only on a change.
    const int anisotropy = config.anisotropy < 0 ? s_anisotropyBeforeRun : config.anisotropy;
    if (anisotropy != s_anisotropyApplied)
    {
        Bitmaps.SetAnisotropy(anisotropy);
        s_anisotropyApplied = anisotropy;
    }

    mu::GetRenderer().SetDrawReorderingEnabled(!config.disableDrawReordering);
}
} // namespace Core::Benchmark::Scene
