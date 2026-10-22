#include <doctest.h>

#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include "Render/Terrain/TerrainHeightGrid.h"

namespace
{

constexpr float kSpecial = 1201.0f;

struct TerrainMap
{
    std::vector<float> heights = std::vector<float>(Render::kTerrainCells, 0.0f);
    std::vector<std::uint16_t> walls = std::vector<std::uint16_t>(Render::kTerrainCells, 0);

    float& Height(int x, int y) { return heights[y * Render::kTerrainSize + x]; }
    std::uint16_t& Wall(int x, int y) { return walls[y * Render::kTerrainSize + x]; }
    float Sample(float x, float y) const { return Render::SampleTerrainHeight(heights.data(), walls.data(), x, y, kSpecial); }
};

} // namespace

TEST_CASE("terrain height interpolates the four corners of a cell [render][terrain_height]")
{
    TerrainMap map;
    map.Height(1, 1) = 10.0f;
    map.Height(2, 1) = 30.0f;
    map.Height(1, 2) = 50.0f;
    map.Height(2, 2) = 70.0f;

    CHECK(map.Sample(100.0f, 100.0f) == doctest::Approx(10.0f));
    CHECK(map.Sample(150.0f, 100.0f) == doctest::Approx(20.0f));
    CHECK(map.Sample(100.0f, 150.0f) == doctest::Approx(30.0f));
    CHECK(map.Sample(150.0f, 150.0f) == doctest::Approx(40.0f));
}

TEST_CASE("terrain height edge rules [render][terrain_height]")
{
    TerrainMap map;
    map.Height(0, 10) = 80.0f;
    map.Wall(5, 5) = Render::kTerrainWallHeight | 0x0001;
    map.Wall(4, 11) = Render::kTerrainWallHeight;

    CHECK(map.Sample(-1.0f, 500.0f) == 0.0f);
    CHECK(map.Sample(500.0f, -1.0f) == 0.0f);
    CHECK(map.Sample(550.0f, 550.0f) == kSpecial);
    // Corners wrap: the last column's right corner is column 0.
    CHECK(map.Sample(25550.0f, 1000.0f) == doctest::Approx(40.0f));
    // The cell lookup does not wrap: column 260 of row 10 is column 4 of row 11.
    CHECK(map.Sample(26050.0f, 1050.0f) == kSpecial);
    CHECK(map.Sample(1000.0f, 25600.0f) == kSpecial);
}

TEST_CASE("the shader's grid gives the same heights as the map [render][terrain_height]")
{
    TerrainMap map;
    std::mt19937 random(7);
    std::uniform_real_distribution<float> height(-500.0f, 1000.0f);
    for (int i = 0; i < Render::kTerrainCells; ++i)
    {
        map.heights[i] = height(random);
        map.walls[i] = static_cast<std::uint16_t>(random() % 7 == 0 ? Render::kTerrainWallHeight : random() & 0xFFBF);
    }

    auto grid = std::make_unique<Render::TerrainHeightGrid>();
    Render::BuildTerrainHeightGrid(map.heights.data(), map.walls.data(), *grid);

    for (int i = 0; i < Render::kTerrainCells; ++i)
    {
        const bool special = ((grid->specialCells[i >> 5] >> (i & 31)) & 1u) != 0;
        REQUIRE(special == ((map.walls[i] & Render::kTerrainWallHeight) == Render::kTerrainWallHeight));
        REQUIRE(grid->heights[i] == map.heights[i]);
    }

    std::uniform_real_distribution<float> position(-200.0f, 26200.0f);
    for (int i = 0; i < 100000; ++i)
    {
        const float x = position(random);
        const float y = position(random);
        REQUIRE(Render::SampleTerrainHeightGrid(*grid, x, y, kSpecial) == doctest::Approx(map.Sample(x, y)).epsilon(1e-6));
    }
}
