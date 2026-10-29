#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterChecks.h"

#include "MonsterAttackChecks.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"

namespace MuEditor::Monsters::MonsterChecks
{
void RunMonsters(std::vector<Data::Items::ItemDataIssue>& issues)
{
    // No check of the kind against the old kind rules of the number: a kind
    // that differs from them is a choice (566 and 567 are NPCs), not a problem.
    g_MonsterDataHandler.Validate(issues);
    AttackChecks::Validate(issues);
}

void RunModels(std::vector<Data::Items::ItemDataIssue>& issues)
{
    g_MonsterDataHandler.ValidateModels(issues);
}
} // namespace MuEditor::Monsters::MonsterChecks

#endif // _EDITOR
