#pragma once

#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <array>
#include <optional>
#include <string_view>

// The moments of the game a sound event plays at (monsters design, Q-MN76)
// and the names the sound data gives them.
namespace Data::Monsters
{
// In the order the data and the editor list them.
constexpr std::array<MonsterSoundMoment, 5> AllMonsterSoundMoments = {
    MonsterSoundMoment::Attack, MonsterSoundMoment::Hit, MonsterSoundMoment::Death, MonsterSoundMoment::Stand,
    MonsterSoundMoment::Walk};

// "attack", "hit", "death", "stand", "walk".
std::string_view GetMonsterSoundMomentName(MonsterSoundMoment moment);

// The moment of a name GetMonsterSoundMomentName gives, or nullopt.
std::optional<MonsterSoundMoment> FindMonsterSoundMoment(std::string_view name);
} // namespace Data::Monsters
