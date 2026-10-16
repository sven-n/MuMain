#include "stdafx.h"
#include "doctest.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

#include <filesystem>
#include <string>
#include <vector>

extern ITEM_ATTRIBUTE* ItemAttribute;

using namespace Data::Items;

// What SetItemAttributes computes from the shipped item data: the values the item
// tooltip and the character window show. The expected values are the ones OpenMU
// computes for the same items.
namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

constexpr int DragonShieldType = MakeItemType(6, 13);
constexpr int BronzeArmorType = MakeItemType(8, 0);

// Loads the shipped item data into a test-owned ItemAttribute, as the game does at
// startup, and restores the previous one.
class ShippedItemAttributes
{
public:
    ShippedItemAttributes()
        : m_items(MAX_ITEM, ITEM_ATTRIBUTE{}), m_previousItems(ItemAttribute),
          m_previousDirectory(std::filesystem::current_path())
    {
        mu::log::Init();
        ItemAttribute = m_items.data();
        std::filesystem::current_path(ClientDirectory);
        I18N::SetLocale("en");
        std::string errorMessage;
        REQUIRE(g_ItemDataHandler.Load(errorMessage));
    }

    ~ShippedItemAttributes()
    {
        // Resetting the locale refills ItemAttribute, so it runs while ItemAttribute
        // still points at the test's table.
        I18N::SetLocale("en");
        std::filesystem::current_path(m_previousDirectory);
        ItemAttribute = m_previousItems;
    }

    ITEM_ATTRIBUTE& operator[](int itemType)
    {
        return m_items[itemType];
    }

private:
    std::vector<ITEM_ATTRIBUTE> m_items;
    ITEM_ATTRIBUTE* m_previousItems;
    std::filesystem::path m_previousDirectory;
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
    items[BronzeArmorType].Level = 50;

    CHECK(ComputeItem(BronzeArmorType, 0).MagicDefense == 10);
    CHECK(ComputeItem(BronzeArmorType, 9).MagicDefense == 37);
}
