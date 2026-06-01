// NewUIInventoryActionController.cpp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NewUI/Inventory/NewUIInventoryActionController.h"
#include "UI/NewUI/Inventory/NewUIMyInventory.h"
#include "UI/NewUI/Inventory/NewUIInventoryCtrl.h"
#include "UI/NewUI/NewUISystem.h"
#include "UI/NewUI/Dialogs/NewUICustomMessageBox.h"
#include "Engine/Object/ZzzInventory.h"
#include "UI/Legacy/UIJewelHarmony.h"
#include "GameLogic/Items/CSItemOption.h"
#include "World/MapInfra/MapManager.h"
#include "Network/Server/SocketSystem.h"
#include "GameLogic/Social/MonkSystem.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInterface.h"
#include "UI/Legacy/UIManager.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "World/MapInfra/PortalMgr.h"
#include "GameLogic/Quests/CSQuest.h"
#include "I18N/All.h"

namespace SEASON3B
{

namespace
{
    // A two-handed weapon can require BOTH hands freed (its slot + the off-hand shield).
    constexpr int MAX_EQUIP_BLOCKERS = 2;

    // Right-click "always equip": the server only moves an item onto an EMPTY equipment slot, one
    // move at a time, and acks asynchronously. So when the target slot — or a conflicting off-hand
    // (e.g. equipping a two-handed weapon over a shield) — is occupied, we unequip each blocking
    // item to inventory first and only equip the new item once every required slot is free. This
    // record carries that multi-step sequence across ReceiveEquipmentItemExtended.
    struct PendingEquip
    {
        bool  active       = false;
        CNewUIInventoryCtrl* pNewItemInvenCtrl = nullptr;     // control holding the item to equip (main or extended)
        int   dstEquipSlot = -1;                              // where the new item will be equipped
        DWORD newItemKey   = 0;                               // identifies the new inventory item
        int   freeSlots[MAX_EQUIP_BLOCKERS] = { -1, -1 };     // equipment slots still to unequip
        int   freeDest[MAX_EQUIP_BLOCKERS]  = { -1, -1 };     // reserved inventory slot for each
        int   freeCount    = 0;
    };

    PendingEquip s_pendingEquip{};

    void ClearPendingEquip()
    {
        s_pendingEquip.active            = false;
        s_pendingEquip.pNewItemInvenCtrl = nullptr;
        s_pendingEquip.freeCount         = 0;
    }

    // Pick an inventory item and request the server to move it to an equipment slot.
    bool EquipFromInventory(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iSrcIndex, int iDstSlot)
    {
        if (targetControl == nullptr || pItem == nullptr)
        {
            return false;
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

        SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSrcIndex, pItem, STORAGE_TYPE::INVENTORY, iDstSlot);
        return true;
    }

    // Pick the item in an equipment slot and request the server to move it to a pre-reserved
    // inventory slot. The destination was reserved up front so the whole multi-item swap is
    // all-or-nothing (see SwapEquipItem); -1 falls back to a fresh search as a safety net.
    bool UnequipToInventory(int nEquipSlot, int nReservedDest)
    {
        if (g_pMyInventory == nullptr || nEquipSlot < 0 || nEquipSlot >= MAX_EQUIPMENT_INDEX)
        {
            return false;
        }

        ITEM* pEquipped = &CharacterMachine->Equipment[nEquipSlot];
        const int iTempSlot = (nReservedDest != -1) ? nReservedDest : g_pMyInventory->FindEmptySlot(pEquipped);
        if (iTempSlot == -1)
        {
            return false;
        }

        if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquipped))
        {
            CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
            g_pMyInventory->UnequipItem(nEquipSlot);
            if (pPickedItem != nullptr)
            {
                pPickedItem->HidePickedItem();
            }
        }

        SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, nEquipSlot, pEquipped, STORAGE_TYPE::INVENTORY, iTempSlot);
        return true;
    }

    bool EquippedSlotItemSize(int nSlot, int& outWidth, int& outHeight)
    {
        if (nSlot < 0 || nSlot >= MAX_EQUIPMENT_INDEX)
        {
            return false;
        }

        const ITEM* pItem = &CharacterMachine->Equipment[nSlot];
        if (pItem->Type == -1)
        {
            return false;
        }

        const ITEM_ATTRIBUTE* pAttr = &ItemAttribute[pItem->Type];
        outWidth  = pAttr->Width;
        outHeight = pAttr->Height;
        return true;
    }

    // Reserve a distinct inventory slot for every blocking equipment item, all at once. Returns
    // false (reserving nothing) unless ALL of them fit, so the caller can abort atomically and
    // leave the character untouched (mirrors WoW: a 2H swap fails cleanly when the bag is full).
    bool ReserveSlotsForBlockers(CNewUIInventoryCtrl* pInvenCtrl, const int* blockers, int nBlockers, int* outDest)
    {
        if (pInvenCtrl == nullptr || nBlockers <= 0)
        {
            return false;
        }

        int wA, hA;
        if (!EquippedSlotItemSize(blockers[0], wA, hA))
        {
            return false;
        }

        int wB, hB;
        const bool bSecond = (nBlockers >= 2) && EquippedSlotItemSize(blockers[1], wB, hB);

        // Only the first slot holds a movable item: a single free slot is enough.
        if (!bSecond)
        {
            const int slot = pInvenCtrl->FindEmptySlot(wA, hA);
            if (slot == -1)
            {
                return false;
            }
            outDest[0] = slot;
            outDest[1] = -1;
            return true;
        }

        // Both items must fit at once in non-overlapping space.
        return pInvenCtrl->FindTwoEmptySlots(wA, hA, wB, hB, outDest[0], outDest[1]);
    }
}

void CancelPendingEquipSwap()
{
    ClearPendingEquip();
}

void ProcessPendingEquipAfterMove()
{
    if (!s_pendingEquip.active)
    {
        return;
    }

    // Unequip the next still-occupied blocking slot, one per move ack, before the final equip.
    while (s_pendingEquip.freeCount > 0)
    {
        const int nIdx  = --s_pendingEquip.freeCount;
        const int nSlot = s_pendingEquip.freeSlots[nIdx];
        const int nDest = s_pendingEquip.freeDest[nIdx];
        if (nSlot < 0 || nSlot >= MAX_EQUIPMENT_INDEX)
        {
            continue;
        }

        if (CharacterMachine->Equipment[nSlot].Type == -1)
        {
            continue;   // already empty -> nothing to move, skip to the next blocker
        }

        if (!UnequipToInventory(nSlot, nDest))
        {
            ClearPendingEquip();   // no inventory space: stop safely, the new item is untouched
            return;
        }

        return;   // wait for this unequip's ack before handling the next step
    }

    // Every blocking slot is now free: equip the new item from the control it lives in.
    const int   iDstSlot  = s_pendingEquip.dstEquipSlot;
    const DWORD dwItemKey = s_pendingEquip.newItemKey;
    CNewUIInventoryCtrl* pInvenCtrl = s_pendingEquip.pNewItemInvenCtrl;
    ClearPendingEquip();

    if (pInvenCtrl == nullptr)
    {
        return;
    }

    ITEM* pNewItem = pInvenCtrl->FindItemByKey(dwItemKey);
    if (pNewItem == nullptr)
    {
        return;   // item moved/changed since we queued the swap; abort safely
    }

    // Final guards: never pick the item unless the destination is genuinely equippable now.
    if (iDstSlot < 0 || iDstSlot >= MAX_EQUIPMENT_INDEX || CharacterMachine->Equipment[iDstSlot].Type != -1)
    {
        return;
    }

    const ITEM_ATTRIBUTE* pNewAttr = &ItemAttribute[pNewItem->Type];
    if (pNewAttr->TwoHand && iDstSlot == EQUIPMENT_WEAPON_RIGHT
        && CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != -1)
    {
        return;   // two-handed weapon still blocked by the off-hand; do not lose the item
    }

    const int iSrcIndex = pInvenCtrl->GetIndexByItem(pNewItem);
    if (iSrcIndex < 0)
    {
        return;
    }

    EquipFromInventory(pInvenCtrl, pNewItem, iSrcIndex, iDstSlot);
}

CNewUIInventoryActionController::CNewUIInventoryActionController()
    : m_pContext(nullptr)
{
}

void CNewUIInventoryActionController::SetContext(IInventoryActionContext* pContext)
{
    m_pContext = pContext;
}

