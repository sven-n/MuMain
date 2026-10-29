#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "World/MapInfra/w_MapProcess.h"

#include <memory>

namespace
{
// A Cursed Temple statue that dies this frame: its effects shake the camera, on any map.
struct DyingStatueScope
{
    std::unique_ptr<BMD[]> models{new BMD[MAX_MODELS]};
    std::unique_ptr<CHARACTER[]> characters{new CHARACTER[MAX_CHARACTERS_CLIENT]{}};
    std::unique_ptr<vec34_t[]> bones{new vec34_t[MAX_BONES]{}};
    BMD* previousModels = Models;
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER* previousHero = Hero;
    MapProcessPtr previousMapProcess = g_MapProcess;
    float previousEarthQuake = EarthQuake;

    DyingStatueScope()
    {
        Models = models.get();
        CharactersClient = characters.get();
        Hero = nullptr;
        g_MapProcess = MapProcess::Make();
        Models[MODEL_CURSEDTEMPLE_STATUE].NumActions = 1;

        OBJECT& o = statue().Object;
        o.Live = true;
        o.Visible = true;
        o.Type = MODEL_CURSEDTEMPLE_STATUE;
        o.CurrentAction = MONSTER01_DIE;
        o.PKKey = 0;
        o.Scale = 1.f;
        o.BoneTransform = bones.get();
    }

    ~DyingStatueScope()
    {
        statue().Object.BoneTransform = nullptr;
        EarthQuake = previousEarthQuake;
        g_MapProcess = previousMapProcess;
        Hero = previousHero;
        CharactersClient = previousCharacters;
        Models = previousModels;
    }

    CHARACTER& statue()
    {
        return characters[1];
    }
};
} // namespace

TEST_CASE("A camera shake the monster effects request reaches the camera after the next damping [gamelogic]")
{
    DyingStatueScope scope;
    constexpr float sampled = 5.f; // no statue shake is this strong
    EarthQuake = sampled;

    UpdateCharacterEffects();

    CHECK(scope.statue().Object.PKKey == 1); // its death effects ran
    CHECK(EarthQuake == sampled);            // the camera of this frame still sees last frame's shake

    ApplyCharacterEffectsEarthQuake();
    CHECK(EarthQuake != sampled);
    CHECK(EarthQuake >= -0.8f);
    CHECK(EarthQuake <= 0.7f);

    EarthQuake = 0.25f;
    ApplyCharacterEffectsEarthQuake();
    CHECK(EarthQuake == 0.25f); // a request is handed over once
}

TEST_CASE("No camera shake is handed over when no monster effect requests one [gamelogic]")
{
    DyingStatueScope scope;
    scope.statue().Object.Live = false;
    EarthQuake = -0.4f;

    UpdateCharacterEffects();
    ApplyCharacterEffectsEarthQuake();

    CHECK(EarthQuake == -0.4f);
}

TEST_CASE("An effect does not keep the visibility of the effect that had its slot [effects]")
{
    for (int i = 0; i < MAX_EFFECTS; ++i)
        Effects[i].Live = false;
    Effects[0].Visible = true;

    vec3_t position{1000.f, 1000.f, 0.f};
    vec3_t angle{0.f, 0.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    CreateEffect(BITMAP_MAGIC, position, angle, light, 8);

    CHECK(Effects[0].Live);
    CHECK(Effects[0].Type == BITMAP_MAGIC);
    CHECK_FALSE(Effects[0].Visible);

    Effects[0].Live = false;
}
