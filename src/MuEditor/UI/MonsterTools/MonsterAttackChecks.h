#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <functional>
#include <optional>
#include <vector>

// The checks of the monsters' attacks for Monster Checks (monsters design,
// section 6). All are warnings: the game plays the attacks as they are.
// - Attacks that no handler reads, also after a model change in Monster
//   Setup takes a monster away from a model-keyed handler.
// - Attacks of a player-shaped monster, which chooses a player attack.
// - Monster skills that packet 0x69 plays no attack for.
// - The animations the attacks can play: one the open model does not have
//   or whose speed is 0, one after which a monster-range model does not go
//   back to stop 1, and one other than attack 1 and 2, which the game does
//   not count as an attack (a hit plays shock over it, no impact frame).
namespace MuEditor::Monsters::AttackChecks
{
// The animation count of the slot's model while the game has it open;
// nullopt when it is not open.
using OpenAnimationCount = std::function<std::optional<int>(int slot)>;

// The game's own: the slot's model in Models.
std::optional<int> GetOpenAnimationCount(int slot);

// Appends the problems of the attacks of the monster table as it is now.
// Each has the monster's number and the field "monster <number> attacks", so
// a click opens Monster Attacks.
void Validate(std::vector<Data::Items::ItemDataIssue>& issues,
              const OpenAnimationCount& openAnimationCount = GetOpenAnimationCount);
} // namespace MuEditor::Monsters::AttackChecks

#endif // _EDITOR
