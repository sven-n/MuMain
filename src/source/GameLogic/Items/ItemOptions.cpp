#include "stdafx.h"

#include "GameLogic/Items/ItemOptions.h"

#include "Character/CharacterManager.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemOptionEffects.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Items/ItemOptionSlots.h"

#include <algorithm>
#include <cstdint>

namespace GameLogic::Items
{
namespace
{
using Data::Items::ItemOption;
using Data::Items::ItemOptionDefinition;
using Data::Items::ItemOptionType;

// The level of a type without levels: its constant counts.
constexpr int LevelOfTypesWithoutLevels = 1;

// The highest value an option slot holds (ITEM::SpecialValue, a WORD).
constexpr std::int64_t MaxSlotValue = 0xFFFF;

// Whether the item's data from the server turns the definition's options on.
bool IsTurnedOn(const ITEM& item, ItemOptionType type)
{
    switch (type)
    {
    case ItemOptionType::Luck:
        return item.HasLuck;
    case ItemOptionType::Option:
        return item.OptionLevel > 0;
    case ItemOptionType::Excellent:
        return item.ExcellentFlags != 0;
    default:
        return false;
    }
}

// Whether the additional option is the one the server sends: by its number, the option type (ITEM::OptionType, the
// option byte's high nibble; 0 for most items), as OpenMU reads it.
bool IsAdditionalOptionSent(int optionType, const ItemOption& option)
{
    return option.number == optionType;
}

// Whether the excellent byte turns the excellent option on: its bit number - 1.
bool IsExcellentBitSet(BYTE excellentFlags, const ItemOption& option)
{
    return option.number >= 1 && option.number <= Data::Items::MaxExcellentOptionNumber &&
           ((excellentFlags >> (option.number - 1)) & 1) != 0;
}

// Whether the item's data turns this one of the definition's options on: the additional option by its number, an
// excellent option by its bit of the excellent byte; luck's one option with the definition.
bool IsOptionTurnedOn(const ITEM& item, ItemOptionType type, const ItemOption& option)
{
    switch (type)
    {
    case ItemOptionType::Option:
        return IsAdditionalOptionSent(item.OptionType, option);
    case ItemOptionType::Excellent:
        return IsExcellentBitSet(item.ExcellentFlags, option);
    case ItemOptionType::Luck:
        break;
    }
    return true;
}

// The value of a hero attribute a term reads; 0 for one the client doesn't read.
int GetHeroTermAttribute(Data::Items::ItemOptionTermAttribute attribute)
{
    if (CharacterAttribute == nullptr)
    {
        return 0;
    }
    switch (attribute)
    {
    case Data::Items::ItemOptionTermAttribute::Level:
        return CharacterAttribute->Level;
    case Data::Items::ItemOptionTermAttribute::TotalLevel:
        return CharacterAttribute->Level + gCharacterManager.GetHeroMasterLevel();
    case Data::Items::ItemOptionTermAttribute::MasterLevel:
        return gCharacterManager.GetHeroMasterLevel();
    case Data::Items::ItemOptionTermAttribute::Unread:
        break;
    }
    return 0;
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
        for (const size_t option : options.GetOptionOrder(link))
        {
            if (!IsOptionTurnedOn(item, type, definition.options[option]))
            {
                continue;
            }
            const Data::Items::ItemOptionEffect& effect = options.GetEffect(link, option);
            const int value = effect.writesValue
                                  ? ComputeOptionValue(definition.options[option].value, GetOptionLevel(item, type))
                                  : 0;
            AddOptionSlot(item, effect.effect, value, {link, static_cast<std::uint16_t>(option)});
        }
    }
}

bool HasExcellentOptions(int itemType, BYTE excellentFlags)
{
    return excellentFlags != 0 && HasExcellentOptions(g_ItemDatabase.GetOptionLinks(itemType), excellentFlags);
}

bool HasExcellentOptions(std::span<const Data::Items::ItemOptionIndex> links, BYTE excellentFlags)
{
    if (excellentFlags == 0)
    {
        return false;
    }
    const Data::Items::ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
    for (const Data::Items::ItemOptionIndex link : links)
    {
        const ItemOptionDefinition& definition = options.Get(link);
        if (definition.type != ItemOptionType::Excellent)
        {
            continue;
        }
        for (const ItemOption& option : definition.options)
        {
            if (IsExcellentBitSet(excellentFlags, option))
            {
                return true;
            }
        }
    }
    return false;
}

bool HasExcellentOptions(const ITEM& item)
{
    return HasExcellentOptions(item.Type, item.ExcellentFlags);
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
            if (IsAdditionalOptionSent(optionType, option))
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

int ComputeOptionValue(const Data::Items::ItemOptionValue& value, int level)
{
    if (value.per.empty())
    {
        return value.At(level);
    }
    // Times a hero attribute a term can pass what the slot's value holds (a WORD); the sum stops at its range.
    std::int64_t result = value.At(level);
    for (const Data::Items::ItemOptionTerm& term : value.per)
    {
        const std::int64_t attribute = GetHeroTermAttribute(term.heroAttribute);
        result += term.divisor > 0 ? attribute / term.divisor : attribute * term.times;
    }
    return static_cast<int>(std::clamp<std::int64_t>(result, 0, MaxSlotValue));
}

int GetSlotOptionValue(const ITEM& item, const SlotOption& slotOption)
{
    return ComputeOptionValue(slotOption.option->value, GetOptionLevel(item, slotOption.definition->type));
}

int GetCurrentSlotValue(const ITEM& item, int slot)
{
    if (slot < 0 || slot >= item.SpecialNum)
    {
        return 0;
    }
    // Only an option that writes its value: the others keep the 0 the stat code reads them with.
    const std::optional<OptionSource> source = FindValidSource(item, slot);
    if (source)
    {
        const Data::Items::ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
        const ItemOptionDefinition& definition = options.Get(source->definition);
        const ItemOption& option = definition.options[source->option];
        if (!option.value.per.empty() && options.GetEffect(source->definition, source->option).writesValue)
        {
            return ComputeOptionValue(option.value, GetOptionLevel(item, definition.type));
        }
    }
    return item.SpecialValue[slot];
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
