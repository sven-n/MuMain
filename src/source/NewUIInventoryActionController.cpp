// NewUIInventoryActionController.cpp: implementation of the CNewUIInventoryActionController class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIInventoryActionController.h"
#include "NewUIMyInventory.h"
#include "NewUIInventoryCtrl.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzInventory.h"
#include "UIJewelHarmony.h"
#include "CSItemOption.h"
#include "MapManager.h"
#include "SocketSystem.h"
#include "MonkSystem.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"
#include "ZzzInterface.h"
#include "UIManager.h"

namespace SEASON3B
{

CNewUIInventoryActionController::CNewUIInventoryActionController()
    : m_pOwner(nullptr)
{
}

void CNewUIInventoryActionController::SetOwner(CNewUIMyInventory* pOwner)
{
    m_pOwner = pOwner;
}

bool CNewUIInventoryActionController::HandleInventoryActions(CNewUIInventoryCtrl* targetControl) const
{
    if (m_pOwner == nullptr || targetControl == nullptr)
    {
        return false;
    }

    if (CNewUIInventoryCtrl::GetPickedItem() && IsRelease(VK_LBUTTON))
    {
        return HandlePickedItemPlacement(targetControl);
    }

    if (m_pOwner->GetRepairMode() == CNewUIMyInventory::REPAIR_MODE_OFF && IsPress(VK_RBUTTON))
    {
        return HandleRightClick(targetControl);
    }

    if (m_pOwner->GetRepairMode() == CNewUIMyInventory::REPAIR_MODE_ON && IsPress(VK_LBUTTON))
    {
        return HandleRepairClick(targetControl);
    }

    return false;
}

bool CNewUIInventoryActionController::HandlePickedItemPlacement(CNewUIInventoryCtrl* targetControl) const
{
    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        return false;
    }

    ITEM* pPickItem = pPickedItem->GetItem();
    if (pPickItem == nullptr)
    {
        return true;
    }

    const int iSourceIndex = pPickedItem->GetSourceLinealPos();
    const int iTargetIndex = pPickedItem->GetTargetLinealPos(targetControl);

    const bool bFromInventorySystem =
        (pPickedItem->GetOwnerInventory() == targetControl)
        || (g_pMyInventoryExt != nullptr && g_pMyInventoryExt->GetOwnerOf(pPickedItem) != nullptr);

    if (bFromInventorySystem)
    {
        if (TryApplyJewel(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex))
        {
            return true;
        }

        if (iTargetIndex != -1 && TryStackItem(targetControl, pPickItem, iSourceIndex, iTargetIndex))
        {
            return true;
        }
    }

    if (TryMoveItem(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex))
    {
        return true;
    }

    return false;
}

bool CNewUIInventoryActionController::TryApplyJewel(CNewUIInventoryCtrl* targetControl,
    CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    const bool bIsJewelType =
        pPickItem->Type == ITEM_JEWEL_OF_BLESS
        || pPickItem->Type == ITEM_JEWEL_OF_SOUL
        || pPickItem->Type == ITEM_JEWEL_OF_LIFE
        || pPickItem->Type == ITEM_JEWEL_OF_HARMONY
        || pPickItem->Type == ITEM_LOWER_REFINE_STONE
        || pPickItem->Type == ITEM_HIGHER_REFINE_STONE
        || pPickItem->Type == ITEM_POTION + 160
        || pPickItem->Type == ITEM_POTION + 161;

    if (!bIsJewelType)
    {
        return false;
    }

    return CNewUIMyInventory::ApplyJewels(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex);
}

bool CNewUIInventoryActionController::TryStackItem(CNewUIInventoryCtrl* targetControl,
    ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    return CNewUIMyInventory::TryStackItems(targetControl, pPickItem, iSourceIndex, iTargetIndex);
}

