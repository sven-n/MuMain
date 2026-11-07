#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The texts of the options' tooltip lines, Data/Items/Tooltips/OptionTexts.json, by name:
//
//   {
//     "formatVersion": 1,
//     "texts": {
//       "luckCriticalDamage": {"en": "Luck (critical damage rate +{value}%)", "de": "...", ...}
//     }
//   }
//
// {value} is the option's value at the item's level. An option names its texts ("text" in Data/Items/Options), and
// names are looked up in this file only. See docs/item-data.md, "Item options".
namespace Data::Items
{
constexpr int OptionTextsFormatVersion = 1;
constexpr const char* OptionTextsFileName = "OptionTexts.json";
// The one placeholder of the option texts.
constexpr const char* OptionValuePlaceholder = "value";

// Reads the file into `texts`, its problems into `issues` (`source` names the file in them). A text without English or
// with a placeholder problem is an error and left out: the lines that name it are left out.
void ReadOptionTextsJson(std::string_view text, const std::string& source, ItemOptionTexts& texts,
                         std::vector<ItemDataIssue>& issues);

// The text of the file: the texts by name, each language on a line of its own, as Tooltip.json's texts.
std::string WriteOptionTextsJson(const ItemOptionTexts& texts);

// Warns about the texts' translations in languages the game has no texts in (`uiLocales`), as for item names.
void ValidateOptionTextLocales(const ItemOptionTexts& texts, std::span<const char* const> uiLocales,
                               const std::string& source, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
