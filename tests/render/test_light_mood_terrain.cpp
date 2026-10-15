#include "stdafx.h"

#include "doctest.h"

#include "Render/Lighting/LightMoodSetting.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Scenes/SceneCore.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <array>
#include <initializer_list>
#include <vector>

extern bool ActiveTerrain;
extern vec3_t TerrainNormal[TERRAIN_SIZE * TERRAIN_SIZE];

using Data::Maps::LightMood;
using Data::Maps::LightMoodData;
using doctest::Approx;
namespace Mood = Render::Lighting::LightMood;

// The light mood against the client's real terrain light buffers: the static
// grid built by CreateTerrainLight, the dynamic lights of AddTerrainLight and
// the samples the characters take.
namespace
{
constexpr int Cells = TERRAIN_SIZE * TERRAIN_SIZE;
constexpr int Channels = 3;
constexpr int NoWorld = -1;
// A point between cells, so the samples are bilinear.
constexpr float PointX = 64.4f * TERRAIN_SCALE;
constexpr float PointY = 80.7f * TERRAIN_SCALE;
constexpr int LightRange = 3;

using Grid = std::vector<std::array<float, Channels>>;

LightMoodData Moods(const LightMood& defaultMood)
{
    LightMoodData data;
    data.defaultMood = defaultMood;
    data.maps.push_back({WD_10HEAVEN, LightMood{0.7, 0.7, 1.0, 0.7}});
    return data;
}

const LightMood Lifted{0.6, 0.85, 1.3, 0.5};

Grid Copy(const vec3_t* buffer)
{
    Grid grid(Cells);
    for (int i = 0; i < Cells; ++i)
    {
        std::copy(buffer[i], buffer[i] + Channels, grid[i].begin());
    }
    return grid;
}

// The frame's terrain light before any dynamic light, as InitTerrainLight copies it.
void ResetFrameLight()
{
    for (int i = 0; i < Cells; ++i)
    {
        VectorCopy(BackTerrainLight[i], PrimaryTerrainLight[i]);
    }
}

// What each cell gained from the dynamic lights of this frame.
Grid DynamicPart()
{
    Grid grid(Cells);
    for (int i = 0; i < Cells; ++i)
    {
        for (int c = 0; c < Channels; ++c)
        {
            grid[i][c] = PrimaryTerrainLight[i][c] - BackTerrainLight[i][c];
        }
    }
    return grid;
}

void CheckScaled(const Grid& actual, const Grid& classic, float scale)
{
    for (int i = 0; i < Cells; ++i)
    {
        for (int c = 0; c < Channels; ++c)
        {
            REQUIRE(actual[i][c] == Approx(classic[i][c] * scale));
        }
    }
}

struct Terrain
{
    Terrain()
    {
        SceneFlag = MAIN_SCENE;
        ActiveTerrain = true;
        gMapManager.WorldActive = WD_0LORENCIA;
        FillBakedLight([](int i, int c) { return static_cast<float>((i * 7 + c * 3) % 11) / 10.f; });
        Mood::UseMood({}, false, Mood::MaxStrength);
        classicStatic = Copy(BackTerrainLight);
    }

    ~Terrain()
    {
        Mood::UseMood({}, false, Mood::MaxStrength);
        gMapManager.WorldActive = NoWorld;
    }

    template <typename TValue> void FillBakedLight(TValue&& value)
    {
        for (int i = 0; i < Cells; ++i)
        {
            for (int c = 0; c < Channels; ++c)
            {
                TerrainLight[i][c] = value(i, c);
            }
        }
    }

    Grid classicStatic;
};
} // namespace

TEST_CASE_FIXTURE(Terrain, "the static light grid scales by the effective base [render][lightmood]")
{
    Mood::UseMood(Moods(Lifted), true, 100);
    CheckScaled(Copy(BackTerrainLight), classicStatic, 0.6f);

    Mood::UseMood(Moods(Lifted), true, 50);
    CheckScaled(Copy(BackTerrainLight), classicStatic, 0.8f);
}

