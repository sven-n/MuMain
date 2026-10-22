#include "stdafx.h"

#include "Render/Lighting/LightMoodSetting.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Look/LookProfileSetting.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <utility>

namespace Render::Lighting::LightMood
{
namespace
{
constexpr int NoWorld = std::numeric_limits<int>::min();

struct State
{
    Data::Maps::LightMoodData data;
    bool enabled = false;
    int strength = MaxStrength;
    // The world and look profile revision `scales` belong to; NoWorld recomputes them.
    int world = NoWorld;
    std::uint32_t lookRevision = 0;
    Scales scales;
};

State s_state;

// The login and character selection scenes stage their own light; before the
// first of them is loaded there is no world (-1).
bool IsWorldMap(int world)
{
    return world >= 0 && world != WD_55LOGINSCENE && world != WD_73NEW_LOGIN_SCENE &&
           world != WD_74NEW_CHARACTER_SCENE && world != WD_77NEW_LOGIN_SCENE && world != WD_78NEW_CHARACTER_SCENE;
}

void LogScales(const char* event)
{
    const Scales& scales = s_state.scales;
    mu::log::Get("render")->info("Light mood {} in world {} ({}, strength {}%): base {} figures {} dynamic gain {} "
                                 "background {} sun {} highlight knee {} shadow keep {} shade cool {} bright {}",
                                 event, s_state.world, s_state.enabled ? "on" : "off", s_state.strength, scales.base,
                                 scales.figures, scales.dynamicGain, scales.background, scales.sun,
                                 scales.highlightKnee, scales.shadowKeep, scales.shadeCool, scales.brightScale);
}

void Recompute(int world)
{
    s_state.world = world;
    s_state.lookRevision = Look::Revision();
    s_state.scales = s_state.enabled && IsWorldMap(world) ? Blend(s_state.data.ForWorld(world), s_state.strength,
                                                                  Look::Value(Data::Maps::LookValue::HighlightKnee))
                                                          : Scales{};
}

void RebuildStaticLight()
{
    Recompute(gMapManager.WorldActive);
    const auto start = std::chrono::steady_clock::now();
    CreateTerrainLight();
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    LogScales("set");
    mu::log::Get("render")->info("Light mood rebuilt the static map light in {:.2f} ms", elapsed.count());
}
} // namespace

void SetMoodData(Data::Maps::LightMoodData data)
{
    s_state.data = std::move(data);
    RebuildStaticLight();
}

void SetMoodSwitch(bool enabled, int strengthPercent)
{
    const int strength = std::clamp(strengthPercent, MinStrength, MaxStrength);
    if (enabled == s_state.enabled && strength == s_state.strength)
    {
        return;
    }
    s_state.enabled = enabled;
    s_state.strength = strength;
    RebuildStaticLight();
}

void UseMood(Data::Maps::LightMoodData data, bool enabled, int strengthPercent)
{
    s_state.data = std::move(data);
    s_state.enabled = enabled;
    s_state.strength = std::clamp(strengthPercent, MinStrength, MaxStrength);
    RebuildStaticLight();
}

const Scales& Active()
{
    const int world = gMapManager.WorldActive;
    if (world != s_state.world || s_state.lookRevision != Look::Revision())
    {
        Recompute(world);
    }
    return s_state.scales;
}

void LogMapLight()
{
    if (s_state.enabled)
    {
        (void)Active();
        LogScales("applied");
    }
}

void CorrectFigureTerrainLight(float x, float y, float light[3])
{
    const Scales& scales = Active();
    if (scales.figures == scales.base)
    {
        return;
    }
    vec3_t correction;
    RequestFigureStaticCorrection(x, y, correction);
    for (int i = 0; i < 3; ++i)
        light[i] += correction[i];
}

void ScaleOwnHeroLight(float x, float y, const float classicLight[3], float light[3])
{
    const Scales& scales = Active();
    ScaleFixedFigureLight(scales, classicLight, light,
                          scales.shadowKeep > 0.f ? RequestStaticLightBrightWeight(x, y) : 0.f);
}

} // namespace Render::Lighting::LightMood
