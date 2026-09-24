#pragma once

#include <cstdint>

namespace GameLogic::Items
{
// Wealth tier of a Zen amount. The original client colored every displayed Zen amount by it;
// the UI themes now own those colors.
enum class GoldAmountTier
{
    Base,
    HundredThousand,
    Million,
    TenMillion,
};

[[nodiscard]] GoldAmountTier ClassifyGoldAmount(std::uint64_t gold);
} // namespace GameLogic::Items
