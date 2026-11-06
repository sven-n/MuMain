#pragma once

#include <array>
#include <cstdint>

namespace Render
{

// TERRAIN_SIZE, TERRAIN_SCALE and TW_HEIGHT; ZzzLodTerrain.cpp checks they still match.
inline constexpr int kTerrainSize = 256;
inline constexpr int kTerrainCells = kTerrainSize * kTerrainSize;
inline constexpr float kTerrainScale = 100.0f;
inline constexpr std::uint16_t kTerrainWallHeight = 0x0040;

inline int TerrainCornerIndex(int x, int y)
{
    return (y & (kTerrainSize - 1)) * kTerrainSize + (x & (kTerrainSize - 1));
}

// RequestTerrainHeight()'s lookup over the raw map arrays, without its scene check.
inline float SampleTerrainHeight(const float* heights, const std::uint16_t* walls, float x, float y,
                                 float specialHeight)
{
    if (x < 0.0f || y < 0.0f)
        return 0.0f;

    x /= kTerrainScale;
    y /= kTerrainScale;
    const int xi = static_cast<int>(x);
    const int yi = static_cast<int>(y);

    // Unlike the corners below, an x past the last column runs into the next row instead of wrapping.
    const auto index = static_cast<unsigned int>(yi * kTerrainSize + xi);
    if (index >= kTerrainCells || (walls[index] & kTerrainWallHeight) == kTerrainWallHeight)
        return specialHeight;

    const float xd = x - static_cast<float>(xi);
    const float yd = y - static_cast<float>(yi);
    const float h1 = heights[TerrainCornerIndex(xi, yi)];
    const float h2 = heights[TerrainCornerIndex(xi, yi + 1)];
    const float h3 = heights[TerrainCornerIndex(xi + 1, yi)];
    const float h4 = heights[TerrainCornerIndex(xi + 1, yi + 1)];

    const float left = h1 + (h2 - h1) * yd;
    const float right = h3 + (h4 - h3) * yd;
    return left + (right - left) * xd;
}

// The terrain as the body-shadow shader reads it: every cell's height, then one TW_HEIGHT bit per cell.
struct TerrainHeightGrid
{
    std::array<float, kTerrainCells> heights;
    std::array<std::uint32_t, kTerrainCells / 32> specialCells;
};

inline void BuildTerrainHeightGrid(const float* heights, const std::uint16_t* walls, TerrainHeightGrid& grid)
{
    grid.specialCells.fill(0);
    for (int i = 0; i < kTerrainCells; ++i)
    {
        grid.heights[i] = heights[i];
        if ((walls[i] & kTerrainWallHeight) == kTerrainWallHeight)
            grid.specialCells[i >> 5] |= 1u << (i & 31);
    }
}

// SampleTerrainHeight() on the grid; the shader does the same.
inline float SampleTerrainHeightGrid(const TerrainHeightGrid& grid, float x, float y, float specialHeight)
{
    if (x < 0.0f || y < 0.0f)
        return 0.0f;

    x /= kTerrainScale;
    y /= kTerrainScale;
    const int xi = static_cast<int>(x);
    const int yi = static_cast<int>(y);

    const auto index = static_cast<unsigned int>(yi * kTerrainSize + xi);
    if (index >= kTerrainCells || ((grid.specialCells[index >> 5] >> (index & 31)) & 1u) != 0)
        return specialHeight;

    const float xd = x - static_cast<float>(xi);
    const float yd = y - static_cast<float>(yi);
    const float h1 = grid.heights[TerrainCornerIndex(xi, yi)];
    const float h2 = grid.heights[TerrainCornerIndex(xi, yi + 1)];
    const float h3 = grid.heights[TerrainCornerIndex(xi + 1, yi)];
    const float h4 = grid.heights[TerrainCornerIndex(xi + 1, yi + 1)];

    const float left = h1 + (h2 - h1) * yd;
    const float right = h3 + (h4 - h3) * yd;
    return left + (right - left) * xd;
}

} // namespace Render
