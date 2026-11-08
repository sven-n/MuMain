#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <vector>

// The checks of the monsters with their numbers for Data Checks (D61). Both
// are warnings: the game plays the monsters as they are.
// - A number whose own attacks are the generic pattern while its monster has
//   attacks: the later copies of P26, which play as in the original game.
// - A map variant on maps where its numbers do not appear (Q-MN61): a shared
//   variant on maps none of the numbers that share it is on (named by the
//   first of them), a number's own one on maps that number is not on.
//   Numbers without maps are not checked.
namespace MuEditor::Monsters::GroupChecks
{
// Appends the problems of the monster table as it is now. Each has the
// number and the field of its attacks ("monster 336 attacks", "monster 145
// mapVariants[0].attacks"), so a click opens Monster Attacks.
void Validate(std::vector<Data::Items::ItemDataIssue>& issues);
} // namespace MuEditor::Monsters::GroupChecks

#endif // _EDITOR
