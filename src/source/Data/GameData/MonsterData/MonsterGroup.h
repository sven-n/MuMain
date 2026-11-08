#pragma once

#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <array>
#include <span>
#include <vector>

namespace Data::Monsters
{
// A monster with its numbers (monsters design, D61): the values its numbers
// share, and the entry of each number as the game uses it, the shared values
// with the number's own. A number's own values are the fields its entry has
// differently from the shared ones (any field may be, Q-MN50); a monster with
// one number shares all of its values with it.
struct MonsterGroup
{
    // The shared values; `number` is not used.
    MonsterDefinition shared;
    // The entries of the numbers, at least one.
    std::vector<MonsterDefinition> numbers;

    bool operator==(const MonsterGroup&) const = default;
};

// The values the monster's numbers share as the monster list file writes
// them: those of the number when it has one.
const MonsterDefinition& GetSharedValues(const MonsterGroup& group);

// The entries of all numbers of the monsters, in their order.
std::vector<MonsterDefinition> CollectNumberEntries(std::span<const MonsterGroup> groups);

// A monster of its own for each entry, which shares all its values with it.
std::vector<MonsterGroup> MakeOneNumberGroups(std::span<const MonsterDefinition> entries);

// The fields a number may have as its own (Q-MN50); `number` and `maps` are
// each number's and never shared.
enum class MonsterValueField
{
    Names,
    Kind,
    Model,
    SoundSet,
    Attacks,
    MapVariants,
    RandomVariants,
};

inline constexpr std::array<MonsterValueField, 7> MonsterValueFields{
    MonsterValueField::Names,          MonsterValueField::Kind,    MonsterValueField::Model,
    MonsterValueField::SoundSet,       MonsterValueField::Attacks, MonsterValueField::MapVariants,
    MonsterValueField::RandomVariants,
};

// Whether the two have the same value in the field.
bool HasSameValue(const MonsterDefinition& left, const MonsterDefinition& right, MonsterValueField field);

// Gives `to` the value `from` has in the field.
void CopyValue(MonsterDefinition& to, const MonsterDefinition& from, MonsterValueField field);

// The values as a monster's shared values hold them: without a number and
// maps.
MonsterDefinition MakeSharedValues(const MonsterDefinition& values);
} // namespace Data::Monsters
