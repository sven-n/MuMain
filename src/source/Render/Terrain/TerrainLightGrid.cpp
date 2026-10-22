#include "Render/Terrain/TerrainLightGrid.h"

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace Render::Terrain::LightGrid
{
namespace
{
constexpr int kColorComponents = 3;

int GridIndex(int x, int y)
{
    return y * TERRAIN_SIZE + x;
}

int RepeatedGridIndex(int x, int y)
{
    return (y & TERRAIN_SIZE_MASK) * TERRAIN_SIZE + (x & TERRAIN_SIZE_MASK);
}

// Adds color * falloff to one vertex and clamps it at 0, as AddLight does for every vertex.
void AddToVertex(float* vertex, const float* color, float falloff)
{
    for (int i = 0; i < kColorComponents; ++i)
    {
        vertex[i] += color[i] * falloff;
        if (vertex[i] < 0.f)
        {
            vertex[i] = 0.f;
        }
    }
}

bool HasDarkeningPart(const Lighting::DynamicLight& light)
{
    return light.color[0] < 0.f || light.color[1] < 0.f || light.color[2] < 0.f;
}

// The static vertex (x, y) plus the darkening parts of the lights, in their recorded order.
void DarkenedStaticVertex(const float (*staticGrid)[3], std::span<const Lighting::DynamicLight> lights, int x, int y,
                          float* vertex)
{
    std::copy_n(staticGrid[GridIndex(x, y)], kColorComponents, vertex);
    const auto vertexX = static_cast<float>(x);
    const auto vertexY = static_cast<float>(y);
    for (const Lighting::DynamicLight& light : lights)
    {
        if (!HasDarkeningPart(light))
        {
            continue;
        }
        // Same expressions as AddLight, so the result matches the dynamic grid bit for bit.
        const float xd = light.position[0] / TERRAIN_SCALE - vertexX;
        const float yd = light.position[1] / TERRAIN_SCALE - vertexY;
        const float falloff = Falloff(sqrtf(xd * xd + yd * yd), light.range);
        if (falloff > 0.f)
        {
            const float darkening[kColorComponents] = {(std::min)(light.color[0], 0.f), (std::min)(light.color[1], 0.f),
                                                       (std::min)(light.color[2], 0.f)};
            AddToVertex(vertex, darkening, falloff);
        }
    }
}

void Interpolate(const float* v1, const float* v2, const float* v3, const float* v4, float xd, float yd, float* light)
{
    for (int i = 0; i < kColorComponents; ++i)
    {
        const float left = v1[i] + (v4[i] - v1[i]) * yd;
        const float right = v2[i] + (v3[i] - v2[i]) * yd;
        light[i] = left + (right - left) * xd;
    }
}

// The grid cell a world position lies in, and the position inside it (0..1 per axis).
struct Cell
{
    int x;
    int y;
    float xd;
    float yd;
};

// nullopt outside the sampled grid.
std::optional<Cell> LocateCell(float x, float y)
{
    const float xf = x / TERRAIN_SCALE;
    const float yf = y / TERRAIN_SCALE;
    const int xi = static_cast<int>(xf);
    const int yi = static_cast<int>(yf);
    if (xi < 0 || yi < 0 || xi > TERRAIN_SIZE_MASK - 1 || yi > TERRAIN_SIZE_MASK - 1)
    {
        return std::nullopt;
    }
    return Cell{xi, yi, xf - static_cast<float>(xi), yf - static_cast<float>(yi)};
}
} // namespace

float Falloff(float distanceInCells, int range)
{
    const auto rangeInCells = static_cast<float>(range);
    return (rangeInCells - distanceInCells) / rangeInCells;
}

void AddLight(float (*grid)[3], float x, float y, const float* color, int range)
{
    const float xf = x / TERRAIN_SCALE;
    const float yf = y / TERRAIN_SCALE;
    const int xi = static_cast<int>(xf);
    const int yi = static_cast<int>(yf);
    for (int syi = yi - range; syi <= yi + range; ++syi)
    {
        const float yd = yf - static_cast<float>(syi);
        for (int sxi = xi - range; sxi <= xi + range; ++sxi)
        {
            const float xd = xf - static_cast<float>(sxi);
            const float falloff = Falloff(sqrtf(xd * xd + yd * yd), range);
            if (falloff > 0.f)
            {
                AddToVertex(grid[RepeatedGridIndex(sxi, syi)], color, falloff);
            }
        }
    }
}

void AddDynamicLight(float (*grid)[3], Lighting::DynamicLightList& frameLights, float x, float y, const float* color,
                     int range, float terrainHeight)
{
    AddLight(grid, x, y, color, range);
    if (range > 0)
    {
        frameLights.Add(MakeDynamicLight(x, y, color, range, terrainHeight));
    }
}

namespace
{
void ApplyCall(const FrameLightTarget& target, const Lighting::LightCall& call)
{
    float color[kColorComponents];
    for (int k = 0; k < kColorComponents; ++k)
        color[k] = call.color[k] > 0.f ? call.color[k] * target.dynamicGain : call.color[k];
    AddDynamicLight(target.grid, *target.frameLights, call.x, call.y, color, call.range, target.height(call.x, call.y));
}
} // namespace

void RecordFrameLight(const FrameLightTarget& target, const Lighting::LightCall& call)
{
    Lighting::DynamicLightHistory& history = *target.history;
    history.SyncWorld(target.world);
    const bool drawing = history.IsDrawing();
    const bool lateDarkening = drawing && (std::min)({call.color[0], call.color[1], call.color[2]}) < 0.f;
    if (!lateDarkening && history.Record(call))
        return;
    if (!drawing)
    {
        // The queued calls must be applied before the refused one: terrain clamps after every
        // call, and the point list retains its first entries. Never reverse those orders.
        for (const auto& queued : history.DrainToClassic())
            ApplyCall(target, queued);
    }
    // While drawing, the queued calls stay for the next frame (they are not reordered into this
    // one); this call joins this frame now, after the calls that already did.
    ApplyCall(target, call);
}

void ReplayFrameLights(const FrameLightTarget& target, float seconds, const Lighting::LightHistorySettings& settings)
{
    Lighting::DynamicLightHistory& history = *target.history;
    history.SyncWorld(target.world);
    const auto lights = history.Take(seconds, settings);
    for (std::size_t i = 0; i < lights.size(); ++i)
    {
        if (i == history.CalledOut())
            target.frameLights->MarkDisplaceable();
        ApplyCall(target, lights[i]);
    }
    target.frameLights->EndDisplaceable();
}

void Sample(const float (*grid)[3], float x, float y, float* light)
{
    const std::optional<Cell> cell = LocateCell(x, y);
    if (!cell)
    {
        std::fill_n(light, kColorComponents, 0.f);
        return;
    }
    Interpolate(grid[GridIndex(cell->x, cell->y)], grid[GridIndex(cell->x + 1, cell->y)],
                grid[GridIndex(cell->x + 1, cell->y + 1)], grid[GridIndex(cell->x, cell->y + 1)], cell->xd, cell->yd,
                light);
}

void SampleStatic(const float (*staticGrid)[3], std::span<const Lighting::DynamicLight> lights, float x, float y,
                  float* light)
{
    const std::optional<Cell> cell = LocateCell(x, y);
    if (!cell)
    {
        std::fill_n(light, kColorComponents, 0.f);
        return;
    }
    float v1[kColorComponents];
    float v2[kColorComponents];
    float v3[kColorComponents];
    float v4[kColorComponents];
    DarkenedStaticVertex(staticGrid, lights, cell->x, cell->y, v1);
    DarkenedStaticVertex(staticGrid, lights, cell->x + 1, cell->y, v2);
    DarkenedStaticVertex(staticGrid, lights, cell->x + 1, cell->y + 1, v3);
    DarkenedStaticVertex(staticGrid, lights, cell->x, cell->y + 1, v4);
    Interpolate(v1, v2, v3, v4, cell->xd, cell->yd, light);
}

void SampleBrightening(std::span<const Lighting::DynamicLight> lights, float x, float y, float* light)
{
    std::fill_n(light, kColorComponents, 0.f);
    for (const Lighting::DynamicLight& dynamicLight : lights)
    {
        const float xd = (dynamicLight.position[0] - x) / TERRAIN_SCALE;
        const float yd = (dynamicLight.position[1] - y) / TERRAIN_SCALE;
        const float falloff = Falloff(sqrtf(xd * xd + yd * yd), dynamicLight.range);
        if (falloff <= 0.f)
        {
            continue;
        }
        for (int i = 0; i < kColorComponents; ++i)
        {
            light[i] += (std::max)(dynamicLight.color[i], 0.f) * falloff;
        }
    }
}

Lighting::DynamicLight MakeDynamicLight(float x, float y, const float* color, int range, float terrainHeight)
{
    Lighting::DynamicLight light{};
    light.position[0] = x;
    light.position[1] = y;
    light.position[2] = terrainHeight + Lighting::kDynamicLightHeightAboveTerrain;
    std::copy_n(color, kColorComponents, light.color);
    light.range = range;
    light.radius = static_cast<float>(range) * TERRAIN_SCALE;
    return light;
}

} // namespace Render::Terrain::LightGrid
