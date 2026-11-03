#include "stdafx.h"

#include "Render/Lighting/DynamicLightsSetting.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Lighting/EnhancedLightingSetting.h"
#include "Render/Look/LookProfileSetting.h"
#include "Render/Renderer/MuRenderer.h"
#include "Scenes/SceneCore.h"

namespace Render::Lighting
{
namespace
{
bool s_dynamicLightsEnabled = false;
} // namespace

void SetDynamicLightsEnabled(bool enabled)
{
    s_dynamicLightsEnabled = enabled;
    if (enabled && !EnhancedLightingActive())
    {
        mu::log::Get("render")->info("Enhanced dynamic lights on (applied once Enhanced lighting is on)");
        return;
    }
    mu::log::Get("render")->info("Enhanced dynamic lights {}", enabled ? "on" : "off");
}

bool DynamicLightsActive()
{
    return s_dynamicLightsEnabled && EnhancedLightingActive();
}

bool SunShadowsActive()
{
    return SceneFlag == MAIN_SCENE && DynamicLightsActive() &&
           Render::Look::Value(Data::Maps::LookValue::SunShadowStrength) > 0.0 &&
           mu::GetRenderer().SunShadowsAvailable();
}

} // namespace Render::Lighting