bool CNewUIInventoryActionController::TryMoveItem(CNewUIInventoryCtrl* targetControl,
    CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    if (iTargetIndex < 0 || !targetControl->CanMove(iTargetIndex, pPickItem))
    {
        return false;
    }

    const auto sourceStorageType = pPickedItem->GetSourceStorageType();
    const auto targetStorageType = targetControl->GetStorageType();

    if (iTargetIndex != iSourceIndex)
    {
        return SendRequestEquipmentItem(sourceStorageType, iSourceIndex,
            pPickItem, targetStorageType, iTargetIndex);
    }

    CNewUIInventoryCtrl::BackupPickedItem();
    return false;
}

bool CNewUIInventoryActionController::HandleRepairClick(CNewUIInventoryCtrl* targetControl) const
{
    return CNewUIMyInventory::RepairItemAtMousePoint(targetControl);
}

bool CNewUIInventoryActionController::HandleRightClick(CNewUIInventoryCtrl* targetControl) const
{
    m_pOwner->ResetMouseRButton();

    if (g_pNewUISystem->IsVisible(INTERFACE_STORAGE))
    {
        return HandleStorageAutoMove(targetControl);
    }

    if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) && g_pNewUISystem->IsVisible(INTERFACE_INVENTORY))
    {
        return HandleSellToNPC(targetControl);
    }

    if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY)
        && !g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP)
        && !g_pNewUISystem->IsVisible(INTERFACE_TRADE)
        && !g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE)
        && !g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE)
        && !g_pNewUISystem->IsVisible(INTERFACE_LUCKYITEMWND)
        && !g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY)
        && !g_pNewUISystem->IsVisible(INTERFACE_MYSHOP_INVENTORY))
    {
        return HandleInventoryRightClickActions(targetControl);
    }

    return false;
}

bool CNewUIInventoryActionController::HandleStorageAutoMove(CNewUIInventoryCtrl* targetControl) const
{
    if (g_pStorageInventory->ProcessMyInvenItemAutoMove(targetControl))
    {
        return true;
    }

    if (g_pNewUISystem->IsVisible(INTERFACE_STORAGE_EXT))
    {
        return g_pStorageInventoryExt->ProcessMyInvenItemAutoMove(targetControl);
    }

    return false;
}

bool CNewUIInventoryActionController::HandleSellToNPC(CNewUIInventoryCtrl* targetControl) const
{
    CNewUIPickedItem* pExistingPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pExistingPickedItem)
    {
        return false;
    }

    if (!targetControl->CheckPtInRect(MouseX, MouseY))
    {
        return false;
    }

    if (targetControl->GetStorageType() != STORAGE_TYPE::INVENTORY)
    {
        return false;
    }

    ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return false;
    }

    if (g_pNPCShop->IsSellingItem())
    {
        return false;
    }

    if (IsSellingBan(pItem))
    {
        g_pSystemLogBox->AddText(GlobalText[668], TYPE_ERROR_MESSAGE);
        return true;
    }

    const int iIndex = targetControl->GetIndexByItem(pItem);
    if (iIndex < 0)
    {
        return false;
    }

    if (!CNewUIInventoryCtrl::CreatePickedItem(targetControl, pItem))
    {
        return false;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        return false;
    }

    targetControl->RemoveItem(pItem);
    pPickedItem->HidePickedItem();

    const int sourceIndex = pPickedItem->GetSourceLinealPos();
    if (sourceIndex < MAX_EQUIPMENT_INDEX || sourceIndex >= MAX_MY_INVENTORY_EX_INDEX)
    {
        CNewUIInventoryCtrl::BackupPickedItem();
        return false;
    }

    if (IsHighValueItem(pItem))
    {
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CHighValueItemCheckMsgBoxLayout));
        return true;
    }

    SocketClient->ToGameServer()->SendSellItemToNpcRequest(sourceIndex);
    g_pNPCShop->SetSellingItem(true);
    return true;
}

