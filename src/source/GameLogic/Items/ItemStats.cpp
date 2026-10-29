#include "stdafx.h"

#include "ItemStats.h"

#include "Data/GameData/ItemData/ItemDatabase.h"

#include <algorithm>
#include <cstdint>

namespace GameLogic::Items
{
namespace
{
using Data::Items::ItemDefinition;
using Data::Items::ItemRequirement;
using Data::Items::ItemRequirementStat;
using Data::Items::ItemSlot;
using Data::Items::ItemStat;
using Data::Items::RequirementByLevel;
using Data::Items::RequirementFormula;
using Data::Items::RequirementTerms;
using Data::Items::StatBonusTerm;
using Data::Items::StatTerms;

constexpr int MaxByteStat = 255;

// The numbers of the item stat file go up to 65535, so the sums and products are computed in
// 64 bits, and the values stop at the 65535 the item's fields hold.
using WideValue = std::int64_t;
constexpr WideValue MaxItemValue = 65535;

int LimitToItemValue(WideValue value)
{
    return static_cast<int>(std::clamp<WideValue>(value, 0, MaxItemValue));
}

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

    WideValue excellent = 0;
    if (context.excellent && context.dropLevel > 0 && terms.excellent)
    {
        excellent = fixedBonusCounts && context.fixedExcellentBonus != 0
                        ? context.fixedExcellentBonus
                        : WideValue{excellentBase} * terms.excellent->times / context.dropLevel + terms.excellent->add +
                              DropLevelPart(*terms.excellent, context.dropLevel);
    }

    const WideValue levelBonus = context.profiles.GetLevelBonus(terms, context.level);

    WideValue ancient = 0;
    if (context.ancient && terms.ancient)
    {
        const int dropLevel = context.dropLevel + context.profiles.GetAncientDropLevelBonus();
        const WideValue value = base + excellent + (terms.ancient->afterLevelBonus ? levelBonus : 0);
        ancient = (dropLevel > 0 ? value * terms.ancient->times / dropLevel : 0) + terms.ancient->add +
                  DropLevelPart(*terms.ancient, dropLevel);
    }

