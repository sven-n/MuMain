#pragma once

#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <optional>

typedef struct tagITEM ITEM;

// The options of an item from the option definitions it links (Data/Items/Options): which of them its data from the
// server turns on, and their values. They go into the item's option slots (ItemOptionSlots.h) with where they came
// from, so the tooltip shows their texts and values from the data. See docs/item-data.md, "Item options".
namespace GameLogic::Items
{
// Gives the item the options of its linked definitions of `type` that its data turns on (luck: ITEM::HasLuck), each in
// its next option slot with the effect the client computes for it.
void AddLinkedOptions(ITEM& item, Data::Items::ItemOptionType type);

// The option of an option slot and the definition it is of.
struct SlotOption
{
    const Data::Items::ItemOptionDefinition* definition = nullptr;
    const Data::Items::ItemOption* option = nullptr;
};

// The option in the item's slot `slot` and its definition; nothing for a slot the code wrote (the skill) or past the
// item's slots.
std::optional<SlotOption> FindSlotOption(const ITEM& item, int slot);

// The value of the slot's option at the item's level, from its definition: what its tooltip lines show.
int GetSlotOptionValue(const ITEM& item, const SlotOption& slotOption);

// The same for the item's slot `slot`; a slot the code wrote gives its SpecialValue.
int GetItemOptionValue(const ITEM& item, int slot);
} // namespace GameLogic::Items
