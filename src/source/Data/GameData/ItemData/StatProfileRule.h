#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <bitset>
#include <span>
#include <string>

// The profile rules of the item stat file (Data/Items/Stats/ItemStats.json): which
// stat profile an item's data gives it. See docs/item-data.md, "Profile rules".
namespace Data::Items
{
constexpr size_t ItemSlotCount = static_cast<size_t>(ItemSlot::Ring) + 1;
constexpr size_t WingTierCount = static_cast<size_t>(WingTier::Third) + 1;
constexpr size_t WeaponKindCount = static_cast<size_t>(WeaponKind::Book) + 1;

// Gives the items it matches a profile. Each condition is a set of values; an
// item meets it when it has one of them, and an empty set is a condition the
// rule does not have. A rule without conditions matches every item.
struct StatProfileRule
{
    // A profile name of the file, or "default".
    std::string profile;
    ItemTagSet tags;
    std::bitset<ItemSlotCount> slots;
    std::bitset<WingTierCount> wingTiers;
    std::bitset<WeaponKindCount> weaponKinds;
    std::bitset<MAX_ITEM_TYPE> groups;
    // A base requirement (ItemDefinition::requirements) above 0; a requirement
    // given by level does not count.
    std::bitset<ItemRequirementCount> requirements;

    bool operator==(const StatProfileRule& other) const = default;
};

constexpr int NoStatProfileRule = -1;

bool Matches(const StatProfileRule& rule, const ItemDefinition& item);

// The first rule `item` matches, or NoStatProfileRule.
int FindMatchingRule(std::span<const StatProfileRule> rules, const ItemDefinition& item);
} // namespace Data::Items
