#pragma once

#include <cstdint>

typedef struct tagITEM ITEM;

namespace GameLogic::Items
{
// Gives the item's next option slot (ITEM::Special, ITEM::SpecialValue) the option and its value;
// the slot keeps the value's low byte. An item has MAX_ITEM_SPECIAL slots: an option past the last
// is dropped, with a warning in the log the first time an item of its type drops one.
void AddOptionSlot(ITEM& item, std::uint16_t option, int value = 0);

// Gives the item's slot of the option the value, or the option the next slot (as AddOptionSlot) when the item has
// none yet: for an option whose value changes after the item got its options, such as the Dark Horse's defense.
void SetOptionSlot(ITEM& item, std::uint16_t option, int value);
} // namespace GameLogic::Items
