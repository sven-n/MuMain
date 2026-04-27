// NewUIInventoryActionController.h
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)
#define AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_

#pragma once

#include "NewUIInventoryCtrl.h"
#include "IInventoryActionContext.h"

namespace SEASON3B
{

class CNewUIInventoryActionController
{
public:
    CNewUIInventoryActionController();
    ~CNewUIInventoryActionController() = default;

    void SetContext(IInventoryActionContext* pContext);
    bool HandleInventoryActions(CNewUIInventoryCtrl* targetControl) const;

private:
    bool HandlePickedItemPlacement(CNewUIInventoryCtrl* targetControl) const;
    bool TryApplyJewel(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;
    bool TryStackItem(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;
    bool TryMoveItem(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;

    bool HandleRepairClick(CNewUIInventoryCtrl* targetControl) const;

    bool HandleRightClick(CNewUIInventoryCtrl* targetControl) const;
    bool HandleStorageAutoMove(CNewUIInventoryCtrl* targetControl) const;
    bool HandleSellToNPC(CNewUIInventoryCtrl* targetControl) const;
    bool HandleInventoryRightClickActions(CNewUIInventoryCtrl* targetControl) const;
    bool TryEquipItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iSrcIndex) const;
    bool TryDropItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem) const;

    int  FindAlternateEquipSlot(int nOriginalSlot, ITEM* pItem) const;
    bool IsSlotOccupied(int nSlot) const;

    bool ApplyJewels(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;
    bool TryStackItems(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;
    bool RepairItemAtMousePoint(CNewUIInventoryCtrl* targetControl) const;
    bool TryConsumeItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iIndex) const;
    bool TryTransferBetweenInventorySections(CNewUIInventoryCtrl* sourceControl) const;

    IInventoryActionContext* m_pContext;
};

} // namespace SEASON3B

#endif // !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)