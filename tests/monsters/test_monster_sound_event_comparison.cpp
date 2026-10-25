#include "stdafx.h"

// One-time comparison of MN7 (monsters design, D50): the sounds that
// PlayMonsterSound's hooks played and the traps' sounds of SetPlayerAttack,
// against the sound events of the model entries and the traps' attacks. Each
// case runs once with the lists without them (the old code plays) and once
// with them; the sounds played and the random numbers drawn must be the same.
// It goes with the old code.

#include "MonsterCreationScope.h"
#include "MonsterModelRecorder.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/Random.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Monsters/MonsterAttackPlay.h"
#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#include <doctest.h>

#include <array>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

BOOL PlayMonsterSound(OBJECT* pObject);

namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterModelDefinition;
using MonsterRecorder::PlayedSound;

// Every map a hook answers on, and maps without one.
constexpr std::array<int, 20> Worlds = {0, 1, 51, 56, 57, 58, 63, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 79, 80, 81};
constexpr int AnimationCount = 13;
constexpr std::array<float, 4> Frames = {0.0f, 1.5f, 7.5f, 9.0f};
constexpr std::array<int, 2> LifeTimes = {100, 0};
// Near the hero, and just past the hooks' cut-off of 500.
constexpr std::array<float, 2> Distances = {100.0f, 500.5f};
constexpr std::array<unsigned, 2> Seeds = {1u, 7u};
constexpr std::array<float, 2> FrameRateFactors = {1.0f, 0.25f};

struct Result
{
    std::vector<PlayedSound> plays;
    int rand = 0;
    double unit = 0.0;

    bool operator==(const Result&) const = default;
};

std::string Describe(const Result& result)
{
    std::ostringstream text;
    for (const PlayedSound& play : result.plays)
    {
        text << play.id << (play.object != nullptr ? "@object" : "") << " ";
    }
    text << "| rand " << result.rand << " unit " << result.unit;
    return text.str();
}

void Seed(unsigned seed, float factor)
{
    std::srand(seed);
    Random::Seed(seed);
    FPS_ANIMATION_FACTOR = factor;
}

Result Finish(MonsterRecorder::LoadScope& scope)
{
    Result result;
    result.plays = scope.TakePlayedSounds();
    result.rand = std::rand();
    result.unit = Random::UnitDouble();
    return result;
}

std::vector<MonsterModelDefinition> WithoutSoundEvents(std::vector<MonsterModelDefinition> models)
{
    for (MonsterModelDefinition& model : models)
    {
        model.soundEvents.clear();
        model.soundEventWorlds.clear();
    }
    return models;
}

std::vector<MonsterDefinition> WithoutAttackSounds(std::vector<MonsterDefinition> monsters)
{
    for (MonsterDefinition& monster : monsters)
    {
        if (monster.attacks)
        {
            monster.attacks->sound.clear();
            monster.attacks->soundAtMonster = false;
        }
    }
    return monsters;
}

// The models the hooks name: those with sound events now, the two they name
// without an entry's events (Medusa plays nothing, no monster has the Wolf
// Status's model), and two that no hook names.
std::vector<int> ComparedSlots(const std::vector<MonsterModelDefinition>& models)
{
    std::vector<int> slots = {MODEL_MEDUSA, MODEL_WOLF_STATUS, MODEL_BULL_FIGHTER, MODEL_SMITH};
    for (const MonsterModelDefinition& model : models)
    {
        if (!model.soundEvents.empty())
        {
            slots.push_back(*Data::Monsters::FindMonsterModelSlot(model.slot));
        }
    }
    return slots;
}

