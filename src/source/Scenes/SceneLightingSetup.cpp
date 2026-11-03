#include "stdafx.h"

#include "Scenes/SceneLightingSetup.h"

#include "Camera/CameraState.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Lighting/DynamicLightList.h"
#include "Render/Lighting/DynamicLightSelection.h"
#include "Render/Lighting/DynamicLightsSetting.h"
#include "Render/Lighting/SunShadow.h"
#include "Render/Lighting/EnhancedLightingSetting.h"
#include "Render/Lighting/LightMoodSetting.h"
#include "Render/Lighting/WorldDynamicLights.h"
#include "Render/Look/LookProfileSetting.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/SceneLighting.h"
#include "Scenes/SceneCore.h"
#include "World/MapInfra/MapManager.h"

#include <chrono>
#include <cstddef>

namespace Scenes::Lighting
{
namespace
{
Render::Lighting::DroppedLightReportLimiter s_droppedLightReport;

void ReportDroppedLights(const Render::Lighting::DynamicLightSelection& selection, std::size_t overflow)
{
    if (!s_droppedLightReport.ShouldReport(std::chrono::steady_clock::now()))
    {
        return;
    }
    mu::log::Get("render")->info(
        "Enhanced dynamic lights: {} of {} lights dropped (cap {}), {} beyond the frame's light list",
        selection.dropped, selection.brighteningLights, mu::kMaxDynamicLights, overflow);
}

// Where the capped lights matter most: the player's character in the game, the camera elsewhere.
void SelectionReference(float (&reference)[3])
{
    const float* position = SceneFlag == MAIN_SCENE && Hero ? Hero->Object.Position : g_Camera.Position;
    reference[0] = position[0];
    reference[1] = position[1];
    reference[2] = position[2];
}
} // namespace

void SubmitSceneLighting()
{
    mu::SceneLighting lighting{};
    lighting.cameraPosition[0] = g_Camera.Position[0];
    lighting.cameraPosition[1] = g_Camera.Position[1];
    lighting.cameraPosition[2] = g_Camera.Position[2];
    Render::Lighting::ApplyTuning(lighting);
    lighting.highlightKnee = Render::Lighting::LightMood::Active().highlightKnee;
    if (Render::Lighting::SunShadowsActive() && Hero)
    {
        using Data::Maps::LookValue;
        // The sun stands where the flat character shadows said it did: they fall towards
        // dynamic.shadowAngle, as long as before.
        const glm::vec3 toSun =
            Render::Lighting::SunDirection(Render::Look::Value(LookValue::GroundShadowAngle),
                                           gMapManager.InBattleCastle() ? Render::Lighting::kSunShadowFallBattleCastle
                                                                        : Render::Lighting::kSunShadowFall);
        lighting.sunDirection[0] = toSun.x;
        lighting.sunDirection[1] = toSun.y;
        lighting.sunDirection[2] = toSun.z;
        lighting.sunShadowDarkness =
            Render::Look::Value(LookValue::SunShadowStrength) * Render::Look::Value(LookValue::GroundShadowOpacity);
        lighting.sunShadowSoftness = Render::Look::Value(LookValue::SunShadowSoftness);
        lighting.sunShadowResolution = static_cast<std::uint32_t>(Render::Look::Value(LookValue::SunShadowResolution));
        // Around the hero: the ground the camera sees lies within these heights.
        constexpr float GroundReach = 400.0f;
        lighting.groundLow = Hero->Object.Position[2] - GroundReach;
        lighting.groundHigh = Hero->Object.Position[2] + GroundReach;
    }
    mu::GetRenderer().SetSceneLighting(lighting);
}

void PinWorldDynamicLights()
{
    if (!Render::Lighting::DynamicLightsActive())
    {
        return;
    }
    float reference[3];
    SelectionReference(reference);
    Render::Lighting::FrameWorldDynamicLights().Pin(Render::Lighting::FrameDynamicLights(), reference);
}

void SubmitDynamicLights()
{

    if (!Render::Lighting::DynamicLightsActive())
    {
        mu::GetRenderer().SetDynamicLights({});
        return;
    }

    const Render::Lighting::DynamicLightList& lights = Render::Lighting::FrameDynamicLights();
    const Render::Lighting::WorldDynamicLights& world = Render::Lighting::FrameWorldDynamicLights();
    float reference[3];
    SelectionReference(reference);
    Render::Lighting::DynamicLightSelection selection =
        world.IsPinned() ? Render::Lighting::SelectDynamicLights(world.Selection(), lights.LightsSinceMark(), reference)
                         : Render::Lighting::SelectDynamicLights(lights.Lights(), reference);
    selection.uniform.wrap = Render::Look::Value(Data::Maps::LookValue::DynamicWrap);
    selection.uniform.groundShadowOpacity = Render::Look::Value(Data::Maps::LookValue::GroundShadowOpacity);
    mu::GetRenderer().SetDynamicLights(selection.uniform);

    if (selection.dropped > 0 || lights.Overflow() > 0)
    {
        ReportDroppedLights(selection, lights.Overflow());
    }
}

} // namespace Scenes::Lighting
