#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

// The monster names of the original client, one file per language
// (Data/Local/<Eng|Por|Spn>/NpcName_<lang>.txt), UTF-8:
//
//   // a comment
//   84	1	"Chief Skeleton Warrior"
//   end
//
// Each line is the monster's number, a number the game ignores (always 1) and
// the name in quotes; "end" ends the list.
namespace Data::Monsters
{
// Reads the names of the text by number. A line that is not a name, a number
// outside 0-1023, a number that is there twice (the first name counts, as in
// the game) and a name that is not UTF-8 are skipped with a warning; a name
// longer than the game holds is cut, with a warning. `source` names the file in
// the problems. Returns how many names were cut.
int ReadNpcNameText(std::string_view text, const std::string& source, std::map<int, std::string>& names,
                    std::vector<Items::ItemDataIssue>& issues);

// The text of a name file with these names, sorted by number, with Windows
// line ends as the original files have. A name holding a quote or a line break
// cannot be written: it is left out, with a warning.
std::string WriteNpcNameText(const std::map<int, std::string>& names, std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
