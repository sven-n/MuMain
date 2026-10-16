#pragma once

#include "Core/Globals/_define.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "I18N/All.h"

#include <filesystem>
#include <string>
#include <vector>

extern ITEM_ATTRIBUTE* ItemAttribute;

// CItemDataHandler::Load and the bmd import read Data\... relative to the
// working directory; Load also fills the global ItemAttribute, like the game
// does at startup. This points both at test-owned state and restores them.
class ClientDataScope
{
public:
    explicit ClientDataScope(const std::filesystem::path& clientDirectory)
        : m_items(MAX_ITEM, ITEM_ATTRIBUTE{}), m_previousItems(ItemAttribute),
          m_previousDirectory(std::filesystem::current_path())
    {
        // Set up the log file before switching directories, so no log file
        // is written into the client data folder.
        mu::log::Init();
        ItemAttribute = m_items.data();
        std::filesystem::current_path(clientDirectory);
    }

    ~ClientDataScope()
    {
        // Resetting the locale refills ItemAttribute, so it runs while
        // ItemAttribute still points at the test's table.
        I18N::SetLocale("en");
        std::filesystem::current_path(m_previousDirectory);
        ItemAttribute = m_previousItems;
    }

    std::wstring Name(int itemType) const
    {
        return m_items[itemType].Name;
    }

private:
    std::vector<ITEM_ATTRIBUTE> m_items;
    ITEM_ATTRIBUTE* m_previousItems;
    std::filesystem::path m_previousDirectory;
};
