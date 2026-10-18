// One-time check for phase 3b-3 of the data-driven items plan: the group ranges and
// lists of swords, gloves, axes, maces, staffs, books, sceptres and magic swords. This
// file keeps a copy of the old code (item_lists_old.inl) and checks that the weapon
// kind and tag queries that replace it give the same answers for every item type and
// item model slot. The group ranges also covered the numbers of their groups that have
// no item data (items the client cannot name; only 5/32 has a model, and OpenMU does
// not define it); the weapon kind does not, and the checks spell that out. It is not meant to stay: once the data is
// the source of truth, a data change would make it fail.

#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "GameLogic/Items/ItemCategories.h"

#include <filesystem>
#include <string>

#include "item_lists_old.inl"

using namespace Data::Items;
using namespace GameLogic::Items;

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

constexpr int SwordGroup = ITEM_SWORD / MAX_ITEM_INDEX;
constexpr int MaceGroup = ITEM_MACE / MAX_ITEM_INDEX;
constexpr int StaffGroup = ITEM_STAFF / MAX_ITEM_INDEX;

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

// A number of the groups that has no item data, which only the old group ranges count.
bool IsUnknownItemOfGroups(int itemType, int firstGroup, int lastGroup)
{
    if (itemType < 0 || g_ItemDatabase.Find(itemType) != nullptr)
    {
        return false;
    }
    const int group = itemType / MAX_ITEM_INDEX;
    return group >= firstGroup && group <= lastGroup;
}
} // namespace

TEST_CASE("3b-3: the sceptre and magic sword lists match the tags [data][items][3b]")
{
    ShippedItems items;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        const ITEM item = MakeItem(itemType);
        CHECK(OldIsCepterItem(itemType) == IsSceptreType(itemType));
        CHECK(OldIsRightMagicSword(&item) == IsMagicSwordType(itemType));
        CHECK(OldTooltipWizardryDamageOption(&item) == IsMagicSwordType(itemType));
        // These checks see a weapon with something in the other hand, so a one-handed one.
        // The two-handed magic swords count now as well, which the hands never allow.
        const bool twoHandedMagicSword = IsMagicSwordType(itemType) && g_ItemDatabase.Find(itemType)->twoHanded;
        CHECK(OldIsImperialSword(itemType) == (IsMagicSwordType(itemType) && !twoHandedMagicSword));
    }
}

TEST_CASE("3b-3: the staff group checks match staffs and books [data][items][3b]")
{
    ShippedItems items;
    const int kris = ITEM_SWORD;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        const ITEM item = MakeItem(itemType);
        const bool staff = IsStaffOrBookType(itemType) || IsUnknownItemOfGroups(itemType, StaffGroup, StaffGroup);
        CHECK(OldRightStaffAddsDamageLeft(&item) == staff);
        CHECK(OldRightStaffShowsLeftDamage(&item) == staff);
        CHECK(OldSummonerTakesInLeftHand(&item) == !staff);
        CHECK(OldWizardryWeaponRight(&item) == (staff || IsMagicSwordType(itemType)));
        CHECK(OldTooltipMagicPowerLine(&item) == (staff || IsMagicSwordType(itemType)));
        // Either hand, with nothing or a sword in the other.
        CHECK(OldWeaponWithoutStaff(-1, itemType) == (itemType != -1 && !staff));
        CHECK(OldWeaponWithoutStaff(itemType, kris) == !staff);
    }
    for (int model = FirstModel; model <= LastModel; ++model)
    {
        CAPTURE(model);
        const bool staff =
            IsStaffOrBookModel(model) || IsUnknownItemOfGroups(ToItemType(model), StaffGroup, StaffGroup);
        CHECK(OldDeathStabRefused(model) == !(model != -1 && !staff));
        CHECK(OldDeathCannonRefused(model) == !staff);
    }
}

TEST_CASE("3b-3: the sword, axe and mace group checks match the weapon kinds [data][items][3b]")
{
    ShippedItems items;
    for (int itemType = FirstItemType; itemType <= LastItemType; ++itemType)
    {
        CAPTURE(itemType);
        const ITEM item = MakeItem(itemType);
        const bool meleeWeapon =
            IsSwordAxeMaceOrGloveType(itemType) || IsUnknownItemOfGroups(itemType, SwordGroup, MaceGroup);
        CHECK(OldRageFighterDualWieldHand(&item) == meleeWeapon);
        CHECK(OldLeftWeaponAddsMagic(&item) ==
              (IsWeaponOrAmmunition(&item) || IsUnknownItemOfGroups(itemType, SwordGroup, StaffGroup)));
        CHECK(OldDualWieldHand(&item) ==
              (IsWeaponOrAmmunition(&item) || IsUnknownItemOfGroups(itemType, SwordGroup, StaffGroup)));
        // MoveHero asks for a staff now, so Book of Neil-J (a book without the summonerBook tag)
        // is no longer moved into the right hand.
        const bool bookOfNeilJ = itemType == ITEM_STAFF + 40;
        const bool staff = IsStaffType(itemType) || IsUnknownItemOfGroups(itemType, StaffGroup, StaffGroup);
        CHECK(OldMoveHeroMovesLeftWeapon(itemType) == (meleeWeapon || staff || bookOfNeilJ));
    }
    for (int model = FirstModel; model <= LastModel; ++model)
    {
        CAPTURE(model);
        const bool sword =
            IsSwordOrGloveModel(model) || IsUnknownItemOfGroups(ToItemType(model), SwordGroup, SwordGroup);
        CHECK(OldSpiralSlashRefused(model) == !sword);
    }
}
