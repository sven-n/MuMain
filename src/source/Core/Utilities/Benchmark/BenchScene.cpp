#include "BenchScene.h"

#include "Scenes/MainScene.h"

namespace Core::Benchmark::Scene
{
    SceneConfig CaptureCurrentConfig()
    {
        SceneConfig config;
        config.disableEffects = IsEffectsDisabledDebug();
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
        SetDisableSprites(config.disableSprites);
        SetDisableParticles(config.disableParticles);
        SetDisableSkillEffectModels(config.disableSkillEffectModels);
        SetDisableJoints(config.disableJoints);
        SetDisableBoids(config.disableBoids);
        SetDisableWingShadow(config.disableWingShadow);
        SetDisableWingExtraLayers(config.disableWingExtraLayers);
    }
}
