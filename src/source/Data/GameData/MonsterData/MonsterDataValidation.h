#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <span>
#include <vector>

namespace Data::Monsters
{
// Checks the names of the monsters: the separator of the LocalizedString
// format and a language code that is not one are errors; an empty English name
// (the game shows no name where there is no translation) and a name longer
// than the game holds (it is cut) are warnings. Appends the problems to
// `issues`.
void ValidateMonsterNames(std::span<const MonsterDefinition> monsters, std::vector<Items::ItemDataIssue>& issues);

// A name in a language the game has no texts for is never shown: a warning for
// each. `uiLocales` are the game's languages (I18N::GetAvailableLocales).
void ValidateMonsterNameLocales(std::span<const MonsterDefinition> monsters, std::span<const char* const> uiLocales,
                                std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
