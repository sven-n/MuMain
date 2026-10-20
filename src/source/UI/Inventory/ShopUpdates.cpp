#include "stdafx.h"

#include "UI/Inventory/ShopUpdates.h"

#include "Core/Globals/_enum.h"
#include "GameLogic/Items/PersonalShopCurrency.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameShop/InGameShop.h"
#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/InventoryCtrl.h"

#include <string>

namespace
{
// The slot, previous presence and price of the latest own shop price update.
int ReplacedPriceSlot = -1;
bool ReplacedPriceHadPrice = false;
GameLogic::PersonalShop::ItemPrice ReplacedPrice{};
} // namespace

namespace UI::Shop
{
void NpcSaleFinished()
{
    g_pNPCShop->SetSellingItem(false);
}

void SetNpcTaxRate(int rate)
{
    g_pNPCShop->SetTaxRate(rate);
}

void ResetOwnShopTitle()
{
    g_pMyShopInventory->ResetSubject();
}

void ShowOwnShopTitle(std::wstring_view title)
{
    std::wstring text(title);
    g_pMyShopInventory->SetTitle(text.data());
    g_pMyShopInventory->ChangePersonal(true);
}

void SetOwnShopOpen(bool open)
{
    g_pMyShopInventory->ChangePersonal(open);
}

void ReplaceOwnShopItems(std::span<const ShopItem> items, std::wstring_view title)
{
    g_pMyShopInventory->GetInventoryCtrl()->RemoveAllItems();
    for (const ShopItem& item : items)
        g_pMyShopInventory->InsertItem(item.slot, item.itemData);
    g_pMyShopInventory->ChangePersonal(true);
    std::wstring text(title);
    g_pMyShopInventory->ChangeTitle(text.data());
}

int OwnShopPriceTargetIndex()
{
    return g_pMyShopInventory->GetTargetIndex();
}

void RememberReplacedOwnShopPrice(int slot)
{
    ReplacedPriceSlot = slot;
    ReplacedPriceHadPrice = GetPersonalItemPrice(slot, ReplacedPrice, PSHOPWNDTYPE_SALE);
}

bool RestoreReplacedOwnShopPrice(int slot)
{
    if (slot != ReplacedPriceSlot)
    {
        return false;
    }

    ReplacedPriceSlot = -1;
    if (ReplacedPriceHadPrice)
    {
        AddPersonalItemPrice(slot, ReplacedPrice, PSHOPWNDTYPE_SALE);
    }
    else
    {
        RemovePersonalItemPrice(slot, PSHOPWNDTYPE_SALE);
    }

    return true;
}

void ForgetReplacedOwnShopPrice(int slot)
{
    if (slot == ReplacedPriceSlot)
    {
        ReplacedPriceSlot = -1;
    }
}

int BrowsedShopCharacterIndex()
{
    return g_pPurchaseShopInventory->GetShopCharacterIndex();
}

void OpenBrowsedShop(std::wstring_view title)
{
    using namespace mu::ui::window;
    if (UI::Windows::IsVisible(INTERFACE_STORAGE))
    {
        UI::Windows::Hide(INTERFACE_STORAGE);
        UI::Windows::Hide(INTERFACE_STORAGE_EXT);
    }
    if (UI::Windows::IsVisible(INTERFACE_INVENTORY))
        UI::Windows::Hide(INTERFACE_INVENTORY);

    std::wstring text(title);
    g_pPurchaseShopInventory->ChangeTitleText(text.data());
    g_pPurchaseShopInventory->GetInventoryCtrl()->RemoveAllItems();

    UI::Windows::Show(INTERFACE_PURCHASESHOP_INVENTORY);
    UI::Windows::Show(INTERFACE_INVENTORY);
    g_pMyInventory->ChangeMyShopButtonStateOpen();
}

void AddBrowsedShopItem(const ShopItem& item)
{
    g_pPurchaseShopInventory->InsertItem(item.slot, item.itemData);
}

void SetBrowsedShopCharacterIndex(int characterIndex)
{
    g_pPurchaseShopInventory->ChangeShopCharacterIndex(characterIndex);
}

void ReplaceBrowsedShopItems(std::span<const ShopItem> items)
{
    g_pPurchaseShopInventory->GetInventoryCtrl()->RemoveAllItems();
    for (const ShopItem& item : items)
        g_pPurchaseShopInventory->InsertItem(item.slot, item.itemData);
}

int BrowsedShopItemCount()
{
    return static_cast<int>(g_pPurchaseShopInventory->GetInventoryCtrl()->GetNumberOfItems());
}

int BrowsedShopPurchaseIndex()
{
    return g_pPurchaseShopInventory->GetSourceIndex();
}

void RemoveBrowsedShopItem(int slot)
{
    g_pPurchaseShopInventory->DeleteItem(slot);
}

char CashShopStorageCode()
{
    return g_pInGameShop->GetCurrentStorageCode();
}

void SetCashShopStoragePage(int totalItems, int pageItems, int totalPages, int page)
{
    g_pInGameShop->InitStorage(totalItems, pageItems, totalPages, page);
}

void AddCashShopStorageItem(const CashShopStorageItem& item)
{
    if (!item.gift)
    {
        g_pInGameShop->AddStorageItem(item.storageIndex, item.itemSequence, item.groupCode, item.productSequence,
                                      item.priceSequence, item.cashPoint, item.itemType);
        return;
    }
    std::wstring sender(item.sender);
    std::wstring message(item.message);
    g_pInGameShop->AddStorageItem(item.storageIndex, item.itemSequence, item.groupCode, item.productSequence,
                                  item.priceSequence, item.cashPoint, item.itemType, sender.data(), message.data());
}

void RefreshCashShopStorage()
{
    g_pInGameShop->UpdateStorageItemList();
}

void ReloadCashShopZones()
{
    g_pInGameShop->InitZoneBtn();
}

void SetCashShopBanner(std::wstring_view fileName, std::wstring_view url)
{
    std::wstring file(fileName);
    std::wstring link(url);
    g_pInGameShop->InitBanner(file.data(), link.data());
}
}
