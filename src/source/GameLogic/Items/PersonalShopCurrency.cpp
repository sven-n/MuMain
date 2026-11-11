#include "stdafx.h"
#include "GameLogic/Items/PersonalShopCurrency.h"

#include <algorithm>
#include <array>
#include <vector>

namespace
{
using GameLogic::PersonalShop::IsAccountCoin;
using GameLogic::PersonalShop::ItemPrice;
using GameLogic::PersonalShop::PriceKind;

std::vector<uint16_t> AllowedPriceTypes;

// The jewels which are commonly used as prices, in the order of their usual value: Bless, Soul,
// Chaos, Life, Creation, Guardian and Harmony (group in the highest 4 bits, number in the others).
constexpr std::array<uint16_t, 7> CommonPriceItemTypes = {0xE00D, 0xE00E, 0xC00F, 0xE010, 0xE016, 0xE01F, 0xE02A};

// The position of a price item type in the menu: account coins, then common jewels, then the rest.
size_t MenuRank(uint16_t priceItemType)
{
    if (IsAccountCoin(priceItemType))
    {
        return priceItemType - GameLogic::PersonalShop::FirstAccountCoinPriceItemType;
    }

    const auto common = std::find(CommonPriceItemTypes.begin(), CommonPriceItemTypes.end(), priceItemType);
    if (common != CommonPriceItemTypes.end())
    {
        return GameLogic::PersonalShop::AccountCoinCount + (common - CommonPriceItemTypes.begin());
    }

    return GameLogic::PersonalShop::AccountCoinCount + CommonPriceItemTypes.size();
}

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

bool IsCommonPriceType(const ItemPrice& price)
{
    return price.Kind != PriceKind::Item ||
           MenuRank(price.PriceItemType) < AccountCoinCount + CommonPriceItemTypes.size();
}

std::vector<ItemPrice> GetAllowedPriceTypes()
{
    std::vector<uint16_t> priceItemTypes = AllowedPriceTypes;
    // Stable, so the items which aren't common keep the order of the server.
    std::stable_sort(priceItemTypes.begin(), priceItemTypes.end(),
                     [](uint16_t a, uint16_t b) { return MenuRank(a) < MenuRank(b); });

    std::vector<ItemPrice> result{ItemPrice{}};
    for (const uint16_t priceItemType : priceItemTypes)
    {
        result.push_back(ToPrice(priceItemType, 0));
    }

    return result;
}
} // namespace GameLogic::PersonalShop
