#include <doctest.h>
#include "UI/Inventory/TradeWorseOffers.h"

#include <array>

using UI::Trade::MarkWorseOffers;
using UI::Trade::OfferedItemCheck;

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

TEST_CASE("a full trade window keeps the warning of the one worse item with no unchanged item of its type")
{
    // More unchanged types than the ten the original client had room for.
    constexpr int ClearedTypes = 15;
    constexpr int LoneWorseType = 99;
    constexpr size_t TradeSlots = 2 * ClearedTypes + 1;
    std::array<OfferedItemCheck, TradeSlots> offers{};
    for (int type = 0; type < ClearedTypes; ++type)
    {
        offers[2 * type] = Worse(type);
        offers[2 * type + 1] = Same(type);
    }
    offers[TradeSlots - 1] = Worse(LoneWorseType);
    std::array<bool, TradeSlots> marked{};

    CHECK(MarkWorseOffers(offers, marked));
    for (size_t i = 0; i + 1 < TradeSlots; ++i)
        CHECK_FALSE(marked[i]);
    CHECK(marked[TradeSlots - 1]);
}

TEST_CASE("no offered items raise no warning")
{
    CHECK_FALSE(MarkWorseOffers({}, {}));
}
