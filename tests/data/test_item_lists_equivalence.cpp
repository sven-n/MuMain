// One-time check for phase 3b of the data-driven items plan: lists of item ids
// outside the phase 3 files that repeat what the item data already says. This file
// keeps a copy of the old code (item_lists_old.inl) and checks that the queries on
// the item data that replace it give the same answers for every item type and
// item model slot. It is not meant to stay: once the data is the source of truth,
// a data change would make it fail.

#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemCategories.h"

#include <filesystem>
#include <string>
#include <vector>

#include "item_lists_old.inl"

using namespace Data::Items;

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

// Every item type, and -1 for no item.
constexpr int FirstItemType = -1;
constexpr int LastItemType = MAX_ITEM - 1;
// Every model slot an equipped helper can have: no model, the models before the
// items, and every item model slot.
constexpr int FirstModel = -1;
constexpr int LastModel = MODEL_ITEM + MAX_ITEM - 1;

class ShippedItems
{
public:
    ShippedItems() : m_client(ClientDirectory)
    {
        I18N::SetLocale("en");
        std::string errorMessage;
        REQUIRE(g_ItemDataHandler.Load(errorMessage));
    }

private:
    ClientDataScope m_client;
};

ITEM MakeItem(int itemType)
{
    ITEM item{};
    item.Type = static_cast<short>(itemType);
    return item;
}

std::vector<int> HighestFirst(ItemTag tag)
{
    const auto itemTypes = g_ItemDatabase.GetItemTypesWithTag(tag);
    return {itemTypes.rbegin(), itemTypes.rend()};
}
} // namespace

TEST_CASE("3b: the helper lists match the mount, pet and gamble data [data][items][3b]")
{
    ShippedItems items;
    for (int model = FirstModel; model <= LastModel; ++model)
    {
        CAPTURE(model);
        CHECK(OldPowerSlashNotRiding(model) == !GameLogic::Items::IsRideableMountModel(model));

        CHECK(OldHasHelperLifeFrame(model) ==
              (GameLogic::Items::IsPetSlotModel(model) && !GameLogic::Items::IsGambleItemModel(model)));

        for (const bool safeZone : {false, true})
        {
            CAPTURE(safeZone);
            CHECK(OldSendMacroChatAnimates(model, safeZone) ==
                  (!(GameLogic::Items::IsRideableMountModel(model) && model != MODEL_HORN_OF_FENRIR) || safeZone));
            // Always true: the rider check of MoveHero never stopped anything.
            CHECK(OldMoveHeroCanOperate(model, safeZone));
        }
    }

    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        const ITEM item = MakeItem(itemType);
        CHECK(OldIsChaosCastleBannedHelper(itemType) ==
              (GameLogic::Items::IsRideableMount(&item) || GameLogic::Items::IsDarkLordPet(&item)));
    }
}

TEST_CASE("3b: the potion loops match the potion tags [data][items][3b]")
{
    ShippedItems items;
    CHECK(OldManaPotionOrder() == HighestFirst(ItemTag::ManaPotion));
    CHECK(OldHealingPotionOrder() == HighestFirst(ItemTag::HealingPotion));
}

TEST_CASE("3b: the ammunition hands match the ammunition tag [data][items][3b]")
{
    ShippedItems items;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        for (const int hand : {EQUIPMENT_WEAPON_RIGHT, EQUIPMENT_WEAPON_LEFT})
        {
            CAPTURE(itemType);
            CAPTURE(hand);
            // Arrows can't be in the left hand (only the Fairy Elf uses them, and only
            // knights, gladiators, fighters and summoners take a right hand item in
            // the left), bolts can't be in the right (a left hand item).
            const bool unreachable = (itemType == ITEM_ARROWS && hand == EQUIPMENT_WEAPON_LEFT) ||
                                     (itemType == ITEM_BOLT && hand == EQUIPMENT_WEAPON_RIGHT);
            if (!unreachable)
            {
                CHECK(OldSkipsAmmunition(itemType, hand) == GameLogic::Items::IsAmmunitionType(itemType));
            }
        }
    }
    CHECK(g_ItemDatabase.GetSlot(ITEM_ARROWS) == ItemSlot::MainHand);
    CHECK(g_ItemDatabase.GetSlot(ITEM_BOLT) == ItemSlot::OffHand);
}

TEST_CASE("3b: the wing lists match the wing tiers [data][items][3b]")
{
    ShippedItems items;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        const ITEM item = MakeItem(itemType);
        CHECK(OldIsCapeOfLordOrFighter(itemType) == GameLogic::Items::IsSecondTierCape(&item));
        CHECK(OldHasSecondTierWingDefensePerLevel(&item) == GameLogic::Items::IsSecondTierWing(&item));

        const OldWingFlags flags = OldMixWingFlags(itemType);
        const WingTier tier = g_ItemDatabase.GetWingTier(itemType);
        CHECK(flags.isWing == (tier == WingTier::First));
        CHECK(flags.isUpgradedWing == (tier == WingTier::Second));
        CHECK(flags.is3rdUpgradedWing == (tier == WingTier::Third));
    }
}

TEST_CASE("3b: the maximum durability is the same for every item and level [data][items][3b]")
{
    ShippedItems items;
    struct Options
    {
        BYTE excellentFlags;
        BYTE ancientDiscriminator;
    };
    for (int itemType = 0; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        for (const Options options : {Options{0, 0}, Options{0x01, 0}, Options{0, 1}, Options{0x01, 1}})
        {
            ITEM item = MakeItem(itemType);
            item.ExcellentFlags = options.excellentFlags;
            item.AncientDiscriminator = options.ancientDiscriminator;
            for (int level = 0; level <= 15; ++level)
            {
                CAPTURE(level);
                CHECK(OldCalcMaxDurability(&item, &ItemAttribute[itemType], level) ==
                      CalcMaxDurability(&item, &ItemAttribute[itemType], level));
            }
        }
    }
}
