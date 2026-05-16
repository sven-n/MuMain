#ifndef _INVENTORYUTILS_H_
#define _INVENTORYUTILS_H_

#pragma once

#include "UI/NewUI/NewUISystem.h"

inline bool IsMainInventorySlot(const int slot)
{
    return slot >= MAX_EQUIPMENT_INDEX && slot < MAX_MY_INVENTORY_INDEX;
}

inline bool IsInventoryExtensionSlot(const int slot)
{
    return slot >= MAX_MY_INVENTORY_INDEX && slot < MAX_MY_INVENTORY_EX_INDEX;
}

inline bool IsMyShopSlot(const int slot)
{
    return slot >= MAX_MY_INVENTORY_EX_INDEX && slot < MAX_MY_SHOP_INVENTORY_INDEX;
}

inline bool IsPlayerInventorySlot(const int slot)
{
    return IsMainInventorySlot(slot) || IsInventoryExtensionSlot(slot);
}

inline const ITEM* FindInventoryItemBySlot(const int slot)
{
    if (!IsPlayerInventorySlot(slot))
    {
        return nullptr;
    }

    if (IsMainInventorySlot(slot))
    {
        return (g_pMyInventory != nullptr) ? g_pMyInventory->FindItem(slot) : nullptr;
    }

    return (g_pMyInventoryExt != nullptr) ? g_pMyInventoryExt->FindItem(slot) : nullptr;
}

#endif // _INVENTORYUTILS_H_