bool CNewUIInventoryActionController::HandleInventoryActions(CNewUIInventoryCtrl* targetControl) const
{
    if (m_pContext == nullptr || targetControl == nullptr)
    {
        return false;
    }

    if (CNewUIInventoryCtrl::GetPickedItem() && IsRelease(VK_LBUTTON))
    {
        return HandlePickedItemPlacement(targetControl);
    }

    if (m_pContext->GetRepairMode() == REPAIR_MODE_OFF && IsPress(VK_RBUTTON))
    {
        return HandleRightClick(targetControl);
    }

    if (m_pContext->GetRepairMode() == REPAIR_MODE_ON && IsPress(VK_LBUTTON))
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
        if (ApplyJewels(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex))
        {
            return true;
        }

        if (iTargetIndex != -1 && TryStackItems(targetControl, pPickItem, iSourceIndex, iTargetIndex))
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

bool CNewUIInventoryActionController::TryApplyJewel(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    return ApplyJewels(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex);
}

bool CNewUIInventoryActionController::TryStackItem(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    return TryStackItems(targetControl, pPickItem, iSourceIndex, iTargetIndex);
}

bool CNewUIInventoryActionController::TryMoveItem(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
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
    return RepairItemAtMousePoint(targetControl);
}

bool CNewUIInventoryActionController::HandleRightClick(CNewUIInventoryCtrl* targetControl) const
{
    m_pContext->ResetMouseRButton();

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
    if (CNewUIInventoryCtrl::GetPickedItem())
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
        g_pSystemLogBox->AddText(I18N::Game::TheseItemsCannotBeTraded, TYPE_ERROR_MESSAGE);
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
        return TryTransferBetweenInventorySections(targetControl);
    }

    ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return false;
    }

    const int iIndex = targetControl->GetIndexByItem(pItem);

    if (iIndex >= 0 && TryConsumeItem(targetControl, pItem, iIndex))
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
        return true;
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

    if (!m_pContext->IsEquipable(nDstIndex, pItem))
    {
        return true;
    }

    // Prefer an empty alternate hand/ring (e.g. equip a 1H weapon to the free off-hand).
    if (IsSlotOccupied(nDstIndex))
    {
        const int nAltSlot = FindAlternateEquipSlot(nDstIndex, pItem);

        if (nAltSlot != -1 && !IsSlotOccupied(nAltSlot))
        {
            nDstIndex = nAltSlot;
        }
    }

    if (!m_pContext->IsEquipable(nDstIndex, pItem))
    {
        return true;
    }

    // Find every equipment slot that must be emptied before this equip can succeed: the target
    // slot itself plus any two-handed-weapon hand conflict (which IsEquipable does not check).
    int blockers[MAX_EQUIP_BLOCKERS];
    const int nBlockers = CollectEquipBlockers(pItem, nDstIndex, blockers);

    if (nBlockers == 0)
    {
        return EquipFromInventory(targetControl, pItem, iSrcIndex, nDstIndex);
    }

    return SwapEquipItem(targetControl, pItem, nDstIndex, blockers, nBlockers);
}

int CNewUIInventoryActionController::CollectEquipBlockers(ITEM* pItem, int nDstIndex, int* outSlots) const
{
    int nCount = 0;
    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];

    if (IsSlotOccupied(nDstIndex))
    {
        outSlots[nCount++] = nDstIndex;
    }

    // A two-handed weapon needs the off-hand empty too.
    if (pItemAttr->TwoHand && nDstIndex == EQUIPMENT_WEAPON_RIGHT && IsSlotOccupied(EQUIPMENT_WEAPON_LEFT))
    {
        outSlots[nCount++] = EQUIPMENT_WEAPON_LEFT;
    }

    // Equipping into the off-hand while the main hand holds a two-handed weapon: free that weapon.
    if (nDstIndex == EQUIPMENT_WEAPON_LEFT)
    {
        const ITEM* pRight = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
        if (pRight->Type != -1 && ItemAttribute[pRight->Type].TwoHand && nCount < MAX_EQUIP_BLOCKERS)
        {
            outSlots[nCount++] = EQUIPMENT_WEAPON_RIGHT;
        }
    }

    return nCount;
}