TEST_CASE_FIXTURE(Terrain, "strength 0 and the switch off leave the static light grid Classic [render][lightmood]")
{
    Mood::UseMood(Moods(Lifted), true, 0);
    CHECK(Copy(BackTerrainLight) == classicStatic);

    Mood::UseMood(Moods(Lifted), false, 100);
    CHECK(Copy(BackTerrainLight) == classicStatic);
}

TEST_CASE_FIXTURE(Terrain, "a map load builds the static light with that map's mood [render][lightmood]")
{
    Mood::UseMood(Moods(Lifted), true, 100);

    gMapManager.WorldActive = WD_10HEAVEN;
    CreateTerrainLight();
    CheckScaled(Copy(BackTerrainLight), classicStatic, 0.7f);
    CHECK(Mood::Active().background == Approx(0.7f));

    gMapManager.WorldActive = WD_0LORENCIA;
    CHECK(Mood::Active().background == Approx(0.5f));
}

TEST_CASE_FIXTURE(Terrain, "login and character selection keep the Classic light [render][lightmood]")
{
    Mood::UseMood(Moods(Lifted), true, 100);
    for (const int world : std::initializer_list<int>{WD_73NEW_LOGIN_SCENE, WD_74NEW_CHARACTER_SCENE,
                                                      WD_77NEW_LOGIN_SCENE, WD_78NEW_CHARACTER_SCENE, NoWorld})
    {
        CAPTURE(world);
        gMapManager.WorldActive = world;
        CreateTerrainLight();
        CHECK(Copy(BackTerrainLight) == classicStatic);
        CHECK(Mood::Active() == Mood::Scales{});

        const float abilityLight[3] = {1.f, 1.f, 1.f};
        float light[3];
        Mood::ScaleOwnHeroLight(0.f, 0.f, abilityLight, light);
        CHECK(light[0] == 1.f);
    }
}

TEST_CASE_FIXTURE(Terrain,
                  "a dynamic light adds its Classic contribution times the gain at any base [render][lightmood]")
{
    // An even baked light keeps every cell away from the 0 and 1 clamps.
    FillBakedLight([](int, int) { return 0.8f; });
    vec3_t candle = {0.2f, 0.12f, 0.05f};
    vec3_t shadow = {-0.1f, -0.1f, -0.1f};

    Mood::UseMood({}, false, 100);
    ResetFrameLight();
    AddTerrainLight(PointX, PointY, candle, LightRange, PrimaryTerrainLight);
    const Grid classicCandle = DynamicPart();
    ResetFrameLight();
    AddTerrainLightClip(PointX, PointY, candle, LightRange, PrimaryTerrainLight);
    const Grid classicClippedCandle = DynamicPart();
    ResetFrameLight();
    AddTerrainLight(PointX, PointY, shadow, LightRange, PrimaryTerrainLight);
    const Grid classicShadow = DynamicPart();

    for (const double base : {0.5, 0.7, 1.0})
    {
        CAPTURE(base);
        Mood::UseMood(Moods(LightMood{base, base, 1.3, 1.0}), true, 100);

        ResetFrameLight();
        AddTerrainLight(PointX, PointY, candle, LightRange, PrimaryTerrainLight);
        CheckScaled(DynamicPart(), classicCandle, 1.3f);

        ResetFrameLight();
        AddTerrainLightClip(PointX, PointY, candle, LightRange, PrimaryTerrainLight);
        CheckScaled(DynamicPart(), classicClippedCandle, 1.3f);

        // Darkening lights are not scaled.
        ResetFrameLight();
        AddTerrainLight(PointX, PointY, shadow, LightRange, PrimaryTerrainLight);
        CheckScaled(DynamicPart(), classicShadow, 1.f);
    }
}

