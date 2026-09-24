#include "doctest.h"

#include <string_view>

#include "GameLogic/Items/GoldAmountTier.h"
#include "UI/RmlBridge/RmlStyleKeys.h"

using GameLogic::Items::ClassifyGoldAmount;
using GameLogic::Items::GoldAmountTier;

TEST_CASE("Zen amounts fall into the original client's wealth tiers")
{
    CHECK(ClassifyGoldAmount(0) == GoldAmountTier::Base);
    CHECK(ClassifyGoldAmount(99'999) == GoldAmountTier::Base);
    CHECK(ClassifyGoldAmount(100'000) == GoldAmountTier::HundredThousand);
    CHECK(ClassifyGoldAmount(999'999) == GoldAmountTier::HundredThousand);
    CHECK(ClassifyGoldAmount(1'000'000) == GoldAmountTier::Million);
    CHECK(ClassifyGoldAmount(9'999'999) == GoldAmountTier::Million);
    CHECK(ClassifyGoldAmount(10'000'000) == GoldAmountTier::TenMillion);
    CHECK(ClassifyGoldAmount(2'000'000'000) == GoldAmountTier::TenMillion);
}

TEST_CASE("Zen wealth tiers have the style keys the themes define")
{
    CHECK(std::string_view(UI::RmlBridge::GoldTierKey(GoldAmountTier::Base)) == "base");
    CHECK(std::string_view(UI::RmlBridge::GoldTierKey(GoldAmountTier::HundredThousand)) == "hundred-thousand");
    CHECK(std::string_view(UI::RmlBridge::GoldTierKey(GoldAmountTier::Million)) == "million");
    CHECK(std::string_view(UI::RmlBridge::GoldTierKey(GoldAmountTier::TenMillion)) == "ten-million");
}
