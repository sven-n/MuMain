#include "stdafx.h"

#include "Render/Lighting/DynamicLightsSetting.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Lighting/EnhancedLightingSetting.h"
#include "Render/Look/LookProfileSetting.h"
#include "Render/Lighting/SunShadow.h"
#include "Render/Renderer/MuRenderer.h"
#include "World/MapInfra/MapManager.h"
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

std::optional<FrameSunDirections> FrameSun()
{
    if (!DynamicLightsActive())
    {
        return std::nullopt;
    }
    const float fall = gMapManager.InBattleCastle() ? kSunShadowFallBattleCastle : kSunShadowFall;
    return FrameSunDirections{SunDirection(Render::Look::Value(Data::Maps::LookValue::GroundShadowAngle), fall),
                              SunDirection(kBakedSunShadowAngle, fall)};
}

glm::vec3 FollowFrameSun(const glm::vec3& baked)
{
    const std::optional<FrameSunDirections> sun = FrameSun();
    if (!sun)
    {
        return baked;
    }
    // Every model asks with the same vector each frame: the last answer is reused.
    static glm::vec3 s_baked{0.0f}, s_sun{0.0f}, s_followed{0.0f};
    if (baked != s_baked || sun->sun != s_sun)
    {
        s_baked = baked;
        s_sun = sun->sun;
        s_followed = FollowSun(baked, sun->baked, sun->sun);
    }
    return s_followed;
}

} // namespace Render::Lighting
