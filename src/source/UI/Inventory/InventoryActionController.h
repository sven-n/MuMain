// NewUIInventoryActionController.h
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)
#define AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_

#pragma once

#include "UI/Inventory/InventoryCtrl.h"
#include "GameLogic/Items/IInventoryActionContext.h"

namespace mu::ui::window
{

// What releasing the held item on a grid does.
enum class DropOutcome
{
    None,       // the grid refuses it
    ApplyJewel, // a jewel (or lucky item potion) is used on the item under it
    Stack,      // it stacks onto the item under it
    Move,       // it moves to the free cells under it
    PutBack,    // it lands back on its own cells
};

class CInventoryActionController
{
public:
    CInventoryActionController();
    ~CInventoryActionController() = default;

    void SetContext(SEASON3B::IInventoryActionContext* pContext);
    bool HandleInventoryActions(CInventoryCtrl* targetControl) const;
    // What releasing the held item on targetControl would do, decided without sending anything. The drop acts on it,
    // and the inventory routes a release over an equipment slot by it, so the two agree.
    DropOutcome DropOutcomeOn(CInventoryCtrl* targetControl) const;

private:
    bool HandlePickedItemPlacement(CInventoryCtrl* targetControl) const;
    bool MoveItem(CInventoryCtrl* targetControl, CPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex,
                  int iTargetIndex) const;

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

    bool ApplyJewel(CInventoryCtrl* targetControl, CPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex,
                    int iTargetIndex) const;
    bool StackItems(ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const;
    bool RepairItemAtMousePoint(CInventoryCtrl* targetControl) const;
    bool TryConsumeItem(CInventoryCtrl* targetControl, ITEM* pItem, int iIndex) const;
    bool TryTransferBetweenInventorySections(CInventoryCtrl* sourceControl) const;

    SEASON3B::IInventoryActionContext* m_pContext;
};

} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIINVENTORYACTIONCONTROLLER_H__INCLUDED_)