bool CNewUIInventoryActionController::SwapEquipItem(CNewUIInventoryCtrl* pNewItemInvenCtrl, ITEM* pItem, int nDstSlot, const int* blockers, int nBlockers) const
{
    // Displaced equipment always returns to the main inventory, so reserve space there.
    CNewUIInventoryCtrl* pInvenCtrl = (g_pMyInventory != nullptr) ? g_pMyInventory->GetInventoryCtrl() : nullptr;
    if (pInvenCtrl == nullptr || pNewItemInvenCtrl == nullptr || nBlockers <= 0)
    {
        return true;
    }

    // Reserve an inventory slot for EVERY item we are about to displace, up front. If they do not
    // all fit at once, abort and unequip nothing — the swap is all-or-nothing, like WoW.
    int dest[MAX_EQUIP_BLOCKERS] = { -1, -1 };
    if (!ReserveSlotsForBlockers(pInvenCtrl, blockers, nBlockers, dest))
    {
        g_pSystemLogBox->AddText(I18N::Game::InventorySpaceIsInsufficient, TYPE_ERROR_MESSAGE);
        return true;
    }

    // Queue the blocking slots to unequip into their reserved destinations, then drive the
    // sequence. The new item is only equipped once every blocker has been cleared (see
    // ProcessPendingEquipAfterMove), so it is never lost even on an intermediate server rejection.
    s_pendingEquip.active            = true;
    s_pendingEquip.pNewItemInvenCtrl = pNewItemInvenCtrl;
    s_pendingEquip.dstEquipSlot      = nDstSlot;
    s_pendingEquip.newItemKey        = pItem->Key;
    s_pendingEquip.freeCount         = 0;

    for (int i = 0; i < nBlockers && i < MAX_EQUIP_BLOCKERS; ++i)
    {
        s_pendingEquip.freeSlots[s_pendingEquip.freeCount] = blockers[i];
        s_pendingEquip.freeDest[s_pendingEquip.freeCount]  = dest[i];
        s_pendingEquip.freeCount++;
    }

    ProcessPendingEquipAfterMove();   // performs the first unequip
    return true;
}

bool CNewUIInventoryActionController::EquipToSlotReplacing(CNewUIInventoryCtrl* pInvenCtrl, DWORD dwItemKey, int nDstSlot) const
{
    if (m_pContext == nullptr || pInvenCtrl == nullptr
        || nDstSlot < 0 || nDstSlot >= MAX_EQUIPMENT_INDEX)
    {
        return false;
    }

    ITEM* pItem = pInvenCtrl->FindItemByKey(dwItemKey);
    if (pItem == nullptr)
    {
        return false;   // not in this inventory; leave it where it was restored
    }

    if (!m_pContext->IsEquipable(nDstSlot, pItem))
    {
        return true;
    }

    const int iSrcIndex = pInvenCtrl->GetIndexByItem(pItem);
    if (iSrcIndex < 0)
    {
        return false;
    }

    // Same unequip-then-equip path as right-click, but into the exact slot the item was dropped on.
    int blockers[MAX_EQUIP_BLOCKERS];
    const int nBlockers = CollectEquipBlockers(pItem, nDstSlot, blockers);

    if (nBlockers == 0)
    {
        return EquipFromInventory(pInvenCtrl, pItem, iSrcIndex, nDstSlot);
    }

    return SwapEquipItem(pInvenCtrl, pItem, nDstSlot, blockers, nBlockers);
}

