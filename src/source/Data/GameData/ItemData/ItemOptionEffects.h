#pragma once

#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <array>
#include <optional>
#include <string_view>

// What an option of the option definitions does in the client: the effect id it writes into the item's option slots
// (ITEM::Special, the AT_* ids the stat code, the price and the mixes read) for its type, attribute and way of adding,
// and whether it writes its value there (ITEM::SpecialValue) or 0, as the code before the definitions did. One row per
// option the client computes; each option group adds its rows with the pull request that moves it into data.
namespace Data::Items
{
struct ItemOptionEffect
{
    WORD effect = 0;
    bool writesValue = false;
    // The effect raises the wizardry damage too (a magic sword's damage option, which the stat code adds to both).
    bool raisesWizardry = false;

    bool operator==(const ItemOptionEffect& other) const = default;
};

// The effect id of an option the client doesn't compute: its slot shows its line and adds nothing.
constexpr WORD NoItemOptionEffect = 0xFFFF;

// The hero attributes a value's term (ItemOptionTerm, "per") can read, by their names in the data
// (GameLogic::Items::ComputeOptionValue). A term on another attribute adds nothing.
struct ItemOptionTermAttributeName
{
    std::string_view name;
    ItemOptionTermAttribute attribute;
};

constexpr std::array<ItemOptionTermAttributeName, 3> ItemOptionTermAttributes = {{
    {"level", ItemOptionTermAttribute::Level},
    {"totalLevel", ItemOptionTermAttribute::TotalLevel},
    {"masterLevel", ItemOptionTermAttribute::MasterLevel},
}};

// The hero attribute of that name; Unread for a name the client doesn't read.
constexpr ItemOptionTermAttribute FindItemOptionTermAttribute(std::string_view name)
{
    for (const ItemOptionTermAttributeName& entry : ItemOptionTermAttributes)
    {
        if (entry.name == name)
        {
            return entry.attribute;
        }
    }
    return ItemOptionTermAttribute::Unread;
}

// Gives each of the value's terms the hero attribute its name names.
inline void ResolveTermAttributes(ItemOptionValue& value)
{
    for (ItemOptionTerm& term : value.per)
    {
        term.heroAttribute = FindItemOptionTermAttribute(term.attribute);
    }
}

// The effect of an option, or none when the client doesn't compute its attribute that way.
std::optional<ItemOptionEffect> FindItemOptionEffect(ItemOptionType type, std::string_view attribute,
                                                     ItemOptionAdd add);
} // namespace Data::Items
