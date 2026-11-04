#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Pets/w_PetProcess.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "Scenes/FixedTick.h"
#include "Scenes/SceneCore.h"

#include <filesystem>
#include <memory>
#include <tuple>
#include <vector>

extern float FPS_ANIMATION_FACTOR;
extern int REFERENCE_FRAME_STEP;

using namespace Scenes::FixedTick;

TEST_CASE("The clock gives whole ticks and keeps the rest as the frame's place between them [fixed-tick]")
{
    Clock clock;
    CHECK(clock.Advance(7.0) == 0);
    CHECK(clock.Alpha() == doctest::Approx(7.f / 40.f));
    CHECK(clock.Advance(33.0) == 1);
    CHECK(clock.Alpha() == doctest::Approx(0.f));
    CHECK(clock.Advance(90.0) == 2);
    CHECK(clock.Alpha() == doctest::Approx(0.25f));
}

TEST_CASE("A stall runs a limited number of ticks and drops the rest [fixed-tick]")
{
    Clock clock;
    CHECK(clock.Advance(TickMs * 100.0 + 20.0) == MaxTicksPerFrame);
    CHECK(clock.Alpha() == doctest::Approx(0.5f));
    CHECK(clock.Advance(0.0) == 0);
}

TEST_CASE("Any frame rate runs 25 ticks a second [fixed-tick]")
{
    for (const double frameMs : {1000.0 / 144.0, 1000.0 / 60.0, 1000.0 / 30.0, 50.0})
    {
        Clock clock;
        int ticks = 0;
        const int frames = static_cast<int>(10000.0 / frameMs + 0.5);
        for (int i = 0; i < frames; ++i)
            ticks += clock.Advance(frameMs);
        CHECK(ticks >= 249);
        CHECK(ticks <= 250);
    }
}

TEST_CASE("A position between ticks is interpolated unless it jumped [fixed-tick]")
{
    vec3_t from = {100.f, 200.f, 0.f};
    vec3_t walked = {110.f, 200.f, 4.f};
    vec3_t out;
    Interpolate(from, walked, 0.5f, out);
    CHECK(out[0] == doctest::Approx(105.f));
    CHECK(out[2] == doctest::Approx(2.f));

    vec3_t teleported = {9000.f, 200.f, 0.f};
    Interpolate(from, teleported, 0.5f, out);
    CHECK(out[0] == doctest::Approx(9000.f));
}

TEST_CASE("An angle takes the short way round, and a new action or a loop holds the tick's frame [fixed-tick]")
{
    CHECK(InterpolateAngle(350.f, 10.f, 0.5f) == doctest::Approx(360.f));
    CHECK(InterpolateAngle(10.f, 350.f, 0.5f) == doctest::Approx(0.f));
    CHECK(InterpolateFrame(2.f, 3.f, 0.5f, true) == doctest::Approx(2.5f));
    CHECK(InterpolateFrame(2.f, 3.f, 0.5f, false) == doctest::Approx(3.f));
    CHECK(InterpolateFrame(9.5f, 0.5f, 0.5f, true) == doctest::Approx(0.5f));
}

TEST_CASE("A tick takes one whole reference frame and gives the frame's values back [fixed-tick]")
{
    const float savedFactor = FPS_ANIMATION_FACTOR;
    const int savedStep = REFERENCE_FRAME_STEP;
    FPS_ANIMATION_FACTOR = 0.2f;
    REFERENCE_FRAME_STEP = 0;
    {
        const TickScope tick;
        CHECK(FPS_ANIMATION_FACTOR == 1.f);
        CHECK(REFERENCE_FRAME_STEP == 1);
    }
    CHECK(FPS_ANIMATION_FACTOR == 0.2f);
    CHECK(REFERENCE_FRAME_STEP == 0);
    FPS_ANIMATION_FACTOR = savedFactor;
    REFERENCE_FRAME_STEP = savedStep;
}

