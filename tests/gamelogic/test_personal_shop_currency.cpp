// Tests of the prices in personal shops: how the extended protocol encodes them, and which currencies the price
// input offers after the server sent its allowed price types.

#include "stdafx.h"

#include "doctest.h"

#include "GameLogic/Items/PersonalShopCurrency.h"

#include <array>
#include <cstdint>

using GameLogic::PersonalShop::DecodeExtendedPrice;
using GameLogic::PersonalShop::EncodeExtendedPrice;
using GameLogic::PersonalShop::GetAllowedPriceTypes;
using GameLogic::PersonalShop::IsCommonPriceType;
using GameLogic::PersonalShop::ItemPrice;
using GameLogic::PersonalShop::PriceKind;
using GameLogic::PersonalShop::SetAllowedPriceTypes;

namespace
{
// Jewel of Bless: group 14, number 13; Jewel of Soul: group 14, number 14; Jewel of Chaos: group 12,
// number 15; Loch's Feather: group 13, number 14; Gemstone: group 14, number 41.
constexpr uint16_t JewelOfBless = 0xE00D;
constexpr uint16_t JewelOfSoul = 0xE00E;
constexpr uint16_t JewelOfChaos = 0xC00F;
constexpr uint16_t LochsFeather = 0xD00E;
constexpr uint16_t Gemstone = 0xE029;
constexpr uint16_t WCoinC = 0xFFF0;
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
    const auto choices = GetAllowedPriceTypes();
    REQUIRE(choices.size() == 1);
    CHECK(choices[0].Kind == PriceKind::Zen);
}

TEST_CASE("the price input offers zen, the coins and the jewels before the other items")
{
    // As a server could send them: items of its configuration first, then the coins.
    const std::array<uint16_t, 7> allowed = { Gemstone, JewelOfChaos, LochsFeather, JewelOfBless, GoblinPoints,
                                              JewelOfSoul, WCoinC };
    SetAllowedPriceTypes(allowed);
    CHECK(GameLogic::PersonalShop::AreOtherPricesThanZenAllowed());

    const auto choices = GetAllowedPriceTypes();
    const std::array<uint16_t, 8> expected = { 0, WCoinC, GoblinPoints, JewelOfBless, JewelOfSoul, JewelOfChaos,
                                               Gemstone, LochsFeather };
    REQUIRE(choices.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        CHECK(choices[i].PriceItemType == expected[i]);
        CHECK(choices[i].Amount == 0);
    }

    CHECK(choices[0].Kind == PriceKind::Zen);
    CHECK(choices[1].Kind == PriceKind::AccountCoin);
    CHECK(choices[3].Kind == PriceKind::Item);
    CHECK(IsCommonPriceType(choices[5]));
    CHECK_FALSE(IsCommonPriceType(choices[6]));
    CHECK_FALSE(IsCommonPriceType(choices[7]));
    SetAllowedPriceTypes({});
}
