#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/CodeReferenceScan.h"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Reading one line of C++ code, with its comments and string literals
// blanked, for the monster numbers it names (see MonsterCodeReferences).
namespace MuEditor::Monsters
{
// A MONSTER_ constant of enum EMonsterType.
struct MonsterConstant
{
    int number = 0;
    // MONSTER_START or MONSTER_END: it names no monster, but bounds a range.
    bool marker = false;
};

// The MONSTER_ constants by name.
using MonsterConstants = std::map<std::string, MonsterConstant, std::less<>>;

// A monster number a line names, and how; the number is always 0-1023.
using MonsterMention = CodeReferenceScan::Mention;

// The constant a line inside enum EMonsterType defines, written as
// MONSTER_BUDGE_DRAGON = 2, (the comma left out after the last one): its
// name, which points into `code`, and its number, 0-1023.
std::optional<std::pair<std::string_view, int>> ReadMonsterConstant(std::string_view code);

// Whether the constant only marks where the monster numbers start or end
// (MONSTER_UNDEFINED, MONSTER_START, MONSTER_END).
bool IsMonsterMarker(std::string_view name);

// The monster numbers the line names, each once, by the first kind that fits:
// - Name: a constant that is not a marker;
// - RawId: a number 0-1023 compared (== != < <= > >=) with a variable whose
//   last name ends in MonsterIndex (c->MonsterIndex == 495);
// - Range: a lower and an upper bound of the same variable, both constants or
//   markers, or numbers compared with a MonsterIndex variable as above
//   (78 <= c->MonsterIndex && c->MonsterIndex <= MONSTER_GOLDEN_WHEEL). A range
//   between two markers names nothing.
std::vector<MonsterMention> ReadMonsterMentions(std::string_view code, const MonsterConstants& constants);
} // namespace MuEditor::Monsters

#endif // _EDITOR
