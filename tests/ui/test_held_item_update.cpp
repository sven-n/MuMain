#include "stdafx.h"
#include "doctest.h"

#include "UI/Inventory/InventoryContents.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "GameLogic/Items/PendingItemUses.h"

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

TEST_CASE("A refused jewel stays on a later update of its target [ui][inventory][held item]")
{
    // The jewel was used on the target and the server refused it (ItemConsumptionFailed), which clears the mark; the
    // target is then updated for another reason, e.g. it's a rented item which expires.
    mu::ui::window::CPickedItem jewel;
    jewel.MarkUsedOnItem(TargetSlot);
    CHECK(
        UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, jewel.GetUsedOnIndex(), TargetSlot));

    jewel.ClearUsedOnItem();
    CHECK_FALSE(jewel.GetUsedOnIndex().has_value());
    CHECK_FALSE(
        UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, jewel.GetUsedOnIndex(), TargetSlot));
}

TEST_CASE("Only the refusal of the held jewel's own use clears its mark [ui][inventory][held item]")
{
    // The jewel came from HeldSlot. A refusal of its use clears the mark; the refusal of another use, e.g. an MU
    // Helper potion sent while the jewel's answer was still due, keeps it, and so does a refusal with no sent use.
    CHECK(UI::Inventory::IsRefusalOfHeldItemUse(STORAGE_TYPE::INVENTORY, HeldSlot, HeldSlot));
    CHECK_FALSE(UI::Inventory::IsRefusalOfHeldItemUse(STORAGE_TYPE::INVENTORY, HeldSlot, OtherSlot));
    CHECK_FALSE(UI::Inventory::IsRefusalOfHeldItemUse(STORAGE_TYPE::INVENTORY, HeldSlot, NotUsed));
    // The same slot number in another storage is another item.
    CHECK_FALSE(UI::Inventory::IsRefusalOfHeldItemUse(STORAGE_TYPE::VAULT, HeldSlot, HeldSlot));
}

namespace
{
constexpr int PotionSlot = 50;
} // namespace

TEST_CASE("A refusal answers the oldest pending use [ui][inventory][held item]")
{
    GameLogic::Items::PendingItemUses uses;
    uses.Add(HeldSlot, TargetSlot);
    uses.Add(PotionSlot, std::nullopt);

    CHECK(uses.TakeOldest() == HeldSlot);
    CHECK(uses.TakeOldest() == PotionSlot);
    CHECK_FALSE(uses.TakeOldest().has_value());
}

TEST_CASE(
    "A successful answer takes its use out, by the used item or the item it was used on [ui][inventory][held item]")
{
    GameLogic::Items::PendingItemUses uses;
    uses.Add(HeldSlot, TargetSlot);
    uses.Add(PotionSlot, std::nullopt);

    uses.Answered(PotionSlot); // e.g. the potion's durability went down
    CHECK(uses.Count() == 1);
    uses.Answered(TargetSlot); // the update of the jewel's target
    CHECK(uses.Count() == 0);

    uses.Add(HeldSlot, TargetSlot);
    uses.Answered(HeldSlot); // the consumed jewel's slot was emptied
    CHECK(uses.Count() == 0);
    uses.Answered(OtherSlot);
    CHECK(uses.Count() == 0);
}

TEST_CASE("The pending uses stay short when answers go missing [ui][inventory][held item]")
{
    GameLogic::Items::PendingItemUses uses;
    for (int slot = 0; slot < static_cast<int>(GameLogic::Items::PendingItemUses::MaxPending) + 2; ++slot)
        uses.Add(slot, std::nullopt);

    CHECK(uses.Count() == GameLogic::Items::PendingItemUses::MaxPending);
    CHECK(uses.TakeOldest() == 2);
    uses.Clear();
    CHECK(uses.Count() == 0);
}

TEST_CASE("A refused jewel sent before a potion clears its mark [ui][inventory][held item]")
{
    // The jewel goes out first, then an MU Helper potion; the server refuses the jewel.
    GameLogic::Items::PendingItemUses uses;
    uses.Add(HeldSlot, TargetSlot);
    uses.Add(PotionSlot, std::nullopt);
    CHECK(UI::Inventory::IsRefusalOfHeldItemUse(STORAGE_TYPE::INVENTORY, HeldSlot, uses.TakeOldest()));
}

TEST_CASE("A refused potion sent before the jewel keeps the jewel's mark [ui][inventory][held item]")
{
    // The potion goes out first, then the jewel; the server refuses the potion and later accepts the jewel.
    GameLogic::Items::PendingItemUses uses;
    uses.Add(PotionSlot, std::nullopt);
    uses.Add(HeldSlot, TargetSlot);
    CHECK_FALSE(UI::Inventory::IsRefusalOfHeldItemUse(STORAGE_TYPE::INVENTORY, HeldSlot, uses.TakeOldest()));
    CHECK(UI::Inventory::IsHeldItemEndedByUpdate(STORAGE_TYPE::INVENTORY, HeldSlot, TargetSlot, TargetSlot));
}