    return LimitToItemValue(base + excellent + ancient + levelBonus);
}

int GetRequirementBase(const ITEM_ATTRIBUTE& attribute, ItemRequirementStat stat)
{
    switch (stat)
    {
    case ItemRequirementStat::Strength:
        return attribute.RequireStrength;
    case ItemRequirementStat::Dexterity:
        return attribute.RequireDexterity;
    case ItemRequirementStat::Energy:
        return attribute.RequireEnergy;
    case ItemRequirementStat::Vitality:
        return attribute.RequireVitality;
    case ItemRequirementStat::Leadership:
        return attribute.RequireCharisma;
    default:
        return 0;
    }
}

const RequirementByLevel* FindRequirementByLevel(const ItemDefinition* definition, ItemRequirement requirement)
{
    if (definition == nullptr)
    {
        return nullptr;
    }
    return definition->requirementsByLevel.Find(requirement);
}

int ComputeStatRequirement(const RequirementFormula& formula, int base, int dropLevel, int levelRequirement, int level)
{
    if (base == 0)
    {
        return 0;
    }
    if (formula.keepBase)
    {
        return base;
    }
    if (formula.levelRequirementBasis && levelRequirement > 0)
    {
        return LimitToItemValue(formula.add + WideValue{base} * levelRequirement * formula.percent / 100);
    }
    // Times the percent this could pass 64 bits. A weight above 100 * 65536 gives the highest
    // value with any percent but 0 (which gives add), so it is cut there.
    const WideValue weighted =
        std::min(WideValue{base} * (dropLevel + WideValue{level} * formula.levelWeight), 100 * (MaxItemValue + 1));
    return LimitToItemValue(formula.add + weighted * formula.percent / 100);
}

void ComputeRequirements(const ITEM_ATTRIBUTE& attribute, const ItemStatInput& input, const ItemDefinition* definition,
                         const RequirementTerms& terms, bool wing, const Data::Items::ItemStatProfiles& profiles,
                         ItemStatValues& values)
{
    const size_t levelIndex = static_cast<size_t>(std::clamp(input.level, 0, Data::Items::ItemLevelCount - 1));
    const bool excellentOptions = input.excellentFlags > 0;

    const RequirementByLevel* levelByLevel = FindRequirementByLevel(definition, ItemRequirement::Level);
    if (levelByLevel != nullptr)
    {
        values.requireLevel = (*levelByLevel)[levelIndex];
    }
    else if (attribute.RequireLevel > 0)
    {
        values.requireLevel =
            LimitToItemValue(attribute.RequireLevel + WideValue{terms.levelPerItemLevel} * input.level);
    }
    const bool raisesLevel = definition == nullptr || definition->excellentRaisesLevel;
    if (excellentOptions && values.requireLevel > 0 && !wing && raisesLevel)
    {
        values.requireLevel = LimitToItemValue(values.requireLevel + WideValue{terms.excellentLevelAdd});
    }

    // The level requirement the "levelRequirement" basis takes: the one given by level, or the
    // base value, without the growth per item level and the excellent addition.
    const int levelRequirementBasis = levelByLevel != nullptr ? (*levelByLevel)[levelIndex] : attribute.RequireLevel;
    const int dropLevel = attribute.Level + (excellentOptions                 ? profiles.GetExcellentDropLevelBonus()
                                             : input.ancientDiscriminator > 0 ? profiles.GetAncientDropLevelBonus()
                                                                              : 0);
    int* const requirements[] = {&values.requireStrength, &values.requireDexterity, &values.requireEnergy,
                                 &values.requireVitality, &values.requireLeadership};
    for (size_t stat = 0; stat < Data::Items::ItemRequirementStatCount; ++stat)
    {
        const auto requirement = static_cast<ItemRequirementStat>(stat);
        // ItemRequirement has the level first, then the stats in the same order.
        if (const RequirementByLevel* levels =
                FindRequirementByLevel(definition, static_cast<ItemRequirement>(stat + 1)))
        {
            *requirements[stat] = (*levels)[levelIndex];
            continue;
        }
        *requirements[stat] = ComputeStatRequirement(terms.Get(requirement), GetRequirementBase(attribute, requirement),
                                                     dropLevel, levelRequirementBasis, input.level);
    }
    values.requireStrength =
        LimitToItemValue(values.requireStrength + WideValue{terms.strengthPerOptionLevel} * input.optionLevel);
}
} // namespace

ItemStatValues ComputeItemStats(const ITEM_ATTRIBUTE& attribute, const ItemStatInput& input)
{
    const Data::Items::ItemStatProfile& profile = g_ItemDatabase.GetStatProfile(input.itemType);
    const Data::Items::ItemDefinition* definition = g_ItemDatabase.Find(input.itemType);
    const bool wing = g_ItemDatabase.GetSlot(input.itemType) == ItemSlot::Wings;
    return ComputeItemStats(ItemStatSources{attribute, definition, profile, wing}, input);
}

ItemStatValues ComputeItemStats(const ItemStatSources& sources, const ItemStatInput& input)
{
    const ITEM_ATTRIBUTE& attribute = sources.attribute;
    const Data::Items::ItemStatProfile& profile = sources.profile;
    const Data::Items::ItemDefinition* definition = sources.definition;
    const bool wing = sources.wing;
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
        values.magicPower = LimitToItemValue(sum / 2 + WideValue{magicPower.perLevelAfterHalving} * input.level);
    }
    values.blockRate = std::min(AddUpStat(profile.Get(ItemStat::BlockRate), attribute.SuccessfulBlocking,
                                          attribute.SuccessfulBlocking, false, context),
                                MaxByteStat);
    values.defense = AddUpStat(profile.Get(ItemStat::Defense), attribute.Defense, attribute.Defense, false, context);
    values.magicDefense =
        AddUpStat(profile.Get(ItemStat::MagicDefense), attribute.MagicDefense, attribute.MagicDefense, false, context);
    ComputeRequirements(attribute, input, definition, profile.requirements, wing, g_ItemDatabase.GetStatProfiles(),
                        values);
    return values;
}
} // namespace GameLogic::Items
