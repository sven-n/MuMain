#include "stdafx.h"

#ifdef _EDITOR

#include "ItemRequirementTable.h"

#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "GameLogic/Items/ItemStats.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <string>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDefinition;
using Data::Items::ItemRequirement;
using Data::Items::RequirementByLevel;

constexpr int MaxBase = 65535;

std::string OrDefault(std::string name)
{
    return name.empty() ? std::string(Data::Items::StatProfileNames::Default) : name;
}

GameLogic::Items::ItemStatValues ComputeValuesAt(const ItemDefinition& item, const ITEM_ATTRIBUTE& attribute,
                                                 const Data::Items::ItemStatProfile& profile,
                                                 const RequirementVariant& variant, int level)
{
    const GameLogic::Items::ItemStatSources sources{attribute, &item, profile,
                                                    item.slot == Data::Items::ItemSlot::Wings};
    const GameLogic::Items::ItemStatInput input{
        Data::Items::MakeItemType(item.group, item.number), level, static_cast<BYTE>(variant.excellent ? 1 : 0),
        static_cast<BYTE>(variant.ancient ? 1 : 0), static_cast<BYTE>(std::clamp(variant.optionLevel, 0, 255))};
    return GameLogic::Items::ComputeItemStats(sources, input);
}

RequirementRow ToRow(const GameLogic::Items::ItemStatValues& values)
{
    return {values.requireLevel,  values.requireStrength, values.requireDexterity,
            values.requireEnergy, values.requireVitality, values.requireLeadership};
}

WORD& BaseOf(Data::Items::ItemRequirements& requirements, ItemRequirement requirement)
{
    switch (requirement)
    {
    case ItemRequirement::Strength:
        return requirements.strength;
    case ItemRequirement::Dexterity:
        return requirements.dexterity;
    case ItemRequirement::Energy:
        return requirements.energy;
    case ItemRequirement::Vitality:
        return requirements.vitality;
    case ItemRequirement::Leadership:
        return requirements.leadership;
    case ItemRequirement::Level:
    case ItemRequirement::Count:
        break;
    }
    return requirements.level;
}

// The requirement's 16 values of a normal item with this base.
RequirementByLevel ValuesWithBase(ItemDefinition item, ItemRequirement requirement, int base)
{
    BaseOf(item.requirements, requirement) = static_cast<WORD>(base);
    const RequirementTable table = ComputeRequirementTable(item, {});
    RequirementByLevel values{};
    const size_t column = static_cast<size_t>(requirement);
    for (size_t level = 0; level < values.size(); ++level)
    {
        values[level] = static_cast<WORD>(table[level][column]);
    }
    return values;
}

// The requirement at `level` of `item` with this weight and without the
// requirement by level. The profile follows the weight as storing the item
// would make it (a rule may ask for a base requirement above 0).
int ValueAt(ItemDefinition item, ItemRequirement requirement, int weight, int level,
            const RequirementVariant& variant = {})
{
    BaseOf(item.requirements, requirement) = static_cast<WORD>(weight);
    item.requirementsByLevel.Remove(requirement);
    return ToRow(ComputeItemValuesAt(item, variant, level))[static_cast<size_t>(requirement)];
}

// The smallest weight whose +0 value is at least `value`, or nullopt.
std::optional<int> FindSmallestWeightReaching(const ItemDefinition& item, ItemRequirement requirement, int value)
{
    if (ValueAt(item, requirement, MaxBase, 0) < value)
    {
        return std::nullopt;
    }
    int low = 0;
    int high = MaxBase;
    while (low < high)
    {
        const int middle = low + (high - low) / 2;
        if (ValueAt(item, requirement, middle, 0) < value)
        {
            low = middle + 1;
        }
        else
        {
            high = middle;
        }
    }
    return low;
}