TEST_CASE("Applying moves a character for the frame, and restoring gives the update's position back [fixed-tick]")
{
    const auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    CHARACTER* const saved = CharactersClient;
    const EGameScene savedScene = SceneFlag;
    CharactersClient = characters.get();
    SceneFlag = MAIN_SCENE;
    OBJECT& o = CharactersClient[3].Object;
    o.Live = true;
    Vector(100.f, 100.f, 0.f, o.Position);

    SetOn(true);
    BeginFrame(0.0);
    BeginFrame(50.0); // one tick, and a quarter of the next
    REQUIRE(TicksThisFrame() == 1);
    o.AnimationFrame = 2.f;
    o.PriorAnimationFrame = 1.f;
    o.Angle[2] = 350.f;
    CaptureTransforms();
    Vector(120.f, 100.f, 0.f, o.Position);
    o.AnimationFrame = 3.f;
    o.PriorAnimationFrame = 2.f;
    o.Angle[2] = 10.f;
    CaptureTickEnd();

    ApplyInterpolation();
    CHECK(o.Position[0] == doctest::Approx(105.f));
    CHECK(o.AnimationFrame == doctest::Approx(2.25f));
    CHECK(o.PriorAnimationFrame == doctest::Approx(1.f));
    CHECK(o.Angle[2] == doctest::Approx(355.f));
    ApplyInterpolation();
    CHECK(o.Position[0] == doctest::Approx(105.f));
    Restore();
    CHECK(o.Position[0] == doctest::Approx(120.f));
    CHECK(o.AnimationFrame == doctest::Approx(3.f));
    CHECK(o.PriorAnimationFrame == doctest::Approx(2.f));
    CHECK(o.Angle[2] == doctest::Approx(10.f));

    SetOn(false);
    CHECK_FALSE(RunsWorld());
    SceneFlag = savedScene;
    CharactersClient = saved;
}

TEST_CASE("Effects and particles draw between ticks; a sprite and an offset follow the moved owner [fixed-tick]")
{
    const auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    CHARACTER* const saved = CharactersClient;
    const EGameScene savedScene = SceneFlag;
    CharactersClient = characters.get();
    SceneFlag = MAIN_SCENE;
    OBJECT& owner = CharactersClient[0].Object;
    owner.Live = true;
    Vector(100.f, 100.f, 0.f, owner.Position);
    OBJECT& effect = Effects[0];
    effect.Live = true;
    Vector(0.f, 0.f, 0.f, effect.Position);
    PARTICLE& particle = Particles[0];
    particle.Live = true;
    Vector(10.f, 0.f, 0.f, particle.Position);
    OBJECT& sprite = Sprites[0];

    SetOn(true);
    BeginFrame(0.0);
    BeginFrame(60.0); // one tick, and half of the next
    CaptureTransforms();
    Vector(120.f, 100.f, 0.f, owner.Position);
    Vector(40.f, 0.f, 0.f, effect.Position);
    Vector(30.f, 0.f, 0.f, particle.Position);
    sprite.Live = true;
    sprite.Owner = &owner;
    Vector(500.f, 0.f, 0.f, sprite.Position);
    // An equipment effect's sprite: owned by its item type's shared object, carried by the character.
    OBJECT itemType{};
    vec3_t worn = {800.f, 0.f, 0.f};
    vec3_t light = {1.f, 1.f, 1.f};
    int wornSprite = -1;
    {
        const SpriteCarrierScope carrier(&owner);
        wornSprite = CreateSprite(BITMAP_LIGHT, worn, 1.f, light, &itemType);
    }
    REQUIRE(wornSprite > 0);
    // Born in this tick: nothing to draw it from.
    OBJECT& born = Effects[1];
    born.Live = true;
    Vector(70.f, 0.f, 0.f, born.Position);
    CaptureTickEnd();

    ApplyInterpolation();
    CHECK(owner.Position[0] == doctest::Approx(110.f));
    CHECK(effect.Position[0] == doctest::Approx(20.f));
    CHECK(particle.Position[0] == doctest::Approx(20.f));
    CHECK(sprite.Position[0] == doctest::Approx(490.f));
    CHECK(Sprites[wornSprite].Position[0] == doctest::Approx(790.f));
    CHECK(born.Position[0] == doctest::Approx(70.f));
    vec3_t offset;
    DrawOffset(&owner, offset);
    CHECK(offset[0] == doctest::Approx(-10.f));
    DrawOffset(&effect, offset);
    CHECK(offset[0] == doctest::Approx(-20.f));
    CHECK(SpritesFollowing() == 2);

    Restore();
    CHECK(owner.Position[0] == doctest::Approx(120.f));
    CHECK(effect.Position[0] == doctest::Approx(40.f));
    CHECK(particle.Position[0] == doctest::Approx(30.f));
    CHECK(sprite.Position[0] == doctest::Approx(500.f));
    CHECK(Sprites[wornSprite].Position[0] == doctest::Approx(800.f));
    DrawOffset(&owner, offset);
    CHECK(offset[0] == doctest::Approx(0.f));

    effect.Live = born.Live = particle.Live = sprite.Live = Sprites[wornSprite].Live = false;
    sprite.Owner = nullptr;
    SetOn(false);
    SceneFlag = savedScene;
    CharactersClient = saved;
}

