#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

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
// problems to `issues`. `source` names the file in messages. An entry without
// a name ({"number": 73}) is read with entryWithoutName.
void ReadItemGroupJson(std::string_view text, const std::string& source, std::vector<ItemDefinition>& items,
                       std::vector<ItemDataIssue>& issues);

// Writes the items of `group` and its entries without a name (other items are ignored).
std::string WriteItemGroupJson(int group, std::span<const ItemDefinition> items);

// The names of the classes in "classRequirements", in the order of their numbers (CLASS_WIZARD, ...).
std::span<const char* const> GetClassKeys();

// A tag that only picked tooltip lines, gone since the tooltip layout presets (D73), and what an item file writes
// instead: the preset its "tooltipLayout" names (`bothPreset` for an item that had both noClassLines and
// noDurabilityLine), or, without one, the line ids its "tooltipLayoutChanges" hides. An item file that still has the
// tag is read as that, with a warning that says so; a save writes it.
struct RemovedTooltipTag
{
    std::string_view tag;
    std::string_view preset;
    std::string_view bothPreset;
    std::span<const std::string_view> hide;
};
std::span<const RemovedTooltipTag> GetRemovedTooltipTags();
// The removed tag of that name; nullptr for another name.
const RemovedTooltipTag* FindRemovedTooltipTag(std::string_view tag);
// What an item file writes instead of the tag: "tooltipLayout": "noClassLines" (...), or "tooltipLayoutChanges":
// {"hide": ["requiredLevel", "lackingLevel"]}.
std::string DescribeRemovedTooltipTag(const RemovedTooltipTag& removed);

// One item as the item files have it, without its group, which the file has. The item
// exchange file with OpenMU (ItemExchangeFile) has its items so too.
Json::OrderedJson WriteItemJson(const ItemDefinition& definition);
// Reads what WriteItemJson writes into `definition`, an item of `group`; false with fatal problems.
// The problems are added to `issues`. Without `nameRequired`, an item without a name is read
// with none (an exchange file that does not carry the names).
bool ReadItemJson(const Json::OrderedJson& json, const std::string& source, int group, ItemDefinition& definition,
                  std::vector<ItemDataIssue>& issues, bool nameRequired = true);
// The layout of the item files (some lists and objects on one line) for a text with items.
std::string LayOutItemJsonText(const std::string& text);
} // namespace Data::Items
