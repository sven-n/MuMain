#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The monster list of the client (Data/Monsters/Monsters.json):
//
//   { "formatVersion": 1,
//     "monsters": [
//       {
//         "number": 2,
//         "name": {"en": "Budge Dragon", "es": "Dragón Escurridizo", "pt": "Dragão Lento"},
//         "kind": "monster"
//       } ] }
//
// "number" is the monster's number as the server sends it (0-1023), "name" its
// names by language (a plain text is the English name), "kind" what the game
// does with it: monster, npc, trap or other.
namespace Data::Monsters
{
constexpr int MonstersFormatVersion = 1;

// Reads the monsters of the text into `monsters`, in their order. A monster
// with an error is left out; a number that is there twice is an error.
void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterDefinition>& monsters,
                      std::vector<Items::ItemDataIssue>& issues);

// The text of the file with these monsters, sorted by number.
std::string WriteMonstersJson(std::span<const MonsterDefinition> monsters);
} // namespace Data::Monsters
