#ifndef _INVENTORYUTILS_H_
#define _INVENTORYUTILS_H_

#pragma once

#include "NewUISystem.h"

inline const ITEM* FindInventoryItemBySlot(const int slot)
{
    if (slot < MAX_EQUIPMENT_INDEX || slot >= MAX_MY_INVENTORY_EX_INDEX)
    {
        return nullptr;
    }

    if (slot < MAX_MY_INVENTORY_INDEX)
    {
        return (g_pMyInventory != nullptr) ? g_pMyInventory->FindItem(slot) : nullptr;
    }

    return (g_pMyInventoryExt != nullptr) ? g_pMyInventoryExt->FindItem(slot) : nullptr;
}

#endif // _INVENTORYUTILS_H_
