#include "stdafx.h"

#include "ItemOptionDatabase.h"

#include <algorithm>
#include <array>

namespace Data::Items
{
namespace
{
// The order the game takes an item's definitions in, by type (the tooltip shows their lines in it): wing options,
// the skill (no definition), luck, the additional option, the Dark Horse, excellent options, fixed options.
constexpr std::array<ItemOptionType, 2> TakeOrder = {ItemOptionType::Luck, ItemOptionType::Option};

size_t GetTakeOrder(ItemOptionType type)
{
    return static_cast<size_t>(std::find(TakeOrder.begin(), TakeOrder.end(), type) - TakeOrder.begin());
}
} // namespace

ItemOptionDatabase::ItemOptionDatabase(std::vector<ItemOptionDefinition> definitions, ItemOptionTexts texts,
                                       std::vector<OptionRule> rules)
    : m_definitions(std::move(definitions)), m_texts(std::move(texts)), m_rules(std::move(rules))
{
    m_effects.reserve(m_definitions.size());
    for (const ItemOptionDefinition& definition : m_definitions)
    {
        std::vector<ItemOptionEffect>& effects = m_effects.emplace_back();
        for (const ItemOption& option : definition.options)
        {
            effects.push_back(FindItemOptionEffect(definition.type, option.attribute, option.add)
                                  .value_or(ItemOptionEffect{NoItemOptionEffect, false}));
        }
    }
}

int ItemOptionDatabase::Find(std::string_view name) const
{
    const auto found = std::find_if(m_definitions.begin(), m_definitions.end(),
                                    [name](const ItemOptionDefinition& definition) { return definition.name == name; });
    return found != m_definitions.end() ? static_cast<int>(found - m_definitions.begin()) : NoDefinition;
}

std::vector<ItemOptionIndex> ItemOptionDatabase::ResolveLinks(std::span<const std::string> names) const
{
    std::vector<std::string> ordered(names.begin(), names.end());
    OrderLinks(ordered);
    std::vector<ItemOptionIndex> links;
    for (const std::string& name : ordered)
    {
        const int index = Find(name);
        if (index == NoDefinition)
        {
            continue;
        }
        // The definitions of one type read the same part of the server's item data: the first counts.
        const ItemOptionType type = m_definitions[static_cast<size_t>(index)].type;
        const bool typeTaken = std::any_of(links.begin(), links.end(),
                                           [&](ItemOptionIndex link) { return m_definitions[link].type == type; });
        if (!typeTaken)
        {
            links.push_back(static_cast<ItemOptionIndex>(index));
        }
    }
    return links;
}

void ItemOptionDatabase::OrderLinks(std::vector<std::string>& names) const
{
    const auto key = [this](const std::string& name)
    {
        const int index = Find(name);
        const size_t order =
            index == NoDefinition ? TakeOrder.size() : GetTakeOrder(m_definitions[static_cast<size_t>(index)].type);
        return std::make_pair(order, index == NoDefinition ? std::string() : name);
    };
    std::stable_sort(names.begin(), names.end(),
                     [&](const std::string& left, const std::string& right) { return key(left) < key(right); });
}
} // namespace Data::Items
