// Tests of the prices in personal shops: how the extended protocol encodes them, and which currencies the price
// input offers after the server sent its allowed price types.

#include "stdafx.h"

#include "doctest.h"

#include "GameLogic/Items/PersonalShopCurrency.h"

#include <array>
#include <cstdint>

using GameLogic::PersonalShop::DecodeExtendedPrice;
using GameLogic::PersonalShop::EncodeExtendedPrice;
using GameLogic::PersonalShop::GetNextAllowedPriceType;
using GameLogic::PersonalShop::ItemPrice;
using GameLogic::PersonalShop::PriceKind;
using GameLogic::PersonalShop::SetAllowedPriceTypes;

namespace
{
// Jewel of Bless: group 14, number 13; Jewel of Soul: group 14, number 14.
constexpr uint16_t JewelOfBless = 0xE00D;
constexpr uint16_t JewelOfSoul = 0xE00E;
constexpr uint16_t GoblinPoints = 0xFFF2;
} // namespace

TEST_CASE("a price without item type and amount is in zen")
{
    const auto price = DecodeExtendedPrice(1000, 0, 0);
    CHECK(price.Amount == 1000);
    CHECK(price.Kind == PriceKind::Zen);
}

TEST_CASE("a price in items has the item as type and the pieces as amount")
{
    const auto price = DecodeExtendedPrice(0, JewelOfBless, 12);
    CHECK(price.Amount == 12);
    CHECK(price.Kind == PriceKind::Item);
    CHECK(price.PriceItemType == JewelOfBless);
}

TEST_CASE("a price in account coins has a reserved item type and the coins as money")
{
    const auto price = DecodeExtendedPrice(500, GoblinPoints, 0);
    CHECK(price.Amount == 500);
    CHECK(price.Kind == PriceKind::AccountCoin);
    CHECK(price.PriceItemType == GoblinPoints);
}

TEST_CASE("a price is encoded like it's decoded")
{
    const std::array<ItemPrice, 3> prices = { {
        { 1000, PriceKind::Zen, 0 },
        { 12, PriceKind::Item, JewelOfBless },
        { 500, PriceKind::AccountCoin, GoblinPoints },
    } };

    for (const auto& price : prices)
    {
        const auto encoded = EncodeExtendedPrice(price);
        const auto decoded = DecodeExtendedPrice(encoded.MoneyPrice, encoded.PriceItemType, encoded.RequiredItemAmount);
        CHECK(decoded.Amount == price.Amount);
        CHECK(decoded.Kind == price.Kind);
        CHECK(decoded.PriceItemType == price.PriceItemType);
    }
}

TEST_CASE("the price input offers only zen until the server allows other prices")
{
    SetAllowedPriceTypes({});
    CHECK_FALSE(GameLogic::PersonalShop::AreOtherPricesThanZenAllowed());
    CHECK(GetNextAllowedPriceType({}).Kind == PriceKind::Zen);
}

TEST_CASE("the price input cycles through zen and the allowed price types")
{
    const std::array<uint16_t, 2> allowed = { JewelOfSoul, GoblinPoints };
    SetAllowedPriceTypes(allowed);
    CHECK(GameLogic::PersonalShop::AreOtherPricesThanZenAllowed());

    const auto soul = GetNextAllowedPriceType({});
    CHECK(soul.Kind == PriceKind::Item);
    CHECK(soul.PriceItemType == JewelOfSoul);

    const auto goblinPoints = GetNextAllowedPriceType(soul);
    CHECK(goblinPoints.Kind == PriceKind::AccountCoin);
    CHECK(goblinPoints.PriceItemType == GoblinPoints);

    CHECK(GetNextAllowedPriceType(goblinPoints).Kind == PriceKind::Zen);
    SetAllowedPriceTypes({});
}
