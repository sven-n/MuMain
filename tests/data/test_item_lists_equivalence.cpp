// One-time check for phase 3b-2 of the data-driven items plan: the lists and group
// ranges of bows, crossbows, spears and shields. This file keeps a copy of the old
// code (item_lists_old.inl) and checks that the weapon kind queries that replace it
// give the same answers for every item type and item model slot. The group ranges
// also covered the numbers of their group that have no item data (items the client
// does not know: no name, no model); the weapon kind does not, and the checks spell
// that out. It is not meant to stay: once the data is the source of truth, a data
// change would make it fail.

#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "GameLogic/Items/ItemCategories.h"

#include <filesystem>
#include <string>
#include <type_traits>

#include "item_lists_old.inl"

using namespace Data::Items;

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

// Every item type, and -1 for no item.
constexpr int FirstItemType = -1;
constexpr int LastItemType = MAX_ITEM - 1;
// Every model slot an equipped weapon can have: no model, the models before the
// items, and every item model slot.
constexpr int FirstModel = -1;
constexpr int LastModel = MODEL_ITEM + MAX_ITEM - 1;

constexpr int SpearGroup = ITEM_SPEAR / MAX_ITEM_INDEX;
constexpr int BowGroup = ITEM_BOW / MAX_ITEM_INDEX;
constexpr int ShieldGroup = ITEM_SHIELD / MAX_ITEM_INDEX;

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

// A number of the group that has no item data, which only the old group ranges count.
bool IsUnknownItemOfGroup(int itemType, int group)
{
    return itemType >= 0 && itemType / MAX_ITEM_INDEX == group && g_ItemDatabase.Find(itemType) == nullptr;
}
} // namespace

TEST_CASE("3b-2: the bow and crossbow lists match the weapon kinds [data][items][3b]")
{
    ShippedItems items;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        CHECK(OldIsGeneralBowItem(itemType) == GameLogic::Items::IsBowType(itemType));
        CHECK(OldIsGeneralCrossbowItem(itemType) == GameLogic::Items::IsCrossbowType(itemType));
    }
    for (int model = FirstModel; model <= LastModel; ++model)
    {
        CAPTURE(model);
        CHECK(OldIsBowModel(model) == GameLogic::Items::IsBowModel(model));
        CHECK(OldIsCrossbowModel(model) == GameLogic::Items::IsCrossbowModel(model));
    }
}

TEST_CASE("3b-2: the bow group checks match bows, crossbows and ammunition [data][items][3b]")
{
    ShippedItems items;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        const ITEM item = MakeItem(itemType);
        const bool unknown = IsUnknownItemOfGroup(itemType, BowGroup);
        CHECK(OldIsBowGroupItem(itemType) == (GameLogic::Items::IsBowCrossbowOrAmmunition(&item) || unknown));
        CHECK(OldElfLeftHandTakesOnlyArrows(itemType) ==
              ((itemType != ITEM_BOLT && GameLogic::Items::IsBowCrossbowOrAmmunition(&item)) || unknown));
    }
}

TEST_CASE("3b-2: the shield and spear ranges match the weapon kinds [data][items][3b]")
{
    ShippedItems items;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        CHECK(OldIsShieldInLeftHand(itemType) ==
              (GameLogic::Items::IsShieldType(itemType) || IsUnknownItemOfGroup(itemType, ShieldGroup)));

        const bool spear = GameLogic::Items::IsSpearType(itemType) || IsUnknownItemOfGroup(itemType, SpearGroup);
        CHECK(OldHasSpearReach(itemType) == spear);
        // Either hand, with nothing in the other.
        CHECK(OldImpaleUnusableWhileRiding(itemType, -1) == !spear);
        CHECK(OldImpaleUnusableWhileRiding(-1, itemType) == !spear);
    }
    for (int model = FirstModel; model <= LastModel; ++model)
    {
        CAPTURE(model);
        CHECK(OldImpaleHasSpearModel(model) ==
              (GameLogic::Items::IsSpearModel(model) || IsUnknownItemOfGroup(ToItemType(model), SpearGroup)));
    }
}
