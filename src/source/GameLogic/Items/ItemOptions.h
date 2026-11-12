#pragma once

#include "Data/GameData/ItemData/ItemOptionDatabase.h"
#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <optional>
#include <span>

typedef struct tagITEM ITEM;

// The options of an item from the option definitions it links (Data/Items/Options): which of them its data from the
// server turns on, and their values. They go into the item's option slots (ItemOptionSlots.h) with where they came
// from, so the tooltip shows their texts and values from the data. See docs/item-data.md, "Item options".
namespace GameLogic::Items
{
// Gives the item the options of its linked definitions of `type` that its data turns on (luck: ITEM::HasLuck; the
// additional option: ITEM::OptionLevel, the option numbered ITEM::OptionType), each in its next option slot with the
// effect the client computes for it.
void AddLinkedOptions(ITEM& item, Data::Items::ItemOptionType type);

// Whether the item has an active excellent option: it links a definition of type excellent with an option whose bit
// its excellent byte has. What "excellent" means for the name, the maximum durability, the tooltip and the stats; wings
// with wing options and the Horn of Fenrir, whose byte means something else, are not excellent by it.
bool HasExcellentOptions(int itemType, BYTE excellentFlags);
bool HasExcellentOptions(const ITEM& item);
// The same for an item with these option links (an item draft's).
bool HasExcellentOptions(std::span<const Data::Items::ItemOptionIndex> links, BYTE excellentFlags);

// Whether the item links a definition of `type`.
bool LinksOptionType(int itemType, Data::Items::ItemOptionType type);

// Whether the item has an additional option: from its linked definition for its option level and type, or for the
// items whose additional option is still code (the wings, the capes and the Horn of Dinorant) for any option level.
// What the "+Option" in the name and the ground label show.
bool HasAdditionalOption(const ITEM& item);

// The level the item's additional option counts at: its option level, the last level of its value list above the
// list, 0 when it has no additional option and for the Horn of Dinorant, whose option level is three option flags.
// What the strength requirement grows by (strengthPerOptionLevel).
int GetAdditionalOptionLevel(const ITEM& item);

// The same for a stored item type (its links and slot in the item database) with this option level and option type
// the server sends: the game's count.
int CountAdditionalOptionLevel(int itemType, int optionLevel, int optionType);

// The same for an item with these option links (indices of the database's options) that is a wing or not: what an
// item draft counts, whose links and slot can differ from the stored item's.
int CountAdditionalOptionLevel(std::span<const Data::Items::ItemOptionIndex> links, bool wing, int optionLevel,
                               int optionType);

// Whether the item's damage option (AT_IMPROVE_DAMAGE) raises its wizardry damage too: the effect of a magic sword's
// option (ItemOptionEffect::raisesWizardry).
bool DamageOptionRaisesWizardry(const ITEM& item);

// The option of an option slot and the definition it is of.
struct SlotOption
{
    const Data::Items::ItemOptionDefinition* definition = nullptr;
    const Data::Items::ItemOption* option = nullptr;
};

// The option in the item's slot `slot` and its definition; nothing for a slot the code wrote (the skill) or past the
// item's slots.
std::optional<SlotOption> FindSlotOption(const ITEM& item, int slot);

// The option value at `level` with its terms on the hero's attributes (level / 20) as they are now; the terms'
// attributes as the option database resolved them (Data::Items::ResolveTermAttributes). A value with terms stops at
// 0 and 65535, what an option slot holds.
int ComputeOptionValue(const Data::Items::ItemOptionValue& value, int level);

// The value of the slot's option at the item's level, from its definition: what its tooltip lines show.
int GetSlotOptionValue(const ITEM& item, const SlotOption& slotOption);

// What the stat code adds for the item's slot `slot`: an option with terms on the hero's attributes (level / 20) is
// computed again, as the hero is now; any other slot gives the SpecialValue written when the item was made.
int GetCurrentSlotValue(const ITEM& item, int slot);

// The same for the item's slot `slot`; a slot the code wrote gives its SpecialValue.
int GetItemOptionValue(const ITEM& item, int slot);
} // namespace GameLogic::Items
