#include "stdafx.h"

#include "doctest.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"

#include <filesystem>
#include <vector>

extern ITEM_ATTRIBUTE* ItemAttribute;
// The item help table (ComputeItemInfo) and the item it was computed for.
extern int g_iCurrentItem;
extern int g_iItemInfo[16][17];

using namespace Data::Items;

// The item help table, the window /<item name> opens, with the item data shipped in src/bin/Data.
namespace
{
constexpr int MaxHelpLevel = 15;

// Loads the shipped items into the item database and a test-owned ItemAttribute, as the game
// does at startup, and gives ComputeItemInfo a hero to mark the levels it can equip. The table
// is marked as computed for no item, so the next call computes it again.
class ItemHelpTableScope
{
public:
    ItemHelpTableScope()
        : m_items(MAX_ITEM, ITEM_ATTRIBUTE{}), m_previousItems(ItemAttribute), m_previousHero(CharacterAttribute)
    {
        mu::log::Init();
        const ItemDataLoadResult result = LoadItemDataDirectory(std::filesystem::path(MU_TEST_DATA_DIR) / "Items");
        REQUIRE_FALSE(HasErrors(result.issues));
        g_ItemDatabase.Build(result.items);
        for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
        {
            const ItemDefinition* definition = g_ItemDatabase.Find(itemType);
            if (definition != nullptr)
            {
                ToItemAttribute(*definition, m_items[itemType]);
            }
        }

        ItemAttribute = m_items.data();
        CharacterAttribute = &m_hero;
        g_iCurrentItem = -1;
    }

    ~ItemHelpTableScope()
    {
        g_iCurrentItem = -1;
        ItemAttribute = m_previousItems;
        CharacterAttribute = m_previousHero;
    }

private:
    std::vector<ITEM_ATTRIBUTE> m_items;
    ITEM_ATTRIBUTE* m_previousItems;
    CHARACTER_ATTRIBUTE m_hero{};
    CHARACTER_ATTRIBUTE* m_previousHero;
};

struct MagicColumnCase
{
    int itemType;
    int column;
};

// The column an item's magic power goes into: Curse for a summoner book, Attack for a sceptre,
// WIZDmg for a staff.
constexpr MagicColumnCase MagicColumnCases[] = {
    {ITEM_BOOK_OF_SAHAMUTT, _COLUMN_TYPE_CURSE},
    {ITEM_GREAT_LORD_SCEPTER, _COLUMN_TYPE_PET_ATTACK},
    {ITEM_LEGENDARY_STAFF, _COLUMN_TYPE_MAGIC},
};

constexpr int MagicColumns[] = {_COLUMN_TYPE_MAGIC, _COLUMN_TYPE_CURSE, _COLUMN_TYPE_PET_ATTACK};
} // namespace

TEST_CASE("The item help table shows only the magic column of the item it shows [data][items]")
{
    ItemHelpTableScope help;

    // g_iItemInfo still holds the item shown before: /Book of Samut and then /Legendary Staff
    // must not show a Curse column under the staff, nor the other way round a WIZDmg column.
    for (const MagicColumnCase& before : MagicColumnCases)
    {
        for (const MagicColumnCase& shown : MagicColumnCases)
        {
            if (shown.itemType == before.itemType)
            {
                continue;
            }

            CAPTURE(before.itemType);
            CAPTURE(shown.itemType);
            ComputeItemInfo(before.itemType);
            ComputeItemInfo(shown.itemType);
            for (int level = 0; level <= MaxHelpLevel; ++level)
            {
                CAPTURE(level);
                CHECK(g_iItemInfo[level][shown.column] > 0);
                for (const int column : MagicColumns)
                {
                    if (column != shown.column)
                    {
                        CAPTURE(column);
                        CHECK(g_iItemInfo[level][column] == 0);
                    }
                }
            }
        }
    }
}
