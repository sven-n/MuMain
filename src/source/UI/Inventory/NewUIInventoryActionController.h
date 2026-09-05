// NewUIInventoryActionController.h
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)
#define AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_

#pragma once

#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "GameLogic/Items/IInventoryActionContext.h"

namespace mu::ui::window
{

class CInventoryActionController
{
public:
    CInventoryActionController();
    ~CInventoryActionController() = default;

    void SetContext(SEASON3B::IInventoryActionContext* pContext);
    bool HandleInventoryActions(CInventoryCtrl* targetControl) const;

private:
    bool HandlePickedItemPlacement(CInventoryCtrl* targetControl) const;
    bool TryApplyJewel(CInventoryCtrl* targetControl, CPickedItem* pPickedItem, ITEM* pPickItem,
                       int iSourceIndex, int iTargetIndex) const;
    bool TryStackItem(CInventoryCtrl* targetControl, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;
    bool TryMoveItem(CInventoryCtrl* targetControl, CPickedItem* pPickedItem, ITEM* pPickItem,
                     int iSourceIndex, int iTargetIndex) const;

    bool HandleRepairClick(CInventoryCtrl* targetControl) const;

    bool HandleRightClick(CInventoryCtrl* targetControl) const;
    bool HandleStorageAutoMove(CInventoryCtrl* targetControl) const;
    bool HandleMixAutoMove(CInventoryCtrl* targetControl) const;
    bool HandleSellToNPC(CInventoryCtrl* targetControl) const;
    bool HandleInventoryRightClickActions(CInventoryCtrl* targetControl) const;
    bool TryEquipItem(CInventoryCtrl* targetControl, ITEM* pItem, int iSrcIndex) const;
    bool TryDropItem(CInventoryCtrl* targetControl, ITEM* pItem) const;

    int FindAlternateEquipSlot(int nOriginalSlot, ITEM* pItem) const;
    bool IsSlotOccupied(int nSlot) const;

    bool ApplyJewels(CInventoryCtrl* targetControl, CPickedItem* pPickedItem, ITEM* pPickItem,
                     int iSourceIndex, int iTargetIndex) const;
    bool TryStackItems(CInventoryCtrl* targetControl, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;
    bool RepairItemAtMousePoint(CInventoryCtrl* targetControl) const;
    bool TryConsumeItem(CInventoryCtrl* targetControl, ITEM* pItem, int iIndex) const;
    bool TryTransferBetweenInventorySections(CInventoryCtrl* sourceControl) const;

    SEASON3B::IInventoryActionContext* m_pContext;
};

} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)