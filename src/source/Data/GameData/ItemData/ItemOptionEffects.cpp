#include "stdafx.h"

#include "ItemOptionEffects.h"

#include "Core/Globals/_enum.h"

#include <array>

namespace Data::Items
{
namespace
{
struct EffectRow
{
    ItemOptionType type;
    std::string_view attribute;
    ItemOptionAdd add;
    ItemOptionEffect effect;
};

// Luck writes no value: the code before the definitions gave AT_LUCK 0, so the client adds no critical damage rate.
constexpr std::array<EffectRow, 1> EffectRows = {{
    {ItemOptionType::Luck, "criticalDamageChance", ItemOptionAdd::Percent, {AT_LUCK, false}},
}};
} // namespace

std::optional<ItemOptionEffect> FindItemOptionEffect(ItemOptionType type, std::string_view attribute, ItemOptionAdd add)
{
    for (const EffectRow& row : EffectRows)
    {
        if (row.type == type && row.attribute == attribute && row.add == add)
        {
            return row.effect;
        }
    }
    return std::nullopt;
}
} // namespace Data::Items
