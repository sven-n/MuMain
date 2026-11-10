#include "stdafx.h"
#include "doctest.h"

#include "Core/Utilities/Random.h"
#include "Engine/AI/GOBoid.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "GameLogic/Quests/GM3rdChangeUp.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Effects/EffectBlurs.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/GameMaps/GMCrywolf1st.h"
#include "World/GameMaps/GMEmpireGuardian1.h"
#include "World/GameMaps/GMEmpireGuardian2.h"
#include "World/GameMaps/GMEmpireGuardian4.h"
#include "World/GameMaps/GMKarutan1.h"
#include "World/GameMaps/GM_Kanturu_2nd.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#include <algorithm>
#include <memory>

namespace
{
// Padding makes the old unchecked lookups observable without depending on an out-of-bounds crash.
struct EffectScope
{
    std::unique_ptr<CHARACTER[]> characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT + 2);
    std::unique_ptr<BMD[]> models = std::make_unique<BMD[]>(MAX_MODELS);
    std::unique_ptr<vec34_t[]> bones = std::make_unique<vec34_t[]>(MAX_BONES);
    std::unique_ptr<CHARACTER_MACHINE> machine = std::make_unique<CHARACTER_MACHINE>();
    CHARACTER_MACHINE* previousMachine = CharacterMachine;
    CHARACTER* previousCharacters = CharactersClient;
    BMD* previousModels = Models;
    int previousWorld = gMapManager.WorldActive;
    float previousFrameFactor = FPS_ANIMATION_FACTOR;
    MapProcessPtr previousMapProcess = g_MapProcess;
    CHARACTER attacker;

    EffectScope()
    {
        g_MapProcess = MapProcess::Make();
        CharacterMachine = machine.get();
        CharactersClient = characters.get() + 1;
        Models = models.get();
        FPS_ANIMATION_FACTOR = 1.f;
        attacker.Object.BoneTransform = bones.get();
        attacker.Object.Scale = 1.f;
        attacker.Object.Live = true;
        for (int slot = -1; slot <= MAX_CHARACTERS_CLIENT; ++slot)
        {
            Vector(500.f, 600.f, 100.f, CharactersClient[slot].Object.Position);
            CharactersClient[slot].Object.BoundingBoxMax[2] = 150.f;
        }
        Clear();
    }

    ~EffectScope()
    {
        Clear();
        attacker.Object.BoneTransform = nullptr;
        g_MapProcess = previousMapProcess;
        FPS_ANIMATION_FACTOR = previousFrameFactor;
        gMapManager.WorldActive = previousWorld;
        Models = previousModels;
        CharactersClient = previousCharacters;
        CharacterMachine = previousMachine;
    }

    static void Clear()
    {
        for (int i = 0; i < MAX_EFFECTS; ++i)
            Effects[i].Live = false;
        for (int i = 0; i < MAX_JOINTS; ++i)
            Joints[i].Live = false;
        for (int i = 0; i < MAX_PARTICLES; ++i)
            Particles[i].Live = false;
        for (int i = 0; i < MAX_SPRITES; ++i)
            Sprites[i].Live = false;
        ClearAllObjectBlurs();
        for (auto& blur : Render::Effects::g_blurs)
            blur.Live = false;
    }

    BMD* Set(int model, int action, float frame, int world)
    {
        auto& o = attacker.Object;
        o.Type = static_cast<short>(model);
        o.CurrentAction = static_cast<unsigned short>(action);
        o.AnimationFrame = frame;
        gMapManager.WorldActive = world;
        auto* b = &Models[model];
        if (!b->Actions)
            b->Actions = new Action_t[MAX_PLAYER_ACTION]{};
        b->Actions[action].PlaySpeed = 1.f;
        return b;
    }
};

int ParticlesOf(int type)
{
    return std::count_if(Particles, Particles + MAX_PARTICLES,
                         [type](const auto& p) { return p.Live && p.Type == type; });
}

int EffectsOf(int type, int subtype = -1)
{
    return std::count_if(Effects, Effects + MAX_EFFECTS, [=](const auto& e)
                         { return e.Live && e.Type == type && (subtype < 0 || e.SubType == subtype); });
}

int LiveJoints()
{
    return std::count_if(Joints, Joints + MAX_JOINTS, [](const auto& j) { return j.Live; });
}

