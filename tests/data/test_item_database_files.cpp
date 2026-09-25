#include "doctest.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemDataLoader.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <filesystem>
#include <string>
#include <vector>

extern ITEM_ATTRIBUTE* ItemAttribute;

using namespace Data::Items;

// These tests read the item files shipped in src/bin/Data, so they also catch
// a broken or missing item file in the repo.
namespace
{
const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;
const std::filesystem::path ShippedClientDirectory = DataDirectory.parent_path();
const std::filesystem::path EnglishItemFile = DataDirectory / "Local" / "Eng" / "item_eng.bmd";
const std::filesystem::path PortugueseItemFile = DataDirectory / "Local" / "Por" / "item_por.bmd";

constexpr int ShortSwordType = MakeItemType(0, 1);

std::vector<ITEM_ATTRIBUTE> LoadItemFile(const std::filesystem::path& path)
{
    std::vector<ITEM_ATTRIBUTE> attributes(MAX_ITEM, ITEM_ATTRIBUTE{});
    REQUIRE(ItemDataLoader::Load(path.wstring().c_str(), attributes.data()));
    return attributes;
}

// CItemDataHandler::Load reads Data\Local\... relative to the working
// directory into the global ItemAttribute, like the game does at startup.
// This points both at test-owned state and restores them afterwards.
class ClientDataScope
{
public:
    explicit ClientDataScope(const std::filesystem::path& clientDirectory)
        : m_items(MAX_ITEM, ITEM_ATTRIBUTE{}),
          m_previousItems(ItemAttribute),
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
        std::filesystem::current_path(m_previousDirectory);
        ItemAttribute = m_previousItems;
    }

    const ITEM_ATTRIBUTE& Item(int itemType) const { return m_items[itemType]; }

private:
    std::vector<ITEM_ATTRIBUTE> m_items;
    ITEM_ATTRIBUTE* m_previousItems;
    std::filesystem::path m_previousDirectory;
};

// A client folder that only has the Portuguese item file.
class ClientWithoutEnglishItems
{
public:
    ClientWithoutEnglishItems()
        : m_directory(std::filesystem::temp_directory_path() / "mu_test_item_database_without_english")
    {
        std::filesystem::remove_all(m_directory);
        const std::filesystem::path portugueseFolder = m_directory / "Data" / "Local" / "Por";
        std::filesystem::create_directories(portugueseFolder);
        std::filesystem::copy_file(PortugueseItemFile, portugueseFolder / "Item_Por.bmd");
    }

    ~ClientWithoutEnglishItems()
    {
        std::error_code ignored;
        std::filesystem::remove_all(m_directory, ignored);
    }

    const std::filesystem::path& Directory() const { return m_directory; }

private:
    std::filesystem::path m_directory;
};
} // namespace

TEST_CASE("Shipped English item file builds the item database [data][items]")
{
    const auto attributes = LoadItemFile(EnglishItemFile);

    ItemDatabase database;
    database.Build(attributes);

    CHECK(database.GetExistingItemCount() > 0);
    CHECK(database.GetLogName(MakeItemType(0, 0)) == "Kris (0,0)");
}

TEST_CASE("Loading English items keeps the English names [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);

    REQUIRE(g_ItemDataHandler.Load(L"Eng"));

    CHECK(g_ItemDatabase.GetLogName(ShortSwordType) == "Short Sword (0,1)");
}

TEST_CASE("Loading a translated item file keeps English log names [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);

    REQUIRE(g_ItemDataHandler.Load(L"Por"));

    const ItemDefinition* shortSword = g_ItemDatabase.Find(ShortSwordType);
    REQUIRE(shortSword != nullptr);
    CHECK(shortSword->name == client.Item(ShortSwordType).Name);
    CHECK(shortSword->englishName == "Short Sword");
    CHECK(g_ItemDatabase.GetLogName(ShortSwordType) == "Short Sword (0,1)");
}

TEST_CASE("Missing English item file falls back to the loaded names [data][items]")
{
    ClientWithoutEnglishItems folder;
    ClientDataScope client(folder.Directory());

    REQUIRE(g_ItemDataHandler.Load(L"Por"));

    const std::string portugueseName = mu_wchar_to_utf8(client.Item(ShortSwordType).Name);
    CHECK(g_ItemDatabase.GetLogName(ShortSwordType) == portugueseName + " (0,1)");
}

TEST_CASE("Loading a language without an item file fails [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);

    CHECK_FALSE(g_ItemDataHandler.Load(L"Xyz"));
}