TEST_CASE("A Doppelganger's rise is the update's: drawing neither repeats nor loses it [fixed-tick]")
{
    const auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    CHARACTER* const saved = CharactersClient;
    BMD* const savedModels = Models;
    const EGameScene savedScene = SceneFlag;
    BMD model;
    model.NumActions = 1;
    Models = &model;
    CharactersClient = characters.get();
    SceneFlag = MAIN_SCENE;
    CHARACTER& c = CharactersClient[0];
    OBJECT& o = c.Object;
    o.Live = true;
    o.Visible = true;
    c.MonsterIndex = MONSTER_TERRIBLE_BUTCHER;
    o.Position[2] = 100.f;
    OBJECT* const body = &o;
    g_CharacterRegisterBuff(body, eBuff_Doppelganger_Ascension);

    SetOn(true);
    BeginFrame(0.0);
    BeginFrame(60.0); // half way to the next tick
    for (int i = 0; i < 3; ++i)
    {
        {
            const TickScope tick;
            CaptureTransforms();
            UpdateDrawnCharacterHeight(&c);
        }
        CaptureTickEnd();
        ApplyInterpolation();
        CHECK(o.Position[2] == doctest::Approx(100.f + 2.f * i + 1.f));
        PrepareCharacterForDraw(&c);
        CHECK(o.Position[2] == doctest::Approx(100.f + 2.f * i + 1.f));
        Restore();
        CHECK(o.Position[2] == doctest::Approx(100.f + 2.f * (i + 1)));
    }

    SetOn(false);
    SceneFlag = savedScene;
    CharactersClient = saved;
    Models = savedModels;
}

TEST_CASE("Sprites stay with a hero who moves on between ticks, by carrier and by owner [fixed-tick]")
{
    const auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    CHARACTER* const saved = CharactersClient;
    const EGameScene savedScene = SceneFlag;
    CharactersClient = characters.get();
    SceneFlag = MAIN_SCENE;
    OBJECT& hero = CharactersClient[0].Object;
    hero.Live = true;
    Vector(120.f, 0.f, 0.f, hero.Position);

    SetOn(true);
    BeginFrame(0.0);
    BeginFrame(40.0);
    CaptureTransforms();
    OBJECT itemType{};
    vec3_t worn = {120.f, 0.f, 0.f};
    vec3_t owned = {125.f, 0.f, 0.f};
    vec3_t light = {1.f, 1.f, 1.f};
    int wornSprite = -1;
    {
        const SpriteCarrierScope carrier(&hero);
        wornSprite = CreateSprite(BITMAP_LIGHT, worn, 1.f, light, &itemType);
    }
    const int ownedSprite = CreateSprite(BITMAP_LIGHT, owned, 1.f, light, &hero);
    REQUIRE(wornSprite >= 0);
    REQUIRE(ownedSprite >= 0);
    CaptureTickEnd();
    ApplyInterpolation();
    Restore();

    // Frames with no tick: the hero's input moves it, the sprites stay where the tick built them.
    for (const auto& [now, x, drawn] : {std::tuple{70.0, 130.f, 127.5f}, std::tuple{75.0, 140.f, 137.5f}})
    {
        BeginFrame(now);
        REQUIRE(TicksThisFrame() == 0);
        hero.Position[0] = x;
        ApplyInterpolation();
        CHECK(hero.Position[0] == doctest::Approx(drawn));
        CHECK(Sprites[wornSprite].Position[0] == doctest::Approx(drawn));
        CHECK(Sprites[ownedSprite].Position[0] == doctest::Approx(drawn + 5.f));
        vec3_t offset;
        DrawOffset(&hero, offset);
        CHECK(offset[0] == doctest::Approx(drawn - 120.f));
        Restore();
        CHECK(hero.Position[0] == doctest::Approx(x));
        CHECK(Sprites[wornSprite].Position[0] == doctest::Approx(120.f));
        CHECK(Sprites[ownedSprite].Position[0] == doctest::Approx(125.f));
    }

    Sprites[wornSprite].Live = Sprites[ownedSprite].Live = false;
    SetOn(false);
    SceneFlag = savedScene;
    CharactersClient = saved;
}

