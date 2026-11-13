#pragma once

#include "../core/ContentMemoryStorage.h"
#include "Core/Content/ContentStore.h"
#include "Core/Globals/_define.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "Data/Translation/ItemTextExpansion.h"
#include "I18N/All.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

extern ITEM_ATTRIBUTE* ItemAttribute;

// CItemDataHandler::Load and the bmd import read Data\... relative to the
// working directory; Load also fills the global ItemAttribute, like the game
// does at startup. This points both at test-owned state and restores them.
// A test that switches the UI language needs one too: once a test has loaded
// the item data, every language switch refills ItemAttribute, which is null
// outside a scope.
//
// With MU_TEST_CONTENT=storage, a scope over the shipped client folder reads its content from a storage of the
// test's own (ContentMemoryStorage, as a port's backend would serve it) and leaves the working directory where it
// is, so a loader that goes around the content layer finds nothing.
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
        if (ReadsShippedContentFromOwnStorage(clientDirectory))
        {
            Content::Open(ContentMemoryStorage::OpenFolder(clientDirectory));
            m_openedContent = true;
        }
        else
        {
            std::filesystem::current_path(clientDirectory);
        }
    }

    ~ClientDataScope()
    {
        // Loading the item data let the texts that name items take their names
        // from it; later tests see them as written again.
        Data::Translation::ItemTexts::SetNameResolver(nullptr);
        // Resetting the locale refills ItemAttribute, so it runs while
        // ItemAttribute still points at the test's table.
        I18N::SetLocale("en");
        std::filesystem::current_path(m_previousDirectory);
        if (m_openedContent)
        {
            Content::Close();
        }
        ItemAttribute = m_previousItems;
    }

    std::wstring Name(int itemType) const
    {
        return m_items[itemType].Name;
    }

private:
    static bool ReadsShippedContentFromOwnStorage(const std::filesystem::path& clientDirectory)
    {
#ifdef MU_TEST_DATA_DIR
        const char* mode = std::getenv("MU_TEST_CONTENT");
        std::error_code error;
        return mode != nullptr && std::string_view(mode) == "storage" &&
               std::filesystem::equivalent(clientDirectory, std::filesystem::path(MU_TEST_DATA_DIR).parent_path(),
                                           error);
#else
        (void)clientDirectory;
        return false;
#endif
    }

    std::vector<ITEM_ATTRIBUTE> m_items;
    ITEM_ATTRIBUTE* m_previousItems;
    std::filesystem::path m_previousDirectory;
    bool m_openedContent = false;
};
