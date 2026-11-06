#include "stdafx.h"

#include "OptionRule.h"

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
} // namespace Data::Items