bool CNewUIInventoryActionController::TryDropItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem) const
{
    if (Hero->Dead != 0)
    {
        return false;
    }

    if (IsHighValueItem(pItem))
    {
        g_pSystemLogBox->AddText(I18N::Game::YouAreNotAllowedToDropThisExpensiveItem, TYPE_ERROR_MESSAGE);
        return true;
    }

    if (IsDropBan(pItem))
    {
        g_pSystemLogBox->AddText(I18N::Game::ThisItemCannotBeDropped, TYPE_ERROR_MESSAGE);
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

bool CNewUIInventoryActionController::RepairItemAtMousePoint(CNewUIInventoryCtrl* targetControl) const
{
    ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return true;
    }

    if (IsRepairBan(pItem))
    {
        return true;
    }

    const int iIndex = targetControl->GetIndex(pItem->x, pItem->y);

    if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
    {
        SocketClient->ToGameServer()->SendRepairItemRequest(iIndex, 0);
    }
    else
    {
        SocketClient->ToGameServer()->SendRepairItemRequest(iIndex, 1);
    }

    return true;
}

bool CNewUIInventoryActionController::ApplyJewels(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
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

    ITEM* pItem = targetControl->FindItem(iTargetIndex);
    if (!pItem)
    {
        return false;
    }

    const int iType       = pItem->Type;
    const int iLevel      = pItem->Level;
    const int iDurability = pItem->Durability;

    bool bSuccess = true;

    if (iType > ITEM_WINGS_OF_DARKNESS
        && iType != ITEM_CAPE_OF_LORD
        && !(iType >= ITEM_WING_OF_STORM && iType <= ITEM_WING_OF_DIMENSION)
        && !(ITEM_WING + 130 <= iType && iType <= ITEM_WING + 134)
        && !(iType >= ITEM_CAPE_OF_FIGHTER && iType <= ITEM_CAPE_OF_OVERRULE)
        && (iType != ITEM_WING + 135))
    {
        bSuccess = false;
    }

    if (iType == ITEM_BOLT || iType == ITEM_ARROWS)
    {
        bSuccess = false;
    }

    if ((pPickItem->Type == ITEM_JEWEL_OF_BLESS && iLevel >= 6)
        || (pPickItem->Type == ITEM_JEWEL_OF_SOUL && iLevel >= 9))
    {
        bSuccess = false;
    }

    if (pPickItem->Type == ITEM_JEWEL_OF_BLESS
        && iType == ITEM_HORN_OF_FENRIR
        && iDurability != 255)
    {
        CFenrirRepairMsgBox* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CFenrirRepairMsgBoxLayout), &pMsgBox);
        pMsgBox->SetSourceIndex(iSourceIndex);

        const int iIndex = targetControl->GetIndex(pItem->x, pItem->y);
        pMsgBox->SetTargetIndex(iIndex);

        pPickedItem->HidePickedItem();
        return true;
    }

    if (pPickItem->Type == ITEM_JEWEL_OF_HARMONY)
    {
        if (g_SocketItemMgr.IsSocketItem(pItem))
        {
            bSuccess = false;
        }
        else if (pItem->Jewel_Of_Harmony_Option != 0)
        {
            bSuccess = false;
        }
        else
        {
            const StrengthenItem strengthitem =
                g_pUIJewelHarmonyinfo->GetItemType(static_cast<int>(pItem->Type));

            if (strengthitem == SI_None)
            {
                bSuccess = false;
            }
        }
    }

    if (pPickItem->Type == ITEM_LOWER_REFINE_STONE
        || pPickItem->Type == ITEM_HIGHER_REFINE_STONE)
    {
        if (g_SocketItemMgr.IsSocketItem(pItem))
        {
            bSuccess = false;
        }
        else if (pItem->Jewel_Of_Harmony_Option == 0)
        {
            bSuccess = false;
        }
    }

    if (Check_LuckyItem(pItem->Type))
    {
        bSuccess = false;
        if (pPickItem->Type == ITEM_POTION + 161)
        {
            if (pItem->Jewel_Of_Harmony_Option == 0) bSuccess = true;
        }
        else if (pPickItem->Type == ITEM_POTION + 160)
        {
            if (pItem->Durability > 0)               bSuccess = true;
        }
    }

    if (bSuccess)
    {
        const int targetIndex = targetControl->GetIndexByItem(pItem);
        SendRequestUse(iSourceIndex, targetIndex);
        PlayBuffer(SOUND_GET_ITEM01);
        return true;
    }

    return false;
}

bool CNewUIInventoryActionController::TryStackItems(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    if (ITEM* pItem = targetControl->FindItem(iTargetIndex))
    {
        if (targetControl->AreItemsStackable(pPickItem, pItem))
        {
            SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex,
                pPickItem, STORAGE_TYPE::INVENTORY, iTargetIndex);
            return true;
        }
    }

    return false;
}

