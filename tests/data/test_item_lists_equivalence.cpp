// One-time check for phase 3b-4 of the data-driven items plan: the lucky set armour
// range, the transformation ring lists, the order in which potions are taken and the
// items the hotkeys take. This file keeps a copy of the old code (item_lists_old.inl)
// and checks that the tag queries and the potion use order that replace it give the
// same answers for every item type.
// The lucky set range also covered the two numbers in it that have no item data (7/71
// and 10/72; the client cannot name them and OpenMU does not define them); the tag does
// not, and the checks spell that out. It is not meant to stay: once the data is the
// source of truth, a data change would make it fail.

#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "GameLogic/Items/ItemCategories.h"
#include "UI/HUD/ItemHotKey.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

#include "item_lists_old.inl"

using namespace Data::Items;
using namespace GameLogic::Items;

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

// Every item type, and -1 for no item.
constexpr int FirstItemType = -1;
constexpr int LastItemType = MAX_ITEM - 1;

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

// A number of the lucky set range without item data, which only the old range counts.
bool IsUnknownLuckySetNumber(int itemType)
{
    if (itemType < 0 || g_ItemDatabase.Find(itemType) != nullptr)
    {
        return false;
    }
    const int group = itemType / MAX_ITEM_INDEX;
    const int number = itemType % MAX_ITEM_INDEX;
    return group >= ITEM_HELM / MAX_ITEM_INDEX && group <= ITEM_BOOTS / MAX_ITEM_INDEX && number >= 62 && number <= 72;
}

std::vector<int> ItemTypesInUseOrder(ItemTag tag)
{
    const auto itemTypes = g_ItemDatabase.GetItemTypesWithTagInUseOrder(tag);
    return std::vector<int>(itemTypes.begin(), itemTypes.end());
}
} // namespace

TEST_CASE("3b-4: the lucky set checks match the luckyItem tag [data][items][3b]")
{
    ShippedItems items;
    int unknownNumbers = 0;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        ITEM item = MakeItem(itemType);
        const bool unknown = IsUnknownLuckySetNumber(itemType);
        unknownNumbers += unknown ? 1 : 0;
        CHECK(OldLuckySet::OldCheck_LuckyItem(itemType) == (IsLuckyItemType(itemType) || unknown));
        CHECK(OldCheck_LuckyItem_Refinery(&item) == (IsLuckyItemType(itemType) || unknown));
    }
    CHECK(unknownNumbers == 2);
}

TEST_CASE("3b-4: the transformation ring checks match the transformationRing tag [data][items][3b]")
{
    ShippedItems items;
    const short none = -1;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        const auto ring = static_cast<short>(itemType);
        CHECK(OldCheckChangeRing(ring) == IsTransformationRingType(itemType));
        CHECK(OldCheckChangeRing(ring) == g_ChangeRingMgr->CheckChangeRing(ring));
        // Either hand, with nothing or a ring in the other.
        for (const short other :
             {none, static_cast<short>(ITEM_PANDA_TRANSFORMATION_RING), static_cast<short>(ITEM_TRANSFORMATION_RING)})
        {
            CAPTURE(other);
            CHECK(OldCheckMoveMap(ring, other) == g_ChangeRingMgr->CheckMoveMap(ring, other));
            CHECK(OldCheckMoveMap(other, ring) == g_ChangeRingMgr->CheckMoveMap(other, ring));
            CHECK(OldCheckBanMoveIcarusMap(ring, other) == g_ChangeRingMgr->CheckBanMoveIcarusMap(ring, other));
            CHECK(OldCheckBanMoveIcarusMap(other, ring) == g_ChangeRingMgr->CheckBanMoveIcarusMap(other, ring));
        }
        // The panda and skeleton rings may enter Icarus.
        CHECK(OldCheckBanMoveIcarusMap(ring, none) ==
              (IsTransformationRingType(itemType) && !IsPandaOrSkeletonType(itemType)));
    }
}

TEST_CASE("3b-4: the potions are taken in the same order [data][items][3b]")
{
    ShippedItems items;
    for (const ItemTag tag :
         {ItemTag::HealingPotion, ItemTag::ManaPotion, ItemTag::ShieldPotion, ItemTag::ComplexPotion})
    {
        CAPTURE(static_cast<int>(tag));
        CHECK(OldItemTypesWithTagHighestFirst(tag) == ItemTypesInUseOrder(tag));
    }
}

TEST_CASE("3b-4: the hotkeys take the same items [data][items][3b]")
{
    ShippedItems items;
    for (int hotKey = mu::ui::window::HOTKEY_Q; hotKey < mu::ui::window::HOTKEY_COUNT; ++hotKey)
    {
        CAPTURE(hotKey);
        for (int boundType = FirstItemType; boundType <= LastItemType; ++boundType)
        {
            CAPTURE(boundType);
            // Remedy of Love counts as one item only at level 0.
            for (const int boundLevel : {0, 1})
            {
                CAPTURE(boundLevel);
                const auto itemTypes = mu::ui::window::GetHotKeyItemTypes(hotKey, boundType, boundLevel);
                CHECK(OldHotKey::OldHotKeyItemTypes(hotKey, boundType, boundLevel) ==
                      std::vector<int>(itemTypes.begin(), itemTypes.end()));
            }
        }
    }
}
