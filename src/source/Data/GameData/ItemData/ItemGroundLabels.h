#pragma once

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemTooltipConditions.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

// The looks of the item names on the ground (Data/Items/Tooltips/GroundLabels.json):
//
//   {
//     "formatVersion": 1,
//     "styles": {
//       "ticket": {"color": "#e58721"},
//       "setItem": {"color": "#00ff00", "background": "#3c3cc8", "bold": true}
//     },
//     "standard": [
//       {"when": ["setItem"], "style": "setItem"},
//       {"style": "level5to6"}
//     ]
//   }
//
// A label style is a text color, a background (black when left out) and bold. An item names its styles in its item
// file ("groundLabels"); at a level none of them is for, its label takes the first standard rule whose conditions hold
// (ItemTooltipConditions.h).
namespace Data::Items
{
// Red, green and blue from 0 to 255, written "#rrggbb".
using GroundLabelColor = std::array<BYTE, 3>;

struct GroundLabelStyle
{
    std::string name;
    GroundLabelColor color{255, 255, 255};
    GroundLabelColor background{0, 0, 0};
    bool bold = false;

    bool operator==(const GroundLabelStyle&) const = default;
};

// A standard rule: the style of the label when its conditions hold (all of them; none: always).
struct GroundLabelRule
{
    std::vector<TooltipCondition> when;
    std::string style;

    bool operator==(const GroundLabelRule&) const = default;
};

struct GroundLabelFile
{
    std::vector<GroundLabelStyle> styles;
    std::vector<GroundLabelRule> standard;
    // The folders MuEditor sorts the styles in ("colorFolders", ColorFolders.h); the game reads and checks them
    // but doesn't use them.
    std::vector<Colors::ColorFolder> folders;

    // The style of that name, or nullptr.
    const GroundLabelStyle* FindStyle(std::string_view name) const;
    // Whether a standard rule gives the style of that name.
    bool IsStandardRuleStyle(std::string_view name) const;

    bool operator==(const GroundLabelFile&) const = default;
};

constexpr int GroundLabelsFormatVersion = 1;
// The file's name in the item tooltip folder (Data/Items/Tooltips).
constexpr const char* GroundLabelsFileName = "GroundLabels.json";

// Reads the file into `file`, with its problems in `issues`; a style or rule that can't be read is left out. Which
// styles the rules and the items name is checked with the items (ValidateGroundLabels).
void ReadGroundLabelsJson(std::string_view text, const std::string& source, GroundLabelFile& file,
                          std::vector<ItemDataIssue>& issues);

// The text of the file: one style a line, one rule a line, as the file is written by hand.
std::string WriteGroundLabelsJson(const GroundLabelFile& file);

// "#rrggbb" of a color, and back (false for a text that is not one).
std::string WriteGroundLabelColor(const GroundLabelColor& color);
bool ReadGroundLabelColor(std::string_view text, GroundLabelColor& color);
} // namespace Data::Items
