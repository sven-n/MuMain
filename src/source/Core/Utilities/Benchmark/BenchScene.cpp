#include "stdafx.h"

#include "BenchScene.h"

#include "Scenes/MainScene.h"
#include "UI/NewUI/NewUISystem.h"

namespace Core::Benchmark::Scene
{
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
        return config;
    }

    void ApplyConfig(const SceneConfig& config)
    {
        SetDisableEffects(config.disableEffects);
        if (g_pOption) g_pOption->SetRenderAllEffects(!config.disableAllEffectsOption);
        SetDisableSprites(config.disableSprites);
        SetDisableParticles(config.disableParticles);
        SetDisableSkillEffectModels(config.disableSkillEffectModels);
        SetDisableJoints(config.disableJoints);
        SetDisableBoids(config.disableBoids);
        SetDisableWingShadow(config.disableWingShadow);
        SetDisableWingExtraLayers(config.disableWingExtraLayers);
    }
}
