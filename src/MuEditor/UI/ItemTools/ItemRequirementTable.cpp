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

// The profile storing the item would give it (CItemDataHandler::WithDerivedFields).
const Data::Items::ItemStatProfile& FindProfile(const ItemDefinition& item)
{
    const ItemDefinition* stored = g_ItemDatabase.Find(item.group, item.number);
    std::string name = item.statProfile;
    if (name.empty())
    {
        name = Data::Items::DeriveStatProfile(item);
    }
    else if (stored != nullptr && name == stored->statProfile)
    {
        name = Data::Items::FollowStatProfile(stored, item);
    }
    const Data::Items::ItemStatProfiles& profiles = g_ItemDatabase.GetStatProfiles();
    const int index = profiles.Find(name);
    return profiles.Get(
        index == Data::Items::ItemStatProfiles::UnknownProfile ? Data::Items::ItemStatProfiles::DefaultProfile : index);
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

RequirementTable ComputeRequirementTable(const ItemDefinition& item, const RequirementVariant& variant)
{
    ITEM_ATTRIBUTE attribute{};
    Data::Items::ToItemAttribute(item, attribute);
    const GameLogic::Items::ItemStatSources sources{attribute, &item, FindProfile(item),
                                                    item.slot == Data::Items::ItemSlot::Wings};
    RequirementTable table{};
    for (int level = 0; level < Data::Items::ItemLevelCount; ++level)
    {
        const GameLogic::Items::ItemStatInput input{
            Data::Items::MakeItemType(item.group, item.number), level, static_cast<BYTE>(variant.excellent ? 1 : 0),
            static_cast<BYTE>(variant.ancient ? 1 : 0), static_cast<BYTE>(std::clamp(variant.optionLevel, 0, 255))};
        table[static_cast<size_t>(level)] = ToRow(GameLogic::Items::ComputeItemStats(sources, input));
    }
    return table;
}

bool OptionLevelAddsStrength(const ItemDefinition& item)
{
    // The profile itself: a requirement at 65535 hides the addition.
    return FindProfile(item).requirements.strengthPerOptionLevel != 0;
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
} // namespace MuEditor::Items

#endif // _EDITOR
