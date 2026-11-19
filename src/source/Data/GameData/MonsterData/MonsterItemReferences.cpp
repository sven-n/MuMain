#include "stdafx.h"

#include "MonsterItemReferences.h"

#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <array>
#include <optional>
#include <utility>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

constexpr std::string_view EquipmentKey = "equipment";

// The slots of the equipment with the names the monster list gives them.
template <typename TEquipment, typename TVisit> void ForEachSlot(TEquipment& equipment, TVisit&& visit)
{
    for (std::size_t i = 0; i < equipment.weapons.size(); ++i)
    {
        visit(equipment.weapons[i], "weapons[" + std::to_string(i) + "]");
    }
    visit(equipment.helm, "helm");
    visit(equipment.armor, "armor");
    visit(equipment.pants, "pants");
    visit(equipment.gloves, "gloves");
    visit(equipment.boots, "boots");
    visit(equipment.wings, "wings");
    visit(equipment.helper, "helper");
}

template <typename TMonster, typename TVisit> void VisitItems(TMonster& monster, TVisit&& visit)
{
    const auto visitEquipment = [&visit](auto& equipment, MonsterItemPlace place)
    {
        ForEachSlot(equipment,
                    [&](auto& item, std::string slot)
                    {
                        if (!item || !item->part.empty())
                            return;
                        place.slot = std::move(slot);
                        visit(*item, place);
                    });
    };
    visitEquipment(monster.setup.equipment, MonsterItemPlace{});
    for (std::size_t i = 0; i < monster.mapVariants.size(); ++i)
    {
        auto& map = monster.mapVariants[i].setup;
        visitEquipment(map.equipment, {MonsterItemPlace::Variant::Map, i, {}, {}});
        for (std::size_t j = 0; j < map.stateVariants.size(); ++j)
        {
            auto& variant = map.stateVariants[j];
            visitEquipment(variant.setup.equipment, {MonsterItemPlace::Variant::MapState, j, variant.state, {}, i});
        }
    }
    for (std::size_t i = 0; i < monster.setup.stateVariants.size(); ++i)
    {
        auto& variant = monster.setup.stateVariants[i];
        visitEquipment(variant.setup.equipment, {MonsterItemPlace::Variant::State, i, variant.state, {}});
    }
    for (std::size_t i = 0; i < monster.randomVariants.size(); ++i)
    {
        if (auto& equipment = monster.randomVariants[i].equipment)
        {
            visitEquipment(*equipment, {MonsterItemPlace::Variant::Random, i, {}, {}});
        }
    }
}

int ItemTypeOf(const MonsterItem& item)
{
    return Items::MakeItemType(item.group, item.number);
}
} // namespace

std::string GetItemPlaceField(const MonsterItemPlace& place)
{
    std::string variant;
    switch (place.variant)
    {
    case MonsterItemPlace::Variant::None:
        break;
    case MonsterItemPlace::Variant::Map:
        variant = "mapVariants[" + std::to_string(place.index) + "].";
        break;
    case MonsterItemPlace::Variant::State:
        variant = "stateVariants[" + std::to_string(place.index) + "].";
        break;
    case MonsterItemPlace::Variant::Random:
        variant = "randomVariants[" + std::to_string(place.index) + "].";
        break;
    case MonsterItemPlace::Variant::MapState:
        variant =
            "mapVariants[" + std::to_string(place.mapIndex) + "].stateVariants[" + std::to_string(place.index) + "].";
        break;
    }
    return variant + std::string(EquipmentKey) + "." + place.slot;
}

void ForEachMonsterItem(const MonsterDefinition& monster,
                        const std::function<void(const MonsterItem& item, const MonsterItemPlace& place)>& visit)
{
    VisitItems(monster, visit);
}

void ForEachMonsterItem(MonsterDefinition& monster,
                        const std::function<void(MonsterItem& item, const MonsterItemPlace& place)>& visit)
{
    VisitItems(monster, visit);
}

std::vector<MonsterItemWearer> FindItemWearers(std::span<const MonsterDefinition> monsters, int itemType)
{
    std::vector<const MonsterDefinition*> pointers;
    pointers.reserve(monsters.size());
    for (const MonsterDefinition& monster : monsters)
    {
        pointers.push_back(&monster);
    }
    return FindItemWearers(std::span<const MonsterDefinition* const>(pointers), itemType);
}

