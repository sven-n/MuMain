#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemCategories.h"
#include "I18N/All.h"

#include <filesystem>
#include <iterator>
#include <string>

// The item help table (ComputeItemInfo) and the item it was computed for.
extern int g_iCurrentItem;
extern int g_iItemInfo[16][17];

using namespace Data::Items;

// What SetItemAttributes computes from the shipped item data: the values the item
// tooltip and the character window show. The values the fixes of #683 changed are
// the ones the client before 815b8828 and OpenMU compute; the "Unchanged" checks
// pin the client's other values.
namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

constexpr int MorningStarType = MakeItemType(2, 1);
constexpr int SmallShieldType = MakeItemType(6, 0);
constexpr int DragonShieldType = MakeItemType(6, 13);
constexpr int BronzeArmorType = MakeItemType(8, 0);
constexpr int LeatherArmorType = MakeItemType(8, 5);
constexpr int StormCrowArmorType = MakeItemType(8, 15);
constexpr int FirstSmallWingType = MakeItemType(12, 130);
constexpr int LastSmallWingType = MakeItemType(12, 135);

// Loads the shipped item data into a test-owned ItemAttribute, as the game does at
// startup; the client data scope restores the previous one, also when loading fails.
class ShippedItemAttributes
{
public:
    ShippedItemAttributes() : m_client(ClientDirectory)
    {
        I18N::SetLocale("en");
        std::string errorMessage;
        REQUIRE(g_ItemDataHandler.Load(errorMessage));
    }

    ITEM_ATTRIBUTE& operator[](int itemType)
    {
        return ItemAttribute[itemType];
    }

private:
    ClientDataScope m_client;
};

// Points the item help table at test-owned state: ComputeItemInfo reads the item from
// ItemHelp and marks the levels the hero can equip. The table was computed from the test's
// ItemAttribute, so it is marked as computed for no item afterwards.
class ItemHelpScope
{
public:
    ItemHelpScope() : m_previousItemHelp(ItemHelp), m_previousHero(CharacterAttribute)
    {
        CharacterAttribute = &m_hero;
    }

    ~ItemHelpScope()
    {
        g_iCurrentItem = -1;
        ItemHelp = m_previousItemHelp;
        CharacterAttribute = m_previousHero;
    }

    void Compute(int itemType)
    {
        ItemHelp = itemType;
        g_iCurrentItem = -1;
        ComputeItemInfo(itemType);
    }

private:
    CHARACTER_ATTRIBUTE m_hero{};
    int m_previousItemHelp;
    CHARACTER_ATTRIBUTE* m_previousHero;
};

ITEM ComputeItem(int itemType, int level, BYTE excellentFlags = 0, BYTE ancientDiscriminator = 0)
{
    ITEM item{};
    item.Type = static_cast<short>(itemType);
    item.Level = level;
    item.ExcellentFlags = excellentFlags;
    item.AncientDiscriminator = ancientDiscriminator;
    SetItemAttributes(&item);
    return item;
}
} // namespace

TEST_CASE("A shield's defense rate grows with its + level [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(DragonShieldType, 0).SuccessfulBlocking == 60);
    CHECK(ComputeItem(DragonShieldType, 9).SuccessfulBlocking == 87);

    for (int number = 0; number < MAX_ITEM_INDEX; ++number)
    {
        const int shieldType = MakeItemType(6, number);
        if (items[shieldType].SuccessfulBlocking == 0)
        {
            continue;
        }
        CAPTURE(number);
        for (int level = 0; level < 15; ++level)
        {
            CAPTURE(level);
            CHECK(ComputeItem(shieldType, level + 1).SuccessfulBlocking >
                  ComputeItem(shieldType, level).SuccessfulBlocking);
        }
    }
}

TEST_CASE("Magic defense grows with the + level [data][items]")
{
    ShippedItemAttributes items;
    // No shipped item has magic defense; the editor can give one some.
    items[BronzeArmorType].MagicDefense = 10;

    CHECK(ComputeItem(BronzeArmorType, 0).MagicDefense == 10);
    CHECK(ComputeItem(BronzeArmorType, 9).MagicDefense == 37);
    CHECK(ComputeItem(BronzeArmorType, 15).MagicDefense == 76);
}

