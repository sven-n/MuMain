#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_crypt.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

extern ITEM_ATTRIBUTE* ItemAttribute;

using namespace Data::Items;

// These tests read the data shipped in src/bin/Data, so they also catch
// broken item data in the repo before it is merged.
namespace
{
const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;
const std::filesystem::path ShippedClientDirectory = DataDirectory.parent_path();

constexpr int KrisType = MakeItemType(0, 0);
constexpr int BladeType = MakeItemType(0, 5);
constexpr int ChaosCastleTicketType = MakeItemType(13, 121);
constexpr int GaionsOrderType = MakeItemType(14, 102);
constexpr BYTE NoSlot = 255;
constexpr WORD ItemFileChecksumKey = 0xE2F1;

// CItemDataHandler::Load and the bmd import read Data\... relative to the
// working directory; Load also fills the global ItemAttribute, like the game
// does at startup. This points both at test-owned state and restores them.
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
        // Resetting the locale refills ItemAttribute, so it runs while
        // ItemAttribute still points at the test's table.
        I18N::SetLocale("en");
        std::filesystem::current_path(m_previousDirectory);
        ItemAttribute = m_previousItems;
    }

    std::wstring Name(int itemType) const { return m_items[itemType].Name; }

private:
    std::vector<ITEM_ATTRIBUTE> m_items;
    ITEM_ATTRIBUTE* m_previousItems;
    std::filesystem::path m_previousDirectory;
};

// A client folder whose item data is written by the test.
class TemporaryClientFolder
{
public:
    TemporaryClientFolder()
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_item_data_" + std::to_string(std::random_device{}())))
    {
        std::filesystem::remove_all(m_directory);
        std::filesystem::create_directories(m_directory / GetItemDataDirectory());
    }

    ~TemporaryClientFolder()
    {
        std::error_code ignored;
        std::filesystem::remove_all(m_directory, ignored);
    }

    void WriteItemFile(const std::string& fileName, const std::string& text) const
    {
        std::ofstream(m_directory / GetItemDataDirectory() / fileName, std::ios::binary) << text;
    }

    const std::filesystem::path& Directory() const { return m_directory; }

    // Data/Local/<language>/Item_<language>.bmd in this folder; creates the
    // language folder. CItemDataHandler::GetItemFilePath uses backslashes,
    // which only the game's file functions turn into folders on Linux and
    // macOS, so the path is built from its parts here.
    std::filesystem::path LegacyItemFilePath(const std::wstring& language) const
    {
        const std::filesystem::path folder = m_directory / "Data" / "Local" / language;
        std::filesystem::create_directories(folder);
        return folder / (L"Item_" + language + L".bmd");
    }

    // The shipped Data/Items, and empty folders for the bmd export.
    void CopyShippedItems() const
    {
        std::filesystem::copy(DataDirectory / "Items", m_directory / GetItemDataDirectory(),
                              std::filesystem::copy_options::recursive |
                                  std::filesystem::copy_options::overwrite_existing);
        for (const ItemBmdLanguage& language : GetItemBmdLanguages())
        {
            std::filesystem::create_directories(m_directory / "Data" / "Local" / language.folder);
        }
    }

private:
    std::filesystem::path m_directory;
};

// A legacy Item_<language>.bmd with 30-byte names, as the original client
// data has it. The repo does not ship these files, so tests build them.
class LegacyItemFile
{
public:
    LegacyItemFile() : m_records(MAX_ITEM) {}

    ITEM_ATTRIBUTE_FILE_LEGACY& Item(int itemType)
    {
        return m_records[itemType];
    }

    // Like in the original files, a name longer than the name field runs on
    // into the fields after it. Set the fields first.
    void SetName(int itemType, std::string_view name)
    {
        auto* bytes = reinterpret_cast<char*>(&m_records[itemType]);
        std::memcpy(bytes, name.data(), name.size());
        bytes[name.size()] = '\0';
    }

    void Write(const std::filesystem::path& path) const
    {
        std::vector<ITEM_ATTRIBUTE_FILE_LEGACY> encrypted = m_records;
        for (ITEM_ATTRIBUTE_FILE_LEGACY& record : encrypted)
        {
            BuxConvert(reinterpret_cast<BYTE*>(&record), sizeof(record));
        }
        const auto* bytes = reinterpret_cast<const BYTE*>(encrypted.data());
        const DWORD size = static_cast<DWORD>(encrypted.size() * sizeof(ITEM_ATTRIBUTE_FILE_LEGACY));
        const DWORD checksum = GenerateCheckSum2(bytes, size, ItemFileChecksumKey);

        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(bytes), size);
        file.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));
    }

private:
    std::vector<ITEM_ATTRIBUTE_FILE_LEGACY> m_records;
};

std::string ReadWholeFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

