#include "stdafx.h"

// The monsters' "model" (Data/Monsters/Monsters.json) is the model their
// creators in the code give them (monsters design, MN3): CreateMonster creates
// each monster on the maps where a creator answers differently than on the
// others, and the test checks the model of the character.

#include "MonsterCreationScope.h"

#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "World/MapInfra/MapManager.h"

#include <doctest.h>

#include <algorithm>
#include <initializer_list>
#include <map>
#include <set>
#include <vector>

namespace
{
using Data::Monsters::MonsterDefinition;

// The Gateway Machine's creator needs Kanturu's entrance object, which only
// the game makes. Its model is read from the code.
const std::set<int> NotCreated = {367};

// The Chaos Castle monsters that get one of two models at random (the Elf or
// the Wizard); their entries name the first.
const std::set<int> RandomModel = {163, 165, 167, 169, 171, 173, 427};

// The maps where a creator answers that does not answer on the others, and
// the monsters it creates there. On other maps these monsters are Bull
// Fighters (the code's default), and the server does not send them there.
const std::map<int, std::set<int>> MapCreators = {
    {9, {310, 311, 312, 313, 314, 315, 316, 317, 340, 341, 344, 345, 348, 349, 409, 410, 411, 412, 436, 437, 440}},
    {30, {104, 215, 216, 217, 218, 219, 220, 221, 222, 223, 277, 278, 283, 285, 286, 287, 288}},
    {34, {310, 311, 312, 313, 314, 315, 316, 317, 340, 341, 344, 345, 348, 349, 440}},
    {35, {310, 315, 316, 317, 318}},
    {39, {361, 362, 363, 364}},
    {41, {409, 410, 411, 412, 436, 437}},
    {42, {409, 410, 411, 412, 436, 437}},
    {63, {480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491}},
};

// On the second Crywolf map these three appear with the models of Crywolf's
// stronger monsters (a creator of their own there).
struct OtherModel
{
    int number;
    int world;
    int slot;
};
const OtherModel OtherModels[] = {
    {310, WD_35CRYWOLF_2ND, MODEL_BALRAM},
    {316, WD_35CRYWOLF_2ND, MODEL_VALAM},
    {317, WD_35CRYWOLF_2ND, MODEL_SOLAM},
};

bool IsCreatedOnWorldByMapCreator(int number, int world)
{
    const auto creators = MapCreators.find(world);
    return creators != MapCreators.end() && creators->second.contains(number);
}

bool HasMapCreator(int number)
{
    return std::any_of(MapCreators.begin(), MapCreators.end(),
                       [&](const auto& creators) { return creators.second.contains(number); });
}

bool IsOtherModel(int number, int world, int slot)
{
    return std::any_of(std::begin(OtherModels), std::end(OtherModels), [&](const OtherModel& other)
                       { return other.number == number && other.world == world && other.slot == slot; });
}

void CheckCreatedModels(std::initializer_list<int> worlds)
{
    const std::vector<MonsterDefinition> monsters =
        Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json")
            .monsters;
    REQUIRE(monsters.size() > 500);
    MonsterRecorder::CreationScope scope(*worlds.begin());
    for (const int world : worlds)
    {
        scope.SetWorld(world);
        for (const MonsterDefinition& monster : monsters)
        {
            if (NotCreated.contains(monster.number))
                continue;
            const std::optional<int> expected = Data::Monsters::FindMonsterModelReference(monster.model);
            REQUIRE(expected);
            const int created = scope.CreatedModel(monster.number);
            INFO("monster " << monster.number << " on world " << world << ": created " << created << ", its model "
                            << monster.model);
            if (RandomModel.contains(monster.number))
            {
                CHECK((created == MODEL_CHAOS_CASTLE_ELF || created == MODEL_CHAOS_CASTLE_WIZARD));
                continue;
            }
            const bool elsewhere = HasMapCreator(monster.number) &&
                                   !IsCreatedOnWorldByMapCreator(monster.number, world) &&
                                   created == MODEL_BULL_FIGHTER;
            CHECK((created == *expected || elsewhere || IsOtherModel(monster.number, world, created)));
        }
    }
}
} // namespace

TEST_CASE("Monster data: the creators give each monster the model it names, on a map without map creators "
          "[monsters][creation]")
{
    CheckCreatedModels({0});
}

TEST_CASE("Monster data: the creators give each monster the model it names, in Devil Square and Battle Castle "
          "[monsters][creation]")
{
    CheckCreatedModels({9, 30});
}

TEST_CASE("Monster data: the creators give each monster the model it names, on the Crywolf maps and Kanturu 3rd "
          "[monsters][creation]")
{
    CheckCreatedModels({34, 35, 39});
}

TEST_CASE("Monster data: the creators give each monster the model it names, in the Balgass maps and the PK Field "
          "[monsters][creation]")
{
    CheckCreatedModels({41, 42, 63});
}
