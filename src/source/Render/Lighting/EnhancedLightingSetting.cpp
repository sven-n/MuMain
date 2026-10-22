#include "stdafx.h"

#include "Render/Lighting/EnhancedLightingSetting.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Look/LookProfileSetting.h"
#include "Render/Renderer/MuRenderer.h"

namespace Render::Lighting
{
namespace
{
bool s_enhancedLightingActive = false;

bool ApplyToRenderer(bool enabled)
{
    if (mu::GetRenderer().SetEnhancedLighting(enabled))
    {
        s_enhancedLightingActive = enabled;
        mu::log::Get("render")->info("Enhanced lighting {}", enabled ? "on" : "off");
        return true;
    }
    s_enhancedLightingActive = false;
    mu::log::Get("render")->warn("Enhanced lighting requested but unavailable; drawing Classic lighting");
    return false;
}
} // namespace

bool SetEnhancedLightingEnabled(bool enabled)
{
    return ApplyToRenderer(enabled);
}

bool EnhancedLightingActive()
{
    return s_enhancedLightingActive;
}

void ApplyTuning(mu::SceneLighting& lighting)
{
    using Data::Maps::LookValue;
    lighting.diffuseScale = Look::Value(LookValue::DiffuseScale);
    lighting.ambient = Look::Value(LookValue::DiffuseAmbient);
    lighting.diffuseFloor = Look::Value(LookValue::DiffuseFloor);
    lighting.rimStrength = Look::Value(LookValue::RimStrength);
    lighting.rimPower = Look::Value(LookValue::RimPower);
    lighting.specularScale = Look::Value(LookValue::SpecularScale);
    lighting.specularPower = Look::Value(LookValue::SpecularPower);
}

} // namespace Render::Lighting
