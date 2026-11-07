#include "stdafx.h"

#include "GameLogic/Items/ItemOptions.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "GameLogic/Items/ItemOptionSlots.h"

namespace GameLogic::Items
{
namespace
{
using Data::Items::ItemOption;
using Data::Items::ItemOptionDefinition;
using Data::Items::ItemOptionType;

// The level of a type without levels: its constant counts.
constexpr int LevelOfTypesWithoutLevels = 1;

// Whether the item's data from the server turns the definition's options on.
bool IsTurnedOn(const ITEM& item, ItemOptionType type)
{
    switch (type)
    {
    case ItemOptionType::Luck:
        return item.HasLuck;
    default:
        return false;
    }
}

// The level the option's value is taken at.
int GetOptionLevel(const ITEM&, ItemOptionType)
{
    return LevelOfTypesWithoutLevels;
}
} // namespace

void AddLinkedOptions(ITEM& item, ItemOptionType type)
{
    if (!IsTurnedOn(item, type))
    {
        return;
    }
    const Data::Items::ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
    for (const Data::Items::ItemOptionIndex link : g_ItemDatabase.GetOptionLinks(item.Type))
    {
        const ItemOptionDefinition& definition = options.Get(link);
        if (definition.type != type)
        {
            continue;
        }
        for (size_t option = 0; option < definition.options.size(); ++option)
        {
            const Data::Items::ItemOptionEffect& effect = options.GetEffect(link, option);
            const int value = effect.writesValue ? definition.options[option].value.At(GetOptionLevel(item, type)) : 0;
            AddOptionSlot(item, effect.effect, value, {link, static_cast<std::uint16_t>(option)});
        }
    }
}

std::optional<SlotOption> FindSlotOption(const ITEM& item, int slot)
{
    const std::optional<OptionSource> source = GetOptionSource(item, slot);
    if (!source)
    {
        return std::nullopt;
    }
    const Data::Items::ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
    if (source->definition >= options.GetDefinitions().size())
    {
        return std::nullopt;
    }
    const ItemOptionDefinition& definition = options.Get(source->definition);
    if (source->option >= definition.options.size())
    {
        return std::nullopt;
    }
    return SlotOption{&definition, &definition.options[source->option]};
}

int GetSlotOptionValue(const ITEM& item, const SlotOption& slotOption)
{
    return slotOption.option->value.At(GetOptionLevel(item, slotOption.definition->type));
}

int GetItemOptionValue(const ITEM& item, int slot)
{
    if (const std::optional<SlotOption> slotOption = FindSlotOption(item, slot))
    {
        return GetSlotOptionValue(item, *slotOption);
    }
    return slot >= 0 && slot < item.SpecialNum ? item.SpecialValue[slot] : 0;
}
} // namespace GameLogic::Items
