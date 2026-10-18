#include <doctest.h>

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"
#include "Render/Lighting/DynamicLightList.h"
#include "Render/Lighting/DynamicLightSelection.h"
#include "Render/Terrain/TerrainLightGrid.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <vector>

namespace LightGrid = Render::Terrain::LightGrid;
using Render::Lighting::DynamicLight;
using Render::Lighting::DynamicLightList;

namespace
{
constexpr std::size_t kGridVertices = static_cast<std::size_t>(TERRAIN_SIZE) * TERRAIN_SIZE;

using Grid = std::vector<std::array<float, 3>>;

Grid MakeGrid(float value)
{
    return Grid(kGridVertices, {value, value * 0.5f, value * 0.25f});
}

float (*Data(Grid& grid))[3]
{
    return reinterpret_cast<float (*)[3]>(grid.data());
}

const float (*Data(const Grid& grid))[3]
{
    return reinterpret_cast<const float (*)[3]>(grid.data());
}

// The Classic AddTerrainLight, verbatim but for the buffer type: the grid must not change.
void ClassicAddTerrainLight(float xf, float yf, const float* Light, int Range, float (*Buffer)[3])
{
    auto rf = (float)Range;

    xf = (xf / TERRAIN_SCALE);
    yf = (yf / TERRAIN_SCALE);
    int xi = (int)xf;
    int yi = (int)yf;
    int syi = yi - Range;
    int eyi = yi + Range;
    auto syf = (float)(syi);
    for (; syi <= eyi; syi++, syf += 1.f)
    {
        int sxi = xi - Range;
        int exi = xi + Range;
        auto sxf = (float)(sxi);
        for (; sxi <= exi; sxi++, sxf += 1.f)
        {
            float xd = xf - sxf;
            float yd = yf - syf;
            float lf = (rf - sqrtf(xd * xd + yd * yd)) / rf;
            if (lf > 0.f)
            {
                float* b = &Buffer[((syi & TERRAIN_SIZE_MASK) * TERRAIN_SIZE) + (sxi & TERRAIN_SIZE_MASK)][0];
                for (int i = 0; i < 3; i++)
                {
                    b[i] += Light[i] * lf;
                    if (b[i] < 0.f)
                        b[i] = 0.f;
                }
            }
        }
    }
}

struct LightCall
{
    float x;
    float y;
    float color[3];
    int range;
};

DynamicLight MakeLight(float x, float y, float r, float g, float b, int range)
{
    const float color[3] = {r, g, b};
    return LightGrid::MakeDynamicLight(x, y, color, range, 0.f);
}

void CheckSameLight(const float* actual, const float* expected)
{
    for (int i = 0; i < 3; ++i)
    {
        CHECK(actual[i] == expected[i]);
    }
}
} // namespace

TEST_CASE("terrain light grid adds lights exactly as Classic AddTerrainLight [render][dynamic_lights]")
{
    Grid classic = MakeGrid(0.3f);
    Grid extracted = classic;
    const LightCall calls[] = {
        {1234.5f, 2345.6f, {1.f, 0.6f, 0.2f}, 3},
        {1300.f, 2300.f, {-0.5f, -0.5f, -0.5f}, 4},
        {120.f, 25560.f, {0.4f, 0.4f, 1.0f}, 5}, // wraps around the grid edge
        {5000.f, 5000.f, {0.7f, -0.2f, 0.1f}, 2},
    };
    for (const LightCall& call : calls)
    {
        ClassicAddTerrainLight(call.x, call.y, call.color, call.range, Data(classic));
        LightGrid::AddLight(Data(extracted), call.x, call.y, call.color, call.range);
    }
    CHECK(classic == extracted);
}

TEST_CASE("dynamic terrain lights are recorded as point lights without changing the grid [render][dynamic_lights]")
{
    Grid plain = MakeGrid(0.2f);
    Grid recorded = plain;
    DynamicLightList frameLights;
    const float fire[3] = {1.f, 0.4f, 0.1f};
    const float shade[3] = {-0.3f, -0.3f, -0.3f};

    LightGrid::AddLight(Data(plain), 1530.f, 2210.f, fire, 3);
    LightGrid::AddLight(Data(plain), 1600.f, 2400.f, shade, 2);
    LightGrid::AddDynamicLight(Data(recorded), frameLights, 1530.f, 2210.f, fire, 3, 170.f);
    LightGrid::AddDynamicLight(Data(recorded), frameLights, 1600.f, 2400.f, shade, 2, 80.f);
    LightGrid::AddDynamicLight(Data(recorded), frameLights, 1700.f, 2500.f, fire, 0, 80.f);

    CHECK(plain == recorded);
    REQUIRE(frameLights.Lights().size() == 2);

    const DynamicLight& first = frameLights.Lights()[0];
    CHECK(first.position[0] == 1530.f);
    CHECK(first.position[1] == 2210.f);
    CHECK(first.position[2] == doctest::Approx(170.f + Render::Lighting::kDynamicLightHeightAboveTerrain));
    CheckSameLight(first.color, fire);
    CHECK(first.range == 3);
    CHECK(first.radius == doctest::Approx(3.f * TERRAIN_SCALE));

    const DynamicLight& second = frameLights.Lights()[1];
    CheckSameLight(second.color, shade);
    CHECK(second.radius == doctest::Approx(2.f * TERRAIN_SCALE));

    frameLights.Clear();
    CHECK(frameLights.Lights().empty());
}

