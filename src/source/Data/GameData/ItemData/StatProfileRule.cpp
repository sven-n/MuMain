#include "stdafx.h"

#include "StatProfileRule.h"

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

bool Matches(const StatProfileRule& rule, const ItemDefinition& item)
{
    if (!rule.tags.IsEmpty() && !item.tags.HasAny(rule.tags))
    {
        return false;
    }
    if (rule.slots.any() && !Contains(rule.slots, item.slot))
    {
        return false;
    }
    if (rule.wingTiers.any() && !Contains(rule.wingTiers, item.wingTier))
    {
        return false;
    }
    if (rule.weaponKinds.any() && !Contains(rule.weaponKinds, item.weaponKind))
    {
        return false;
    }
    if (rule.groups.any() && (item.group < 0 || !Contains(rule.groups, item.group)))
    {
        return false;
    }
    return rule.requirements.none() || HasRequirement(rule.requirements, item.requirements);
}

int FindMatchingRule(std::span<const StatProfileRule> rules, const ItemDefinition& item)
{
    for (size_t index = 0; index < rules.size(); ++index)
    {
        if (Matches(rules[index], item))
        {
            return static_cast<int>(index);
        }
    }
    return NoStatProfileRule;
}
} // namespace Data::Items
