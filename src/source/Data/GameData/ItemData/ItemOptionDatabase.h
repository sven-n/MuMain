#pragma once

#include "Data/GameData/ItemData/ItemOptionDefinition.h"
#include "Data/GameData/ItemData/ItemOptionEffects.h"
#include "Data/GameData/ItemData/OptionRule.h"

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The option definitions in memory (Data/Items/Options/*.json), their tooltip texts (OptionTexts.json) and, in the
// editor, the option rules (OptionRules.json). Items link definitions by name; ItemDatabase resolves each item's links
// to definition indices when it stores the item, so the game never looks a name up. See docs/item-data.md, "Item
// options".
namespace Data::Items
{
// A definition's index, as an item's resolved links hold it.
using ItemOptionIndex = std::uint16_t;

class ItemOptionDatabase
{
public:
    static constexpr int NoDefinition = -1;

    ItemOptionDatabase() = default;
    // Each option's effect (FindItemOptionEffect) and its terms' hero attributes are looked up here once.
    ItemOptionDatabase(std::vector<ItemOptionDefinition> definitions, ItemOptionTexts texts,
                       std::vector<OptionRule> rules = {});

    std::span<const ItemOptionDefinition> GetDefinitions() const
    {
        return m_definitions;
    }

    const ItemOptionDefinition& Get(ItemOptionIndex index) const
    {
        return m_definitions[index];
    }

    // The index of the definition of that name, or NoDefinition.
    int Find(std::string_view name) const;

    // What the option does in the client: NoItemOptionEffect, writing no value, for an attribute it doesn't compute.
    const ItemOptionEffect& GetEffect(ItemOptionIndex definition, size_t option) const
    {
        return m_effects[definition][option];
    }

    // The order an item takes the definition's options in, as indexes of its options: as the code before the
    // definitions tested their bits, the excellent options from the highest number down (the tooltip shows them so),
    // the others as written. Worked out when the options load.
    const std::vector<size_t>& GetOptionOrder(ItemOptionIndex definition) const
    {
        return m_optionOrders[definition];
    }

    const ItemOptionTexts& GetTexts() const
    {
        return m_texts;
    }

    // The option rules; empty in the game, which reads only the items' lists.
    std::span<const OptionRule> GetRules() const
    {
        return m_rules;
    }

    // The definitions an item's names link, in the order they are written in and taken in (by type, then by name); a
    // name of no definition is left out (the loader reports it), and so is a second definition of a type whose byte a
    // definition before it reads.
    std::vector<ItemOptionIndex> ResolveLinks(std::span<const std::string> names) const;

    // The names in the order ResolveLinks takes them, so a saved list is the same however it was made; names of no
    // definition go last, in the order they had.
    void OrderLinks(std::vector<std::string>& names) const;

private:
    std::vector<ItemOptionDefinition> m_definitions;
    std::vector<std::vector<ItemOptionEffect>> m_effects;
    std::vector<std::vector<size_t>> m_optionOrders;
    ItemOptionTexts m_texts;
    std::vector<OptionRule> m_rules;
};
} // namespace Data::Items
