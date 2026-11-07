#include "stdafx.h"

#include "ItemRuleConditions.h"

namespace Data::Items
{
namespace
{
// An enum value beyond the set (ItemSlot::None) is in no set.
template <size_t Size, typename TEnum> bool Contains(const std::bitset<Size>& values, TEnum value)
{
    const auto index = static_cast<size_t>(value);
    return index < Size && values.test(index);
}

WORD GetBaseRequirement(const ItemRequirements& requirements, ItemRequirement requirement)
{
    switch (requirement)
    {
    case ItemRequirement::Level:
        return requirements.level;
    case ItemRequirement::Strength:
        return requirements.strength;
    case ItemRequirement::Dexterity:
        return requirements.dexterity;
    case ItemRequirement::Energy:
        return requirements.energy;
    case ItemRequirement::Vitality:
        return requirements.vitality;
    case ItemRequirement::Leadership:
        return requirements.leadership;
    default:
        return 0;
    }
}

bool HasRequirement(const std::bitset<ItemRequirementCount>& requirements, const ItemRequirements& values)
{
    for (size_t index = 0; index < ItemRequirementCount; ++index)
    {
        if (requirements.test(index) && GetBaseRequirement(values, static_cast<ItemRequirement>(index)) > 0)
        {
            return true;
        }
    }
    return false;
}
} // namespace

bool Matches(const ItemRuleConditions& conditions, const ItemDefinition& item)
{
    if (!conditions.tags.IsEmpty() && !item.tags.HasAny(conditions.tags))
    {
        return false;
    }
    if (conditions.slots.any() && !Contains(conditions.slots, item.slot))
    {
        return false;
    }
    if (conditions.wingTiers.any() && !Contains(conditions.wingTiers, item.wingTier))
    {
        return false;
    }
    if (conditions.weaponKinds.any() && !Contains(conditions.weaponKinds, item.weaponKind))
    {
        return false;
    }
    if (conditions.groups.any() && (item.group < 0 || !Contains(conditions.groups, item.group)))
    {
        return false;
    }
    if (conditions.numbers.any() && (item.number < 0 || !Contains(conditions.numbers, item.number)))
    {
        return false;
    }
    if (item.tags.HasAny(conditions.withoutTags))
    {
        return false;
    }
    return conditions.requirements.none() || HasRequirement(conditions.requirements, item.requirements);
}
} // namespace Data::Items