std::vector<Result> PlayAll(MonsterRecorder::LoadScope& scope, const std::vector<int>& slots)
{
    std::vector<Result> results;
    OBJECT object;
    for (const int slot : slots)
    {
        for (const int world : Worlds)
        {
            gMapManager.WorldActive = world;
            for (int animation = 0; animation < AnimationCount; ++animation)
            {
                for (const float frame : Frames)
                {
                    for (const int lifeTime : LifeTimes)
                    {
                        for (const float distance : Distances)
                        {
                            for (const unsigned seed : Seeds)
                            {
                                for (const float factor : FrameRateFactors)
                                {
                                    object.Type = static_cast<short>(slot);
                                    object.CurrentAction = static_cast<unsigned short>(animation);
                                    object.AnimationFrame = frame;
                                    object.LifeTime = lifeTime;
                                    object.Position[0] = Hero->Object.Position[0] + distance;
                                    object.Position[1] = Hero->Object.Position[1];
                                    object.Position[2] = Hero->Object.Position[2];
                                    Seed(seed, factor);
                                    PlayMonsterSound(&object);
                                    results.push_back(Finish(scope));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return results;
}
} // namespace

TEST_CASE("Monster sounds: the sound events play what PlayMonsterSound's hooks played (MN7 comparison) "
          "[monsters][sounds][comparison]")
{
    MonsterRecorder::CreationScope creation(0);
    MonsterRecorder::LoadScope& scope = creation.Load();
    g_MapProcess = MapProcess::Make();
    std::vector<MonsterModelDefinition> models;
    for (const MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
    {
        models.push_back(*model);
    }
    const std::vector<int> slots = ComparedSlots(models);
    REQUIRE(slots.size() == 74);
    const float factor = FPS_ANIMATION_FACTOR;
    scope.TakePlayedSounds();

    g_MonsterModelDatabase.Build(WithoutSoundEvents(models));
    const std::vector<Result> old = PlayAll(scope, slots);
    g_MonsterModelDatabase.Build(models);
    const std::vector<Result> now = PlayAll(scope, slots);
    FPS_ANIMATION_FACTOR = factor;

    REQUIRE(old.size() == now.size());
    int differences = 0;
    int played = 0;
    for (size_t i = 0; i < old.size(); ++i)
    {
        played += old[i].plays.empty() ? 0 : 1;
        if (old[i] == now[i])
        {
            continue;
        }
        if (++differences <= 20)
        {
            const size_t perSlot = old.size() / slots.size();
            MESSAGE("case " << i << " slot " << slots[i / perSlot] << ": old " << Describe(old[i]) << " / now "
                            << Describe(now[i]));
        }
    }
    MESSAGE(old.size() << " cases, " << played << " with sounds");
    CHECK(differences == 0);
    // Sanity checks: the comparison saw sounds play, and the sound events, not
    // the hooks, played them in the second run.
    CHECK(played > 10000);
    OBJECT raymond;
    raymond.Type = MODEL_RAYMOND;
    raymond.CurrentAction = MONSTER01_ATTACK1;
    raymond.Position[0] = Hero->Object.Position[0];
    raymond.Position[1] = Hero->Object.Position[1];
    gMapManager.WorldActive = WD_69EMPIREGUARDIAN1;
    scope.TakePlayedSounds();
    CHECK(GameLogic::Monsters::SoundEvents::Play(raymond));
    CHECK(scope.TakePlayedSounds() == std::vector<PlayedSound>{{SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}});
    gMapManager.WorldActive = 0;
}

TEST_CASE("Monster sounds: the traps' attacks play the sounds SetPlayerAttack played (MN7 comparison) "
          "[monsters][sounds][comparison]")
{
    MonsterRecorder::CreationScope creation(1);
    MonsterRecorder::LoadScope& scope = creation.Load();
    g_MapProcess = MapProcess::Make();
    const std::vector<MonsterDefinition> monsters =
        Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json")
            .monsters;
    REQUIRE(monsters.size() > 500);

    const auto attackAll = [&]
    {
        std::vector<Result> results;
        for (const int world : {1, 0, 56})
        {
            creation.SetWorld(world);
            for (const int number : {100, 101, 102, 106})
            {
                for (const float frame : {0.0f, 3.0f})
                {
                    CHARACTER* character = creation.Create(number);
                    REQUIRE(character != nullptr);
                    character->Object.AnimationFrame = frame;
                    Seed(3u, 1.0f);
                    scope.TakePlayedSounds();
                    SetPlayerAttack(character);
                    Result result = Finish(scope);
                    // The trap's object, as the address of this run's character.
                    for (PlayedSound& play : result.plays)
                    {
                        play.object = play.object == &character->Object ? reinterpret_cast<const void*>(1) : nullptr;
                    }
                    results.push_back(std::move(result));
                }
            }
        }
        return results;
    };
    g_MonsterDatabase.Build(WithoutAttackSounds(monsters), "en");
    const std::vector<Result> old = attackAll();
    g_MonsterDatabase.Build(monsters, "en");
    const std::vector<Result> now = attackAll();
    // A sanity check: the attacks, not the code, played the sound.
    CHECK(GameLogic::Monsters::Attacks::PlaysSound(*creation.Create(101)));

    REQUIRE(old.size() == now.size());
    for (size_t i = 0; i < old.size(); ++i)
    {
        INFO("case " << i << ": old " << Describe(old[i]) << " / now " << Describe(now[i]));
        CHECK(old[i] == now[i]);
        CHECK_FALSE(now[i].plays.empty());
    }
}