TEST_CASE("the frame light list keeps its first lights and counts the overflow [render][dynamic_lights]")
{
    DynamicLightList frameLights;
    for (std::size_t i = 0; i < DynamicLightList::kCapacity + 5; ++i)
    {
        frameLights.Add(MakeLight(static_cast<float>(i), 0.f, 1.f, 1.f, 1.f, 1));
    }
    CHECK(frameLights.Lights().size() == DynamicLightList::kCapacity);
    CHECK(frameLights.Overflow() == 5);
    CHECK(frameLights.Lights()[0].position[0] == 0.f);

    frameLights.Clear();
    CHECK(frameLights.Overflow() == 0);
}

TEST_CASE("the static model light keeps darkening lights and leaves out brightening ones [render][dynamic_lights]")
{
    const Grid staticGrid = MakeGrid(0.6f);
    const float samplePoints[][2] = {{1250.f, 2250.f}, {1310.f, 2290.f}, {1199.f, 2401.f}, {10.f, 10.f}};

    SUBCASE("only darkening lights: equals the dynamic grid's sample")
    {
        Grid dynamicGrid = staticGrid;
        DynamicLightList frameLights;
        const float shade[3] = {-0.4f, -0.2f, -0.7f};
        const float deepShade[3] = {-0.5f, -0.5f, -0.5f};
        LightGrid::AddDynamicLight(Data(dynamicGrid), frameLights, 1260.f, 2240.f, shade, 3, 0.f);
        LightGrid::AddDynamicLight(Data(dynamicGrid), frameLights, 1300.f, 2300.f, deepShade, 2, 0.f);
        for (const auto& point : samplePoints)
        {
            float expected[3];
            float actual[3];
            LightGrid::Sample(Data(dynamicGrid), point[0], point[1], expected);
            LightGrid::SampleStatic(Data(staticGrid), frameLights.Lights(), point[0], point[1], actual);
            CheckSameLight(actual, expected);
        }
    }

    SUBCASE("only brightening lights: equals the static grid's sample")
    {
        Grid dynamicGrid = staticGrid;
        DynamicLightList frameLights;
        const float fire[3] = {1.f, 0.5f, 0.1f};
        LightGrid::AddDynamicLight(Data(dynamicGrid), frameLights, 1260.f, 2240.f, fire, 3, 0.f);
        for (const auto& point : samplePoints)
        {
            float expected[3];
            float actual[3];
            LightGrid::Sample(Data(staticGrid), point[0], point[1], expected);
            LightGrid::SampleStatic(Data(staticGrid), frameLights.Lights(), point[0], point[1], actual);
            CheckSameLight(actual, expected);
        }
        float centre[3];
        float staticCentre[3];
        LightGrid::Sample(Data(dynamicGrid), 1260.f, 2240.f, centre);
        LightGrid::Sample(Data(staticGrid), 1260.f, 2240.f, staticCentre);
        CHECK(centre[0] > staticCentre[0]); // the grid itself still gets the light
    }

    SUBCASE("outside the grid: black")
    {
        float light[3] = {1.f, 1.f, 1.f};
        LightGrid::SampleStatic(Data(staticGrid), {}, -150.f, 100.f, light);
        CHECK(light[0] == 0.f);
        CHECK(light[1] == 0.f);
        CHECK(light[2] == 0.f);
    }
}

TEST_CASE("per-vertex models get the brightening lights back with the grid's falloff [render][dynamic_lights]")
{
    const std::vector<DynamicLight> lights = {
        MakeLight(1000.f, 1000.f, 1.f, 0.5f, -0.5f, 3),
        MakeLight(1000.f, 1000.f, -1.f, -1.f, -1.f, 3), // darkening only: already in the base light
        MakeLight(9000.f, 9000.f, 1.f, 1.f, 1.f, 3),    // out of range
    };
    float light[3];
    LightGrid::SampleBrightening(lights, 1150.f, 1000.f, light);
    CHECK(light[0] == doctest::Approx(0.5f));
    CHECK(light[1] == doctest::Approx(0.25f));
    CHECK(light[2] == 0.f);
}

TEST_CASE("a light contributes nothing at or beyond its radius [render][dynamic_lights]")
{
    const DynamicLight light = MakeLight(1000.f, 1000.f, 1.f, 0.5f, 0.f, 3);
    const float centre[3] = {1000.f, 1000.f, 500.f};
    const float halfway[3] = {1150.f, 1000.f, 0.f};
    const float edge[3] = {1300.f, 1000.f, 0.f};
    const float beyond[3] = {1000.f, 1400.f, 0.f};

    CHECK(Render::Lighting::ContributionAt(light, centre) == doctest::Approx(1.5f));
    CHECK(Render::Lighting::ContributionAt(light, halfway) == doctest::Approx(0.75f));
    CHECK(Render::Lighting::ContributionAt(light, edge) == 0.f);
    CHECK(Render::Lighting::ContributionAt(light, beyond) == 0.f);
}

