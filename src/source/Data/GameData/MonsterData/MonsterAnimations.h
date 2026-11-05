#pragma once

#include <optional>
#include <string>
#include <string_view>

// The animations of monster and NPC models by number (_define.h MONSTER01_*)
// and the names the model data gives them.
namespace Data::Monsters
{
// stop1 0, stop2 1, walk 2, attack1 3, attack2 4, shock 5, die 6, appear 7,
// attack3 8, attack4 9, run 10, attack5 11; a later animation goes by its
// number ("12").
std::string GetMonsterAnimationName(int animation);

// The number of a name GetMonsterAnimationName gives, or nullopt.
std::optional<int> FindMonsterAnimation(std::string_view name);

// The speeds and the held last frame of the monster range's models before
// their own values (OpenMonsterModel): 0 for an animation without one.
float GetMonsterRangeDefaultSpeed(int animation);
bool IsHeldByDefaultInMonsterRange(int animation);

// Every animation of an NPC model plays at this speed before its own values.
constexpr float NpcRangeDefaultSpeed = 0.25f;
} // namespace Data::Monsters