bool CNewUIInventoryActionController::TryConsumeItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iIndex) const
{
    if (pItem == nullptr)
    {
        return false;
    }

    if (pItem->Type == ITEM_TOWN_PORTAL_SCROLL)
    {
        SendRequestUse(iIndex, 0);
        return true;
    }

    const auto isApple  = pItem->Type == ITEM_APPLE;
    const auto isPotion =
        (pItem->Type >= ITEM_APPLE && pItem->Type <= ITEM_ALE)
        || (pItem->Type >= ITEM_SMALL_SHIELD_POTION && pItem->Type <= ITEM_LARGE_COMPLEX_POTION);

    if (isApple || isPotion
        || (pItem->Type == ITEM_POTION + 20 && pItem->Level == 0)
        || (pItem->Type >= ITEM_JACK_OLANTERN_BLESSINGS && pItem->Type <= ITEM_JACK_OLANTERN_DRINK)
        || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 14)
        || (pItem->Type >= ITEM_POTION + 70 && pItem->Type <= ITEM_POTION + 71)
        || (pItem->Type >= ITEM_POTION + 72 && pItem->Type <= ITEM_POTION + 77)
        || pItem->Type == ITEM_HELPER + 60
        || pItem->Type == ITEM_POTION + 94
        || (pItem->Type >= ITEM_CHERRY_BLOSSOM_WINE && pItem->Type <= ITEM_CHERRY_BLOSSOM_FLOWER_PETAL)
        || (pItem->Type >= ITEM_POTION + 97 && pItem->Type <= ITEM_POTION + 98)
        || pItem->Type == ITEM_HELPER + 81
        || pItem->Type == ITEM_HELPER + 82
        || pItem->Type == ITEM_POTION + 133)
    {
        SendRequestUse(iIndex, 0);
        if (isApple)
        {
            PlayBuffer(SOUND_EAT_APPLE01);
        }
        else if (isPotion)
        {
            PlayBuffer(SOUND_DRINK01);
        }

        return true;
    }

    if (pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82)
    {
        std::list<eBuffState> secretPotionbufflist;
        secretPotionbufflist.push_back(eBuff_SecretPotion1);
        secretPotionbufflist.push_back(eBuff_SecretPotion2);
        secretPotionbufflist.push_back(eBuff_SecretPotion3);
        secretPotionbufflist.push_back(eBuff_SecretPotion4);
        secretPotionbufflist.push_back(eBuff_SecretPotion5);

        if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) == eBuffNone)
        {
            SendRequestUse(iIndex, 0);
            return true;
        }

        CreateOkMessageBox(I18N::Game::YouCannotUseThisItemWhileThePotionEffectsRemainActive, RGBA(255, 30, 0, 255));
        return false;
    }

    if ((pItem->Type >= ITEM_HELPER + 54 && pItem->Type <= ITEM_HELPER + 57)
        || (pItem->Type == ITEM_HELPER + 58
            && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD))
    {
        WORD point[5] = { 0, };
        point[0] = CharacterAttribute->Strength  + CharacterAttribute->AddStrength;
        point[1] = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        point[2] = CharacterAttribute->Vitality  + CharacterAttribute->AddVitality;
        point[3] = CharacterAttribute->Energy    + CharacterAttribute->AddEnergy;
        point[4] = CharacterAttribute->Charisma  + CharacterAttribute->AddCharisma;

        const unsigned char nStat[MAX_CLASS][5] =
        {
            18, 18, 15, 30,  0,
            28, 20, 25, 10,  0,
            22, 25, 20, 15,  0,
            26, 26, 26, 26,  0,
            26, 20, 20, 15, 25,
            21, 21, 18, 23,  0,
            32, 27, 25, 20,  0,
        };

        const int attributeType   = pItem->Type - (ITEM_HELPER + 54);
        const int characterClass  = gCharacterManager.GetBaseClass(Hero->Class);
        point[attributeType]     -= nStat[characterClass][attributeType];

        if (point[attributeType] < (pItem->Durability * 10))
        {
            g_pMyInventory->SetStandbyItemKey(pItem->Key);
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePartChargeFruitMsgBoxLayout));
            return false;
        }

        SendRequestUse(iIndex, 0);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 58
        && gCharacterManager.GetBaseClass(Hero->Class) != CLASS_DARK_LORD)
    {
        CreateOkMessageBox(I18N::Game::OnlyDarklordCanUseIt);
        return true;
    }

    if (pItem->Type == ITEM_ARMOR_OF_GUARDSMAN)
    {
        if (IsUnitedMarketPlace())
        {
            wchar_t szOutputText[512];
            mu_swprintf(szOutputText, L"%ls %ls", I18N::Game::YouCannotEnterChaosCastle, I18N::Game::FromTheMarketInLorencia);
            CreateOkMessageBox(szOutputText);
            return true;
        }

        if (Hero->SafeZone == false)
        {
            CreateOkMessageBox(I18N::Game::YouCanOnlyUseThisInASafeZone);
            return false;
        }

        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::ChaosCastle, pItem->Level);
        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 46)
    {
        const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_DEVILSQUARE);
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::DevilSquare, byPossibleLevel);
        return false;
    }

    if (pItem->Type == ITEM_HELPER + 47)
    {
        const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_BLOODCASTLE);
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::BloodCastle, byPossibleLevel);
        return false;
    }

    if (pItem->Type == ITEM_HELPER + 48)
    {
        if (Hero->SafeZone || gMapManager.InHellas())
        {
            g_pSystemLogBox->AddText(I18N::Game::CanTBeUsedInTheSafeZone, TYPE_ERROR_MESSAGE);
            return false;
        }

        SendRequestUse(iIndex, 0);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 61)
    {
        const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_CURSEDTEMPLE);
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::CursedTemple, byPossibleLevel);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 121)
    {
        if (Hero->SafeZone == false)
        {
            CreateOkMessageBox(I18N::Game::YouCanOnlyUseThisInASafeZone);
            return false;
        }

        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::ChaosCastle, pItem->Level);
        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        return true;
    }

    if (pItem->Type == ITEM_SCROLL_OF_BLOOD)
    {
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::CursedTemple, pItem->Level);
        return true;
    }

    if (pItem->Type == ITEM_DEVILS_INVITATION)
    {
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::DevilSquare, pItem->Level);
        return true;
    }

    if (pItem->Type == ITEM_INVISIBILITY_CLOAK)
    {
        if (pItem->Level == 0)
        {
            g_pSystemLogBox->AddText(I18N::Game::IncorrectItem, TYPE_ERROR_MESSAGE);
        }
        else
        {
            SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::BloodCastle, pItem->Level - 1);
        }

        return true;
    }

    if ((pItem->Type >= ITEM_SCROLL_OF_POISON && pItem->Type < ITEM_ETC + MAX_ITEM_INDEX)
        || (pItem->Type >= ITEM_ORB_OF_TWISTING_SLASH && pItem->Type <= ITEM_ORB_OF_GREATER_FORTITUDE)
        || (pItem->Type >= ITEM_ORB_OF_FIRE_SLASH && pItem->Type <= ITEM_ORB_OF_DEATH_STAB)
        || (pItem->Type == ITEM_WING + 20)
        || (pItem->Type >= ITEM_SCROLL_OF_FIREBURST && pItem->Type <= ITEM_SCROLL_OF_ELECTRIC_SPARK)
        || (pItem->Type == ITEM_SCROLL_OF_FIRE_SCREAM)
        || (pItem->Type == ITEM_CRYSTAL_OF_DESTRUCTION)
        || (pItem->Type == ITEM_CRYSTAL_OF_FLAME_STRIKE)
        || (pItem->Type == ITEM_CRYSTAL_OF_RECOVERY)
        || (pItem->Type == ITEM_CRYSTAL_OF_MULTI_SHOT)
        || (pItem->Type == ITEM_SCROLL_OF_CHAOTIC_DISEIER)
        || (pItem->Type == ITEM_SCROLL_OF_GIGANTIC_STORM)
        || (pItem->Type == ITEM_SCROLL_OF_WIZARDRY_ENHANCE))
    {
        bool bReadBookGem = true;

        if (pItem->Type == ITEM_SCROLL_OF_NOVA
            || pItem->Type == ITEM_SCROLL_OF_WIZARDRY_ENHANCE
            || pItem->Type == ITEM_CRYSTAL_OF_MULTI_SHOT
            || pItem->Type == ITEM_CRYSTAL_OF_RECOVERY
            || pItem->Type == ITEM_CRYSTAL_OF_DESTRUCTION)
        {
            if (g_csQuest.getQuestState2(QUEST_CHANGE_UP_3) != QUEST_END)
            {
                bReadBookGem = false;
            }
        }

        if (pItem->Type == ITEM_SCROLL_OF_CHAOTIC_DISEIER)
        {
            if (CharacterAttribute->Level < 220)
            {
                bReadBookGem = false;
            }
        }

        if (bReadBookGem)
        {
            const WORD wStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
            const WORD wEnergy   = CharacterAttribute->Energy   + CharacterAttribute->AddEnergy;

            if (CharacterAttribute->Level >= ItemAttribute[pItem->Type].RequireLevel
                && wEnergy   >= pItem->RequireEnergy
                && wStrength >= pItem->RequireStrength)
            {
                SendRequestUse(iIndex, 0);
            }

            return true;
        }

        return false;
    }

    if (pItem->Type == ITEM_FRUITS)
    {
        if (CharacterAttribute->Level < 10)
        {
            CreateOkMessageBox(I18N::Game::MustBeOverLevel10ToUseFruits);
            return true;
        }

        bool bEquipmentEmpty = true;
        for (int i = 0; i < MAX_EQUIPMENT; i++)
        {
            if (CharacterMachine->Equipment[i].Type != -1)
            {
                bEquipmentEmpty = false;
                break;
            }
        }

        if (!bEquipmentEmpty)
        {
            CreateOkMessageBox(I18N::Game::ToDecreaseTheFruitWeaponsArmorsAndOthersMustBeRemoved);
            return true;
        }

        if (pItem->Level == 4) // Command Fruit
        {
            if (gCharacterManager.GetBaseClass(CharacterAttribute->Class) != CLASS_DARK_LORD)
            {
                CreateOkMessageBox(I18N::Game::OnlyDarklordCanUseIt);
                return true;
            }
        }

        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseFruitMsgBoxLayout));
        return true;
    }

    if (pItem->Type == ITEM_LIFE_STONE_ITEM)
    {
        bool bUse = false;
        switch (pItem->Level)
        {
        case 0: bUse = true; break;
        case 1: if (Hero->GuildStatus != G_MASTER) bUse = true; break;
        }

        if (bUse)
        {
            SendRequestUse(iIndex, 0);
            return true;
        }

        return false;
    }

    if (pItem->Type == ITEM_HELPER + 69)
    {
        if (g_PortalMgr.IsRevivePositionSaved())
        {
            if (g_PortalMgr.IsPortalUsable())
            {
                g_pMyInventory->SetStandbyItemKey(pItem->Key);
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseReviveCharmMsgBoxLayout));
            }
            else
            {
                CreateOkMessageBox(I18N::Game::YouCannotUseTheItemAtCertainApplicableLocations);
            }
        }

        return false;
    }

    if (pItem->Type == ITEM_HELPER + 70)
    {
        if (g_PortalMgr.IsPortalUsable())
        {
            if (pItem->Durability == 2)
            {
                if (g_PortalMgr.IsPortalPositionSaved())
                {
                    CreateOkMessageBox(I18N::Game::ThisItemCannotBeUsedAlongWithAnItemThatSAlreadyInUse);
                }
                else
                {
                    g_pMyInventory->SetStandbyItemKey(pItem->Key);
                    CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePortalCharmMsgBoxLayout));
                }
            }
            else if (pItem->Durability == 1)
            {
                g_pMyInventory->SetStandbyItemKey(pItem->Key);
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CReturnPortalCharmMsgBoxLayout));
            }
        }
        else
        {
            CreateOkMessageBox(I18N::Game::YouCannotUseTheItemAtCertainApplicableLocations);
        }

        return false;
    }

    if (pItem->Type == ITEM_HELPER + 66)
    {
        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseSantaInvitationMsgBoxLayout));
    }

    return false;
}

