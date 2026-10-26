#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <string>

// What an undo step of the monster tools did, for the editor bar's tooltips:
// "Add Budge Dragon (2)", "Budge Dragon (2): kind", "Budge Dragon (2): name
// (Deutsch)".
namespace MuEditor::Monsters::StepLabel
{
// "<English name> (<number>)", as the logs name a monster.
std::string LogName(const Data::Monsters::MonsterDefinition& monster);

std::string Add(const Data::Monsters::MonsterDefinition& monster);
std::string Remove(const Data::Monsters::MonsterDefinition& monster);
std::string Kind(const Data::Monsters::MonsterDefinition& monster);
// The name in one language.
std::string Name(const Data::Monsters::MonsterDefinition& monster, const char* locale);
} // namespace MuEditor::Monsters::StepLabel

#endif // _EDITOR