TEST_CASE("The late wings, crystals and small wings keep their level requirement [data][items]")
{
    ShippedItemAttributes items;

    for (int level = 0; level <= 15; ++level)
    {
        CAPTURE(level);
        CHECK(ComputeItem(ITEM_WING_OF_DIMENSION, level).RequireLevel == 400);
        CHECK(ComputeItem(ITEM_CAPE_OF_OVERRULE, level).RequireLevel == 400);
        for (int type = ITEM_CRYSTAL_OF_DESTRUCTION; type <= ITEM_SCROLL_OF_CHAOTIC_DISEIER; ++type)
        {
            CAPTURE(type);
            CHECK(ComputeItem(type, level).RequireLevel == 220);
        }
        for (int type = FirstSmallWingType; type <= LastSmallWingType; ++type)
        {
            CAPTURE(type);
            CHECK(ComputeItem(type, level).RequireLevel == 1);
        }

        // Unchanged: the other third tier wings keep theirs, the Cape of Fighter's grows.
        CHECK(ComputeItem(ITEM_WING_OF_STORM, level).RequireLevel == 400);
        CHECK(ComputeItem(ITEM_CAPE_OF_EMPEROR, level).RequireLevel == 400);
        CHECK(ComputeItem(ITEM_CAPE_OF_FIGHTER, level).RequireLevel == 180 + level * 4);
    }
}

TEST_CASE("A wing's option does not count as excellent [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(ITEM_WING_OF_STORM, 0, 0x01).Defense == 60);
    CHECK(ComputeItem(ITEM_CAPE_OF_LORD, 0, 0x01).Defense == 15);

    for (int type = MakeItemType(12, 0); type < MakeItemType(14, 0); ++type)
    {
        ITEM wing{};
        wing.Type = static_cast<short>(type);
        if (items[type].Name[0] == L'\0' || !GameLogic::Items::IsWingItem(&wing))
        {
            continue;
        }
        CAPTURE(type);
        for (int level = 0; level <= 15; ++level)
        {
            CAPTURE(level);
            CHECK(ComputeItem(type, level, 0x01).Defense == ComputeItem(type, level).Defense);
        }
    }
}

TEST_CASE("Ancient items get the excellent bonus and their base defense once [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(LeatherArmorType, 0, 0, 1).Defense == 33);
    CHECK(ComputeItem(StormCrowArmorType, 0, 0, 1).Defense == 76);
    CHECK(ComputeItem(SmallShieldType, 0, 0, 1).SuccessfulBlocking == 33);
    CHECK(ComputeItem(MorningStarType, 0, 0, 1).DamageMin == 49);

    // Unchanged: excellent armor, and the stat requirements of ancient items.
    CHECK(ComputeItem(LeatherArmorType, 0, 0x01).Defense == 28);
    CHECK(ComputeItem(BronzeArmorType, 0, 0, 1).RequireStrength == 135);
}

TEST_CASE("The excellent bonus adds the same to minimum and maximum damage [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(MorningStarType, 0, 0, 1).DamageMax == 58);

    for (int type = MakeItemType(0, 0); type < MakeItemType(6, 0); ++type)
    {
        if (items[type].DamageMin <= 0 || items[type].DamageMax <= 0 || items[type].Level == 0)
        {
            continue;
        }
        CAPTURE(type);
        const ITEM normal = ComputeItem(type, 0);
        const ITEM excellent = ComputeItem(type, 0, 0x01);
        CHECK(excellent.DamageMax - normal.DamageMax == excellent.DamageMin - normal.DamageMin);
    }
}

