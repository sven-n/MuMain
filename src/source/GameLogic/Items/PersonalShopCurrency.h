#pragma once

#include <cstdint>
#include <span>

// The prices of items in personal shops. Zen always works; prices in items
// (e.g. jewels) and account coins only when the server allows them and told
// the client which ones with the price types message.
namespace GameLogic::PersonalShop
{
/// Bits of the item in the price item type: the group in the highest 4
/// bits, the number in the remaining 12, as the server encodes it.
inline constexpr int PriceItemTypeGroupShift = 12;
inline constexpr uint16_t PriceItemTypeNumberMask = (1u << PriceItemTypeGroupShift) - 1;

/// The server marks prices in account coins with these price item types,
/// in the order WCoin (C), WCoin (P), Goblin Points. The amount is in the
/// money field.
inline constexpr uint16_t FirstAccountCoinPriceItemType = 0xFFF0;
inline constexpr uint16_t AccountCoinCount = 3;

constexpr bool IsAccountCoin(uint16_t priceItemType)
{
    return priceItemType >= FirstAccountCoinPriceItemType &&
           priceItemType < FirstAccountCoinPriceItemType + AccountCoinCount;
}

enum class PriceKind : uint8_t
{
    Zen,
    Item,
    AccountCoin,
};

/// The price of an item in a personal shop.
struct ItemPrice
{
    /// Zen, pieces of the price item or account coins, depending on the kind.
    int Amount = 0;
    PriceKind Kind = PriceKind::Zen;
    /// The price type as the server names it: the item for item prices
    /// (group in the highest 4 bits, number in the others), the reserved
    /// value of the coin for coin prices, 0 for zen.
    uint16_t PriceItemType = 0;
};

/// The price fields of the extended protocol, which the shop item list
/// and the set price request share.
struct EncodedPrice
{
    uint32_t MoneyPrice = 0;
    uint16_t PriceItemType = 0;
    uint16_t RequiredItemAmount = 0;
};

ItemPrice DecodeExtendedPrice(uint32_t moneyPrice, uint16_t priceItemType, uint16_t requiredItemAmount);
EncodedPrice EncodeExtendedPrice(const ItemPrice& price);

/// Stores the price types which the server allows in addition to zen.
void SetAllowedPriceTypes(std::span<const uint16_t> priceItemTypes);

/// True when the server allows other prices than zen, so the price can be
/// set with the extended packet.
bool AreOtherPricesThanZenAllowed();

/// The allowed price type after the one of the given price, wrapping
/// around to zen. The amount of the result is the one of the given price.
ItemPrice GetNextAllowedPriceType(const ItemPrice& current);

/// The name of the currency of the price, e.g. "Jewel of Bless".
/// Defined in PersonalShopCurrencyName.cpp, as it needs the item and
/// text tables of the game.
const wchar_t* GetCurrencyName(const ItemPrice& price);
} // namespace GameLogic::PersonalShop
