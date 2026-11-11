#pragma once

#include "Data/GameData/ItemData/ItemRuleConditions.h"

#include <span>
#include <string>

// The profile rules of the item stat file (Data/Items/Stats/ItemStats.json): which
// stat profile an item's data gives it. See docs/item-data.md, "Profile rules".
namespace Data::Items
{
// Gives the items its conditions match a profile.
struct StatProfileRule
{
    // A profile name of the file, or "default".
    std::string profile;
    ItemRuleConditions conditions;

    bool operator==(const StatProfileRule& other) const = default;
};

constexpr int NoStatProfileRule = -1;

// The first rule `item` matches, or NoStatProfileRule.
int FindMatchingRule(std::span<const StatProfileRule> rules, const ItemDefinition& item);
} // namespace Data::Items
