#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "MonsterModelRecorder.h"

#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "UI/MonsterTools/MonsterAttackChecks.h"

#include <set>
#include <string>
#include <vector>

// Data Checks with the models open, as in the game once the monsters
// appeared: the shipped attacks play no animation that their model lacks.
TEST_CASE("Monster attacks: the shipped attacks play only animations that their opened models have "
          "[monsters][models][editor]")
{
    MonsterRecorder::LoadScope scope(0);
    const std::vector<Data::Monsters::MonsterDefinition> monsters =
        Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json")
            .monsters;
    REQUIRE(monsters.size() > 500);
    g_MonsterDatabase.Build(monsters, "en");

    std::set<int> slots;
    for (const Data::Monsters::MonsterDefinition& monster : monsters)
    {
        const std::optional<int> slot = Data::Monsters::FindMonsterModelReference(monster.model);
        if (monster.attacks && slot && g_MonsterModelDatabase.Find(*slot) != nullptr)
        {
            slots.insert(*slot);
        }
    }
    int opened = 0;
    scope.Run(
        [&]
        {
            for (const int slot : slots)
            {
                Data::Monsters::ModelLoader::Open(slot);
                opened += Data::Monsters::ModelLoader::IsOpen(slot) ? 1 : 0;
            }
        });
    std::vector<Data::Items::ItemDataIssue> issues;
    MuEditor::Monsters::AttackChecks::Validate(issues);
    g_MonsterDatabase.Build({}, "en");

    CHECK(opened == static_cast<int>(slots.size()));
    for (const Data::Items::ItemDataIssue& issue : issues)
    {
        INFO(issue.number << ": " << issue.message);
        CHECK(issue.message.find("does not have") == std::string::npos);
    }
    // The warnings of the data alone (tests/data, test_monster_attack_checks).
    CHECK(issues.size() == 48);
}

#endif // _EDITOR
