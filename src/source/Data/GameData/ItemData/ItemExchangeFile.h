#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStatFile.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The item exchange file with OpenMU (phase 7, the items design's section 6): the item data
// both sides have and the item formulas, one JSON file in the layout of the item files.
//
//   {
//     "formatVersion": 1,
//     "source": "MuMain",
//     "gameVersion": "season6",
//     "formulas": { "dropLevelBonus": ..., "tables": [...], "default": {...}, "profiles": [...] },
//     "items": [
//       {"group": 0, "number": 0, "name": {"en": "Kris", ...}, "width": 1, ..., "statProfile": "weapon"}
//     ]
//   }
//
// The items have the fields of the item files that OpenMU has too, in their names and
// meanings, and their group; a file from OpenMU also names the server ("server"). The formulas
// are the item stat file without its profile rules, which stay with the client. Items are
// written sorted by group and number.
namespace Data::Items
{
constexpr int ItemExchangeFormatVersion = 1;
constexpr const char* ItemExchangeSourceMuMain = "MuMain";
constexpr const char* ItemExchangeSourceOpenMU = "OpenMU";
constexpr const char* ItemExchangeGameVersion = "season6";

struct ItemExchangeFile
{
    std::string source = ItemExchangeSourceMuMain;
    // Only in a file from OpenMU: the server's installation.
    std::string server;
    std::string gameVersion = ItemExchangeGameVersion;
    // Without profile rules. Nothing when the file has no formulas.
    std::optional<ItemStatFile> formulas;
    // The shared fields only (ToExchangeItem).
    std::vector<ItemDefinition> items;
};

// The item as the exchange file has it: the shared fields, the client-only ones at their
// defaults. `statProfile` names the profile the item uses ("default" for the default).
ItemDefinition ToExchangeItem(const ItemDefinition& item);

// The exchange file of the client's items and formulas.
ItemExchangeFile BuildItemExchange(std::span<const ItemDefinition> items, const ItemStatFile& formulas);

// Reads the file into `file` and adds its problems to `issues`; with errors it must not be
// used. A (group, number) that is there twice is an error, and neither item is read. Fields
// the exchange file has not got are left out with a warning. `source` names the file in
// messages.
void ReadItemExchangeJson(std::string_view text, const std::string& source, ItemExchangeFile& file,
                          std::vector<ItemDataIssue>& issues);

std::string WriteItemExchangeJson(const ItemExchangeFile& file);
} // namespace Data::Items

#endif // _EDITOR
