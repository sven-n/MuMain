#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <bitset>

// The conditions of a rule that gives items something by their data: the profile rules of the item stat file
// (StatProfileRule) and the option rules (OptionRule). See docs/item-data.md, "Profile rules".
namespace Data::Items
{
constexpr size_t ItemSlotCount = static_cast<size_t>(ItemSlot::Ring) + 1;
constexpr size_t WingTierCount = static_cast<size_t>(WingTier::Third) + 1;
constexpr size_t WeaponKindCount = static_cast<size_t>(WeaponKind::Book) + 1;

// Each condition is a set of values; an item meets it when it has one of them, and an empty set is a condition the
// rule does not have. A rule without conditions matches every item.
struct ItemRuleConditions
{
    ItemTagSet tags;
    std::bitset<ItemSlotCount> slots;
    std::bitset<WingTierCount> wingTiers;
    std::bitset<WeaponKindCount> weaponKinds;
    std::bitset<MAX_ITEM_TYPE> groups;
    // A base requirement (ItemDefinition::requirements) above 0; a requirement given by level does not count.
    std::bitset<ItemRequirementCount> requirements;
    // Only the option rules have these two: the item numbers (with groups), and the tags an item must not have
    // ("!ammunition"), which it meets when it has none of them.
    std::bitset<MAX_ITEM_INDEX> numbers;
    ItemTagSet withoutTags;

    bool IsEmpty() const
    {
        return tags.IsEmpty() && slots.none() && wingTiers.none() && weaponKinds.none() && groups.none() &&
               requirements.none() && numbers.none() && withoutTags.IsEmpty();
    }

    bool operator==(const ItemRuleConditions& other) const = default;
};

bool Matches(const ItemRuleConditions& conditions, const ItemDefinition& item);
} // namespace Data::Items
