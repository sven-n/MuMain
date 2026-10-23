#include "stdafx.h"

#include "GameLogic/Items/TransformationRing.h"

#include "Data/GameData/MonsterData/MonsterDatabase.h"

#include <array>

namespace GameLogic::Items::TransformationRing
{
namespace
{
// The form of each ring level: Budge Dragon, Giant, Skeleton Warrior, Poison
// Bull, Thunder Lich and Death Cow.
constexpr std::array<int, 6> FormByLevel = {2, 7, 14, 8, 9, 41};
} // namespace

const wchar_t* MonsterName(int ringLevel)
{
    if (ringLevel < 0 || ringLevel >= static_cast<int>(FormByLevel.size()))
    {
        return nullptr;
    }
    return g_MonsterDatabase.FindDisplayName(FormByLevel[static_cast<std::size_t>(ringLevel)]);
}
} // namespace GameLogic::Items::TransformationRing
