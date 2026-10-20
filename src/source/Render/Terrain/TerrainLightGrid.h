// TerrainLightGrid.h: the terrain light grid's arithmetic, independent of the game's global grids.
//
// The terrain light is one colour per grid vertex, TERRAIN_SIZE x TERRAIN_SIZE vertices
// TERRAIN_SCALE world units apart. Each frame the game copies the static light (BackTerrainLight)
// into PrimaryTerrainLight and adds its dynamic lights to it; models and terrain read the result.
#pragma once

#include "Render/Lighting/DynamicLight.h"
#include "Render/Lighting/DynamicLightList.h"
#include "Render/Lighting/DynamicLightHistory.h"

#include <span>

namespace Render::Terrain::LightGrid
{

// Share of a light's colour a point gets at distanceInCells from it: (range - d) / range, 0 or
// less outside the range. The same linear falloff applies to the grid and to models.
[[nodiscard]] float Falloff(float distanceInCells, int range);

// Adds color * Falloff to every grid vertex within range cells of world (x, y), clamping each
// vertex at 0 after the addition (Classic AddTerrainLight).
void AddLight(float (*grid)[3], float x, float y, const float* color, int range);

// AddLight, and records the light in the frame's light list as a point light over terrain of the
// given height. A light without range changes no vertex and is not recorded.
void AddDynamicLight(float (*grid)[3], Lighting::DynamicLightList& frameLights, float x, float y, const float* color,
                     int range, float terrainHeight);

using HeightSampler = float (*)(float, float);

// Where the frame's dynamic lights go: the grid and light list of the frame, the light history,
// the world the calls belong to, the light mood's dynamic gain and the terrain height under a light.
struct FrameLightTarget
{
    float (*grid)[3] = nullptr;
    Lighting::DynamicLightList* frameLights = nullptr;
    Lighting::DynamicLightHistory* history = nullptr;
    int world = 0;
    float dynamicGain = 1.f;
    HeightSampler height = nullptr;
};

// Production frame-light orchestration, also used by the grid regression tests. Both first move
// the history to the target's world, so a call never meets another world's queued calls.
// Before the frame is drawn, a call the history refuses first drains the queued calls, then
// applies itself, in call order (Classic order for the rest of that frame). While the frame is
// drawn, the queued calls belong to the next frame; a darkening call or a refused call applies to
// this frame at once, in arrival order.
void RecordFrameLight(const FrameLightTarget& target, const Lighting::LightCall& call);
void ReplayFrameLights(const FrameLightTarget& target, float seconds, const Lighting::LightHistorySettings& settings);

// Bilinear sample of the grid at world (x, y); black outside the grid (Classic RequestTerrainLight).
void Sample(const float (*grid)[3], float x, float y, float* light);

// What Sample() of the dynamic grid would return at world (x, y) if only the darkening parts of
// this frame's lights had been added to the static grid: each light's negative colour components
// are added to the four surrounding static vertices with the grid's falloff and clamping, then
// the vertices are interpolated as in Sample(). Positive components are left out; Enhanced
// dynamic lights apply them per pixel instead.
void SampleStatic(const float (*staticGrid)[3], std::span<const Lighting::DynamicLight> lights, float x, float y,
                  float* light);

// The brightening parts of the lights at world (x, y), with the grid's falloff but evaluated at
// the point itself: the light a model there gets from them without per-pixel lighting.
void SampleBrightening(std::span<const Lighting::DynamicLight> lights, float x, float y, float* light);

// The point light for a light call at world (x, y) over terrain of the given height.
[[nodiscard]] Lighting::DynamicLight MakeDynamicLight(float x, float y, const float* color, int range,
                                                      float terrainHeight);

} // namespace Render::Terrain::LightGrid
