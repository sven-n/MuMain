#pragma once

#include <cstdint>
#include <optional>

typedef struct tagITEM ITEM;

namespace GameLogic::Items
{
// Where the option of a slot came from: a definition of the item option files (an index of the item database's
// options, Data::Items::ItemOptionIndex) and its option there.
struct OptionSource
{
    std::uint16_t definition = 0;
    std::uint16_t option = 0;

    bool operator==(const OptionSource& other) const = default;
};

// Gives the item's next option slot (ITEM::Special, ITEM::SpecialValue) the option and its value;
// a slot holds values up to 65535. An item has MAX_ITEM_SPECIAL slots: an option past the last
// is dropped, with a warning in the log the first time an item of its type drops one.
void AddOptionSlot(ITEM& item, std::uint16_t option, int value = 0);

// The same for an option of an option definition: the slot also keeps where it came from (ITEM::SpecialSource).
void AddOptionSlot(ITEM& item, std::uint16_t option, int value, OptionSource source);

// Gives the item's slot of the option the value, or the option the next slot (as AddOptionSlot) when the item has
// none yet: for an option whose value changes after the item got its options, such as the Dark Horse's defense.
void SetOptionSlot(ITEM& item, std::uint16_t option, int value);

// Where the option of the item's slot came from; nothing for a slot the code wrote (the skill) or past its slots.
std::optional<OptionSource> GetOptionSource(const ITEM& item, int slot);
} // namespace GameLogic::Items
