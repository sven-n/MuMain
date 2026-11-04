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
//     "formatVersion": 2,
//     "texts": {
//       "defense": {"en": "Defense: {defense}", "de": "Verteidigung: {defense}", ...},
//       "lacking": {"en": "(lacking {missing})", ...}
//     },
//     "layouts": [
//       {
//         "name": "default",
//         "lines": [
//           {"id": "firstRow", "text": ""},
//           {"section": "name"},
//           {"section": "itemCases"},
//           {"text": "@defense", "when": ["defense"],
//            "colors": [{"when": ["defenseHarmonyBonus"], "color": "yellow"},
//                       {"when": ["armour", "excellent"], "color": "blue"}]},
//           {"section": "description"},
//           {"text": "@lacking", "values": {"missing": "missingLevel"}, "when": ["missingLevel"], "color": "red"},
//           {"section": "classes"},
//           {"id": "classLines", "text": "@canBeEquippedBy", "forEachClass": true},
//           {"section": "options"},
//           {"id": "blankAfterOptions", "text": ""},
//           {"section": "afterOptions"}
//         ]
//       },
//       {"name": "noClassLines", "base": "default", "hide": ["classLines"]}
//     ]
//   }
//
// A line shows when every entry of its "when" holds (ItemTooltipConditions.h). Its color is that of the
// first rule of "colors" whose "when" holds, else "color" (white when left out). Its text is a shared text
// ("@name"), a text written in place, or "" for a blank line; "values" names the built-in value of a placeholder
// of the text for this line. A line with "forEachClass" is written once for each class the item's
// classRequirements name, with {className} and the state classUsable for that class. A section is a part of the
// tooltip the code still writes; each layout has every section once, in their order, and the lines after a section
// come right after what the code writes there (the lines before the first come first).
//
// A layout with a "base" is a preset (D73): its base's lines without the lines whose "id" its "hide" lists, and with
// the lines of each "add" entry after the line of that entry's "after" id. An item names its layout with
// "tooltipLayout" (the default one without) and makes its own changes on top of it with "tooltipLayoutChanges".
namespace Data::Items
{
constexpr int ItemTooltipJsonFormatVersion = 2;
// The oldest version this client reads. Version 1 is from before the tooltip's code lines became layout lines (D72):
// its layout has neither those lines nor the sections they go between.
constexpr int ItemTooltipJsonOldestFormatVersion = 2;
constexpr const char* ItemTooltipFileName = "Tooltip.json";
// The layout of an item that names none.
constexpr const char* DefaultTooltipLayout = "default";
// The id of the default layout's class lines ("Can be equipped by ..."); Item Checks warns about an item of a kind
// without class lines whose layout has them.
constexpr const char* ClassLinesId = "classLines";

// The parts of a tooltip that stay code, in the order a layout has them.
enum class TooltipSection : BYTE
{
    None, // a line, not a section
    // The item's name, and what the code writes before it (the shop prices).
    Name,
    // The item cases whose lines need the game (the reset fruits' points, the saved places, the socket seeds, ...).
    ItemCases,
    // The item's own texts: the code's item texts, the description of the item data, durability.
    Description,
    // The classes that can use the item, and the boots' and gloves' level texts.
    Classes,
    // The excellent, Jewel of Harmony and other options.
    Options,
    // The description's lines after the options and the Horn of Fenrir's; the set and socket options come after the
    // lines of this section.
    AfterOptions,
};
std::string_view GetTooltipSectionName(TooltipSection section);

struct TooltipLayoutLine
{
    TooltipSection section = TooltipSection::None;
    // The name a preset's or an item's "hide" and "after" use for the line; empty for most lines.
    std::string id;
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
    // Written once for each class the item's classRequirements name, in their order ("forEachClass").
    bool forEachClass = false;

