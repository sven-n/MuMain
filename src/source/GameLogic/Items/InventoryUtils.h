#ifndef _INVENTORYUTILS_H_
#define _INVENTORYUTILS_H_

#pragma once

#include "Core/Globals/_define.h"

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

#endif // _INVENTORYUTILS_H_
