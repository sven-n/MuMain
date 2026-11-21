#pragma once

#include "doctest.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"

#include <filesystem>
#include <vector>

// Fills the global item database with the shipped item list for a test and puts the items it had back when it goes,
// also when one of its checks stops it. The Items folder is read once for all tests.
class ShippedItemsScope
{
public:
    ShippedItemsScope()
    {
        for (const Data::Items::ItemDefinition& slot : g_ItemDatabase.GetAllSlots())
        {
            if (slot.Exists())
            {
                m_previous.push_back(slot);
            }
        }
        const Data::Items::ItemDataLoadResult& items = Shipped();
        REQUIRE_FALSE(Data::Items::HasFatal(items.issues));
        g_ItemDatabase.Build(items.items);
    }

    ~ShippedItemsScope()
    {
        g_ItemDatabase.Build(m_previous);
    }

    ShippedItemsScope(const ShippedItemsScope&) = delete;
    ShippedItemsScope& operator=(const ShippedItemsScope&) = delete;

private:
    static const Data::Items::ItemDataLoadResult& Shipped()
    {
        static const Data::Items::ItemDataLoadResult items = []
        {
            mu::log::Init();
            return Data::Items::LoadItemDataDirectory(std::filesystem::path(MU_TEST_DATA_DIR) / "Items");
        }();
        return items;
    }

    std::vector<Data::Items::ItemDefinition> m_previous;
};
