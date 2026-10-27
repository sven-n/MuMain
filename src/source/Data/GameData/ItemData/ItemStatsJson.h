#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemStatFile.h"

#include <string>
#include <string_view>
#include <vector>

// The format of the item stat file, Data/Items/Stats/ItemStats.json:
//
//   {
//     "formatVersion": 1,
//     "dropLevelBonus": {"excellent": 25, "ancient": 30},
//     "tables": [
//       {
//         "name": "damageIncreaseWeapons",
//         "caption": {"en": "Damage Increase (Weapons)"},
//         "values": [0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 31, 36, 42, 49, 57, 66]
//       },
//       {"name": "defenseIncreaseShields", "caption": {...}, "perLevel": 1}
//     ],
//     "default": {
//       "damage": {"table": "damageIncreaseWeapons", "excellent": {"times": 25, "add": 5},
//                  "ancient": {"add": 5, "dropLevelDivisor": 40}},
//       ... "magicPower", "blockRate", "defense", "magicDefense",
//       "requirements": {
//         "levelPerItemLevel": 0,
//         "excellentLevelAdd": 20,
//         "strength": {"add": 20, "levelWeight": 3, "percent": 3},
//         ... "dexterity", "energy", "vitality",
//         "leadership": "base",
//         "strengthPerOptionLevel": 0
//       }
//     },
//     "profiles": [
//       {"name": "sceptre", "magicPower": {"perLevelAfterHalving": 0}}
//     ],
//     "profileRules": [
//       {"profile": "default", "tags": ["ammunition"]},
//       {"profile": "armor", "slot": ["helm", "armor", "pants", "gloves", "boots"]},
//       {"profile": "helper", "group": [13, 14], "requirements": ["level"]}
//     ]
//   }
//
// Names have only letters and digits. "values" is one number for every item
// level or one per item level 0 to 15. The default writes every term of every
// stat ("none" for a term the stat does not have); a profile writes only what
// differs from it, and a term it writes replaces the default's as a whole. A
// left-out number in a term is 0. A requirement formula is an object or "base"
// (the base value stays). The writer sorts tables and profiles by name.
//
// The profile rules give each item the profile its data gives it, in order:
// the first rule an item matches names it ("default" for the default), and an
// item no rule matches has the default. A condition lists values ("group",
// "slot", "wingTier", "weaponKind", "tags", base "requirements" above 0); an
// item meets it with one of them. The writer keeps the rules in order. An
// unknown condition or value is an error, so a reader refuses rules it cannot
// read; a new condition needs a new format version.
namespace Data::Items
{
constexpr int ItemStatsJsonFormatVersion = 1;
constexpr const char* ItemStatsFileName = "ItemStats.json";

// Reads the file into `file` and adds its problems to `issues`; with errors
// the file must not be used. `source` names the file in messages.
void ReadItemStatsJson(std::string_view text, const std::string& source, ItemStatFile& file,
                       std::vector<ItemDataIssue>& issues);

std::string WriteItemStatsJson(const ItemStatFile& file);

// The file without its format version and profile rules, as one object: the formulas of the
// item exchange file with OpenMU (ItemExchangeFile), in the layout of the file.
Json::OrderedJson WriteItemStatFormulasJson(const ItemStatFile& file);
// Reads what WriteItemStatFormulasJson writes; profile rules in it are left out with a warning,
// as they stay with the client.
void ReadItemStatFormulasJson(const Json::OrderedJson& json, const std::string& source, ItemStatFile& file,
                              std::vector<ItemDataIssue>& issues);
// The layout of the item stat file (some lists and objects on one line) for a text with its
// objects.
std::string LayOutItemStatsText(std::string text);
} // namespace Data::Items
