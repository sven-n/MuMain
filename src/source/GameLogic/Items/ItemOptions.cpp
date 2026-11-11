#include "stdafx.h"

#include "GameLogic/Items/ItemOptions.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "GameLogic/Items/ItemOptionSlots.h"

#include <algorithm>

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
    case ItemOptionType::Option:
        return item.OptionLevel > 0;
    default:
        return false;
    }
}

// Whether the item's data turns this one of the definition's options on: the additional option's by its number, which
// the server sends as the option type (ITEM::OptionType, the option byte's high nibble; 0 for most items), as OpenMU
// reads it.
bool IsOptionTurnedOn(int optionType, ItemOptionType type, const ItemOption& option)
{
    return type != ItemOptionType::Option || option.number == optionType;
}

// The source of the item's slot `slot` when it names an option the database has; nothing for a slot the code wrote.
std::optional<OptionSource> FindValidSource(const ITEM& item, int slot)
{
    const std::optional<OptionSource> source = GetOptionSource(item, slot);
    if (!source)
    {
        return std::nullopt;
    }
    const Data::Items::ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
    if (source->definition >= options.GetDefinitions().size() ||
        source->option >= options.Get(source->definition).options.size())
    {
        return std::nullopt;
    }
    return source;
}

// The level the option's value is taken at: the additional option's level, else the constant's.
int GetOptionLevel(const ITEM& item, ItemOptionType type)
{
    return type == ItemOptionType::Option ? item.OptionLevel : LevelOfTypesWithoutLevels;
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
            if (!IsOptionTurnedOn(item.OptionType, type, definition.options[option]))
            {
                continue;
            }
            const Data::Items::ItemOptionEffect& effect = options.GetEffect(link, option);
            const int value = effect.writesValue ? definition.options[option].value.At(GetOptionLevel(item, type)) : 0;
            AddOptionSlot(item, effect.effect, value, {link, static_cast<std::uint16_t>(option)});
        }
    }
}

bool LinksOptionType(int itemType, ItemOptionType type)
{
    const Data::Items::ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
    for (const Data::Items::ItemOptionIndex link : g_ItemDatabase.GetOptionLinks(itemType))
    {
        if (options.Get(link).type == type)
        {
            return true;
        }
    }
    return false;
}

bool HasAdditionalOption(const ITEM& item)
{
    // The Horn of Dinorant's option level is three option flags, which count as no level.
    return GetAdditionalOptionLevel(item) > 0 || (item.Type == ITEM_HORN_OF_DINORANT && item.OptionLevel > 0);
}

int GetAdditionalOptionLevel(const ITEM& item)
{
    return CountAdditionalOptionLevel(item.Type, item.OptionLevel, item.OptionType);
}

int CountAdditionalOptionLevel(int itemType, int optionLevel, int optionType)
{
    return CountAdditionalOptionLevel(g_ItemDatabase.GetOptionLinks(itemType),
                                      g_ItemDatabase.GetSlot(itemType) == Data::Items::ItemSlot::Wings, optionLevel,
                                      optionType);
}

int CountAdditionalOptionLevel(std::span<const Data::Items::ItemOptionIndex> links, bool wing, int optionLevel,
                               int optionType)
{
    if (optionLevel <= 0)
    {
        return 0;
    }
    // One pass over the links: the ground label asks every frame for each item on the ground.
    bool linksOption = false;
    const Data::Items::ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
    for (const Data::Items::ItemOptionIndex link : links)
    {
        const ItemOptionDefinition& definition = options.Get(link);
        if (definition.type != ItemOptionType::Option)
        {
            continue;
        }
        linksOption = true;
        for (const ItemOption& option : definition.options)
        {
            if (IsOptionTurnedOn(optionType, definition.type, option))
            {
                // Above a list its last level counts, as its value does.
                return option.value.isList ? std::min(optionLevel, static_cast<int>(option.value.values.size()))
                                           : optionLevel;
            }
        }
    }
    // The wings' option is still code and counts at its option level; the Horn of Dinorant's option level is three
    // option flags (SetItemAttributes), no level, until it moves into data (9b5).
    return !linksOption && wing ? optionLevel : 0;
}

bool DamageOptionRaisesWizardry(const ITEM& item)
{
    const Data::Items::ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
    for (int slot = 0; slot < item.SpecialNum; ++slot)
    {
        const std::optional<OptionSource> source = FindValidSource(item, slot);
        if (source && options.GetEffect(source->definition, source->option).raisesWizardry)
        {
            return true;
        }
    }
    return false;
}

std::optional<SlotOption> FindSlotOption(const ITEM& item, int slot)
{
    const std::optional<OptionSource> source = FindValidSource(item, slot);
    if (!source)
    {
        return std::nullopt;
    }
    const ItemOptionDefinition& definition = g_ItemDatabase.GetOptions().Get(source->definition);
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
