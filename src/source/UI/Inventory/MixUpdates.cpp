#include "stdafx.h"

#include "UI/Inventory/MixUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/InventoryCtrl.h"

namespace UI::Mix
{
void SetFinished()
{
    g_pMixInventory->SetMixState(mu::ui::window::CMixInventory::MIX_FINISHED);
}

void SetReady()
{
    g_pMixInventory->SetMixState(mu::ui::window::CMixInventory::MIX_READY);
}

void ClearItems()
{
    g_pMixInventory->DeleteAllItems();
}

void InsertItem(int slot, std::span<const std::uint8_t> itemData)
{
    g_pMixInventory->InsertItem(slot, itemData);
}

void PlaceMovedItem(int slot, std::span<const std::uint8_t> itemData)
{
    mu::ui::window::CInventoryCtrl::DeletePickedItem();
    if (slot >= 0 && slot < MAX_MIX_INVENTORY)
        g_pMixInventory->InsertItem(slot, itemData);
}

bool IsLuckyItemAwaiting(bool requireVisible)
{
    if (requireVisible && !UI::Windows::IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND))
        return false;
    return static_cast<int>(g_pLuckyItemWnd->GetAct()) != 0;
}

void LuckyItemResult(bool success, int code, std::span<const std::uint8_t> itemData)
{
    g_pLuckyItemWnd->GetResult(success ? 1 : 0, code, itemData);
}
}
