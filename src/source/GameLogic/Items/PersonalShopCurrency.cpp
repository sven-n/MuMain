#include "stdafx.h"
#include "GameLogic/Items/PersonalShopCurrency.h"

#include <algorithm>
#include <vector>

namespace
{
using GameLogic::PersonalShop::IsAccountCoin;
using GameLogic::PersonalShop::ItemPrice;
using GameLogic::PersonalShop::PriceKind;

std::vector<uint16_t> AllowedPriceTypes;

ItemPrice ToPrice(uint16_t priceItemType, int amount)
{
    const PriceKind kind = IsAccountCoin(priceItemType) ? PriceKind::AccountCoin : PriceKind::Item;
    return {amount, kind, priceItemType};
}
} // namespace

namespace GameLogic::PersonalShop
{
ItemPrice DecodeExtendedPrice(uint32_t moneyPrice, uint16_t priceItemType, uint16_t requiredItemAmount)
{
    if (requiredItemAmount > 0)
    {
        return {requiredItemAmount, PriceKind::Item, priceItemType};
    }

    if (IsAccountCoin(priceItemType))
    {
        return {static_cast<int>(moneyPrice), PriceKind::AccountCoin, priceItemType};
    }

    return {static_cast<int>(moneyPrice), PriceKind::Zen, 0};
}

EncodedPrice EncodeExtendedPrice(const ItemPrice& price)
{
    switch (price.Kind)
    {
    case PriceKind::Item:
        return {0, price.PriceItemType, static_cast<uint16_t>(price.Amount)};
    case PriceKind::AccountCoin:
        return {static_cast<uint32_t>(price.Amount), price.PriceItemType, 0};
    case PriceKind::Zen:
    default:
        return {static_cast<uint32_t>(price.Amount), 0, 0};
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
        return {current.Amount, PriceKind::Zen, 0};
    }

    return ToPrice(*next, current.Amount);
}
} // namespace GameLogic::PersonalShop
