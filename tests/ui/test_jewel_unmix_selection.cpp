#include <doctest.h>
#include "GameLogic/Items/JewelUnmixSelection.h"

#include <array>

using GameLogic::Items::JewelUnmixSelection;

TEST_CASE("unmix selection follows the item through list updates")
{
    JewelUnmixSelection selection;
    const JewelUnmixSelection::Entry first{12, 100, 10, 1};
    const JewelUnmixSelection::Entry second{15, 101, 10, 2};
    std::array entries{first, second};
    CHECK(selection.Update(entries));
    CHECK_FALSE(selection.Selected());
    REQUIRE(selection.Select(second));
    CHECK_FALSE(selection.Update(entries));
    CHECK(selection.Select(second));
    std::swap(entries[0], entries[1]);
    CHECK(selection.Update(entries));
    REQUIRE(selection.Selected());
    CHECK(*selection.Selected() == second);
}

TEST_CASE("unmix rejects stale clicks and invalidates replaced items")
{
    JewelUnmixSelection selection;
    const JewelUnmixSelection::Entry original{12, 100, 10, 1};
    std::array entries{original};
    selection.Update(entries);
    REQUIRE(selection.Select(original));
    SUBCASE("same slot and jewel with a new key") { ++entries[0].key; }
    SUBCASE("same item with a different quantity") { ++entries[0].level; }
    SUBCASE("same slot with a different jewel") { ++entries[0].type; }
    selection.Update(entries);
    CHECK_FALSE(selection.Selected());
    CHECK_FALSE(selection.Select(original));
    CHECK(selection.Select(entries[0]));
}

TEST_CASE("unmix removal and reopening clear selection")
{
    JewelUnmixSelection selection;
    const std::array entries{JewelUnmixSelection::Entry{12, 100, 10, 1}};
    selection.Update(entries);
    REQUIRE(selection.Select(entries[0]));
    SUBCASE("inventory removal") { selection.Update({}); }
    SUBCASE("window reset") { selection.Clear(); }
    CHECK_FALSE(selection.Selected());
    CHECK(selection.Entries().empty());
    selection.Update(entries);
    CHECK_FALSE(selection.Selected());
}
