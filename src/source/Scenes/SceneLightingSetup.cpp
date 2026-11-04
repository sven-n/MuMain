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
#include "Render/Lighting/SunShadow.h"
#include "Render/Renderer/SceneLighting.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Scenes/SceneCore.h"

#include <algorithm>
#include <chrono>
#include <cmath>
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

// How far the clouds have drifted at `speed` world units per second, along one fixed wind.
// shortcut: wraps every 2^20 world units (a jump every few hours of play); fine while clouds stay subtle.
static glm::vec2 CloudDrift(float speed)
{
    constexpr float WindDegrees = 30.0f;
    constexpr double Wrap = 1048576.0;
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
    const float along = static_cast<float>(std::fmod(seconds * speed, Wrap));
    const float wind = glm::radians(WindDegrees);
    return glm::vec2(std::cos(wind), std::sin(wind)) * along;
}

void SubmitSceneLighting()
{
    mu::SceneLighting lighting{};
    lighting.cameraPosition[0] = g_Camera.Position[0];
    lighting.cameraPosition[1] = g_Camera.Position[1];
    lighting.cameraPosition[2] = g_Camera.Position[2];
    Render::Lighting::ApplyTuning(lighting);
    lighting.highlightKnee = Render::Lighting::LightMood::Active().highlightKnee;
    const std::optional<Render::Lighting::FrameSunDirections> sun = Render::Lighting::FrameSun();
    if (Render::Lighting::SunShadowsActive() && Hero && sun)
    {
        using Data::Maps::LookValue;
        // The sun stands where the flat character shadows said it did: they fall towards
        // dynamic.shadowAngle, as long as before.
        const glm::vec3 toSun = sun->sun;
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
    if (sun)
    {
        if (!Render::Lighting::SunShadowsActive())
        {
            lighting.sunDirection[0] = sun->sun.x;
            lighting.sunDirection[1] = sun->sun.y;
            lighting.sunDirection[2] = sun->sun.z;
        }
    }
    // Clouds, the sky's tint of the shade and the sun-following rim: only under a sun that casts shadows.
    if (Render::Lighting::SunShadowsActive() && sun)
    {
        using Data::Maps::LookValue;
        lighting.cloudStrength = Render::Look::Value(LookValue::CloudStrength);
        lighting.cloudScale = Render::Look::Value(LookValue::CloudScale);
        lighting.cloudCover = Render::Look::Value(LookValue::CloudCover);
        const glm::vec2 drift = CloudDrift(Render::Look::Value(LookValue::CloudSpeed));
        // The cloud layer stands this high: its shadow falls slanted by the sun.
        constexpr float CloudHeight = 2000.0f;
        const glm::vec2 slant = glm::vec2(sun->sun.x, sun->sun.y) * (CloudHeight / std::max(sun->sun.z, 0.2f));
        lighting.cloudOffset[0] = drift.x + slant.x;
        lighting.cloudOffset[1] = drift.y + slant.y;
        lighting.skyFill = Render::Look::Value(LookValue::SkyFill);
        lighting.rimSunFollow = Render::Look::Value(LookValue::RimSunFollow);
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
    BuildGroundLight();
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
