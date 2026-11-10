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

// Luck writes its value: the character window adds it up into the critical damage rate
// (GameLogic::Combat::GetHeroCriticalDamageRate).
constexpr std::array<EffectRow, 1> EffectRows = {{
    {ItemOptionType::Luck, "criticalDamageChance", ItemOptionAdd::Percent, {AT_LUCK, true}},
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
