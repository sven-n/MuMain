#include "stdafx.h"
#include "GameLogic/Items/PersonalShopCurrency.h"

#include <array>

#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

// Kept apart from the protocol logic in PersonalShopCurrency.cpp, so that the
// tests of that logic don't need the item and text tables of the game.
namespace
{
int ToClientItemType(uint16_t priceItemType)
{
    const int group = priceItemType >> GameLogic::PersonalShop::PriceItemTypeGroupShift;
    const int number = priceItemType & GameLogic::PersonalShop::PriceItemTypeNumberMask;
    return group * MAX_ITEM_INDEX + number;
}
} // namespace

namespace GameLogic::PersonalShop
{
const wchar_t* GetCurrencyName(const ItemPrice& price)
{
    switch (price.Kind)
    {
    case PriceKind::Item:
    {
        const int itemType = ToClientItemType(price.PriceItemType);
        return itemType < MAX_ITEM ? ItemAttribute[itemType].Name : I18N::Game::Zen;
    }
    case PriceKind::AccountCoin:
    {
        static const std::array<const wchar_t*, AccountCoinCount> CoinNames = {
            I18N::Game::WCoinC,
            I18N::Game::WCoinP,
            I18N::Game::GoblinPoints,
        };
        return CoinNames[price.PriceItemType - FirstAccountCoinPriceItemType];
    }
    case PriceKind::Zen:
    default:
        return I18N::Game::Zen;
    }
}
} // namespace GameLogic::PersonalShop
