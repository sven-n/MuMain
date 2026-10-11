#include "stdafx.h"
#include "GameLogic/Items/PersonalShopCurrency.h"

#include <algorithm>
#include <array>
#include <vector>

#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

namespace
{
using GameLogic::PersonalShop::ItemPrice;
using GameLogic::PersonalShop::PriceKind;

// Bits of the item in the price item type: the group in the highest 4 bits,
// the number in the remaining 12, as the server encodes it.
constexpr int PriceItemTypeGroupShift = 12;
constexpr uint16_t PriceItemTypeNumberMask = (1u << PriceItemTypeGroupShift) - 1;

// The server marks prices in account coins with these price item types, in
// the order WCoin (C), WCoin (P), Goblin Points. The amount is in the money field.
constexpr uint16_t FirstAccountCoinPriceItemType = 0xFFF0;
constexpr uint16_t AccountCoinCount = 3;

std::vector<uint16_t> AllowedPriceTypes;

bool IsAccountCoin(uint16_t priceItemType)
{
    return priceItemType >= FirstAccountCoinPriceItemType
           && priceItemType < FirstAccountCoinPriceItemType + AccountCoinCount;
}

ItemPrice ToPrice(uint16_t priceItemType, int amount)
{
    const PriceKind kind = IsAccountCoin(priceItemType) ? PriceKind::AccountCoin : PriceKind::Item;
    return { amount, kind, priceItemType };
}

int ToClientItemType(uint16_t priceItemType)
{
    const int group = priceItemType >> PriceItemTypeGroupShift;
    const int number = priceItemType & PriceItemTypeNumberMask;
    return group * MAX_ITEM_INDEX + number;
}
} // namespace

namespace GameLogic::PersonalShop
{
    ItemPrice DecodeExtendedPrice(uint32_t moneyPrice, uint16_t priceItemType, uint16_t requiredItemAmount)
    {
        if (requiredItemAmount > 0)
        {
            return { requiredItemAmount, PriceKind::Item, priceItemType };
        }

        if (IsAccountCoin(priceItemType))
        {
            return { static_cast<int>(moneyPrice), PriceKind::AccountCoin, priceItemType };
        }

        return { static_cast<int>(moneyPrice), PriceKind::Zen, 0 };
    }

    EncodedPrice EncodeExtendedPrice(const ItemPrice& price)
    {
        switch (price.Kind)
        {
        case PriceKind::Item:
            return { 0, price.PriceItemType, static_cast<uint16_t>(price.Amount) };
        case PriceKind::AccountCoin:
            return { static_cast<uint32_t>(price.Amount), price.PriceItemType, 0 };
        case PriceKind::Zen:
        default:
            return { static_cast<uint32_t>(price.Amount), 0, 0 };
        }
    }

    void SetAllowedPriceTypes(std::span<const uint16_t> priceItemTypes)
    {
        AllowedPriceTypes.assign(priceItemTypes.begin(), priceItemTypes.end());
    }

    bool AreOtherPricesThanZenAllowed()
    {
        return !AllowedPriceTypes.empty();
    }

    ItemPrice GetNextAllowedPriceType(const ItemPrice& current)
    {
        // The order of the choices: zen, then the allowed price types as the server sent them.
        auto next = AllowedPriceTypes.begin();
        if (current.Kind != PriceKind::Zen)
        {
            next = std::find(AllowedPriceTypes.begin(), AllowedPriceTypes.end(), current.PriceItemType);
            if (next != AllowedPriceTypes.end())
            {
                ++next;
            }
        }

        if (next == AllowedPriceTypes.end())
        {
            return { current.Amount, PriceKind::Zen, 0 };
        }

        return ToPrice(*next, current.Amount);
    }

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
}