// The smallest base whose values reach `values` at the first `levels` + levels;
// each value grows with the base.
int FindSmallestBaseReaching(const ItemDefinition& item, ItemRequirement requirement, const RequirementByLevel& values,
                             size_t levels)
{
    int low = 0;
    int high = MaxBase;
    while (low < high)
    {
        const int middle = low + (high - low) / 2;
        const RequirementByLevel candidate = ValuesWithBase(item, requirement, middle);
        bool belowSome = false;
        for (size_t level = 0; level < levels; ++level)
        {
            belowSome = belowSome || candidate[level] < values[level];
        }
        if (belowSome)
        {
            low = middle + 1;
        }
        else
        {
            high = middle;
        }
    }
    return low;
}
} // namespace

StatProfileChoice FindStatProfileChoice(const ItemDefinition& item)
{
    const ItemDefinition* stored = g_ItemDatabase.Find(item.group, item.number);
    const Data::Items::ItemStatProfiles& profiles = g_ItemDatabase.GetStatProfiles();
    std::string name = item.statProfile;
    if (name.empty())
    {
        name = Data::Items::DeriveStatProfile(profiles, item);
    }
    else if (stored != nullptr && name == stored->statProfile)
    {
        name = Data::Items::FollowStatProfile(profiles, stored, item);
    }
    StatProfileChoice choice;
    choice.rule = profiles.FindRule(item);
    const std::string given = profiles.GetRuleProfile(choice.rule);
    choice.reason = name == given ? StatProfileChoice::Reason::ByRules : StatProfileChoice::Reason::Picked;
    choice.name = OrDefault(std::move(name));
    choice.given = OrDefault(given);
    return choice;
}

const Data::Items::ItemStatProfile& FindStatProfile(const ItemDefinition& item)
{
    const Data::Items::ItemStatProfiles& profiles = g_ItemDatabase.GetStatProfiles();
    const int index = profiles.Find(FindStatProfileChoice(item).name);
    return profiles.Get(
        index == Data::Items::ItemStatProfiles::UnknownProfile ? Data::Items::ItemStatProfiles::DefaultProfile : index);
}

ItemLevelValues ComputeItemLevelValues(const ItemDefinition& item, const RequirementVariant& variant)
{
    ITEM_ATTRIBUTE attribute{};
    Data::Items::ToItemAttribute(item, attribute);
    const Data::Items::ItemStatProfile& profile = FindStatProfile(item);
    ItemLevelValues values{};
    for (int level = 0; level < Data::Items::ItemLevelCount; ++level)
    {
        values[static_cast<size_t>(level)] = ComputeValuesAt(item, attribute, profile, variant, level);
    }
    return values;
}

GameLogic::Items::ItemStatValues ComputeItemValuesAt(const ItemDefinition& item, const RequirementVariant& variant,
                                                     int level)
{
    ITEM_ATTRIBUTE attribute{};
    Data::Items::ToItemAttribute(item, attribute);
    return ComputeValuesAt(item, attribute, FindStatProfile(item), variant, level);
}

RequirementTable ComputeRequirementTable(const ItemDefinition& item, const RequirementVariant& variant)
{
    const ItemLevelValues values = ComputeItemLevelValues(item, variant);
    RequirementTable table{};
    for (size_t level = 0; level < table.size(); ++level)
    {
        table[level] = ToRow(values[level]);
    }
    return table;
}

bool OptionLevelAddsStrength(const ItemDefinition& item)
{
    // The profile itself: a requirement at 65535 hides the addition.
    return FindStatProfile(item).requirements.strengthPerOptionLevel != 0;
}

