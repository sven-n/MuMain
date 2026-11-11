#include "stdafx.h"

#include "UI/Inventory/InventoryContents.h"

#include "GameLogic/Items/InventoryUtils.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/InventoryCtrl.h"

namespace UI::Inventory
{
void ClearAllItems()
{
    g_pMyInventory->UnequipAllItems();
    g_pMyInventory->DeleteAllItems();
    g_pMyInventoryExt->DeleteAllItems();
    g_pMyShopInventory->DeleteAllItems();
}

void RemoveItem(int index)
{
    if (index >= 0 && index < MAX_EQUIPMENT_INDEX)
        g_pMyInventory->UnequipItem(index);
    else if (IsMainInventorySlot(index))
        g_pMyInventory->DeleteItem(index);
    else if (IsInventoryExtensionSlot(index))
        g_pMyInventoryExt->DeleteItem(index);
    else if (IsMyShopSlot(index))
        g_pMyShopInventory->DeleteItem(index);
}

void RemoveDroppedItem(int index)
{
    if (index >= 0 && index < MAX_EQUIPMENT_INDEX)
        g_pMyInventory->UnequipItem(index);
    else
        g_pMyInventory->DeleteItem(index);
}

bool InsertItem(int index, std::span<const std::uint8_t> itemData)
{
    if (index >= 0 && index < MAX_EQUIPMENT_INDEX)
        return g_pMyInventory->EquipItem(index, itemData);
    if (IsMainInventorySlot(index))
        return g_pMyInventory->InsertItem(index, itemData);
    if (IsInventoryExtensionSlot(index))
        return g_pMyInventoryExt->InsertItem(index, itemData);
    if (IsMyShopSlot(index))
        return g_pMyShopInventory->InsertItem(index, itemData);
    return false;
}

void DiscardPickedItem()
{
    mu::ui::window::CInventoryCtrl::DeletePickedItem();
}

void RestorePickedItem()
{
    mu::ui::window::CInventoryCtrl::BackupPickedItem();
}

int TransferSourceIndex()
{
    if (auto* pickedItem = mu::ui::window::CInventoryCtrl::GetPickedItem())
        return pickedItem->GetSourceLinealPos();

    return g_pMyShopInventory->GetSourceIndex();
}

bool ReceivePlayerTransfer(int index, std::span<const std::uint8_t> itemData)
{
    DiscardPickedItem();

    if (index >= 0 && index < MAX_EQUIPMENT_INDEX)
    {
        g_pMyInventory->EquipItem(index, itemData);
        return true;
    }

    if (IsPlayerInventorySlot(index))
    {
        g_pStorageInventory->ProcessStorageItemAutoMoveSuccess();
        g_pStorageInventoryExt->ProcessStorageItemAutoMoveSuccess();
        return InsertItem(index, itemData);
    }

    if (IsMyShopSlot(index))
        return g_pMyShopInventory->InsertItem(index, itemData);

    return true;
}

void RejectTransfer()
{
    RestorePickedItem();
    if (g_pStorageInventory->IsItemAutoMove())
        g_pStorageInventory->ProcessStorageItemAutoMoveFailure();
    if (g_pStorageInventoryExt->IsItemAutoMove())
        g_pStorageInventoryExt->ProcessStorageItemAutoMoveFailure();
}

bool HasPickedItem()
{
    return mu::ui::window::CInventoryCtrl::GetPickedItem() != nullptr;
}

ITEM* FindPlayerItem(int index)
{
    if (!IsPlayerInventorySlot(index))
        return nullptr;

    if (IsMainInventorySlot(index))
        return g_pMyInventory != nullptr ? g_pMyInventory->FindItem(index) : nullptr;

    return g_pMyInventoryExt != nullptr ? g_pMyInventoryExt->FindItem(index) : nullptr;
}

ITEM* FindMainInventoryItem(int index)
{
    return g_pMyInventory->FindItem(index);
}

void DeleteMainInventoryItem(int index)
{
    g_pMyInventory->DeleteItem(index);
}

ITEM* StandbyItem()
{
    return g_pMyInventory->GetStandbyItem();
}

int StandbyItemIndex()
{
    return g_pMyInventory->GetStandbyItemIndex();
}
} // namespace UI::Inventory
