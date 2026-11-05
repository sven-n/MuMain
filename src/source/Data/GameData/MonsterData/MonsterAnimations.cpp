#include "stdafx.h"

#include "MonsterAnimations.h"

#include "Core/Globals/_define.h"

#include <array>
#include <charconv>

namespace Data::Monsters
{
namespace
{
constexpr std::array<std::string_view, 12> AnimationNames = {
    "stop1", "stop2", "walk", "attack1", "attack2", "shock", "die", "appear", "attack3", "attack4", "run", "attack5",
};
static_assert(MONSTER01_STOP1 == 0 && MONSTER01_STOP2 == 1 && MONSTER01_WALK == 2 && MONSTER01_ATTACK1 == 3 &&
              MONSTER01_ATTACK2 == 4 && MONSTER01_SHOCK == 5 && MONSTER01_DIE == 6 && MONSTER01_APEAR == 7 &&
              MONSTER01_ATTACK3 == 8 && MONSTER01_ATTACK4 == 9 && MONSTER01_RUN == 10 && MONSTER01_ATTACK5 == 11);

// OpenMonsterModel's speeds before a model's own: stop 1 and 2, walk, the
// first two attacks, shock and die.
constexpr std::array<float, 7> MonsterRangeSpeeds = {0.25f, 0.2f, 0.34f, 0.33f, 0.33f, 0.5f, 0.55f};
} // namespace

std::string GetMonsterAnimationName(int animation)
{
    if (animation >= 0 && animation < static_cast<int>(AnimationNames.size()))
    {
        return std::string(AnimationNames[static_cast<size_t>(animation)]);
    }
    return std::to_string(animation);
}

std::optional<int> FindMonsterAnimation(std::string_view name)
{
    for (size_t i = 0; i < AnimationNames.size(); ++i)
    {
        if (AnimationNames[i] == name)
        {
            return static_cast<int>(i);
        }
    }
    int number = 0;
    const auto [end, error] = std::from_chars(name.data(), name.data() + name.size(), number);
    if (error != std::errc() || end != name.data() + name.size() || number < static_cast<int>(AnimationNames.size()))
    {
        return std::nullopt;
    }
    return number;
}

float GetMonsterRangeDefaultSpeed(int animation)
{
    if (animation >= 0 && animation < static_cast<int>(MonsterRangeSpeeds.size()))
    {
        return MonsterRangeSpeeds[static_cast<size_t>(animation)];
    }
    return 0.0f;
}

bool IsHeldByDefaultInMonsterRange(int animation)
{
    return animation == MONSTER01_DIE;
}
} // namespace Data::Monsters
