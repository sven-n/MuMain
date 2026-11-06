#include "stdafx.h"

#include "OptionRule.h"

#include <algorithm>

namespace Data::Items
{
std::vector<std::string> DeriveItemOptions(std::span<const OptionRule> rules, const ItemDefinition& item)
{
    for (const OptionRule& rule : rules)
    {
        if (Matches(rule.conditions, item))
        {
            return rule.options;
        }
    }
    return {};
}

namespace
{
bool SameNames(std::vector<std::string> left, std::vector<std::string> right)
{
    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());
    return left == right;
}
} // namespace

std::vector<std::string> FollowItemOptions(std::span<const OptionRule> rules, const ItemDefinition* previous,
                                           const ItemDefinition& updated)
{
    if (previous == nullptr)
    {
        return DeriveItemOptions(rules, updated);
    }
    const bool pickedByTheChange = !SameNames(updated.options, previous->options);
    const bool differedFromItsData = !SameNames(previous->options, DeriveItemOptions(rules, *previous));
    return pickedByTheChange || differedFromItsData ? updated.options : DeriveItemOptions(rules, updated);
}
} // namespace Data::Items
