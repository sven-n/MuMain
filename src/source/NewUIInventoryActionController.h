// NewUIInventoryActionController.h: interface for the CNewUIInventoryActionController class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)
#define AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_

#pragma once

#include "NewUIInventoryCtrl.h"

namespace SEASON3B
{

class CNewUIMyInventory;

class CNewUIInventoryActionController
{
public:
    CNewUIInventoryActionController();
    ~CNewUIInventoryActionController() = default;

    void SetOwner(CNewUIMyInventory* pOwner);
    bool HandleInventoryActions(CNewUIInventoryCtrl* targetControl) const;

private:
    bool HandlePickedItemPlacement(CNewUIInventoryCtrl* targetControl) const;
    bool TryApplyJewel(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem,
                       ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;
    bool TryStackItem(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem,
                      int iSourceIndex, int iTargetIndex) const;
    bool TryMoveItem(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem,
                     ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;

    bool HandleRepairClick(CNewUIInventoryCtrl* targetControl) const;

    bool HandleRightClick(CNewUIInventoryCtrl* targetControl) const;
    bool HandleStorageAutoMove(CNewUIInventoryCtrl* targetControl) const;
    bool HandleSellToNPC(CNewUIInventoryCtrl* targetControl) const;
    bool HandleInventoryRightClickActions(CNewUIInventoryCtrl* targetControl) const;
    bool TryEquipItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iSrcIndex) const;
    bool TryDropItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem) const;

    int FindAlternateEquipSlot(int nOriginalSlot, ITEM* pItem) const;
    bool IsSlotOccupied(int nSlot) const;

    CNewUIMyInventory* m_pOwner;
};

} // namespace SEASON3B

#endif // !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)