TEST_CASE(
    "A new object in a reused slot starts where it was born; its neighbour still moves between ticks [fixed-tick]")
{
    const EGameScene savedScene = SceneFlag;
    SceneFlag = MAIN_SCENE;
    OBJECT& effect = Effects[0];
    OBJECT& neighbour = Effects[1];
    effect.Live = neighbour.Live = true;
    Vector(100.f, 0.f, 0.f, effect.Position);
    Vector(100.f, 0.f, 0.f, neighbour.Position);
    vec3_t born = {100.f, 0.f, 0.f};
    vec3_t angle = {0.f, 0.f, 0.f};
    vec3_t light = {1.f, 1.f, 1.f};
    const int first = CreateParticle(BITMAP_SMOKE, born, angle, light);
    REQUIRE(first >= 0);
    PARTICLE& particle = Particles[first];
    PARTICLE& neighbourParticle = Particles[first + 1];
    neighbourParticle.Live = true;
    neighbourParticle.Type = particle.Type;
    Vector(100.f, 0.f, 0.f, neighbourParticle.Position);

    SetOn(true);
    BeginFrame(0.0);
    BeginFrame(60.0); // one tick, and half of the next
    CaptureTransforms();
    // Retired and refilled within the tick, near the last one and of the same type.
    effect.Birth = NextBirth();
    Vector(180.f, 0.f, 0.f, effect.Position);
    Vector(180.f, 0.f, 0.f, neighbour.Position);
    particle.Live = false;
    Vector(180.f, 0.f, 0.f, born);
    REQUIRE(CreateParticle(BITMAP_SMOKE, born, angle, light) == first);
    const float particleBorn = particle.Position[0];
    Vector(180.f, 0.f, 0.f, neighbourParticle.Position);
    CaptureTickEnd();

    ApplyInterpolation();
    CHECK(effect.Position[0] == doctest::Approx(180.f));
    CHECK(particle.Position[0] == doctest::Approx(particleBorn));
    CHECK(neighbour.Position[0] == doctest::Approx(140.f));
    CHECK(neighbourParticle.Position[0] == doctest::Approx(140.f));
    Restore();

    effect.Live = neighbour.Live = particle.Live = neighbourParticle.Live = false;
    SetOn(false);
    SceneFlag = savedScene;
}

TEST_CASE("Removing a pet doesn't give another its place's history [fixed-tick]")
{
    const std::filesystem::path previousFolder = std::filesystem::current_path();
    std::filesystem::current_path(MU_TEST_BIN_DIR);
    g_petProcess = PetProcess::Make();
    std::filesystem::current_path(previousFolder);
    const EGameScene savedScene = SceneFlag;
    SceneFlag = MAIN_SCENE;
    CHARACTER removedOwner;
    CHARACTER keptOwner;
    constexpr int skeleton = ITEM_PET_SKELETON;
    vec3_t position = {100.f, 0.f, 0.f};
    REQUIRE(ThePetProcess().CreatePet(skeleton, MODEL_PET_SKELETON, position, &removedOwner));
    position[0] = 200.f;
    REQUIRE(ThePetProcess().CreatePet(skeleton, MODEL_PET_SKELETON, position, &keptOwner));
    std::vector<OBJECT*> pets;
    ThePetProcess().ForEachObject([&](OBJECT& pet) { pets.push_back(&pet); });
    REQUIRE(pets.size() == 2);
    OBJECT* const kept = pets[1];
    Vector(100.f, 0.f, 0.f, pets[0]->Position);
    Vector(200.f, 0.f, 0.f, kept->Position);

    SetOn(true);
    BeginFrame(0.0);
    BeginFrame(60.0); // one tick, and half of the next
    CaptureTransforms();
    ThePetProcess().DeletePet(&removedOwner, skeleton);
    CaptureTickEnd();

    ApplyInterpolation();
    CHECK(kept->Position[0] == doctest::Approx(200.f));
    Restore();
    CHECK(kept->Position[0] == doctest::Approx(200.f));
    // The next frame, with no tick to capture again.
    BeginFrame(70.0);
    REQUIRE(TicksThisFrame() == 0);
    ApplyInterpolation();
    CHECK(kept->Position[0] == doctest::Approx(200.f));
    Restore();

    SetOn(false);
    SceneFlag = savedScene;
    g_petProcess.reset();
}
