#include "stdafx.h"

#include "GameLogic/Items/ItemNameParts.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"

namespace GameLogic::Items::NameParts
{
const wchar_t* MonsterName(int monsterNumber)
{
    return g_ItemDataHandler.GetTranslateItemNames() ? g_MonsterDatabase.FindDisplayName(monsterNumber)
                                                     : g_MonsterDatabase.FindEnglishName(monsterNumber);
}
} // namespace GameLogic::Items::NameParts
