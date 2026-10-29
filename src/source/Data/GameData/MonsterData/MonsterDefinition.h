#pragma once

#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MapData/MapRef.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterKind.h"

#include <optional>
#include <string>
#include <vector>

namespace Data::Monsters
{
// Monster numbers are 0 to MonsterNumberCount - 1: the server sends them in
// 10 bits (packet 0x13).
constexpr int MonsterNumberCount = MAX_MONSTER;

// The game holds a monster's name in MAX_MONSTER_NAME wide characters; a longer
// name is cut, counted in UTF-16 units.
constexpr int MaxMonsterNameLength = MAX_MONSTER_NAME;

// What a monster does differently on some maps (monsters design, D62): on
// its maps, a field the variant has replaces the entry's (Q-MN68). The
// attacks PR brings `attacks`; the setup fields follow in their PR.
struct MonsterMapVariant
{
    // The maps, by the numbers the server sends and the stages of Devil
    // Square (D67): sorted, at least one.
    std::vector<Data::Maps::MapRef> maps;
    std::optional<MonsterAttacks> attacks;

    bool operator==(const MonsterMapVariant&) const = default;
};

// What the client knows about one monster number (monsters design, D45).
struct MonsterDefinition
{
    int number = 0;
    // The names by language; the English name may be empty (252).
    LocalizedString names;
    MonsterKind kind = MonsterKind::Monster;
    // The model its monsters appear with (MonsterModelSlots.h
    // FindMonsterModelReference); empty when the list does not say. The
    // creators in the code still choose the model (until MN4); a test checks
    // that they choose this one.
    std::string model;
    // Which animation each skill plays (D48); without them the code decides
    // (the generic attack 1, attack 2, attack 2 pattern).
    std::optional<MonsterAttacks> attacks;
    // What the monster does differently on some maps; no map is in two.
    std::vector<MonsterMapVariant> mapVariants;

    bool operator==(const MonsterDefinition& other) const = default;
};

// The attacks the monster plays on a map (a server map number and stage):
// those of the map variant that names the map and has attacks, else the
// entry's own.
const std::optional<MonsterAttacks>& GetAttacksOnMap(const MonsterDefinition& monster, int map, int stage = 0);

// The maps and stages of the map variant at `variant` that another variant of
// the monster names too (Data::Maps::Overlaps); a map in two variants is
// fatal (Q-MN65).
std::vector<Data::Maps::MapRef> FindMapsOfOtherVariants(const MonsterDefinition& monster, size_t variant);
} // namespace Data::Monsters
