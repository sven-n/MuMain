#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterGroup.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

// The items the monsters wear (monsters design, D60, Q-MN41): a monster names
// an item by its group and number in its equipment, in its own setup or in a
// variant. The item tools list them in "used by" and in the move and remove
// dialogs, a move can update them, and Data Checks reports one that names an
// item the item list does not have.
namespace Data::Monsters
{
// Where an entry names an item: its setup's equipment, or a map, state or
// random variant's, and the slot.
struct MonsterItemPlace
{
    enum class Variant
    {
        // The entry's own setup.
        None,
        // mapVariants[index].setup.
        Map,
        // setup.stateVariants[index].setup; `state` is its state.
        State,
        // randomVariants[index].
        Random,
        // mapVariants[mapIndex].setup.stateVariants[index].setup; `state` is
        // its state.
        MapState,
    };

    Variant variant = Variant::None;
    std::size_t index = 0;
    std::string state;
    // As the monster list names it: "weapons[0]", "helm", "helper".
    std::string slot;
    // The map variant of a MapState place.
    std::size_t mapIndex = 0;

    bool operator==(const MonsterItemPlace&) const = default;
};

// The place as the monster list's fields name it: "equipment.helm",
// "mapVariants[0].equipment.weapons[1]",
// "stateVariants[1].equipment.armor", "randomVariants[0].equipment.helm",
// "mapVariants[0].stateVariants[1].equipment.helm".
std::string GetItemPlaceField(const MonsterItemPlace& place);

// Calls `visit` for each item the entry names; a body that is no item
// (MonsterItem::part) is skipped.
void ForEachMonsterItem(const MonsterDefinition& monster,
                        const std::function<void(const MonsterItem& item, const MonsterItemPlace& place)>& visit);
void ForEachMonsterItem(MonsterDefinition& monster,
                        const std::function<void(MonsterItem& item, const MonsterItemPlace& place)>& visit);

// A monster number that names an item, and where.
struct MonsterItemWearer
{
    int number = 0;
    MonsterItemPlace place;

    bool operator==(const MonsterItemWearer&) const = default;
};

// Where the monsters name the item of the type, in the order of the
// monsters and their places.
std::vector<MonsterItemWearer> FindItemWearers(std::span<const MonsterDefinition> monsters, int itemType);
std::vector<MonsterItemWearer> FindItemWearers(std::span<const MonsterDefinition* const> monsters, int itemType);

// Makes each place of the entry that names the item `fromItemType` name
// `toItemType`; its level and excellent flags stay. The number of places it
// changed.
int ReplaceItemReferences(MonsterDefinition& monster, int fromItemType, int toItemType);

// A place whose item a move changed in the monster table, which the item
// tools' Save writes into the monster list file (Q-MN41): the file names
// `saved` there, the table `current`.
struct MonsterItemChange
{
    int number = 0;
    MonsterItemPlace place;
    int saved = 0;
    int current = 0;

    bool operator==(const MonsterItemChange&) const = default;
};

// The item type the entry names at the place; nullopt when it names none
// there.
std::optional<int> FindItemAt(const MonsterDefinition& monster, const MonsterItemPlace& place);

// Writes the changes into the monsters of a monster list file: the entry of
// the number that names `saved` at the place names `current`; one that names
// another item there was changed since and stays. A value a monster's numbers
// shared before (MonsterGroup::shared) is shared again when they all have
// the same new value. False when no entry changed.
bool ApplyMonsterItemChanges(std::vector<MonsterGroup>& groups, std::span<const MonsterItemChange> changes);

// An item a monster names that the item list does not have is an error (D65):
// the monster wears nothing there. `itemExists` tells whether the list has an
// item of the type. Each problem has the monster's number and the field
// "monster <number> <place>" (GetItemPlaceField). Appends them to `issues`.
void ValidateMonsterItems(std::span<const MonsterDefinition* const> monsters,
                          const std::function<bool(int itemType)>& itemExists, const std::string& source,
                          std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
