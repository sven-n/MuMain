#pragma once

typedef struct tagITEM ITEM;

namespace GameLogic::Items
{
    bool IsPersonalShopBan(const ITEM* pItem);
    bool IsSellingBan(const ITEM* pItem);
    bool IsRepairBan(const ITEM* pItem);
}