// Each path must spawn with a real slot, never with either invalid slot. Repeat random-gated joints
// from the same seeds so a false random gate cannot make the missing-target check pass by accident.
// Both sequences are seeded: the legacy one (Rand) and the engine's, which rand_fps_check draws from.
template <typename Spawn, typename Count> void CheckTargets(EffectScope& scope, Spawn spawn, Count count)
{
    for (const short target :
         {static_cast<short>(-1), static_cast<short>(MAX_CHARACTERS_CLIENT), static_cast<short>(2)})
    {
        CAPTURE(target);
        EffectScope::Clear();
        scope.attacker.TargetCharacter = target;
        scope.attacker.LastAttackEffectTime = -1;
        scope.attacker.AttackTime = 1.f;
        Random::SeedRand(977);
        Random::Seed(977);
        for (int i = 0; i < 8; ++i)
            spawn();
        if (target == 2)
            CHECK(count() > 0);
        else
            CHECK(count() == 0);
    }
}
} // namespace

TEST_CASE("Crypos and Twin Tail skip target particles for missing slots [combat][target_effects]")
{
    EffectScope scope;
    auto* c = &scope.attacker;
    auto* o = &c->Object;
    auto* b = scope.Set(MODEL_CRYPOS, MONSTER01_ATTACK2, 4.f, WD_80KARUTAN1);
    auto karutan = CGMKarutan1::Make();
    CheckTargets(
        scope, [&] { CHECK(karutan->AttackEffectMonster(c, o, b)); }, [] { return ParticlesOf(BITMAP_SMOKE); });
    for (const int world : {WD_38KANTURU_2ND, WD_39KANTURU_3RD})
    {
        b = scope.Set(MODEL_TWIN_TAIL, MONSTER01_ATTACK2, 6.f, world);
        CheckTargets(
            scope, [&] { M38Kanturu2nd::Move_Kanturu2nd_MonsterVisual(c, o, b); },
            [] { return ParticlesOf(BITMAP_TWINTAIL_WATER); });
        CheckTargets(scope, [&] { M38Kanturu2nd::Move_Kanturu2nd_BlurEffect(c, o, b); }, LiveJoints);
    }
}

TEST_CASE("Trap cannon skips missing-target joints and consumes attack step [combat][target_effects]")
{
    EffectScope scope;
    auto* c = &scope.attacker;
    auto* b = scope.Set(MODEL_TRAP_CANON, MONSTER01_ATTACK1, 1.f, WD_38KANTURU_2ND);
    CheckTargets(scope, [&] { g_TrapCanon.Spawn_AttackEffect(c, &c->Object, b); }, LiveJoints);
    CHECK(c->LastAttackEffectTime == 1);
}

TEST_CASE("Fred and Forsaker preserve local effects without a target [combat][target_effects]")
{
    EffectScope scope;
    auto map = GMEmpireGuardian1::Make();
    auto* c = &scope.attacker;
    auto* o = &c->Object;
    auto* b = scope.Set(MODEL_FRED, MONSTER01_ATTACK2, 4.6f, WD_69EMPIREGUARDIAN1);
    CheckTargets(scope, [&] { map->MoveMonsterVisual(c, o, b); }, [] { return EffectsOf(MODEL_DEASULER); });
    b = scope.Set(MODEL_FORSAKER, MONSTER01_ATTACK1, 11.f, WD_69EMPIREGUARDIAN1);
    CheckTargets(
        scope,
        [&]
        {
            map->MoveMonsterVisual(c, o, b);
            CHECK(ParticlesOf(BITMAP_ENERGY) > 0);
        },
        LiveJoints);
    b = scope.Set(MODEL_FORSAKER, MONSTER01_ATTACK2, 4.6f, WD_69EMPIREGUARDIAN1);
    CheckTargets(
        scope,
        [&]
        {
            map->MoveMonsterVisual(c, o, b);
            CHECK(Sprites[0].Live);
        },
        [] { return EffectsOf(BITMAP_MAGIC + 1); });
}

TEST_CASE("Gaion and Jerint skip aimed effects for missing slots but retain local swords [combat][target_effects]")
{
    EffectScope scope;
    auto map = GMEmpireGuardian4::Make();
    auto* c = &scope.attacker;
    auto* o = &c->Object;
    for (const int action : {MONSTER01_ATTACK1, MONSTER01_ATTACK2, MONSTER01_ATTACK3, MONSTER01_ATTACK4})
    {
        CAPTURE(action);
        auto* b = scope.Set(MODEL_GAYION, action, 0.3f, WD_72EMPIREGUARDIAN4);
        CheckTargets(
            scope,
            [&]
            {
                map->MoveMonsterVisual(c, o, b);
                if (action != MONSTER01_ATTACK3)
                    CHECK(EffectsOf(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_, 0) +
                              EffectsOf(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_, 0) >
                          0);
            },
            [=]
            {
                return std::count_if(Effects, Effects + MAX_EFFECTS,
                                     [](const auto& e) { return e.Live && e.SubType > 0; });
            });
    }
    auto* b = scope.Set(MODEL_JERRY, MONSTER01_ATTACK3, 0.3f, WD_72EMPIREGUARDIAN4);
    CheckTargets(
        scope, [&] { map->MoveMonsterVisual(c, o, b); },
        [] { return EffectsOf(MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION); });
}

