#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterChecks.h"

#include "MonsterAttackChecks.h"
#include "MonsterGroupChecks.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "I18N/All.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterItemReferences.h"

namespace MuEditor::Monsters::MonsterChecks
{
namespace
{
std::vector<const Data::Monsters::MonsterDefinition*> AllMonsters()
{
    std::vector<const Data::Monsters::MonsterDefinition*> monsters;
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        if (const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(number))
            monsters.push_back(monster);
    }
    return monsters;
}

// A monster naming an item the item list does not have (D60).
void ValidateItems(std::vector<Data::Items::ItemDataIssue>& issues)
{
    const std::vector<const Data::Monsters::MonsterDefinition*> monsters = AllMonsters();
    Data::Monsters::ValidateMonsterItems(
        monsters, [](int itemType) { return g_ItemDatabase.FindEntry(itemType) != nullptr; },
        Data::Monsters::GetMonstersFile().generic_string(), issues);
}
} // namespace

void RunMonsters(std::vector<Data::Items::ItemDataIssue>& issues)
{
    // No check of the kind against the old kind rules of the number: a kind
    // that differs from them is a choice (566 and 567 are NPCs), not a problem.
    g_MonsterDataHandler.Validate(issues);
    AttackChecks::Validate(issues);
    GroupChecks::Validate(issues);
    ValidateItems(issues);
    Data::Monsters::ValidateMonsterSetups(AllMonsters(), Data::Monsters::GetMonstersFile().generic_string(), issues);
    Data::Monsters::ValidateMonsterTalkSteps(AllMonsters(), g_MonsterDatabase.GetTalkTexts(),
                                             Data::Monsters::GetMonstersFile().generic_string(), issues);
    Data::Monsters::ValidateMonsterTalkTexts(AllMonsters(), g_MonsterDatabase.GetTalkTexts(),
                                             I18N::GetAvailableLocales(),
                                             Data::Monsters::GetMonstersFile().generic_string(), issues);
}

void RunModels(std::vector<Data::Items::ItemDataIssue>& issues)
{
    g_MonsterDataHandler.ValidateModels(issues);
}

void RunSounds(std::vector<Data::Items::ItemDataIssue>& issues)
{
    g_MonsterDataHandler.ValidateSounds(issues);
}
} // namespace MuEditor::Monsters::MonsterChecks

#endif // _EDITOR
