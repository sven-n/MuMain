#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterChecks.h"

#include "MonsterAttackChecks.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"

namespace MuEditor::Monsters::MonsterChecks
{
void Run(std::vector<Data::Items::ItemDataIssue>& issues)
{
    // No check of the kind against the old kind rules of the number: a kind
    // that differs from them is a choice (566 and 567 are NPCs), not a problem.
    g_MonsterDataHandler.Validate(issues);
    g_MonsterDataHandler.ValidateModels(issues);
    AttackChecks::Validate(issues);
}
} // namespace MuEditor::Monsters::MonsterChecks

#endif // _EDITOR
