#include "stdafx.h"

#include "GameLogic/Items/TransformationRing.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"

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
    const int form = FormByLevel[static_cast<std::size_t>(ringLevel)];
    // The monster's name is part of an item name, so it follows Translate
    // item names like the rest of it.
    return g_ItemDataHandler.GetTranslateItemNames() ? g_MonsterDatabase.FindDisplayName(form)
                                                     : g_MonsterDatabase.FindEnglishName(form);
}
} // namespace GameLogic::Items::TransformationRing
