#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "World/MapInfra/w_MapProcess.h"

#include <memory>

namespace
{
// One drawn character of the given model, alone in the client's character list.
struct CharacterScope
{
    std::unique_ptr<BMD[]> models{new BMD[MAX_MODELS]};
    std::unique_ptr<CHARACTER[]> characters{new CHARACTER[MAX_CHARACTERS_CLIENT]{}};
    std::unique_ptr<vec34_t[]> bones{new vec34_t[MAX_BONES]{}};
    BMD* previousModels = Models;
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER* previousHero = Hero;
    MapProcessPtr previousMapProcess = g_MapProcess;
    float previousEarthQuake = EarthQuake;

    explicit CharacterScope(int type)
    {
        Models = models.get();
        CharactersClient = characters.get();
        Hero = nullptr;
        g_MapProcess = MapProcess::Make();
        Models[type].NumActions = 1;

        OBJECT& o = character().Object;
        o.Live = true;
        o.Visible = true;
        o.Type = type;
        o.Scale = 1.f;
        o.BoneTransform = bones.get();
    }

    ~CharacterScope()
    {
        character().Object.BoneTransform = nullptr;
        EarthQuake = previousEarthQuake;
        g_MapProcess = previousMapProcess;
        Hero = previousHero;
        CharactersClient = previousCharacters;
        Models = previousModels;
    }

    CHARACTER& character()
    {
        return characters[1];
    }
};

// A Cursed Temple statue that dies this frame: its effects shake the camera, on any map.
struct DyingStatueScope : CharacterScope
{
    DyingStatueScope() : CharacterScope(MODEL_CURSEDTEMPLE_STATUE)
    {
        character().Object.CurrentAction = MONSTER01_DIE;
        character().Object.PKKey = 0;
    }
};
} // namespace

TEST_CASE("A camera shake the monster effects request reaches the camera after the next damping [gamelogic]")
{
    DyingStatueScope scope;
    constexpr float sampled = 5.f; // no statue shake is this strong
    EarthQuake = sampled;

    UpdateCharacterEffects();

    CHECK(scope.character().Object.PKKey == 1); // its death effects ran
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
    scope.character().Object.Live = false;
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

TEST_CASE("The sprites of the monster effects stay out of the bloom, as when they were drawn [effects]")
{
    for (int i = 0; i < MAX_SPRITES; ++i)
        Sprites[i].Live = false;

    vec3_t position{1000.f, 1000.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    const int outside = CreateSprite(BITMAP_LIGHT, position, 1.f, light, nullptr);
    REQUIRE(Sprites[outside].Live);
    CHECK_FALSE(IsSpriteBloomExcluded(outside));
    Sprites[outside].Live = false;

    CharacterScope scope(MODEL_CRUST); // its eyes are two sprites
    UpdateCharacterEffects();

    int created = 0;
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        if (!Sprites[i].Live)
            continue;
        ++created;
        CHECK(IsSpriteBloomExcluded(i));
        Sprites[i].Live = false;
    }
    CHECK(created == 2);
}

TEST_CASE("A client character's drawn parts get a slot each, in draw order, with their own bones [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    CharacterScope scope(MODEL_PLAYER);
    CHARACTER* c = &scope.character();
    PART_t temporary;

    Attachments::BeginFrame();
    Attachments::BeginCharacter(c);
    Attachments::Slot* weapon =
        Attachments::Record(c, &c->Weapon[0], MODEL_SWORD, 0.f, 0.f, 0.f, 3, 0, false, true, 0, false);
    Attachments::Slot* copy =
        Attachments::Record(c, &temporary, MODEL_SWORD, 0.f, 0.f, 15.f, 3, 0, true, true, 0, true);
    REQUIRE(weapon != nullptr);
    REQUIRE(copy != nullptr);
    CHECK(weapon != copy);
    CHECK(weapon->part == &c->Weapon[0]);
    CHECK(copy->part == nullptr);
    CHECK(copy->z == 15.f);
    CHECK(Attachments::Bones(*weapon) != Attachments::Bones(*copy));

    // The next frame's draw of the same character reuses the slots in the same order.
    float (*bones)[3][4] = Attachments::Bones(*weapon);
    Attachments::BeginFrame();
    Attachments::BeginCharacter(c);
    Attachments::Slot* again =
        Attachments::Record(c, &c->Weapon[0], MODEL_SWORD, 0.f, 0.f, 0.f, 3, 0, false, true, 0, false);
    CHECK(Attachments::Bones(*again) == bones);

    CHARACTER preview;
    CHECK(Attachments::Record(&preview, &preview.Weapon[0], MODEL_SWORD, 0.f, 0.f, 0.f, 0, 0, false, true, 0, false) ==
          nullptr);
}
