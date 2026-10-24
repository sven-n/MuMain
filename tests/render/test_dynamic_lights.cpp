#include <doctest.h>

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"
#include "Render/Lighting/DynamicLightList.h"
#include "Render/Lighting/DynamicLightSelection.h"
#include "Render/Lighting/LightShadowCasters.h"
#include "Render/Lighting/WorldDynamicLights.h"
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

TEST_CASE("the lights added after a mark stay apart when a lingering light gives way [render][dynamic_lights]")
{
    DynamicLightList frameLights;
    CHECK(frameLights.LightsSinceMark().empty());
    frameLights.MarkDisplaceable();
    for (std::size_t i = 0; i + 1 < DynamicLightList::kCapacity; ++i)
    {
        frameLights.Add(MakeLight(static_cast<float>(i), 0.f, 1.f, 1.f, 1.f, 1));
    }
    frameLights.EndDisplaceable();
    CHECK(frameLights.LightsSinceMark().size() == DynamicLightList::kCapacity - 1);

    frameLights.Mark();
    frameLights.Add(MakeLight(5000.f, 0.f, 1.f, 1.f, 1.f, 1));
    // The list is full: the last lingering light gives way to this one.
    frameLights.Add(MakeLight(6000.f, 0.f, 1.f, 1.f, 1.f, 1));

    REQUIRE(frameLights.LightsSinceMark().size() == 2);
    CHECK(frameLights.LightsSinceMark()[0].position[0] == 5000.f);
    CHECK(frameLights.LightsSinceMark()[1].position[0] == 6000.f);
}

TEST_CASE("the ground's pinned lights come first and later lights fill the slots left [render][dynamic_lights]")
{
    const float hero[3] = {1000.f, 1000.f, 0.f};
    const std::vector<DynamicLight> early = {MakeLight(1100.f, 1000.f, 0.5f, 0.5f, 0.5f, 3),
                                             MakeLight(1000.f, 1000.f, 1.f, 1.f, 1.f, 3)};
    const std::vector<DynamicLight> later = {MakeLight(1000.f, 1000.f, 2.f, 2.f, 2.f, 3)};

    const Render::Lighting::DynamicLightSelection pinned = Render::Lighting::SelectDynamicLights(early, hero);
    const Render::Lighting::DynamicLightSelection selection =
        Render::Lighting::SelectDynamicLights(pinned, later, hero);

    REQUIRE(selection.uniform.lightCount == 3);
    CHECK(selection.uniform.worldLightCount == 2);
    CHECK(selection.uniform.positionAndRadius[0][0] == 1000.f);
    CHECK(selection.uniform.positionAndRadius[1][0] == 1100.f);
    CHECK(selection.uniform.color[2][0] == 2.f); // stronger, but added after the pin
    CHECK(selection.brighteningLights == 3);
    CHECK(selection.dropped == 0);

    SUBCASE("a full pin leaves later lights out")
    {
        const std::vector<DynamicLight> many(mu::kMaxDynamicLights, early.front());
        const Render::Lighting::DynamicLightSelection full =
            Render::Lighting::SelectDynamicLights(Render::Lighting::SelectDynamicLights(many, hero), later, hero);
        CHECK(full.uniform.lightCount == mu::kMaxDynamicLights);
        CHECK(full.uniform.worldLightCount == mu::kMaxDynamicLights);
        CHECK(full.dropped == 1);
    }
}

TEST_CASE("the ground's lights are pinned once per frame [render][dynamic_lights]")
{
    const float hero[3] = {1000.f, 1000.f, 0.f};
    DynamicLightList frameLights;
    frameLights.Add(MakeLight(1000.f, 1000.f, 1.f, 1.f, 1.f, 3));
    Render::Lighting::WorldDynamicLights world;
    CHECK_FALSE(world.IsPinned());

    world.Pin(frameLights, hero);
    frameLights.Add(MakeLight(1100.f, 1000.f, 1.f, 1.f, 1.f, 3));
    world.Pin(frameLights, hero);

    CHECK(world.IsPinned());
    CHECK(world.Selection().uniform.lightCount == 1);
    CHECK(world.Selection().uniform.worldLightCount == 1);
    CHECK(frameLights.LightsSinceMark().size() == 1);
    world.Clear();
    CHECK_FALSE(world.IsPinned());
}

