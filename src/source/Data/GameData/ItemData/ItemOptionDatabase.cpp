#include "stdafx.h"

#include "ItemOptionDatabase.h"

#include <algorithm>
#include <array>
#include <optional>

namespace Data::Items
{
namespace
{
// The order the game takes an item's definitions in, by type (the tooltip shows the option slots' lines in it): wing
// options, the skill (no definition), luck, the additional option, the Dark Horse, excellent options, fixed options.
// The ancient types follow them only for the order of a saved list (names of no definition last): they take no slot,
// and the tooltip shows the ancient bonus above the slots' lines.
constexpr std::array<ItemOptionType, 6> TakeOrder = {ItemOptionType::Wing,          ItemOptionType::Luck,
                                                     ItemOptionType::Option,        ItemOptionType::Excellent,
                                                     ItemOptionType::AncientOption, ItemOptionType::AncientBonus};

// The part of the server's item data a type reads, as one of the types that read it: excellent and wing options both
// read the excellent byte.
ItemOptionType ReadDataOf(ItemOptionType type)
{
    return type == ItemOptionType::Wing ? ItemOptionType::Excellent : type;
}

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
    m_optionOrders.reserve(m_definitions.size());
    for (ItemOptionDefinition& definition : m_definitions)
    {
        std::vector<ItemOptionEffect>& effects = m_effects.emplace_back();
        for (ItemOption& option : definition.options)
        {
            ResolveTermAttributes(option.value);
            effects.push_back(FindItemOptionEffect(definition.type, option.attribute, option.add)
                                  .value_or(ItemOptionEffect{NoItemOptionEffect, false}));
        }
        std::vector<size_t>& order = m_optionOrders.emplace_back(definition.options.size());
        for (size_t index = 0; index < order.size(); ++index)
        {
            order[index] = index;
        }
        if (definition.type == ItemOptionType::Excellent)
        {
            std::stable_sort(order.begin(), order.end(), [&definition](size_t left, size_t right)
                             { return definition.options[left].number > definition.options[right].number; });
        }
        // The wing options by their bit, as CalcWingOptions wrote them, whatever order the file has.
        else if (definition.type == ItemOptionType::Wing)
        {
            std::stable_sort(order.begin(), order.end(), [&definition](size_t left, size_t right)
                             { return definition.options[left].number < definition.options[right].number; });
        }
    }
    for (size_t attackType = 0; attackType < AncientBonusNames.size(); ++attackType)
    {
        const int index = Find(AncientBonusNames[attackType]);
        if (index != NoDefinition && m_definitions[static_cast<size_t>(index)].type == ItemOptionType::AncientBonus)
        {
            m_ancientBonuses[attackType] = index;
        }
    }
}

int ItemOptionDatabase::Find(std::string_view name) const
{
    const auto found = std::find_if(m_definitions.begin(), m_definitions.end(),
                                    [name](const ItemOptionDefinition& definition) { return definition.name == name; });
    return found != m_definitions.end() ? static_cast<int>(found - m_definitions.begin()) : NoDefinition;
}

std::vector<ItemOptionIndex> ItemOptionDatabase::ResolveLinks(const ItemDefinition& item) const
{
    return ResolveLinks(item.options, ExcellentByteMeansOther(item) ? ItemOptionType::Wing : ItemOptionType::Excellent);
}

std::vector<ItemOptionIndex> ItemOptionDatabase::ResolveLinks(std::span<const std::string> names,
                                                              ItemOptionType excellentByte) const
{
    std::vector<std::string> ordered(names.begin(), names.end());
    OrderLinks(ordered);
    const auto typeOf = [this](const std::string& name)
    {
        const int index = Find(name);
        return index == NoDefinition ? std::optional<ItemOptionType>() : m_definitions[static_cast<size_t>(index)].type;
    };
    // A link of the excellent byte's type leaves out the other type that reads the byte.
    const bool linksExcellentByteType =
        std::ranges::any_of(ordered, [&](const std::string& name) { return typeOf(name) == excellentByte; });
    std::vector<ItemOptionIndex> links;
    for (const std::string& name : ordered)
    {
        const int index = Find(name);
        if (index == NoDefinition)
        {
            continue;
        }
        // The definitions of one type read the same part of the server's item data, and so do excellent and wing
        // options (the excellent byte): the first counts, of the two byte types the one the byte holds on this item.
        const ItemOptionType type = m_definitions[static_cast<size_t>(index)].type;
        const bool otherByteType =
            linksExcellentByteType && type != excellentByte && ReadsSameItemData(type, excellentByte);
        const bool typeTaken = std::any_of(links.begin(), links.end(), [&](ItemOptionIndex link)
                                           { return ReadsSameItemData(m_definitions[link].type, type); });
        if (!typeTaken && !otherByteType)
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

bool ExcellentByteMeansOther(const ItemDefinition& item)
{
    return item.slot == ItemSlot::Wings || item.tags.Has(ItemTag::HornMount);
}

bool ReadsSameItemData(ItemOptionType left, ItemOptionType right)
{
    return ReadDataOf(left) == ReadDataOf(right);
}
} // namespace Data::Items
