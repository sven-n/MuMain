#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The option definition files, every *.json of Data/Items/Options but OptionRules.json:
//
//   {
//     "formatVersion": 1,
//     "definitions": [
//       {
//         "name": "luck",
//         "type": "luck",
//         "displayName": {"en": "Luck"},
//         "openMu": {"id": "00000083-0001-0000-0000-000000000000", "name": "Luck"},
//         "server": {"addsRandomly": true, "addChance": 0.25, "maximumOptionsPerItem": 1},
//         "options": [
//           {"attribute": "criticalDamageChance", "percent": {"values": 5},
//            "text": ["luckJewelOfSoul", "luckCriticalDamage"]}
//         ]
//       }
//     ]
//   }
//
// An option adds its value with "add", "addFinal" or "percent" (exactly one), the value an object: {"values": 5} for a
// constant, {"values": [4, 8, 12, 16]} for one value per level from level 1. "text" names its tooltip lines in
// OptionTexts.json, one name or a list. The file a definition is in is a convention: its type says what it is. See
// docs/item-data.md, "Item options".
namespace Data::Items
{
constexpr int ItemOptionsFormatVersion = 1;

// Reads one file's definitions into `definitions`, each with `file` set to `fileName`, and its problems into `issues`
// (`source` names the file in them); a definition with fatal problems is left out. Which names are used twice, across
// the files too, which texts the options name and which attributes the client knows are checked with all of them
// (ValidateItemOptions).
void ReadItemOptionsJson(std::string_view text, const std::string& source, const std::string& fileName,
                         std::vector<ItemOptionDefinition>& definitions, std::vector<ItemDataIssue>& issues);

// The text of a file with these definitions, in this order.
std::string WriteItemOptionsJson(std::span<const ItemOptionDefinition> definitions);
} // namespace Data::Items
