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
    const std::optional<FrameSunDirections> sun = FrameSun();
    return SceneFlag == MAIN_SCENE && DynamicLightsActive() && sun &&
           Render::Look::Value(Data::Maps::LookValue::SunShadowStrength) > 0.0 &&
           mu::GetRenderer().SunShadowsAvailable();
}

namespace
{
std::optional<FrameSunDirections> s_frameSun;
} // namespace

std::optional<FrameSunDirections> FrameSun()
{
    return s_frameSun;
}

void UpdateFrameSun()
{
    s_frameSun.reset();
    if (!DynamicLightsActive())
    {
        return;
    }
    const float fall = gMapManager.InBattleCastle() ? kSunShadowFallBattleCastle : kSunShadowFall;
    s_frameSun = FrameSunDirections{SunDirection(Render::Look::Value(Data::Maps::LookValue::GroundShadowAngle), fall),
                                    SunDirection(kBakedSunShadowAngle, fall)};
}

glm::vec3 FollowFrameSun(const glm::vec3& baked)
{
    return s_frameSun ? FollowSun(baked, s_frameSun->baked, s_frameSun->sun) : baked;
}

} // namespace Render::Lighting