TEST_CASE_FIXTURE(Terrain, "light baked into the map in the editor gets no gain [render][lightmood]")
{
    vec3_t brush = {0.05f, 0.05f, 0.05f};
    FillBakedLight([](int, int) { return 0.5f; });
    AddTerrainLightClip(PointX, PointY, brush, LightRange, TerrainLight);
    const Grid classicBaked = Copy(TerrainLight);

    FillBakedLight([](int, int) { return 0.5f; });
    Mood::UseMood(Moods(Lifted), true, 100);
    AddTerrainLightClip(PointX, PointY, brush, LightRange, TerrainLight);
    CHECK(Copy(TerrainLight) == classicBaked);
}

TEST_CASE_FIXTURE(Terrain, "a figure gets figures times the static light plus the dynamic light [render][lightmood]")
{
    vec3_t candle = {0.2f, 0.12f, 0.05f};
    vec3_t classicStaticSample;
    RequestBackTerrainLight(PointX, PointY, classicStaticSample);

    Mood::UseMood(Moods(Lifted), true, 100);
    ResetFrameLight();
    AddTerrainLight(PointX, PointY, candle, LightRange, PrimaryTerrainLight);
    vec3_t darkenedStatic;
    RequestBackTerrainLight(PointX, PointY, darkenedStatic);
    vec3_t light;
    RequestTerrainLight(PointX, PointY, light);
    vec3_t dynamic;
    VectorSubtract(light, darkenedStatic, dynamic);

    Mood::CorrectFigureTerrainLight(PointX, PointY, light);

    for (int c = 0; c < Channels; ++c)
    {
        CHECK(darkenedStatic[c] == Approx(classicStaticSample[c] * 0.6f));
        CHECK(light[c] == Approx(classicStaticSample[c] * 0.85f + dynamic[c]));
    }
}

TEST_CASE_FIXTURE(Terrain, "the own hero's fixed light takes the effective figure scale [render][lightmood]")
{
    const float abilityLight[3] = {0.9f, 0.5f, 0.5f};
    float light[3];

    Mood::UseMood(Moods(Lifted), true, 100);
    Mood::ScaleOwnHeroLight(0.f, 0.f, abilityLight, light);
    CHECK(light[0] == Approx(0.9f * 0.85f));
    CHECK(light[1] == Approx(0.5f * 0.85f));

    Mood::UseMood(Moods(Lifted), true, 0);
    Mood::ScaleOwnHeroLight(0.f, 0.f, abilityLight, light);
    CHECK(light[0] == 0.9f);
    CHECK(light[1] == 0.5f);
}

TEST_CASE_FIXTURE(Terrain, "heterogeneous shadow boundary never brightens figures above Classic [render][lightmood]")
{
    for (int i = 0; i < Cells; ++i)
    {
        Vector(0.f, 0.f, 1.f, TerrainNormal[i]);
    }
    FillBakedLight([](int i, int) { return i % TERRAIN_SIZE % 2 == 0 ? 0.2f : 1.f; });
    Mood::UseMood({}, false, 100);
    vec3_t classic;
    RequestBackTerrainLight(6450.f, 8000.f, classic);
    LightMood candidate{0.2, 1.0, 1.0, 1.0, 1.0};
    candidate.shadowKeep = 0.8;
    Mood::UseMood(Moods(candidate), true, 100);
    ResetFrameLight();
    vec3_t light;
    RequestTerrainLight(6450.f, 8000.f, light);
    Mood::CorrectFigureTerrainLight(6450.f, 8000.f, light);
    for (int c = 0; c < Channels; ++c)
    {
        CHECK(classic[c] == Approx(0.6f));
        CHECK(light[c] == Approx(classic[c]));
    }
}