const ItemDefinition* FindItem(const std::vector<ItemDefinition>& items, int itemType)
{
    const auto found = std::find_if(items.begin(), items.end(), [&](const ItemDefinition& item) {
        return MakeItemType(item.group, item.number) == itemType;
    });
    return found != items.end() ? &*found : nullptr;
}
} // namespace

TEST_CASE("Shipped item data loads without problems [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");

    for (const ItemDataIssue& issue : result.issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
    CHECK(result.items.size() > 900);
}

TEST_CASE("Loading item data fills ItemAttribute in the UI locale [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");

    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    CHECK(g_ItemDatabase.GetLogName(KrisType) == "Kris (0,0)");
    CHECK(client.Name(BladeType) == L"Blade");

    I18N::SetLocale("pt");
    CHECK(client.Name(BladeType) == L"Lâmina");
    CHECK(g_ItemDatabase.GetLogName(BladeType) == "Blade (0,5)");

    // No German item names: English is shown.
    I18N::SetLocale("de");
    CHECK(client.Name(BladeType) == L"Blade");
}

TEST_CASE("Item data errors stop loading [data][items]")
{
    TemporaryClientFolder folder;
    folder.WriteItemFile("Group00_Sword.json", R"({"formatVersion": 1, "group": 0, "items": [{"number": 0}]})");
    ClientDataScope client(folder.Directory());

    std::string errorMessage;
    CHECK_FALSE(g_ItemDataHandler.Load(errorMessage));
    CHECK(errorMessage.find("Group00_Sword.json") != std::string::npos);
}

TEST_CASE("Saved item data loads back the same [data][items]")
{
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    TemporaryClientFolder folder;
    const std::filesystem::path savedDirectory = folder.Directory() / GetItemDataDirectory();

    std::vector<ItemDataIssue> issues;
    REQUIRE(SaveItemDataDirectory(savedDirectory, shipped.items, issues) == ItemDataSaveResult::Saved);

    const ItemDataLoadResult saved = LoadItemDataDirectory(savedDirectory);
    CHECK(saved.issues.empty());
    CHECK(saved.items.size() == shipped.items.size());
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        const std::string fileName = GetItemGroupFileName(group);
        std::ifstream shippedFile(DataDirectory / "Items" / fileName, std::ios::binary);
        std::ifstream savedFile(savedDirectory / fileName, std::ios::binary);
        const std::string shippedText((std::istreambuf_iterator<char>(shippedFile)), std::istreambuf_iterator<char>());
        const std::string savedText((std::istreambuf_iterator<char>(savedFile)), std::istreambuf_iterator<char>());
        INFO(fileName);
        CHECK(savedText == shippedText);
    }
}

TEST_CASE("Bmd import recovers long names and repairs the fields they overwrote [data][items]")
{
    TemporaryClientFolder folder;
    LegacyItemFile english;
    english.Item(ChaosCastleTicketType).m_byItemSlot = NoSlot;
    // The name runs on into TwoHand, Level and, with its null byte, the slot.
    english.SetName(ChaosCastleTicketType, "Open Access Ticket to Chaos Castle");
    english.Write(folder.LegacyItemFilePath(L"Eng"));
    LegacyItemFile portuguese;
    portuguese.Item(ChaosCastleTicketType).m_byItemSlot = NoSlot;
    portuguese.SetName(ChaosCastleTicketType, "Ingresso Castelo");
    portuguese.Write(folder.LegacyItemFilePath(L"Por"));
    ClientDataScope client(folder.Directory());

    const ItemBmdImportResult result = ImportItemBmdFiles();
    REQUIRE_FALSE(HasErrors(result.issues));
    CHECK(result.importedLocales == std::vector<std::string>{"en", "pt"});
    CHECK(result.recoveredNameCount == 1);

    const ItemDefinition* ticket = FindItem(result.items, ChaosCastleTicketType);
    REQUIRE(ticket != nullptr);
    CHECK(ticket->names.GetNeutral() == "Open Access Ticket to Chaos Castle");
    CHECK_FALSE(ticket->twoHanded);
    CHECK(ticket->level == 0);
    CHECK(ticket->slot == ItemSlot::None);
    REQUIRE_FALSE(result.repairs.empty());
    CHECK(result.repairs.front().fromLocale == "pt");
}

