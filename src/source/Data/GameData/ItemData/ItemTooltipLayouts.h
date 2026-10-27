#pragma once

#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemTooltipConditions.h"

#include <map>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The item tooltip file, Data/Items/Tooltips/Tooltip.json: shared texts, and the layouts that say which lines a
// tooltip shows, in which order, color and style (docs/item-data.md, "Tooltip layouts"):
//
//   {
//     "formatVersion": 1,
//     "texts": {
//       "defense": {"en": "Defense: {defense}", "de": "Verteidigung: {defense}", ...},
//       "lacking": {"en": "(lacking {missing})", ...}
//     },
//     "layouts": [
//       {
//         "name": "default",
//         "lines": [
//           {"section": "name"},
//           {"text": "@defense", "when": ["defense"],
//            "colors": [{"when": ["defenseHarmonyBonus"], "color": "yellow"},
//                       {"when": ["armour", "excellent"], "color": "blue"}]},
//           {"section": "description"},
//           {"text": "@lacking", "values": {"missing": "missingLevel"}, "when": ["missingLevel"], "color": "red"},
//           {"section": "classes"},
//           {"text": "", "when": ["tag:sceptre"]},
//           {"section": "options"}
//         ]
//       }
//     ]
//   }
//
// A line shows when every entry of its "when" holds (ItemTooltipConditions.h). Its color is that of the
// first rule of "colors" whose "when" holds, else "color" (white when left out). Its text is a shared text
// ("@name"), a text written in place, or "" for a blank line; "values" names the built-in value of a placeholder
// of the text for this line. A section is a part of the tooltip the code still writes; each layout has every
// section once, in their order.
namespace Data::Items
{
constexpr int ItemTooltipJsonFormatVersion = 1;
constexpr const char* ItemTooltipFileName = "Tooltip.json";
// The layout every item's tooltip uses (until layouts are chosen by item).
constexpr const char* DefaultTooltipLayout = "default";

// The parts of a tooltip that stay code, in the order a layout has them.
enum class TooltipSection : BYTE
{
    None, // a line, not a section
    // The item's name, and what the code writes before it.
    Name,
    // The item's own texts: the code's item texts, the description of the item data, durability.
    Description,
    // The classes that can use the item, and the boots' and gloves' level texts.
    Classes,
    // The excellent, set, socket, Jewel of Harmony and other options, and the rest of the tooltip.
    Options,
};
std::string_view GetTooltipSectionName(TooltipSection section);

struct TooltipLayoutLine
{
    TooltipSection section = TooltipSection::None;
    // The shared text, without its "@"; empty for a text written in place.
    std::string textName;
    // The text written in place; empty (and no textName) for a blank line.
    LocalizedString text;
    std::vector<TooltipCondition> when;
    ItemTextColor color = ItemTextColor::White;
    std::vector<TooltipColorRule> colors;
    bool bold = false;
    // A placeholder of the text and the built-in value it shows on this line.
    std::vector<std::pair<std::string, std::string>> values;

    bool operator==(const TooltipLayoutLine& other) const = default;
};

struct TooltipLayout
{
    std::string name;
    std::vector<TooltipLayoutLine> lines;
};

struct ItemTooltipFile
{
    std::map<std::string, LocalizedString, std::less<>> texts;
    std::vector<TooltipLayout> layouts;

    const LocalizedString* FindText(std::string_view name) const;
    const TooltipLayout* FindLayout(std::string_view name) const;
};

// Reads the file into `file` and adds its problems to `issues`; with errors the file must not be used. `source`
// names the file in messages. Whether every shared text is used is checked with the items (ValidateDescriptionTexts).
void ReadItemTooltipJson(std::string_view text, const std::string& source, ItemTooltipFile& file,
                         std::vector<ItemDataIssue>& issues);
// Warns about the texts' translations in languages the game has no texts in (`uiLocales`), as for item names.
void ValidateTooltipTextLocales(const ItemTooltipFile& file, std::span<const char* const> uiLocales,
                                const std::string& source, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