TEST_CASE("The + level bonus adds up above +9 [data][items]")
{
    ShippedItemAttributes items;
    // The Season 6 client and OpenMU: 27 at +9, then 31, 36, 42, 49, 57 and 66 at +10 to +15.
    constexpr int FirstLevelAboveNine = 10;
    constexpr int BonusFromTen[] = {31, 36, 42, 49, 57, 66};

    for (int i = 0; i < static_cast<int>(std::size(BonusFromTen)); ++i)
    {
        const int level = FirstLevelAboveNine + i;
        CAPTURE(level);
        CHECK(ComputeItem(ITEM_DOUBLE_BLADE, level).DamageMin == items[ITEM_DOUBLE_BLADE].DamageMin + BonusFromTen[i]);
        CHECK(ComputeItem(ITEM_DOUBLE_BLADE, level).DamageMax == items[ITEM_DOUBLE_BLADE].DamageMax + BonusFromTen[i]);
        CHECK(ComputeItem(ITEM_DRAGON_ARMOR, level).Defense == items[ITEM_DRAGON_ARMOR].Defense + BonusFromTen[i]);
        CHECK(ComputeItem(DragonShieldType, level).SuccessfulBlocking ==
              items[DragonShieldType].SuccessfulBlocking + BonusFromTen[i]);
    }

    // Magic power is halved after the bonus; staffs add 2 per level, sceptres don't (OpenMU's
    // staff and sceptre rise tables).
    CHECK(ComputeItem(ITEM_LEGENDARY_STAFF, 15).MagicPower == 92);
    CHECK(ComputeItem(ITEM_GREAT_LORD_SCEPTER, 15).MagicPower == 66);
    CHECK(ComputeItem(ITEM_DIVINE_STAFF_OF_ARCHANGEL, 15).MagicPower == 141);
    // With the excellent bonus the sum passes 255 before the halving.
    CHECK(ComputeItem(ITEM_DIVINE_STAFF_OF_ARCHANGEL, 15, 0x01).MagicPower == 162);
}

TEST_CASE("Wing defense adds up above +9, from 5 for the third tier wings [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(ITEM_WING_OF_STORM, 10).Defense == 101);
    CHECK(ComputeItem(ITEM_WINGS_OF_DRAGON, 10).Defense == 67);

    CHECK(ComputeItem(ITEM_WING_OF_STORM, 15).Defense == 141);
    CHECK(ComputeItem(ITEM_WINGS_OF_DRAGON, 15).Defense == 102);
    CHECK(ComputeItem(ITEM_WINGS_OF_SATAN, 15).Defense == 86);
    CHECK(ComputeItem(ITEM_CAPE_OF_LORD, 15).Defense == 72);
}

TEST_CASE("The item help table shows the stats the item has at each level [data][items]")
{
    ShippedItemAttributes items;
    ItemHelpScope help;

    help.Compute(ITEM_DOUBLE_BLADE);
    CHECK(g_iItemInfo[15][_COLUMN_TYPE_ATTMIN] == items[ITEM_DOUBLE_BLADE].DamageMin + 66);

    for (const int type :
         {static_cast<int>(ITEM_DOUBLE_BLADE), static_cast<int>(ITEM_LEGENDARY_STAFF),
          static_cast<int>(ITEM_GREAT_LORD_SCEPTER), static_cast<int>(ITEM_DRAGON_ARMOR), DragonShieldType})
    {
        CAPTURE(type);
        help.Compute(type);
        const int magicColumn = IsCepterItem(type) ? _COLUMN_TYPE_PET_ATTACK : _COLUMN_TYPE_MAGIC;
        for (int level = 0; level <= 15; ++level)
        {
            CAPTURE(level);
            const ITEM item = ComputeItem(type, level);
            CHECK(g_iItemInfo[level][_COLUMN_TYPE_ATTMIN] == item.DamageMin);
            CHECK(g_iItemInfo[level][_COLUMN_TYPE_ATTMAX] == item.DamageMax);
            CHECK(g_iItemInfo[level][magicColumn] == item.MagicPower);
            CHECK(g_iItemInfo[level][_COLUMN_TYPE_DEFENCE] == item.Defense);
            CHECK(g_iItemInfo[level][_COLUMN_TYPE_DEFRATE] == item.SuccessfulBlocking);
        }
    }
}