bool CNewUIInventoryActionController::HandleInventoryRightClickActions(CNewUIInventoryCtrl* targetControl) const
{
    if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY_EXT))
    {
        return CNewUIMyInventory::TryTransferBetweenInventorySections(targetControl);
    }

    ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return false;
    }

    const int iIndex = targetControl->GetIndexByItem(pItem);

    if (iIndex >= 0 && CNewUIMyInventory::TryConsumeItem(targetControl, pItem, iIndex))
    {
        return true;
    }

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    if (g_pMyInventory->IsInvenItem(pItem->Type))
    {
#ifdef LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
        if (MouseLButton || MouseLButtonPop || MouseLButtonPush)
        {
            return false;
        }
#endif
        if (pItem->Durability == 0)
        {
            return false;
        }

        int iChangeInvenItemStatus = 0;
        (pItem->Durability == 255) ? iChangeInvenItemStatus = 254 : iChangeInvenItemStatus = 255;
        SendRequestEquippingInventoryItem(iIndex, iChangeInvenItemStatus);
        return false;
    }
#endif

    if (!EquipmentItem)
    {
        if (TryEquipItem(targetControl, pItem, iIndex))
        {
            return true;
        }
    }

    if (TryDropItem(targetControl, pItem))
    {
        return true;
    }

    return false;
}

int CNewUIInventoryActionController::FindAlternateEquipSlot(int nOriginalSlot, ITEM* pItem) const
{
    if (nOriginalSlot == EQUIPMENT_WEAPON_RIGHT)
    {
        const auto baseClass = gCharacterManager.GetBaseClass(Hero->Class);
        if (baseClass == CLASS_KNIGHT || baseClass == CLASS_DARK || baseClass == CLASS_RAGEFIGHTER)
        {
            const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
            if (!pItemAttr->TwoHand)
            {
                return EQUIPMENT_WEAPON_LEFT;
            }
        }
    }
    else if (nOriginalSlot == EQUIPMENT_RING_RIGHT)
    {
        return EQUIPMENT_RING_LEFT;
    }

    return -1;
}

bool CNewUIInventoryActionController::IsSlotOccupied(int nSlot) const
{
    if (nSlot < 0 || nSlot >= MAX_EQUIPMENT_INDEX)
    {
        return true;
    }

    const ITEM* pEquipment = &CharacterMachine->Equipment[nSlot];
    return (pEquipment->Type != -1);
}

bool CNewUIInventoryActionController::TryEquipItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iSrcIndex) const
{
    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    int nDstIndex = pItemAttr->m_byItemSlot;

    if (nDstIndex < 0 || nDstIndex >= MAX_EQUIPMENT_INDEX)
    {
        return false;
    }

    if (!m_pOwner->IsEquipable(nDstIndex, pItem))
    {
        return false;
    }

    if (IsSlotOccupied(nDstIndex))
    {
        const int nAltSlot = FindAlternateEquipSlot(nDstIndex, pItem);

        if (nAltSlot != -1 && !IsSlotOccupied(nAltSlot))
        {
            nDstIndex = nAltSlot;
        }
        else
        {
            return true;
        }
    }

    if (!m_pOwner->IsEquipable(nDstIndex, pItem))
    {
        return true;
    }

    if (!CNewUIInventoryCtrl::CreatePickedItem(nullptr, pItem))
    {
        return false;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        return false;
    }

    targetControl->RemoveItem(pItem);
    pPickedItem->HidePickedItem();

    SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSrcIndex, pItem, STORAGE_TYPE::INVENTORY, nDstIndex);
    return true;
}

bool CNewUIInventoryActionController::TryDropItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem) const
{
    if (Hero->Dead != 0)
    {
        return false;
    }

    if (IsHighValueItem(pItem))
    {
        g_pSystemLogBox->AddText(GlobalText[269], TYPE_ERROR_MESSAGE);
        return true;
    }

    if (IsDropBan(pItem))
    {
        g_pSystemLogBox->AddText(GlobalText[1915], TYPE_ERROR_MESSAGE);
        return true;
    }

    if (!CNewUIInventoryCtrl::CreatePickedItem(targetControl, pItem))
    {
        return false;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        return false;
    }

    targetControl->RemoveItem(pItem);
    pPickedItem->HidePickedItem();

    const int tx = Hero->PositionX;
    const int ty = Hero->PositionY;
    const int sourceIndex = pPickedItem->GetSourceLinealPos();

    SocketClient->ToGameServer()->SendDropItemRequest(tx, ty, sourceIndex);
    SendDropItem = sourceIndex;

    return true;
}

} // namespace SEASON3B