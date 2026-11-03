#include <doctest.h>
#include "UI/Inventory/TradeChangeWarning.h"

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_enum.h"

using UI::Inventory::IsTrackedForChangeWarning;

namespace
{
bool Tracked(int itemType, int level = 0, int excellentFlags = 0, bool compiledGem = false)
{
    return IsTrackedForChangeWarning(itemType, level, excellentFlags, compiledGem);
}
} // namespace

TEST_CASE("the trade warning remembers the items its list names")
{
    CHECK(Tracked(ITEM_HELPER));                  // the first pet, up to the Dark Horse
    CHECK(Tracked(ITEM_DARK_HORSE_ITEM));
    CHECK(Tracked(ITEM_JEWEL_OF_BLESS));
    CHECK(Tracked(ITEM_JEWEL_OF_SOUL));
    CHECK(Tracked(ITEM_JEWEL_OF_LIFE));
    CHECK(Tracked(ITEM_JEWEL_OF_GUARDIAN));
    CHECK(Tracked(ITEM_JEWEL_OF_CHAOS));
    CHECK(Tracked(ITEM_WING));
    CHECK(Tracked(ITEM_WINGS_OF_DARKNESS));
    CHECK(Tracked(ITEM_WING_OF_STORM));
    CHECK(Tracked(ITEM_WING_OF_DIMENSION));
    CHECK(Tracked(ITEM_CAPE_OF_LORD));
    CHECK(Tracked(ITEM_CAPE_OF_FIGHTER));
    CHECK(Tracked(ITEM_CAPE_OF_OVERRULE));
    CHECK(Tracked(INDEX_COMPILED_SOUL, 0, 0, true)); // a packed jewel
}

TEST_CASE("the trade warning remembers weapons and armor from +5 and anything excellent")
{
    CHECK_FALSE(Tracked(ITEM_SHORT_SWORD, 4));
    CHECK(Tracked(ITEM_SHORT_SWORD, 5));
    CHECK(Tracked(ITEM_SHORT_BOW, 9));
    CHECK(Tracked(ITEM_SHORT_SWORD, 0, 1));
    CHECK(Tracked(ITEM_LARGE_HEALING_POTION, 0, 1));
}

// sven-n/MuMain#671: two lines used >= for one item each, so every item from (13,30) on was remembered.
TEST_CASE("the trade warning doesn't remember potions, boxes and the other items after the Cape of Lord")
{
    CHECK_FALSE(Tracked(ITEM_LARGE_HEALING_POTION));  // a smaller stack of a potion isn't a worse copy
    CHECK_FALSE(Tracked(ITEM_BOX_OF_LUCK, 6));       // nor a box of luck of another level
    CHECK_FALSE(Tracked(ITEM_BOX_OF_LUCK, 3));
    CHECK_FALSE(Tracked(ITEM_DEVILS_KEY, 2));
    CHECK_FALSE(Tracked(ITEM_SCROLL_OF_BLOOD, 3));
    CHECK_FALSE(Tracked(ITEM_SPIRIT));                // the item right after the Cape of Lord
    CHECK_FALSE(Tracked(ITEM_HORN_OF_FENRIR));
    CHECK_FALSE(Tracked(ITEM_JEWEL_OF_CREATION));
}
