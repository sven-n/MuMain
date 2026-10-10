#pragma once

typedef struct tagITEM ITEM;

namespace GameLogic::Items
{
    bool IsTradeBan(const ITEM* pItem);
    bool IsDropBan(const ITEM* pItem);
    bool IsStoreBan(const ITEM* pItem);
}
