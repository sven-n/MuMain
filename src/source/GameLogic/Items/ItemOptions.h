#pragma once

#include "Data/GameData/ItemData/ItemOptionDefinition.h"

typedef struct tagITEM ITEM;

// The options of an item from the option definitions it links (Data/Items/Options): which of them its data from the
// server turns on, and their values. They go into the item's option slots (ItemOptionSlots.h) with where they came
// from, so the tooltip shows their texts and values from the data. See docs/item-data.md, "Item options".
namespace GameLogic::Items
{
// Gives the item the options of its linked definitions of `type` that its data turns on (luck: ITEM::HasLuck), each in
// its next option slot with the effect the client computes for it.
void AddLinkedOptions(ITEM& item, Data::Items::ItemOptionType type);

// The option in the item's slot `slot`, from its definition; nullptr for a slot the code wrote (the skill) or past
// the item's slots.
const Data::Items::ItemOption* FindSlotOption(const ITEM& item, int slot);

// The value of the option in the slot at the item's level, from its definition: what its tooltip lines show. A slot
// the code wrote gives its SpecialValue.
int GetItemOptionValue(const ITEM& item, int slot);
} // namespace GameLogic::Items
