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
constexpr std::array<EffectRow, 24> EffectRows = {{
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
    // The excellent options: the ones the server computes write no value, as the code before them (the client shows
    // only their lines); the damage, wizardry and defense rate per cent and level / 20 write theirs, which the stat
    // code adds. The attack speed writes its value for its line only: the server computes the speed.
    {ItemOptionType::Excellent, "maximumHealth", ItemOptionAdd::Percent, {AT_IMPROVE_LIFE, false}},
    {ItemOptionType::Excellent, "maximumMana", ItemOptionAdd::Percent, {AT_IMPROVE_MANA, false}},
    {ItemOptionType::Excellent, "armorDamageDecrease", ItemOptionAdd::Percent, {AT_DECREASE_DAMAGE, false}},
    {ItemOptionType::Excellent, "damageReflection", ItemOptionAdd::Percent, {AT_REFLECTION_DAMAGE, false}},
    {ItemOptionType::Excellent, "defenseRatePvm", ItemOptionAdd::Percent, {AT_IMPROVE_BLOCKING_PERCENT, true}},
    {ItemOptionType::Excellent, "moneyAmountRate", ItemOptionAdd::Percent, {AT_IMPROVE_GAIN_GOLD, false}},
    {ItemOptionType::Excellent, "excellentDamageChance", ItemOptionAdd::Percent, {AT_EXCELLENT_DAMAGE, false}},
    {ItemOptionType::Excellent, "physicalBaseDmg", ItemOptionAdd::Add, {AT_IMPROVE_DAMAGE_LEVEL, true}},
    {ItemOptionType::Excellent, "wizardryBaseDmg", ItemOptionAdd::Add, {AT_IMPROVE_MAGIC_LEVEL, true}},
    {ItemOptionType::Excellent, "physicalBaseDmgIncrease", ItemOptionAdd::Percent, {AT_IMPROVE_DAMAGE_PERCENT, true}},
    {ItemOptionType::Excellent, "wizardryBaseDmgIncrease", ItemOptionAdd::Percent, {AT_IMPROVE_MAGIC_PERCENT, true}},
    {ItemOptionType::Excellent, "attackSpeedAny", ItemOptionAdd::Add, {AT_IMPROVE_ATTACK_SPEED, true}},
    {ItemOptionType::Excellent, "healthAfterMonsterKillMultiplier", ItemOptionAdd::Add, {AT_IMPROVE_GAIN_LIFE, false}},
    {ItemOptionType::Excellent, "manaAfterMonsterKillMultiplier", ItemOptionAdd::Add, {AT_IMPROVE_GAIN_MANA, false}},
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
