#include "stdafx.h"
#include "doctest.h"

#include "UI/Inventory/InventoryContents.h"

#include <optional>

namespace
{
constexpr int HeldSlot = 20;
constexpr int TargetSlot = 30;
constexpr int OtherSlot = 40;
constexpr std::optional<int> NotUsed;
} // namespace

TEST_CASE("An item update of the server keeps an unrelated held item [ui][inventory][held item]")
{
    CHECK_FALSE(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, NotUsed, OtherSlot));
    // The same slot number in another storage is another item.
    CHECK_FALSE(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::VAULT, HeldSlot, NotUsed, HeldSlot));
}

TEST_CASE("An item update of the server ends the updated held item [ui][inventory][held item]")
{
    CHECK(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, NotUsed, HeldSlot));
}

TEST_CASE("The update of the target item ends a held jewel used on it [ui][inventory][held item]")
{
    CHECK(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, TargetSlot, TargetSlot));
}

TEST_CASE("A held jewel whose use was refused stays on an unrelated update [ui][inventory][held item]")
{
    // The server refused the jewel (ItemConsumptionFailed), so the target is never updated; a rented item expiring
    // in another slot must not end the jewel, which the server still has.
    CHECK_FALSE(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, TargetSlot, OtherSlot));
}
