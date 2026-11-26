#include "stdafx.h"
#include "doctest.h"

#include "UI/Inventory/InventoryContents.h"

namespace
{
constexpr int HeldSlot = 20;
constexpr int OtherSlot = 30;
} // namespace

TEST_CASE("An item update of the server keeps an unrelated held item [ui][inventory][held item]")
{
    CHECK_FALSE(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, false, OtherSlot));
    // The same slot number in another storage is another item.
    CHECK_FALSE(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::VAULT, HeldSlot, false, HeldSlot));
}

TEST_CASE("An item update of the server ends the updated held item [ui][inventory][held item]")
{
    CHECK(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, false, HeldSlot));
}

TEST_CASE("The update of the target item ends a held jewel used on it [ui][inventory][held item]")
{
    CHECK(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, true, OtherSlot));
}
