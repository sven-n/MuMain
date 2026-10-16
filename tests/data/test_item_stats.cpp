#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Items/ItemCategories.h"
#include "I18N/All.h"

#include <filesystem>
#include <string>

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
