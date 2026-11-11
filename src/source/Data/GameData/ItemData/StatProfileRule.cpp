#include "stdafx.h"

#include "StatProfileRule.h"

namespace Data::Items
{
int FindMatchingRule(std::span<const StatProfileRule> rules, const ItemDefinition& item)
{
    for (size_t index = 0; index < rules.size(); ++index)
    {
        if (Matches(rules[index].conditions, item))
        {
            return static_cast<int>(index);
        }
    }
    return NoStatProfileRule;
}
} // namespace Data::Items