TEST_CASE("point lights cast a character's shadow away from them by their share of its light [render][dynamic_lights]")
{
    const float hero[3] = {1000.f, 1000.f, 0.f};
    const std::vector<DynamicLight> lights = {
        MakeLight(1000.f, 1000.f, 1.f, 1.f, 1.f, 3),      // the character's own light: casts none, still counts
        MakeLight(1200.f, 1000.f, 1.f, 1.f, 1.f, 3),      // east of it
        MakeLight(1000.f, 900.f, 0.45f, 0.45f, 0.45f, 3), // south of it, weaker
    };
    mu::DynamicLightsUniform uniform = Render::Lighting::SelectDynamicLights(lights, hero).uniform;
    uniform.worldLightCount = uniform.lightCount;

    const Render::Lighting::LightShadowCasters casters = Render::Lighting::SelectLightShadowCasters(uniform, hero, 4);

    REQUIRE(casters.count == 2);
    // East light: the shadow falls west. Contributions 3, 3 * 1/3 = 1 and 1.35 * 2/3 = 0.9 of 4.9.
    CHECK(casters.casters[0].fallAngle == doctest::Approx(3.14159265f));
    CHECK(casters.casters[0].strength == doctest::Approx(1.f / 4.9f));
    CHECK(casters.casters[1].fallAngle == doctest::Approx(3.14159265f / 2.f));
    CHECK(casters.casters[1].strength == doctest::Approx(0.9f / 4.9f));
    CHECK(Render::Lighting::SelectLightShadowCasters(uniform, hero, 1).count == 1);
    CHECK(Render::Lighting::SelectLightShadowCasters(uniform, hero, 0).count == 0);
}