    bool operator==(const TooltipLayoutLine& other) const = default;
};

// Lines a preset puts after the line of `after`'s id.
using TooltipLayoutAddition = TooltipLineAddition<TooltipLayoutLine>;

struct TooltipLayout
{
    std::string name;
    // A preset's base layout; empty for a layout of its own lines.
    std::string base;
    // A layout of its own lines (none for a preset).
    std::vector<TooltipLayoutLine> lines;
    // A preset's changes to its base: the ids of the lines it leaves out, and the lines it adds.
    std::vector<std::string> hide;
    std::vector<TooltipLayoutAddition> add;

    bool IsPreset() const
    {
        return !base.empty();
    }
};

struct ItemTooltipFile
{
    std::map<std::string, LocalizedString, std::less<>> texts;
    std::vector<TooltipLayout> layouts;

    const LocalizedString* FindText(std::string_view name) const;
    const TooltipLayout* FindLayout(std::string_view name) const;
};

// A line of an item's layout as built: a line of the tooltip file, or one the item adds itself.
struct BuiltTooltipLine
{
    const TooltipLayoutLine* layout = nullptr;
    const ItemDescriptionLine* item = nullptr;

    TooltipSection GetSection() const
    {
        return layout != nullptr ? layout->section : TooltipSection::None;
    }
    std::string_view GetId() const
    {
        return layout != nullptr ? std::string_view(layout->id) : std::string_view();
    }
};

// The lines of the layout of `name` as built: a preset's base first, then its changes. Into `lines`, which it
// clears; empty for a layout the file doesn't have (or a preset whose bases go round in a circle). The lines point
// into `file`.
void BuildTooltipLayout(const ItemTooltipFile& file, std::string_view name, std::vector<BuiltTooltipLine>& lines);
// The layout of an item's tooltip as built: the one it names ("tooltipLayout"; default without), then its own changes
// ("tooltipLayoutChanges"). The lines point into `file` and `definition`.
void BuildItemTooltipLayout(const ItemTooltipFile& file, const ItemDefinition& definition,
                            std::vector<BuiltTooltipLine>& lines);
// Whether the built lines have a line of that id.
bool HasTooltipLine(std::span<const BuiltTooltipLine> lines, std::string_view id);
// The name of the layout an item uses: its "tooltipLayout", or the default layout without one.
std::string_view GetItemTooltipLayoutName(const ItemDefinition& definition);

// The warning for a line without "forEachClass" that names what only such a line fills in: {className}, which is empty
// there, or the state classUsable, which never holds there; empty when it names neither. For a layout line, and for an
// item's own line (a description, name or added line), which never has "forEachClass".
std::string CheckClassOnlyValues(const TooltipLayoutLine& line, const ItemTooltipFile& file);
std::string CheckClassOnlyValues(const ItemDescriptionLine& line, const ItemTooltipFile& file);
// The same warning for conditions alone (a ground label rule's): empty when they don't name classUsable.
std::string CheckClassOnlyConditions(std::span<const TooltipCondition> conditions);

// The problems of the layout an item names, each a field of the item and its message: a layout the tooltip file
// doesn't have, an id its changes hide or add after that the layout doesn't have; none when it is fine.
std::vector<std::pair<std::string, std::string>> CheckItemTooltipLayout(const ItemTooltipFile& file,
                                                                        const ItemDefinition& definition);
// The problems of an item's own changes on the layout of `name`, as CheckItemTooltipLayout gives them: an id they hide
// or add after that the layout doesn't have; none when they fit it, or when the file has no such layout.
std::vector<std::pair<std::string, std::string>>
CheckTooltipLayoutChanges(const ItemTooltipFile& file, std::string_view name, const ItemTooltipLayoutChanges& changes);

// Reads the file into `file` and adds its problems to `issues`; with fatal problems the file must not be used. `source`
// names the file in messages. Whether every shared text is used is checked with the items (ValidateDescriptionTexts).
void ReadItemTooltipJson(std::string_view text, const std::string& source, ItemTooltipFile& file,
                         std::vector<ItemDataIssue>& issues);
// Warns about the texts' translations in languages the game has no texts in (`uiLocales`), as for item names.
void ValidateTooltipTextLocales(const ItemTooltipFile& file, std::span<const char* const> uiLocales,
                                const std::string& source, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
