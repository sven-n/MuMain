#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterGroup.h"
#include "Data/GameData/MonsterData/UnknownMonsterLook.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The monster list of the client (Data/Monsters/Monsters.json):
//
//   { "formatVersion": 2,
//     "monsters": [
//       {
//         "name": {"en": "Death Centurion", "es": "Centurión de la Muerte"},
//         "kind": "monster",
//         "model": "MONSTER_MODEL_DEATH_CENTURION",
//         "numbers": [
//           {"number": 145, "maps": [24]},
//           {"number": 336, "maps": [36], "attacks": {"otherwise": "generic", "clearSkill": "never"}}
//         ]
//       } ] }
//
// A monster holds the values its numbers share; "numbers" lists each of its
// numbers as the server sends them (0-1023) with the maps it appears on and
// the values that number has as its own (monsters design, D61, Q-MN61).
// "name" are the names by language (a plain text is the English name), "kind"
// what the game does with it: monster, npc, trap or other. A number's own
// value may be empty where the monster's is not: "model": "", "soundSet": "",
// "attacks": null, "mapVariants": [] and "randomVariants": [].
//
// "unknownMonsters" holds how the game shows a monster it cannot show as
// itself (UnknownMonsterLook.h, D64); a file without it has the look's
// defaults.
//
// Version 1 listed each number with its values ("number" next to "name");
// such a file is read as a monster for each entry.
namespace Data::Monsters
{
constexpr int MonstersFormatVersion = 2;

// Reads the monsters of the text into `monsters`, in their order. A monster
// with an error in its shared values is left out, a number with an error in
// its own values; a number that is there twice is an error.
void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterGroup>& monsters,
                      std::vector<Items::ItemDataIssue>& issues);
// The same, with the unknown monsters' look of the file.
void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterGroup>& monsters,
                      UnknownMonsterLook& unknown, std::vector<Items::ItemDataIssue>& issues);

// The entries of the numbers the text has, as ReadMonstersJson reads them.
void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterDefinition>& numbers,
                      std::vector<Items::ItemDataIssue>& issues);

// The text of the file with these monsters, sorted by their first number, and
// their numbers sorted; without the unknown monsters' look.
std::string WriteMonstersJson(std::span<const MonsterGroup> monsters);
// The same, with the unknown monsters' look (the file the game reads).
std::string WriteMonstersJson(std::span<const MonsterGroup> monsters, const UnknownMonsterLook& unknown);

// The text of the file with a monster for each entry.
std::string WriteMonstersJson(std::span<const MonsterDefinition> numbers);
} // namespace Data::Monsters
