#include "stdafx.h"

#include "UI/Inventory/StorageUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/StorageInventory.h"

namespace UI::Storage
{
void ContainerListed(std::span<const ContainerItem> items)
{
    if (UI::Windows::IsVisible(mu::ui::window::INTERFACE_NPCSHOP))
    {
        g_pNPCShop->DeleteAllItems();
        for (const ContainerItem& item : items)
            g_pNPCShop->InsertItem(item.slot, item.itemData);
    }
    else if (UI::Windows::IsVisible(mu::ui::window::INTERFACE_STORAGE))
    {
        for (const ContainerItem& item : items)
        {
            if (item.slot < MAX_SHOP_INVENTORY)
                g_pStorageInventory->InsertItem(item.slot, item.itemData);
            else
                g_pStorageInventoryExt->InsertItem(item.slot, item.itemData);
        }
    }
}

void VaultItemPlaced(int slot, std::span<const std::uint8_t> itemData)
{
    if (slot < MAX_SHOP_INVENTORY)
        g_pStorageInventory->ProcessToReceiveStorageItems(slot, itemData);
    else
        g_pStorageInventoryExt->ProcessToReceiveStorageItems(slot, itemData);
}

void VaultStatusChanged(VaultStatus status)
{
    g_pStorageInventory->ProcessToReceiveStorageStatus(status);
}
}