TEST_CASE("Bmd import reads Portuguese and Spanish names as Windows-1252 [data][items]")
{
    TemporaryClientFolder folder;
    LegacyItemFile english;
    english.SetName(BladeType, "Blade");
    english.SetName(GaionsOrderType, "Gaion\xA1\xAFs Order"); // A1 AF: a right quote on Korean systems
    english.Write(folder.LegacyItemFilePath(L"Eng"));
    LegacyItemFile portuguese;
    portuguese.SetName(BladeType, "L\xE2mina");
    portuguese.Write(folder.LegacyItemFilePath(L"Por"));
    ClientDataScope client(folder.Directory());

    const ItemBmdImportResult result = ImportItemBmdFiles();
    REQUIRE_FALSE(HasErrors(result.issues));

    const ItemDefinition* blade = FindItem(result.items, BladeType);
    REQUIRE(blade != nullptr);
    CHECK(blade->names.Get("pt") == "L\xC3\xA2mina");

    const ItemDefinition* gaionsOrder = FindItem(result.items, GaionsOrderType);
    REQUIRE(gaionsOrder != nullptr);
    CHECK(gaionsOrder->names.GetNeutral() == "Gaion's Order");
}

TEST_CASE("A folder that cannot be written is a write failure, not a data error [data][items]")
{
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    TemporaryClientFolder folder;
    // A file where the item folder should be: nothing can be written there.
    const std::filesystem::path blocked = folder.Directory() / "blocked";
    std::ofstream(blocked) << "not a folder";

    std::vector<ItemDataIssue> issues;
    CHECK(SaveItemDataDirectory(blocked, shipped.items, issues) == ItemDataSaveResult::WriteFailed);
    CHECK_FALSE(issues.empty());
}

#ifdef _EDITOR
TEST_CASE("Item editor changes go into the item database [data][items][editor]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    SUBCASE("a changed value is copied")
    {
        ItemAttribute[BladeType].Width = 2;
        g_ItemDataHandler.OnItemEdited(BladeType);

        CHECK(g_ItemDatabase.Find(BladeType)->width == 2);
        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pt") == "L\xC3\xA2mina");
    }

    SUBCASE("a changed name is stored for the current UI locale")
    {
        I18N::SetLocale("pt");
        const std::wstring newName = L"Lâmina Nova";
        std::fill(std::begin(ItemAttribute[BladeType].Name), std::end(ItemAttribute[BladeType].Name), L'\0');
        newName.copy(ItemAttribute[BladeType].Name, newName.size());
        g_ItemDataHandler.OnItemEdited(BladeType);

        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pt") == "L\xC3\xA2mina Nova");
        CHECK(g_ItemDatabase.Find(BladeType)->names.GetNeutral() == "Blade");
    }

    SUBCASE("a value change does not turn the English fallback into a translation")
    {
        I18N::SetLocale("pt");
        REQUIRE(client.Name(KrisType) == L"Kris");
        ItemAttribute[KrisType].Width = 2;
        g_ItemDataHandler.OnItemEdited(KrisType);

        CHECK(g_ItemDatabase.Find(KrisType)->names.GetTranslations().empty());
    }

    SUBCASE("a value change keeps a name longer than ITEM_ATTRIBUTE can hold")
    {
        const std::string longName(MAX_ITEM_NAME + 10, 'x');
        ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
        kris.names.Set("en", longName);
        g_ItemDatabase.Set(kris);
        ToItemAttribute(*g_ItemDatabase.Find(KrisType), ItemAttribute[KrisType]);

        ItemAttribute[KrisType].Width = 2;
        g_ItemDataHandler.OnItemEdited(KrisType);

        CHECK(g_ItemDatabase.Find(KrisType)->names.GetNeutral() == longName);
        CHECK(g_ItemDatabase.Find(KrisType)->width == 2);
    }

    SUBCASE("a moved item keeps all its names")
    {
        const int emptyType = MakeItemType(0, 100);
        REQUIRE(g_ItemDatabase.Find(emptyType) == nullptr);
        std::swap(ItemAttribute[BladeType], ItemAttribute[emptyType]);
        g_ItemDataHandler.OnItemsSwapped(BladeType, emptyType);

        const ItemDefinition* moved = g_ItemDatabase.Find(emptyType);
        REQUIRE(moved != nullptr);
        CHECK(moved->names.Get("pt") == "L\xC3\xA2mina");
        CHECK(g_ItemDatabase.Find(BladeType) == nullptr);
    }
}

TEST_CASE("Exporting the items as bmd and importing them again gives the same data [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasErrors(result.issues));
    CHECK(result.validationIssues.empty());
    // Tags, wing tiers and rule flags are not in the bmd files; the items keep
    // the ones they have, so the files below match.
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        INFO(GetItemGroupFileName(group));
        CHECK(WriteItemGroupJson(group, g_ItemDatabase.GetAllSlots()) ==
              ReadWholeFile(DataDirectory / "Items" / GetItemGroupFileName(group)));
    }
}

TEST_CASE("Exporting unchanged bmd files is not a failure [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    std::string firstChangeLog;
    CHECK(g_ItemDataHandler.ExportAsBmd(firstChangeLog));

    std::string secondChangeLog;
    CHECK(g_ItemDataHandler.ExportAsBmd(secondChangeLog));
}
#endif
