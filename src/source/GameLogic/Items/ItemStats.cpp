#include "stdafx.h"

#include "ItemStats.h"

#include "Data/GameData/ItemData/ItemDatabase.h"

#include <algorithm>

namespace GameLogic::Items
{
namespace
{
using Data::Items::ItemSlot;
using Data::Items::ItemStat;
using Data::Items::StatBonusTerm;
using Data::Items::StatTerms;

constexpr int MaxByteStat = 255;

struct StatContext
{
    const Data::Items::ItemStatProfiles& profiles;
    int level = 0;
    int dropLevel = 0;
    bool excellent = false;
    bool ancient = false;
    int fixedExcellentBonus = 0;
};

int DropLevelPart(const StatBonusTerm& term, int dropLevel)
{
    return term.dropLevelDivisor > 0 ? dropLevel / term.dropLevelDivisor : 0;
}

// base + excellent part + ancient part + level bonus. Every / divides whole numbers, from
// left to right, as the item code always did.
int AddUpStat(const StatTerms& terms, int base, int excellentBase, bool fixedBonusCounts, const StatContext& context)
{
    if (base == 0)
    {
        return 0;
    }

    int excellent = 0;
    if (context.excellent && context.dropLevel > 0 && terms.excellent)
    {
        excellent = fixedBonusCounts && context.fixedExcellentBonus != 0
                        ? context.fixedExcellentBonus
                        : excellentBase * terms.excellent->times / context.dropLevel + terms.excellent->add +
                              DropLevelPart(*terms.excellent, context.dropLevel);
    }

    const int levelBonus = context.profiles.GetLevelBonus(terms, context.level);

    int ancient = 0;
    if (context.ancient && terms.ancient)
    {
        const int dropLevel = context.dropLevel + context.profiles.GetAncientDropLevelBonus();
        const int value = base + excellent + (terms.ancient->afterLevelBonus ? levelBonus : 0);
        ancient = (dropLevel > 0 ? value * terms.ancient->times / dropLevel : 0) + terms.ancient->add +
                  DropLevelPart(*terms.ancient, dropLevel);
    }

    return base + excellent + ancient + levelBonus;
}
} // namespace

ItemStatValues ComputeItemStats(const ITEM_ATTRIBUTE& attribute, const ItemStatInput& input)
{
    const Data::Items::ItemStatProfile& profile = g_ItemDatabase.GetStatProfile(input.itemType);
    const Data::Items::ItemDefinition* definition = g_ItemDatabase.Find(input.itemType);
    const bool wing = g_ItemDatabase.GetSlot(input.itemType) == ItemSlot::Wings;
    const StatContext context{g_ItemDatabase.GetStatProfiles(),
                              input.level,
                              attribute.Level,
                              (input.excellentFlags > 0 && !wing) || input.ancientDiscriminator > 0,
                              input.ancientDiscriminator > 0,
                              definition != nullptr ? definition->excellentBonus : 0};

    // The excellent damage part comes from the minimum damage for both values.
    const StatTerms& damage = profile.Get(ItemStat::Damage);
    const StatTerms& magicPower = profile.Get(ItemStat::MagicPower);
    ItemStatValues values;
    values.damageMin = AddUpStat(damage, attribute.DamageMin, attribute.DamageMin, true, context);
    values.damageMax = AddUpStat(damage, attribute.DamageMax, attribute.DamageMin, true, context);
    if (attribute.MagicPower > 0)
    {
        const int sum = AddUpStat(magicPower, attribute.MagicPower, attribute.MagicPower, true, context);
        values.magicPower = std::min(sum / 2 + magicPower.perLevelAfterHalving * input.level, MaxByteStat);
    }
    values.blockRate = std::min(AddUpStat(profile.Get(ItemStat::BlockRate), attribute.SuccessfulBlocking,
                                          attribute.SuccessfulBlocking, false, context),
                                MaxByteStat);
    values.defense = AddUpStat(profile.Get(ItemStat::Defense), attribute.Defense, attribute.Defense, false, context);
    values.magicDefense =
        AddUpStat(profile.Get(ItemStat::MagicDefense), attribute.MagicDefense, attribute.MagicDefense, false, context);
    return values;
}
} // namespace GameLogic::Items
