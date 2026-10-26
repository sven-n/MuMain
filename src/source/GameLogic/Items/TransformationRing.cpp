#include "stdafx.h"

#include "GameLogic/Items/TransformationRing.h"

#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "GameLogic/Items/ItemNameParts.h"

#include <array>

namespace GameLogic::Items::TransformationRing
{
namespace
{
// The monster each ring level turns into.
constexpr std::array<EMonsterType, 6> FormByLevel = {MONSTER_BUDGE_DRAGON,     MONSTER_GIANT,
                                                     MONSTER_SKELETON_WARRIOR, MONSTER_POISON_BULL,
                                                     MONSTER_THUNDER_LICH,     MONSTER_DEATH_COW};
} // namespace

const wchar_t* MonsterName(int ringLevel)
{
    if (ringLevel < 0 || ringLevel >= static_cast<int>(FormByLevel.size()))
    {
        return nullptr;
    }
    return NameParts::MonsterName(FormByLevel[static_cast<std::size_t>(ringLevel)]);
}
} // namespace GameLogic::Items::TransformationRing
