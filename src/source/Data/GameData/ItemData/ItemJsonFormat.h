#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The item data file format: one JSON file per item group.
//
//   {
//     "formatVersion": 1,
//     "group": 0,
//     "items": [
//       { "number": 1, "name": { "en": "Short Sword", "es": "Espada Corta", "pt": "Espada curta" }, ... }
//     ]
//   }
//
// "name" holds the English name ("en", required) and the translations, keyed
// by UI locale code. A plain text is read as the English name only.
//
// Fields with their default value are left out when writing and get the
// default when reading, so adding a field never breaks older files. Items
// are written sorted by number with a fixed field order, so the same data
// always gives the same text.
namespace Data::Items
{
constexpr int ItemJsonFormatVersion = 1;

// Reads one group file. Appends the items it could read to `items` and the
// problems to `issues`. `source` names the file in messages.
void ReadItemGroupJson(std::string_view text, const std::string& source, std::vector<ItemDefinition>& items,
                       std::vector<ItemDataIssue>& issues);

// Writes the items of `group` (other items are ignored).
std::string WriteItemGroupJson(int group, std::span<const ItemDefinition> items);
} // namespace Data::Items
