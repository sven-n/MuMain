#include <doctest.h>
#include "UI/Inventory/TradeWorseOffers.h"
#include "Engine/Object/CompareItemResult.h"

#include <array>

namespace CompareResult = Engine::Items::CompareResult;
using UI::Trade::CheckAgainstRemembered;
using UI::Trade::CheckOffer;
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

TEST_CASE("an offered item worse than the remembered copy counts as worse")
{
    const OfferedItemCheck check = CheckOffer(SwordType, CompareResult::FirstIsBetter);

    CHECK(check.itemType == SwordType);
    CHECK(check.worseThanRemembered);
    CHECK_FALSE(check.sameAsRemembered);
}

TEST_CASE("an offered item the same as the remembered copy counts as unchanged")
{
    const OfferedItemCheck check = CheckOffer(SwordType, CompareResult::Same);

    CHECK_FALSE(check.worseThanRemembered);
    CHECK(check.sameAsRemembered);
}

TEST_CASE("an offered item better than the remembered copy is neither worse nor unchanged")
{
    const OfferedItemCheck check = CheckOffer(SwordType, CompareResult::SecondIsBetter);

    CHECK_FALSE(check.worseThanRemembered);
    CHECK_FALSE(check.sameAsRemembered);
}

TEST_CASE("an offered item with no remembered copy of its type is neither worse nor unchanged")
{
    const OfferedItemCheck check = CheckOffer(ShieldType, CompareResult::OtherType);

    CHECK(check.itemType == ShieldType);
    CHECK_FALSE(check.worseThanRemembered);
    CHECK_FALSE(check.sameAsRemembered);
}

namespace
{
// Stands in for ITEM: CompareItem's level rule is enough to tell the argument order.
struct LeveledItem
{
    int Type = 0;
    int Level = 0;
};

int CompareByLevel(const LeveledItem& first, const LeveledItem& second)
{
    if (first.Level > second.Level)
        return CompareResult::FirstIsBetter;
    if (first.Level < second.Level)
        return CompareResult::SecondIsBetter;
    return CompareResult::Same;
}

constexpr int HighLevel = 9;
constexpr int LowLevel = 7;
} // namespace

TEST_CASE("an offered copy below the remembered one is checked as worse, not better")
{
    const std::array remembered{LeveledItem{.Type = SwordType, .Level = HighLevel}};
    const LeveledItem offered{.Type = SwordType, .Level = LowLevel};

    const OfferedItemCheck check =
        CheckAgainstRemembered(std::span<const LeveledItem>(remembered), offered, CompareByLevel);

    CHECK(check.worseThanRemembered);
    CHECK_FALSE(check.sameAsRemembered);
}

TEST_CASE("an offered copy above the remembered one is not checked as worse")
{
    const std::array remembered{LeveledItem{.Type = SwordType, .Level = LowLevel}};
    const LeveledItem offered{.Type = SwordType, .Level = HighLevel};

    const OfferedItemCheck check =
        CheckAgainstRemembered(std::span<const LeveledItem>(remembered), offered, CompareByLevel);

    CHECK_FALSE(check.worseThanRemembered);
    CHECK_FALSE(check.sameAsRemembered);
}

TEST_CASE("an offered item is held only against the remembered copy of its own type")
{
    const std::array remembered{LeveledItem{.Type = ShieldType, .Level = HighLevel},
                                LeveledItem{.Type = SwordType, .Level = LowLevel}};
    const LeveledItem offered{.Type = SwordType, .Level = LowLevel};

    const OfferedItemCheck check =
        CheckAgainstRemembered(std::span<const LeveledItem>(remembered), offered, CompareByLevel);

    CHECK_FALSE(check.worseThanRemembered);
    CHECK(check.sameAsRemembered);
}
