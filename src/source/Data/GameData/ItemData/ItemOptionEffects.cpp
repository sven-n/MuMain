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
// (GameLogic::Combat::GetHeroCriticalDamageRate). The additional options write theirs. A magic sword's option
// (baseDamageBonus) is one slot, AT_IMPROVE_DAMAGE, as before, which the stat code adds to the wizardry damage too;
// its definition shows two lines.
constexpr std::array<EffectRow, 10> EffectRows = {{
    {ItemOptionType::Luck, "criticalDamageChance", ItemOptionAdd::Percent, {AT_LUCK, true}},
    {ItemOptionType::Option, "physicalBaseDmg", ItemOptionAdd::Add, {AT_IMPROVE_DAMAGE, true}},
    {ItemOptionType::Option, "baseDamageBonus", ItemOptionAdd::Add, {AT_IMPROVE_DAMAGE, true, true}},
    {ItemOptionType::Option, "wizardryBaseDmg", ItemOptionAdd::Add, {AT_IMPROVE_MAGIC, true}},
    {ItemOptionType::Option, "curseBaseDmg", ItemOptionAdd::Add, {AT_IMPROVE_CURSE, true}},
    {ItemOptionType::Option, "defenseRatePvm", ItemOptionAdd::Add, {AT_IMPROVE_BLOCKING, true}},
    {ItemOptionType::Option, "defenseBase", ItemOptionAdd::Add, {AT_IMPROVE_DEFENSE, true}},
    {ItemOptionType::Option, "healthRecoveryMultiplier", ItemOptionAdd::Percent, {AT_LIFE_REGENERATION, true}},
    {ItemOptionType::Option, "maximumMana", ItemOptionAdd::Percent, {AT_IMPROVE_MAX_MANA, true}},
    {ItemOptionType::Option, "maximumAbility", ItemOptionAdd::Percent, {AT_IMPROVE_MAX_AG, true}},
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
