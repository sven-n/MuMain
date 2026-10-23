#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStatFile.h"

#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
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
    // Read from a file: the (group, number) pairs it has more than once (OpenMU does not keep
    // them unique). None of those items is in `items`.
    std::vector<std::pair<int, int>> repeatedItems;
};

// The tags the exchange file has, as OpenMU has them too (IsAmmunition, IsQuestItem); the client's
// other tags stay with it.
inline constexpr ItemTag ExchangeTags[] = {ItemTag::Ammunition, ItemTag::SecondClassQuestItem,
                                           ItemTag::ThirdClassQuestItem};

// Copies the fields the exchange file has from `from` to `to`, all but the group, the number and
// the stat profile (each direction treats those itself); the other fields of `to` stay. The one
// list of them, for the export (ToExchangeItem) and the import (ToClientItem).
void CopyExchangeFields(const ItemDefinition& from, ItemDefinition& to);

// The file's items by (group, number), each once.
using ExchangeItemIndex = std::map<std::pair<int, int>, const ItemDefinition*>;
ExchangeItemIndex IndexExchangeItems(const ItemExchangeFile& file);

// The item as the exchange file has it: the shared fields, the client-only ones at their
// defaults. `statProfile` names the profile the item uses ("default" for the default).
ItemDefinition ToExchangeItem(const ItemDefinition& item);

// The exchange file of the client's items and formulas.
ItemExchangeFile BuildItemExchange(std::span<const ItemDefinition> items, const ItemStatFile& formulas);

// Reads the file into `file` and adds its problems to `issues`; with errors it must not be
// used, except for an item that is there twice: an error of that item, which then is in
// `repeatedItems` and not read, while the rest of the file can be used. Fields
// the exchange file has not got are left out with a warning. `source` names the file in
// messages.
void ReadItemExchangeJson(std::string_view text, const std::string& source, ItemExchangeFile& file,
                          std::vector<ItemDataIssue>& issues);

// Whether a file read with `issues` can be imported: its only errors are those of its repeated
// items.
bool CanImportItemExchange(const ItemExchangeFile& file, std::span<const ItemDataIssue> issues);

std::string WriteItemExchangeJson(const ItemExchangeFile& file);
} // namespace Data::Items

#endif // _EDITOR