TEST_CASE("dynamic light selection keeps the brightening lights in the shader layout [render][dynamic_lights]")
{
    const std::vector<DynamicLight> lights = {
        MakeLight(1000.f, 1000.f, 1.f, 0.5f, 0.f, 3),
        MakeLight(1100.f, 1000.f, -1.f, -1.f, -1.f, 3), // darkening only: not applied per pixel
        MakeLight(1200.f, 1000.f, 0.8f, -0.3f, 0.2f, 2),
    };
    const float hero[3] = {1000.f, 1000.f, 0.f};

    const Render::Lighting::DynamicLightSelection selection = Render::Lighting::SelectDynamicLights(lights, hero);

    CHECK(selection.brighteningLights == 2);
    CHECK(selection.dropped == 0);
    REQUIRE(selection.uniform.lightCount == 2);
    CHECK(selection.uniform.positionAndRadius[0][0] == 1000.f);
    CHECK(selection.uniform.positionAndRadius[0][2] ==
          doctest::Approx(Render::Lighting::kDynamicLightHeightAboveTerrain));
    CHECK(selection.uniform.positionAndRadius[0][3] == doctest::Approx(300.f));
    CHECK(selection.uniform.color[0][0] == 1.f);
    CHECK(selection.uniform.positionAndRadius[1][0] == 1200.f);
    CHECK(selection.uniform.color[1][1] == 0.f); // negative component zeroed
    CHECK(selection.uniform.color[1][2] == doctest::Approx(0.2f));
}

TEST_CASE("over the cap the lights contributing most at the hero win [render][dynamic_lights]")
{
    constexpr std::size_t kExtraLights = 8;
    std::vector<DynamicLight> lights;
    lights.reserve(mu::kMaxDynamicLights + kExtraLights);
    const float hero[3] = {5000.f, 5000.f, 0.f};
    // Lights at growing distance from the hero: the nearer, the larger the contribution. Far
    // lights contribute nothing there and rank by distance among themselves.
    for (std::size_t i = 0; i < mu::kMaxDynamicLights + kExtraLights; ++i)
    {
        const float offset = static_cast<float>(i) * 50.f;
        lights.push_back(MakeLight(hero[0] + offset, hero[1], 1.f, 1.f, 1.f, 10));
    }
    // Reverse so that the input order does not decide.
    std::vector<DynamicLight> reversed(lights.rbegin(), lights.rend());

    const Render::Lighting::DynamicLightSelection selection = Render::Lighting::SelectDynamicLights(reversed, hero);

    CHECK(selection.brighteningLights == mu::kMaxDynamicLights + kExtraLights);
    CHECK(selection.dropped == kExtraLights);
    REQUIRE(selection.uniform.lightCount == mu::kMaxDynamicLights);
    for (std::size_t slot = 0; slot < mu::kMaxDynamicLights; ++slot)
    {
        CHECK(selection.uniform.positionAndRadius[slot][0] == lights[slot].position[0]);
    }

    SUBCASE("equal contributions go to the nearer light")
    {
        std::vector<DynamicLight> farLights;
        farLights.reserve(mu::kMaxDynamicLights + 1);
        for (std::size_t i = 0; i < mu::kMaxDynamicLights + 1; ++i)
        {
            // All beyond their radius from the hero: contribution 0.
            farLights.push_back(MakeLight(hero[0] + 2000.f + static_cast<float>(i), hero[1], 1.f, 1.f, 1.f, 1));
        }
        const Render::Lighting::DynamicLightSelection farSelection = Render::Lighting::SelectDynamicLights(
            std::vector<DynamicLight>(farLights.rbegin(), farLights.rend()), hero);
        CHECK(farSelection.dropped == 1);
        CHECK(farSelection.uniform.positionAndRadius[0][0] == farLights.front().position[0]);
        CHECK(farSelection.uniform.positionAndRadius[mu::kMaxDynamicLights - 1][0] ==
              farLights[mu::kMaxDynamicLights - 1].position[0]);
    }
}

TEST_CASE("the dropped-light report runs at most once per minute [render][dynamic_lights]")
{
    using std::chrono::seconds;
    CHECK(Render::Lighting::kDroppedLightReportInterval == std::chrono::minutes(1));

    Render::Lighting::DroppedLightReportLimiter limiter;
    const std::chrono::steady_clock::time_point start{std::chrono::hours(1)};

    CHECK(limiter.ShouldReport(start));
    CHECK_FALSE(limiter.ShouldReport(start + seconds(1)));
    CHECK_FALSE(limiter.ShouldReport(start + seconds(59)));
    CHECK(limiter.ShouldReport(start + seconds(60)));
    CHECK_FALSE(limiter.ShouldReport(start + seconds(90)));
    CHECK(limiter.ShouldReport(start + seconds(125)));
}
