#include "GameLogic/Items/GoldAmountTier.h"

namespace
{
constexpr std::uint64_t kHundredThousand = 100'000;
constexpr std::uint64_t kMillion = 1'000'000;
constexpr std::uint64_t kTenMillion = 10'000'000;
} // namespace

GameLogic::Items::GoldAmountTier GameLogic::Items::ClassifyGoldAmount(std::uint64_t gold)
{
    if (gold >= kTenMillion)
        return GoldAmountTier::TenMillion;
    if (gold >= kMillion)
        return GoldAmountTier::Million;
    if (gold >= kHundredThousand)
        return GoldAmountTier::HundredThousand;
    return GoldAmountTier::Base;
}