TEST_CASE("skill lights light the ground but cast no character shadow [render][dynamic_lights]")
{
    const float hero[3] = {1000.f, 1000.f, 0.f};
    DynamicLight beam = MakeLight(1200.f, 1000.f, 1.f, 1.f, 1.f, 3);
    beam.castsShadows = false;
    const std::vector<DynamicLight> lights = {beam, MakeLight(1000.f, 1200.f, 1.f, 1.f, 1.f, 3)};
    mu::DynamicLightsUniform uniform = Render::Lighting::SelectDynamicLights(lights, hero).uniform;
    uniform.worldLightCount = uniform.lightCount;

    const Render::Lighting::LightShadowCasters casters = Render::Lighting::SelectLightShadowCasters(uniform, hero, 4);

    REQUIRE(casters.count == 1);
    // Only the north light casts, with its half of the light.
    CHECK(casters.casters[0].fallAngle == doctest::Approx(-3.14159265f / 2.f));
    CHECK(casters.casters[0].strength == doctest::Approx(0.5f));
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

TEST_CASE(
    "a point-light shadow lands where the light's ray through the point meets the ground [render][dynamic_lights]")
{
    using Render::Lighting::ProjectShadow;
    const float east[2] = {1.f, 0.f};
    // Light 300 along the axis, 400 up; points around a character at the origin, off the axis too.
    constexpr float lightAlong = 300.f;
    constexpr float lightHeight = 400.f;
    for (const float across : {-50.f, 0.f, 40.f})
    {
        for (const float along : {-40.f, 0.f, 35.f})
        {
            for (const float height : {0.f, 90.f, 180.f})
            {
                float ground[2] = {along, across};
                ProjectShadow(ground, height, east, lightAlong, lightHeight, true);
                // Light, point and ground hit are collinear in 3D: the point lies (1 - height /
                // lightHeight) of the way from the light to the ground hit.
                const float t = 1.f - height / lightHeight;
                CHECK(lightAlong + t * (ground[0] - lightAlong) == doctest::Approx(along).epsilon(1e-4));
                CHECK(t * ground[1] == doctest::Approx(across).epsilon(1e-4));
            }
        }
    }
    // Light (300, 0, 400) and point (0, 40, 180): the shadow lands at (-245.45, 72.73), not at y 40.
    float offAxis[2] = {0.f, 40.f};
    ProjectShadow(offAxis, 180.f, east, lightAlong, lightHeight, true);
    CHECK(offAxis[0] == doctest::Approx(-245.4545f));
    CHECK(offAxis[1] == doctest::Approx(72.7273f));
    // A point on the ground stays where it is.
    float onGround[2] = {20.f, -10.f};
    ProjectShadow(onGround, 0.f, east, lightAlong, lightHeight, true);
    CHECK(onGround[0] == doctest::Approx(20.f));
    CHECK(onGround[1] == doctest::Approx(-10.f));
    // The legacy shear is unchanged: along the axis only.
    float legacy[2] = {20.f, 30.f};
    ProjectShadow(legacy, 100.f, east, 2000.f, 4000.f, false);
    CHECK(legacy[0] == doctest::Approx(20.f + 100.f * 2020.f / (100.f - 4000.f)));
    CHECK(legacy[1] == 30.f);
}

TEST_CASE("a point light low beside a character casts a long but finite shadow [render][dynamic_lights]")
{
    using namespace Render::Lighting;
    const float east[2] = {1.f, 0.f};
    constexpr float floor = kMinLightHeightRatio * kShadowCasterHeight;
    // High: its own height. At chest height (a skill cast at mid-body): raised to the floor.
    CHECK(PointLightShadowHeight(1000.f) == 1000.f);
    CHECK(PointLightShadowHeight(100.f) == floor);
    // From a light 100 away at the floor, the top of the caster lands 4 times further out.
    float top[2] = {0.f, 0.f};
    ProjectShadow(top, kShadowCasterHeight, east, 100.f, floor, true);
    CHECK(top[0] == doctest::Approx(-400.f));
    // A model taller than the light is clamped instead of flipping towards the light.
    float tall[2] = {0.f, 0.f};
    ProjectShadow(tall, 2.f * floor, east, 100.f, floor, true);
    float atClamp[2] = {0.f, 0.f};
    ProjectShadow(atClamp, kMaxShadowHeightRatio * floor, east, 100.f, floor, true);
    CHECK(tall[0] < 0.f);
    CHECK(tall[0] == doctest::Approx(atClamp[0]));
}

TEST_CASE("a point light's shadow carries its distance and height [render][dynamic_lights]")
{
    const float hero[3] = {1000.f, 1000.f, 0.f};
    const std::vector<DynamicLight> lights = {MakeLight(1300.f, 1000.f, 1.f, 1.f, 1.f, 8)};
    mu::DynamicLightsUniform uniform = Render::Lighting::SelectDynamicLights(lights, hero).uniform;
    uniform.worldLightCount = uniform.lightCount;

    const Render::Lighting::LightShadowCasters casters = Render::Lighting::SelectLightShadowCasters(uniform, hero, 1);

    REQUIRE(casters.count == 1);
    CHECK(casters.casters[0].shearX == doctest::Approx(300.f));
    CHECK(casters.casters[0].shearHeight ==
          doctest::Approx(Render::Lighting::PointLightShadowHeight(uniform.positionAndRadius[0][2])));
}

TEST_CASE("a beam's segments cast one shadow with their shares added [render][dynamic_lights]")
{
    const float hero[3] = {1000.f, 1000.f, 0.f};
    // Aqua Beam: lights every 50 units running east from the character, plus a torch to the north.
    std::vector<DynamicLight> lights;
    for (int segment = 1; segment <= 6; ++segment)
    {
        lights.push_back(MakeLight(1000.f + 50.f * static_cast<float>(segment), 1000.f, 0.2f, 0.2f, 0.2f, 4));
    }
    lights.push_back(MakeLight(1000.f, 1250.f, 0.3f, 0.3f, 0.3f, 4));
    mu::DynamicLightsUniform uniform = Render::Lighting::SelectDynamicLights(lights, hero).uniform;
    uniform.worldLightCount = uniform.lightCount;

    const Render::Lighting::LightShadowCasters casters = Render::Lighting::SelectLightShadowCasters(uniform, hero, 2);

    REQUIRE(casters.count == 2);
    // The beam's shadow falls west, with all six segments' share; the torch's falls south.
    CHECK(std::abs(casters.casters[0].fallAngle) == doctest::Approx(3.14159265f));
    CHECK(casters.casters[1].fallAngle == doctest::Approx(-3.14159265f / 2.f));
    CHECK(casters.casters[0].strength + casters.casters[1].strength == doctest::Approx(1.f));
    CHECK(casters.casters[0].strength > 0.75f);
}
