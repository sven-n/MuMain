#include <doctest.h>
#include "UI/Inventory/TradeWorseOffers.h"

#include <array>

using UI::Inventory::MarkWorseOffers;
using UI::Inventory::OfferedItemCheck;

namespace
{
constexpr int SwordType = 10;
constexpr int ShieldType = 20;

OfferedItemCheck Worse(int itemType)
{
    return {.itemType = itemType, .worseThanRemembered = true};
}

OfferedItemCheck Same(int itemType)
{
    return {.itemType = itemType, .sameAsRemembered = true};
}
} // namespace

TEST_CASE("the trade window marks an offered item that is worse than the remembered copy")
{
    const std::array offers{Worse(SwordType)};
    std::array<bool, 1> marked{};

    CHECK(MarkWorseOffers(offers, marked));
    CHECK(marked[0]);
}

TEST_CASE("an unchanged item of another type keeps the warning of a worse one")
{
    const std::array offers{Worse(SwordType), Same(ShieldType)};
    std::array<bool, 2> marked{};

    CHECK(MarkWorseOffers(offers, marked));
    CHECK(marked[0]);
    CHECK_FALSE(marked[1]);
}

TEST_CASE("an unchanged item of the same type clears the warning of a worse one")
{
    const std::array offers{Worse(SwordType), Same(SwordType)};
    std::array<bool, 2> marked{};

    CHECK_FALSE(MarkWorseOffers(offers, marked));
    CHECK_FALSE(marked[0]);
    CHECK_FALSE(marked[1]);
}

TEST_CASE("a full trade window of unchanged items marks none")
{
    constexpr size_t TradeSlots = 32;
    std::array<OfferedItemCheck, TradeSlots> offers{};
    offers.fill(Same(SwordType));
    std::array<bool, TradeSlots> marked{};

    CHECK_FALSE(MarkWorseOffers(offers, marked));
}

TEST_CASE("no offered items raise no warning")
{
    CHECK_FALSE(MarkWorseOffers({}, {}));
}
