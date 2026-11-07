#pragma once

#include "Data/GameData/ItemData/ItemOptionDefinition.h"

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

    bool operator==(const ItemOptionEffect& other) const = default;
};

// The effect id of an option the client doesn't compute: its slot shows its line and adds nothing.
constexpr WORD NoItemOptionEffect = 0xFFFF;

// The effect of an option, or none when the client doesn't compute its attribute that way.
std::optional<ItemOptionEffect> FindItemOptionEffect(ItemOptionType type, std::string_view attribute,
                                                     ItemOptionAdd add);
} // namespace Data::Items
