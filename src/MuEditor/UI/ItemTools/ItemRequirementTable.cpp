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
#include <optional>
#include <string>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDefinition;
using Data::Items::ItemRequirement;
using Data::Items::RequirementByLevel;

constexpr int MaxBase = 65535;
// The bases from the smallest that reaches the +0 value that are tried for all
// 16 values: a +0 value that hardly grows with the base must not make the
// search take every base.
constexpr int MaxCandidates = 1024;

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
            static_cast<BYTE>(variant.ancient ? 1 : 0), static_cast<BYTE>(variant.optionLevel)};
        table[static_cast<size_t>(level)] = ToRow(GameLogic::Items::ComputeItemStats(sources, input));
    }
    return table;
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
    // The +0 value grows with the base: the smallest base that reaches it.
    int low = 0;
    int high = MaxBase;
    while (low < high)
    {
        const int middle = low + (high - low) / 2;
        if (ValuesWithBase(withoutLevels, requirement, middle)[0] < first)
        {
            low = middle + 1;
        }
        else
        {
            high = middle;
        }
    }
    std::optional<int> firstMatch;
    for (int base = low; base <= MaxBase && base < low + MaxCandidates; ++base)
    {
        const RequirementByLevel candidate = ValuesWithBase(withoutLevels, requirement, base);
        if (candidate[0] != first)
        {
            break;
        }
        if (candidate == values)
        {
            return static_cast<WORD>(base);
        }
        if (!firstMatch)
        {
            firstMatch = base;
        }
    }
    if (firstMatch)
    {
        return static_cast<WORD>(*firstMatch);
    }
    // No base gives the +0 value: the one whose +0 value is nearest.
    const int below = std::max(low - 1, 0);
    const int distanceBelow = std::abs(ValuesWithBase(withoutLevels, requirement, below)[0] - first);
    const int distanceAbove = std::abs(ValuesWithBase(withoutLevels, requirement, low)[0] - first);
    return static_cast<WORD>(distanceBelow <= distanceAbove ? below : low);
}
} // namespace MuEditor::Items

#endif // _EDITOR
