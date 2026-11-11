#pragma once

#ifdef _EDITOR

#include "MonsterShownValues.h"

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
// The kind of the values a tool shows, named by ShownValues::LogName: "Budge
// Dragon (2): kind", "Death Centurion (3 numbers): kind".
std::string Kind(const ShownValues::ShownMonster& shown);
// The name in one language.
std::string Name(const Data::Monsters::MonsterDefinition& monster, const char* locale);
// The attacks of the values a tool shows: "Ice Walker (456): attacks".
std::string Attacks(const ShownValues::ShownMonster& shown);
// A change of a model entry: "MONSTER_MODEL_HOUND: speed of walk".
std::string Model(const std::string& slot, const std::string& what);
// The sound set the values a tool shows name, also their variants': "Hound
// (1): sound set".
std::string SoundSetOf(const ShownValues::ShownMonster& shown);
// A change of a sound set: "Hound (sound set): sound events".
std::string SoundSet(const std::string& name, const std::string& what);
// The Monster Browser's number actions (D61): "Add number 600 to Death
// Centurion", "Copy Death Centurion (336) as 601", "Move Balram (409) to
// Balram", "Split Death Centurion (175) out of Death Centurion", "Remove
// Death Centurion with 7 numbers", "Death Centurion (145): maps".
std::string AddNumber(int number, const std::string& monster);
std::string CopyNumber(const Data::Monsters::MonsterDefinition& source, int number);
std::string MoveNumber(const Data::Monsters::MonsterDefinition& number, const std::string& monster);
std::string SplitNumber(const Data::Monsters::MonsterDefinition& number, const std::string& monster);
std::string RemoveMonsterWithNumbers(const std::string& monster, int numberCount);
std::string Maps(const Data::Monsters::MonsterDefinition& number);
} // namespace MuEditor::Monsters::StepLabel

#endif // _EDITOR
