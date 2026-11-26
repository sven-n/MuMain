#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterChecks.h"

#include "MonsterAttackChecks.h"
#include "MonsterGroupChecks.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "I18N/All.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterItemReferences.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"

namespace MuEditor::Monsters::MonsterChecks
{
namespace
{
// A monster naming an item the item list does not have (D60).
void ValidateItems(std::vector<Data::Items::ItemDataIssue>& issues)
{
    Data::Monsters::ValidateMonsterItems(
        g_MonsterDatabase.GetAll(), [](int itemType) { return g_ItemDatabase.FindEntry(itemType) != nullptr; },
        Data::Monsters::GetMonstersFile().generic_string(), issues);
}

// The looks the game has (MN8): a monster or model names one by its name.
bool RenderStyleExists(std::string_view name)
{
    return Data::Looks::g_LookStore.FindRenderStyle(name) != nullptr;
}

bool EffectExists(std::string_view name)
{
    return Data::Looks::g_LookStore.FindEffect(name) != nullptr;
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
    Data::Monsters::ValidateMonsterLooks(g_MonsterDatabase.GetAll(), RenderStyleExists, EffectExists,
                                         Data::Monsters::GetMonstersFile().generic_string(), issues,
                                         Data::Looks::EffectRunsInUpdate);
    Data::Monsters::ValidateMonsterSetups(g_MonsterDatabase.GetAll(),
                                          Data::Monsters::GetMonstersFile().generic_string(), issues);
    Data::Monsters::ValidateMonsterTalkSteps(g_MonsterDatabase.GetAll(), g_MonsterDatabase.GetTalkTexts(),
                                             Data::Monsters::GetMonstersFile().generic_string(), issues);
    Data::Monsters::ValidateMonsterTalkTexts(g_MonsterDatabase.GetAll(), g_MonsterDatabase.GetTalkTexts(),
                                             I18N::GetAvailableLocales(),
                                             Data::Monsters::GetMonstersFile().generic_string(), issues);
}

void RunModels(std::vector<Data::Items::ItemDataIssue>& issues)
{
    g_MonsterDataHandler.ValidateModels(issues);
    Data::Monsters::ValidateMonsterModelLooks(g_MonsterModelDatabase.GetAll(), RenderStyleExists, EffectExists,
                                              Data::Monsters::GetMonsterModelsFile().generic_string(), issues,
                                              Data::Looks::EffectRunsInUpdate);
}

void RunSounds(std::vector<Data::Items::ItemDataIssue>& issues)
{
    g_MonsterDataHandler.ValidateSounds(issues);
}
} // namespace MuEditor::Monsters::MonsterChecks

#endif // _EDITOR