bool CNewUIInventoryActionController::TryTransferBetweenInventorySections(CNewUIInventoryCtrl* sourceControl) const
{
    if (sourceControl == nullptr || g_pMyInventoryExt == nullptr)
    {
        return false;
    }

    ITEM* pItem = sourceControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return false;
    }

    const int sourceIndex = sourceControl->GetIndexByItem(pItem);
    if (sourceIndex < 0)
    {
        return false;
    }

    const ITEM_ATTRIBUTE* itemAttribute = &ItemAttribute[pItem->Type];
    int destinationIndex = -1;

    if (sourceControl == g_pMyInventory->GetInventoryCtrl())
    {
        destinationIndex = g_pMyInventoryExt->FindEmptySlot(
            itemAttribute->Width, itemAttribute->Height);
    }
    else
    {
        destinationIndex = m_pContext->FindEmptySlot(pItem);
    }

    if (destinationIndex == -1 || destinationIndex == sourceIndex)
    {
        return true;
    }

    if (!CNewUIInventoryCtrl::CreatePickedItem(sourceControl, pItem))
    {
        return false;
    }

    sourceControl->RemoveItem(pItem);

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr || pPickedItem->GetItem() == nullptr)
    {
        CNewUIInventoryCtrl::BackupPickedItem();
        return false;
    }

    pPickedItem->HidePickedItem();

    if (!SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, sourceIndex,
        pPickedItem->GetItem(), STORAGE_TYPE::INVENTORY, destinationIndex))
    {
        CNewUIInventoryCtrl::BackupPickedItem();
        return false;
    }

    return true;
}

} // namespace SEASON3B