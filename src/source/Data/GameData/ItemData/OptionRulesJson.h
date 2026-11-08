#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/OptionRule.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The option rules, Data/Items/Options/OptionRules.json:
//
//   {
//     "formatVersion": 1,
//     "rules": [
//       {"options": [], "tags": ["ammunition"]},
//       {"options": ["luck"], "group": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]},
//       {"options": ["luck"], "group": [12], "numbers": [0, 1, 2, 3, 4, 5, 6, 41, 42, 49]}
//     ]
//   }
//
// The first rule an item matches gives its definitions; an item no rule matches has none. A rule has the conditions of
// the item stat file's profile rules, and "numbers" (item numbers, with "group") and tags an item must not have
// ("!ammunition"). See docs/item-data.md, "Option rules".
namespace Data::Items
{
constexpr int OptionRulesFormatVersion = 1;
constexpr const char* OptionRulesFileName = "OptionRules.json";

// Reads the file into `rules`, its problems into `issues` (`source` names the file in them); a rule with fatal problems
// is left out. Whether the definitions the rules name exist is checked with them (ValidateOptionRules).
void ReadOptionRulesJson(std::string_view text, const std::string& source, std::vector<OptionRule>& rules,
                         std::vector<ItemDataIssue>& issues);

// The text of the file: one rule a line, in order.
std::string WriteOptionRulesJson(std::span<const OptionRule> rules);
} // namespace Data::Items
