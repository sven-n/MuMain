#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <vector>

// What Monster Checks checks. Today these are the checks of Load and Save
// (MonsterDataHandler::Validate): the names (the separator, language codes, an
// empty English name, the length the game holds) and names in a language the
// game has no texts for; and the checks of the model list's load
// (MonsterDataHandler::ValidateModels), among them the model files that are
// not in the client folder (a monster without a body, P1); and the checks of
// the monsters' attacks (MonsterAttackChecks). The checks of later monster
// fields go here too.
namespace MuEditor::Monsters::MonsterChecks
{
// Appends the problems of the monster and model tables as they are now to
// `issues`. Each problem of a monster has its number (ItemDataIssue::number)
// and a field that MonsterCheckTargets reads; a problem of a model has no
// number and names the model in its field ("model MONSTER_MODEL_MOLT file").
void Run(std::vector<Data::Items::ItemDataIssue>& issues);
} // namespace MuEditor::Monsters::MonsterChecks

#endif // _EDITOR
