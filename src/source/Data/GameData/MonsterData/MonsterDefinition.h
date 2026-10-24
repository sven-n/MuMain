#pragma once

#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MonsterData/MonsterKind.h"

namespace Data::Monsters
{
// Monster numbers are 0 to MonsterNumberCount - 1: the server sends them in
// 10 bits (packet 0x13).
constexpr int MonsterNumberCount = MAX_MONSTER;

// The game holds a monster's name in MAX_MONSTER_NAME wide characters; a longer
// name is cut, counted in UTF-16 units.
constexpr int MaxMonsterNameLength = MAX_MONSTER_NAME;

// What the client knows about one monster number (monsters design, D45).
struct MonsterDefinition
{
    int number = 0;
    // The names by language; the English name may be empty (252).
    LocalizedString names;
    MonsterKind kind = MonsterKind::Monster;

    bool operator==(const MonsterDefinition& other) const = default;
};
} // namespace Data::Monsters
