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
    bool HandleInventoryActions(CNewUIInventoryCtrl* targetControl);

private:
    bool HandlePickedItemPlacement(CNewUIInventoryCtrl* targetControl);
    bool TryApplyJewel(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem,
                       ITEM* pPickItem, int iSourceIndex, int iTargetIndex);
    bool TryStackItem(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem,
                      int iSourceIndex, int iTargetIndex);
    bool TryMoveItem(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem,
                     ITEM* pPickItem, int iSourceIndex, int iTargetIndex);

    bool HandleRepairClick(CNewUIInventoryCtrl* targetControl);

    bool HandleRightClick(CNewUIInventoryCtrl* targetControl);
    bool HandleStorageAutoMove(CNewUIInventoryCtrl* targetControl);
    bool HandleSellToNPC(CNewUIInventoryCtrl* targetControl);
    bool HandleInventoryRightClickActions(CNewUIInventoryCtrl* targetControl);
    bool TryEquipItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iSrcIndex);
    bool TryDropItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem);

    int FindAlternateEquipSlot(int nOriginalSlot, ITEM* pItem) const;
    bool IsSlotOccupied(int nSlot) const;

    CNewUIMyInventory* m_pOwner;
};

} // namespace SEASON3B

#endif // !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)