TEST_CASE("Hammerize attack three skips missing-target destruction [combat][target_effects]")
{
    EffectScope scope;
    auto map = GMEmpireGuardian2::Make();
    auto* c = &scope.attacker;
    auto* b = scope.Set(MODEL_HAMMERIZE, MONSTER01_ATTACK3, 2.f, WD_70EMPIREGUARDIAN2);
    CheckTargets(
        scope, [&] { map->MoveMonsterVisual(c, &c->Object, b); },
        [] { return EffectsOf(MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION); });
}

TEST_CASE("Death Spirit attacks skip missing-target projectiles on both maps [combat][target_effects]")
{
    EffectScope scope;
    auto* c = &scope.attacker;
    for (const int action : {MONSTER01_ATTACK1, MONSTER01_ATTACK2})
    {
        auto* b = scope.Set(MODEL_DEATH_SPIRIT, action, 4.7f, WD_34CRYWOLF_1ST);
        const auto checkBlur = [c]
        {
            CHECK(std::any_of(Render::Effects::g_blurs.begin(), Render::Effects::g_blurs.end(),
                              [c](const auto& blur) { return blur.Live && blur.Owner == c; }));
        };
        CheckTargets(
            scope,
            [&]
            {
                M34CryWolf1st::MoveCryWolf1stBlurEffect(c, &c->Object, b);
                checkBlur();
            },
            [] { return EffectsOf(MODEL_DEATH_SPI_SKILL); });
        CheckTargets(
            scope,
            [&]
            {
                SEASON3A::CGM3rdChangeUp::Instance().MoveBalgasBarrackBlurEffect(c, &c->Object, b);
                checkBlur();
            },
            [] { return EffectsOf(MODEL_DEATH_SPI_SKILL); });
    }
}

TEST_CASE("Earthquake skill skips missing-target stones and strike but keeps its step [combat][target_effects]")
{
    constexpr int MaxSkill = 3;
    EffectScope scope;
    auto* c = &scope.attacker;
    auto* b = scope.Set(MODEL_HAMMERIZE, MONSTER01_ATTACK2, 0.f, WD_70EMPIREGUARDIAN2);
    const int previousStep = REFERENCE_FRAME_STEP;
    REFERENCE_FRAME_STEP = 1;
    CheckTargets(
        scope,
        [&]
        {
            const int level = c->Object.WeaponLevel;
            SpawnSkillEarthQuake(c, &c->Object, b, MaxSkill);
            CHECK(c->Object.WeaponLevel == (level + 1 > MaxSkill ? 0 : level + 1));
        },
        [] { return EffectsOf(MODEL_GROUND_STONE + 1) + EffectsOf(MODEL_SKILL_FURY_STRIKE); });
    REFERENCE_FRAME_STEP = previousStep;
}

TEST_CASE("Dark Ghost flame and poison spirit smoke skip missing slots [combat][target_effects]")
{
    EffectScope scope;
    auto* c = &scope.attacker;
    auto map = GMEmpireGuardian2::Make();
    c->MonsterIndex = MONSTER_DARK_GHOST;
    auto* b = scope.Set(MODEL_DARK_GHOST, MONSTER01_ATTACK2, 0.f, WD_70EMPIREGUARDIAN2);
    CheckTargets(
        scope,
        [&]
        {
            c->MonsterSkill = 51;
            CHECK(map->AttackEffectMonster(c, &c->Object, b));
        },
        [] { return EffectsOf(BITMAP_FLAME); });
    c->MonsterIndex = MONSTER_ILLUSION_SORCERER_SPIRIT1_POISON;
    b = scope.Set(MODEL_DARK_GHOST, MONSTER01_ATTACK1, 0.f, WD_45CURSEDTEMPLE_LV1);
    CheckTargets(
        scope, [&] { CHECK(g_CursedTemple->AttackEffectMonster(c, &c->Object, b)); },
        [] { return ParticlesOf(BITMAP_SMOKE); });
}
