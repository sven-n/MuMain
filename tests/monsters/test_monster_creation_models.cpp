#include "stdafx.h"

// The monsters' "model" (Data/Monsters/Monsters.json) is the model their
// creators in the code give them (monsters design, MN3), and the model they
// appear with on every map: CreateMonster creates each monster on the maps
// where a creator answered differently than on the others, and the test checks
// the model of the character. Off its creators' maps a monster that only map
// creators made was a Bull Fighter; its entry's model now (Q-MN35). An entry
// that names no model and a model whose file is missing have the unknown
// monsters' look, the Ghost (D64).

#include "MonsterCreationScope.h"

#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "World/MapInfra/MapManager.h"

#include <doctest.h>

#include <algorithm>
#include <initializer_list>
#include <set>
#include <vector>

namespace
{
using Data::Monsters::MonsterDefinition;

const std::set<int> NotCreated = {};

// The unknown monsters' body (Monsters.json, unknownMonsters).
constexpr int UnknownLook = MODEL_MONSTER01 + static_cast<int>(MONSTER_MODEL_GHOST);

// The Chaos Castle monsters that get one of two models at random (the Elf or
// the Wizard); their entries name the first.
const std::set<int> RandomModel = {163, 165, 167, 169, 171, 173, 427};

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
            const int created = scope.CreatedModel(monster.number);
            INFO("monster " << monster.number << " on world " << world << ": created " << created << ", its model "
                            << monster.model);
            if (!expected || Data::Monsters::ModelLoader::IsFileMissing(*expected))
            {
                CHECK(created == UnknownLook);
                continue;
            }
            if (RandomModel.contains(monster.number))
            {
                CHECK((created == MODEL_CHAOS_CASTLE_ELF || created == MODEL_CHAOS_CASTLE_WIZARD));
                continue;
            }
            CHECK((created == *expected || IsOtherModel(monster.number, world, created)));
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