std::vector<MonsterItemWearer> FindItemWearers(std::span<const MonsterDefinition* const> monsters, int itemType)
{
    std::vector<MonsterItemWearer> wearers;
    for (const MonsterDefinition* monster : monsters)
    {
        ForEachMonsterItem(*monster,
                           [&](const MonsterItem& item, const MonsterItemPlace& place)
                           {
                               if (ItemTypeOf(item) == itemType)
                                   wearers.push_back({monster->number, place});
                           });
    }
    return wearers;
}

std::optional<int> FindItemAt(const MonsterDefinition& monster, const MonsterItemPlace& place)
{
    std::optional<int> found;
    ForEachMonsterItem(monster,
                       [&](const MonsterItem& item, const MonsterItemPlace& at)
                       {
                           if (at == place)
                               found = ItemTypeOf(item);
                       });
    return found;
}

bool ApplyMonsterItemChanges(std::vector<MonsterGroup>& groups, std::span<const MonsterItemChange> changes)
{
    bool changedAny = false;
    for (MonsterGroup& group : groups)
    {
        // The numbers that shared each value before.
        const std::span<const MonsterValueField> fields = GetMonsterValueFields();
        std::vector<std::vector<size_t>> sharers(fields.size());
        for (size_t field = 0; field < fields.size(); ++field)
        {
            for (size_t i = 0; i < group.numbers.size(); ++i)
            {
                if (HasSameValue(group.numbers[i], group.shared, fields[field]))
                    sharers[field].push_back(i);
            }
        }
        bool changed = false;
        for (MonsterDefinition& entry : group.numbers)
        {
            for (const MonsterItemChange& change : changes)
            {
                if (change.number != entry.number)
                    continue;
                ForEachMonsterItem(entry,
                                   [&](MonsterItem& item, const MonsterItemPlace& place)
                                   {
                                       if (place != change.place || ItemTypeOf(item) != change.saved)
                                           return;
                                       item.group = Items::GetItemGroup(change.current);
                                       item.number = Items::GetItemNumber(change.current);
                                       changed = true;
                                   });
            }
        }
        if (!changed)
            continue;
        changedAny = true;
        for (size_t field = 0; field < fields.size(); ++field)
        {
            const std::vector<size_t>& numbers = sharers[field];
            if (numbers.empty())
                continue;
            const MonsterDefinition& first = group.numbers[numbers.front()];
            const bool same = std::ranges::all_of(numbers, [&](size_t i)
                                                  { return HasSameValue(group.numbers[i], first, fields[field]); });
            if (same)
                CopyValue(group.shared, first, fields[field]);
        }
    }
    return changedAny;
}

int ReplaceItemReferences(MonsterDefinition& monster, int fromItemType, int toItemType)
{
    int changed = 0;
    ForEachMonsterItem(monster,
                       [&](MonsterItem& item, const MonsterItemPlace&)
                       {
                           if (ItemTypeOf(item) != fromItemType)
                               return;
                           item.group = Items::GetItemGroup(toItemType);
                           item.number = Items::GetItemNumber(toItemType);
                           ++changed;
                       });
    return changed;
}

void ValidateMonsterItems(std::span<const MonsterDefinition* const> monsters,
                          const std::function<bool(int itemType)>& itemExists, const std::string& source,
                          std::vector<ItemDataIssue>& issues)
{
    for (const MonsterDefinition* monster : monsters)
    {
        ForEachMonsterItem(*monster,
                           [&](const MonsterItem& item, const MonsterItemPlace& place)
                           {
                               if (itemExists(ItemTypeOf(item)))
                                   return;
                               issues.push_back(
                                   {ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, monster->number,
                                    "monster " + std::to_string(monster->number) + " " + GetItemPlaceField(place),
                                    "names item (" + std::to_string(item.group) + ", " + std::to_string(item.number) +
                                        "), which the item list does not have; the monster wears nothing there"});
                           });
    }
}
} // namespace Data::Monsters
