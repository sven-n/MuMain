#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <optional>
#include <span>
#include <string>

// What the monster tools show and change for the selection (D61): a monster
// with several numbers selected as a whole shows the values its numbers
// share, with the selected number and its maps; else the selected number's
// entry.
namespace MuEditor::Monsters::ShownValues
{
struct ShownMonster
{
    Data::Monsters::MonsterDefinition monster;
    // The values are the shared ones of the monster's numbers.
    bool wholeMonster = false;
    // How many numbers the monster has.
    int numberCount = 1;

    bool operator==(const ShownMonster&) const = default;
};

// The key of the selection for a tool's draft: the number, with
// WholeMonsterKey added for its monster as a whole.
constexpr int WholeMonsterKey = 1 << 16;
int GetSelectedKey();

// The values of a key; nullopt when its number has no entry.
std::optional<ShownMonster> Load(int key);
std::optional<ShownMonster> ReadSelected();

// Stores a change of the shown values, as one undo step: the shared values of
// a whole monster (MonsterEdits::SetSharedValues), else the number's entry
// (SetMonster). False when the store refuses it.
bool Store(const std::string& label, const ShownMonster& shown);

// "Death Centurion (145)", or "Death Centurion (3 numbers)" for a whole
// monster, as the undo steps name it.
std::string LogName(const ShownMonster& shown);

// The English name of the number's monster, the one its numbers share
// ("Death Centurion"); empty for a number without an entry.
std::string MonsterName(int number);
// "145, 175, 336".
std::string ListNumbers(std::span<const int> numbers);
} // namespace MuEditor::Monsters::ShownValues

#endif // _EDITOR
