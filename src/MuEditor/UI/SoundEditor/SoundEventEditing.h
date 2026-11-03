#pragma once

#ifdef _EDITOR

#include "Data/GameData/SoundData/SoundEvents.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// What the Sound Editor's Events tab shows and reads of a rule (sound editor
// design, Q-SE1).
namespace MuEditor::SoundEditor
{
// "7, 24-29, 36, 67": sorted, runs of three or more as ranges; "" for none.
std::string NumbersText(const std::vector<int>& numbers);

// The numbers of such a text, from 0 to `highest`, sorted and each once;
// {} for "". Nullopt for anything else ("a", "5-2", a number past `highest`).
std::optional<std::vector<int>> ParseNumbers(std::string_view text, int highest);

// "confirm, cancel"; "" for none.
std::string KindsText(const std::vector<std::string>& kinds);

// The click kinds of such a text, each once, in its order; {} for "".
// Nullopt for anything else ("Confirm", "a b", "a,,b").
std::optional<std::vector<std::string>> ParseKinds(std::string_view text);

// What the tab calls an event: "Your footsteps", "A player's swing"; its name
// for one it does not know.
std::string EventTitle(std::string_view event);

// The fields of an event's rules the tab shows: the ones its code fills in
// (the footsteps' world, tile and safe zone; a click's kind), every field
// for an event it does not know, and any field a rule of it uses.
struct EventFields
{
    bool worlds = false;
    bool tiles = false;
    bool safeZone = false;
    bool holds = false;
    bool kinds = false;

    bool operator==(const EventFields&) const = default;
};
EventFields FieldsOf(const Data::Sounds::SoundEvent& event);

// The rule's sounds by the names the Sound Editor gave them, else their ids.
std::string PlaysText(const Data::Sounds::SoundEventRule& rule);

// The rule with `id` added to its sounds (once) or taken out of them; false
// when nothing changed or the id is no SOUND_ id.
bool AddPlay(Data::Sounds::SoundEventRule& rule, const std::string& id);
bool RemovePlay(Data::Sounds::SoundEventRule& rule, size_t index);

// The event without its rule `index`; false for its only rule, which "Add
// rule" copies, or past its rules.
bool RemoveRule(Data::Sounds::SoundEvent& event, size_t index);
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
