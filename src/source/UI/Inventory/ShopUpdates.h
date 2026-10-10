#pragma once

#include <cstdint>
#include <span>
#include <string_view>

// Shop changes the server reports: the NPC shop, this player's personal shop, another player's
// personal shop being browsed, and the cash shop. Item data is the item's serialized form; strings
// and spans are borrowed for the call.
namespace UI::Shop
{
struct ShopItem
{
    int slot;
    std::span<const std::uint8_t> itemData;
};

// NPC shop.
void NpcSaleFinished();
void SetNpcTaxRate(int rate);

// This player's personal shop.
void ResetOwnShopTitle();
// The shop's title as players around see it, shown when the shop opens.
void ShowOwnShopTitle(std::wstring_view title);
void SetOwnShopOpen(bool open);
void ReplaceOwnShopItems(std::span<const ShopItem> items, std::wstring_view title);
int OwnShopPriceTargetIndex();

// Another player's personal shop.
int BrowsedShopCharacterIndex();
// Closes the vault and inventory, then opens the browsed shop beside the inventory.
void OpenBrowsedShop(std::wstring_view title);
void AddBrowsedShopItem(const ShopItem& item);
void SetBrowsedShopCharacterIndex(int characterIndex);
void ReplaceBrowsedShopItems(std::span<const ShopItem> items);
int BrowsedShopItemCount();
// The slot of the item being bought.
int BrowsedShopPurchaseIndex();
void RemoveBrowsedShopItem(int slot);

// Cash shop.
struct CashShopStorageItem
{
    int storageIndex;
    int itemSequence;
    int groupCode;
    int productSequence;
    int priceSequence;
    int cashPoint;
    char itemType;
    // A gift carries its sender and message.
    bool gift = false;
    std::wstring_view sender;
    std::wstring_view message;
};

char CashShopStorageCode();
void SetCashShopStoragePage(int totalItems, int pageItems, int totalPages, int page);
void AddCashShopStorageItem(const CashShopStorageItem& item);
void RefreshCashShopStorage();
void ReloadCashShopZones();
void SetCashShopBanner(std::wstring_view fileName, std::wstring_view url);
}
