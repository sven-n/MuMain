#pragma once

#include "Data/GameData/ItemData/ItemRuleConditions.h"

#include <span>
#include <string>
#include <vector>

// The option rules (Data/Items/Options/OptionRules.json): which option definitions an item's data gives it. The game
// reads the lists the items have ("options" in the item files); the editor writes them from these rules, and the data
// test checks the shipped lists against them. See docs/item-data.md, "Option rules".
namespace Data::Items
{
// Gives the items its conditions match its definitions (an empty list: no options).
struct OptionRule
{
    std::vector<std::string> options;
    ItemRuleConditions conditions;

    bool operator==(const OptionRule& other) const = default;
};

// The definitions the first rule `item` matches gives, in the rule's order; none when it matches no rule.
std::vector<std::string> DeriveItemOptions(std::span<const OptionRule> rules, const ItemDefinition& item);

// The definitions of an item after the bmd import or the item editor changed it (previous: the item before the change,
// nullptr for a new item), as its stat profile follows the profile rules: while the item has the list its data gave it,
// or is new, the list follows its data; a list that differs from it stays as long as it differs, an empty one too, and
// so does a list the change itself picked. Lists are compared by their names, in any order.
std::vector<std::string> FollowItemOptions(std::span<const OptionRule> rules, const ItemDefinition* previous,
                                           const ItemDefinition& updated);
} // namespace Data::Items