TEST_CASE_FIXTURE(Terrain, "heterogeneous figure correction preserves shade bounds and darkening [render][lightmood]")
{
    for (int i = 0; i < Cells; ++i)
        Vector(0.f, 0.f, 1.f, TerrainNormal[i]);
    FillBakedLight([](int i, int) { return i % TERRAIN_SIZE % 2 == 0 ? 0.2f : 1.f; });
    LightMood candidate{0.5, 0.6, 1.0, 1.0, 1.0};
    candidate.shadowKeep = 0.8;
    candidate.brightScale = 0.85;
    SUBCASE("selected figure factor never below terrain or above Classic")
    {
        Mood::UseMood(Moods(candidate), true, 100);
        for (float fraction : {0.f, 0.25f, 0.5f, 0.75f, 1.f})
        {
            const float x = 6400.f + fraction * 100.f;
            vec3_t terrain;
            vec3_t correction;
            RequestBackTerrainLight(x, 8000.f, terrain);
            RequestFigureStaticCorrection(x, 8000.f, correction);
            for (int c = 0; c < Channels; ++c)
            {
                const float figure = terrain[c] + correction[c];
                CHECK(figure >= terrain[c]);
                CHECK(figure <= 0.2f + fraction * 0.8f);
                CHECK(figure == Approx(0.2f * 0.6f * (1.f - fraction) + 0.85f * fraction));
            }
        }
    }
    SUBCASE("negative light cannot resurrect a clipped dark cell")
    {
        candidate.base = 0.2;
        candidate.figures = 1.0;
        Mood::UseMood(Moods(candidate), true, 100);
        ResetFrameLight();
        vec3_t shade = {-2.f, -2.f, -2.f};
        AddTerrainLight(6400.f, 8000.f, shade, 4, PrimaryTerrainLight);
        vec3_t ambient;
        RequestStaticTerrainLight(6450.f, 8000.f, ambient);
        Mood::CorrectFigureTerrainLight(6450.f, 8000.f, ambient);
        for (int c = 0; c < Channels; ++c)
            CHECK(ambient[c] == Approx(0.f));
    }
    SUBCASE("strength zero retains heterogeneous Classic samples")
    {
        Mood::UseMood(Moods(candidate), true, 0);
        vec3_t correction;
        RequestFigureStaticCorrection(6450.f, 8000.f, correction);
        for (float channel : correction)
            CHECK(channel == 0.f);
    }
}

TEST_CASE_FIXTURE(Terrain, "the own hero's fixed light follows the shadow keep and the cool shade [render][lightmood]")
{
    const float abilityLight[3] = {0.9f, 0.5f, 0.5f};
    LightMood mood{0.5, 0.6, 1.0, 1.0, 1.0};
    mood.shadowKeep = 0.8;
    mood.shadeCool = 0.2;
    mood.brightScale = 0.85;
    float light[3];
    for (int i = 0; i < Cells; ++i)
    {
        Vector(0.f, 0.f, 1.f, TerrainNormal[i]);
    }

    SUBCASE("in the shade: the figure scale, tinted")
    {
        FillBakedLight([](int, int) { return 0.2f; });
        Mood::UseMood(Moods(mood), true, 100);
        Mood::ScaleOwnHeroLight(PointX, PointY, abilityLight, light);
        CHECK(light[0] == Approx(0.9f * 0.6f * 0.8f));
        CHECK(light[1] == Approx(0.5f * 0.6f * 0.9f));
        CHECK(light[2] == Approx(0.5f * 0.6f));
    }
    SUBCASE("at the shadow keep: the bright scale, untinted")
    {
        FillBakedLight([](int, int) { return 1.f; });
        Mood::UseMood(Moods(mood), true, 100);
        Mood::ScaleOwnHeroLight(PointX, PointY, abilityLight, light);
        CHECK(light[0] == Approx(0.9f * 0.85f));
        CHECK(light[1] == Approx(0.5f * 0.85f));
        CHECK(light[2] == Approx(0.5f * 0.85f));
    }
    SUBCASE("strength 0: Classic")
    {
        FillBakedLight([](int, int) { return 0.2f; });
        Mood::UseMood(Moods(mood), true, 0);
        Mood::ScaleOwnHeroLight(PointX, PointY, abilityLight, light);
        CHECK(light[0] == abilityLight[0]);
        CHECK(light[1] == abilityLight[1]);
        CHECK(light[2] == abilityLight[2]);
    }
}