WORD FindRequirementBase(const ItemDefinition& item, ItemRequirement requirement, const RequirementByLevel& values)
{
    ItemDefinition withoutLevels = item;
    withoutLevels.requirementsByLevel.Remove(requirement);
    const int first = values[0];
    if (requirement == ItemRequirement::Level || ValuesWithBase(withoutLevels, requirement, first) == values)
    {
        return static_cast<WORD>(first);
    }
    // A smaller base than the smallest that reaches all 16 values falls short
    // at some level, so only that one can give them.
    const int reachingAll = FindSmallestBaseReaching(withoutLevels, requirement, values, values.size());
    if (ValuesWithBase(withoutLevels, requirement, reachingAll) == values)
    {
        return static_cast<WORD>(reachingAll);
    }
    // Else the smallest base that gives the +0 value, else the one whose +0
    // value is nearest.
    const int low = FindSmallestBaseReaching(withoutLevels, requirement, values, 1);
    const int atLow = ValuesWithBase(withoutLevels, requirement, low)[0];
    if (atLow == first)
    {
        return static_cast<WORD>(low);
    }
    const int below = std::max(low - 1, 0);
    const int distanceBelow = std::abs(ValuesWithBase(withoutLevels, requirement, below)[0] - first);
    return static_cast<WORD>(distanceBelow <= std::abs(atLow - first) ? below : low);
}
bool HasWeight(const ItemDefinition& item, ItemRequirement requirement)
{
    if (requirement == ItemRequirement::Level || requirement == ItemRequirement::Count)
    {
        return false;
    }
    const auto stat = static_cast<Data::Items::ItemRequirementStat>(static_cast<int>(requirement) - 1);
    return !FindStatProfile(item).requirements.Get(stat).keepBase;
}

int ComputeZeroValue(const ItemDefinition& item, ItemRequirement requirement)
{
    return ToRow(ComputeItemValuesAt(item, {}, 0))[static_cast<size_t>(requirement)];
}

WeightRange FindWeightsGiving(const ItemDefinition& item, ItemRequirement requirement, int value)
{
    const std::optional<int> first = FindSmallestWeightReaching(item, requirement, value);
    if (!first || ValueAt(item, requirement, *first, 0) != value)
    {
        return {};
    }
    const std::optional<int> above = FindSmallestWeightReaching(item, requirement, value + 1);
    return {*first, above ? *above - 1 : MaxBase};
}

WeightValues ComputeWeightValues(const ItemDefinition& item, ItemRequirement requirement, int weight)
{
    const RequirementVariant excellent{true, false, 0};
    return {weight, ValueAt(item, requirement, weight, Data::Items::ItemLevelCount - 1),
            ValueAt(item, requirement, weight, 0, excellent),
            ValueAt(item, requirement, weight, Data::Items::ItemLevelCount - 1, excellent)};
}

NearestZeroValues FindNearestZeroValues(const ItemDefinition& item, ItemRequirement requirement, int value)
{
    const auto reachable = [&](int weight)
    {
        const int reached = ValueAt(item, requirement, weight, 0);
        return ReachableValue{reached, FindWeightsGiving(item, requirement, reached)};
    };
    NearestZeroValues nearest;
    const std::optional<int> first = FindSmallestWeightReaching(item, requirement, value);
    if (!first)
    {
        nearest.below = reachable(MaxBase);
        return nearest;
    }
    nearest.above = reachable(*first);
    if (*first > 0)
    {
        nearest.below = reachable(*first - 1);
    }
    return nearest;
}

WeightChoice ChooseWeight(const ItemDefinition& item, ItemRequirement requirement, int value, int currentWeight)
{
    WeightChoice choice;
    choice.weights = FindWeightsGiving(item, requirement, value);
    if (choice.weights.IsEmpty())
    {
        choice.kind = WeightChoice::Kind::Unreachable;
        choice.weight = currentWeight;
        choice.nearest = FindNearestZeroValues(item, requirement, value);
    }
    else if (choice.weights.Contains(currentWeight))
    {
        choice.kind = WeightChoice::Kind::Unchanged;
        choice.weight = currentWeight;
    }
    else
    {
        choice.kind = choice.weights.GetCount() == 1 ? WeightChoice::Kind::Take : WeightChoice::Kind::Pick;
        choice.weight = choice.weights.first;
    }
    return choice;
}
} // namespace MuEditor::Items

#endif // _EDITOR
