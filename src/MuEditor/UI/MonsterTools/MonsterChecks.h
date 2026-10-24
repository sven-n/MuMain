#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <vector>

// What Monster Checks checks. Today these are the checks of Load and Save
// (MonsterDataHandler::Validate): the names (the separator, language codes, an
// empty English name, the length the game holds) and names in a language the
// game has no texts for. The checks of later monster fields go here too.
namespace MuEditor::Monsters::MonsterChecks
{
// Appends the problems of the monster table as it is now to `issues`. Each
// problem of a monster has its number (ItemDataIssue::number) and a field
// that MonsterCheckTargets reads.
void Run(std::vector<Data::Items::ItemDataIssue>& issues);
} // namespace MuEditor::Monsters::MonsterChecks

#endif // _EDITOR
