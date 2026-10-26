#pragma once

#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterKind.h"

#include <optional>
#include <string>

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
    // The model its monsters appear with (MonsterModelSlots.h
    // FindMonsterModelReference); empty when the list does not say. The
    // creators in the code still choose the model (until MN4); a test checks
    // that they choose this one.
    std::string model;
    // Which animation each skill plays (D48); without them the code decides
    // (the generic attack 1, attack 2, attack 2 pattern).
    std::optional<MonsterAttacks> attacks;

    bool operator==(const MonsterDefinition& other) const = default;
};
} // namespace Data::Monsters
