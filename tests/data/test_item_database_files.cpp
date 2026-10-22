#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "ItemStatProfilesScope.h"
#include "TestFiles.h"

#include "Core/Globals/_crypt.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
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

    // The shipped glow color list, which the item models name their colors
    // from.
    void CopyShippedGlowColors() const
    {
        const std::filesystem::path file = m_directory / GetGlowColorsFile();
        std::filesystem::create_directories(file.parent_path());
        std::filesystem::copy_file(ShippedClientDirectory / GetGlowColorsFile(), file);
    }

private:
    std::filesystem::path m_directory;
};

// Empties the item model database when the test ends, also when a check
// stops it, so the next test starts without models.
class ItemModelDatabaseScope
{
public:
    ItemModelDatabaseScope() = default;
    ~ItemModelDatabaseScope()
    {
        g_ItemModelDatabase.Build({}, Data::Effects::GlowColorList{});
    }

    ItemModelDatabaseScope(const ItemModelDatabaseScope&) = delete;
    ItemModelDatabaseScope& operator=(const ItemModelDatabaseScope&) = delete;
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

TEST_CASE("The shipped item names are in languages the game shows [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    std::vector<ItemDataIssue> issues;
    ValidateNameLocales(result.items, I18N::GetAvailableLocales(), issues);
    for (const ItemDataIssue& issue : issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
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

    I18N::SetLocale("de");
    CHECK(client.Name(BladeType) == L"Klinge");
    // The German name is the English one.
    CHECK(client.Name(KrisType) == L"Kris");
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

// The bmd import and the item editor keep a weapon kind that its group gives
// following the group and slot; every shipped item has that kind.
TEST_CASE("The shipped weapon kinds are the ones their groups give [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    for (const ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists())
        {
            continue;
        }
        CAPTURE(definition.group);
        CAPTURE(definition.number);
        CHECK(definition.weaponKind == GetGroupWeaponKind(definition));
    }
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

// The items and their models are saved together: when one of the files
// cannot be written, none is replaced, so a moved item never loses its model
// on disk.
TEST_CASE("A save that cannot write all its files replaces none of them [data][items]")
{
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    REQUIRE_FALSE(HasErrors(shipped.issues));
    const ItemModelDataLoadResult shippedModels = LoadItemModelDataDirectory(DataDirectory / "Items" / "Models");
    REQUIRE_FALSE(HasErrors(shippedModels.issues));
    ItemModelDatabase models;
    models.Build(shippedModels.models, Data::Effects::GlowColorList{});
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    const std::filesystem::path itemDirectory = folder.Directory() / GetItemDataDirectory();
    // Kris moves to a free number, so its group file changes.
    std::vector<ItemDefinition> items = shipped.items;
    const auto kris = std::find_if(items.begin(), items.end(), [](const ItemDefinition& item)
                                   { return MakeItemType(item.group, item.number) == KrisType; });
    REQUIRE(kris != items.end());
    kris->number = 100;
    // A file where the model folder should be: no model file can be written.
    const std::filesystem::path blockedModels = folder.Directory() / "blocked";
    std::ofstream(blockedModels) << "not a folder";
    ItemGroupSet modelGroups;
    modelGroups.set(0);

    std::vector<ItemDataIssue> issues;
    CHECK(SaveItemAndModelDataDirectories(itemDirectory, items, blockedModels, models.GetAllSlots(), modelGroups,
                                          issues) == ItemDataSaveResult::WriteFailed);
    CHECK_FALSE(issues.empty());
    const std::string swordFile = GetItemGroupFileName(0);
    CHECK(TestFiles::ReadWholeFile(itemDirectory / swordFile) ==
          TestFiles::ReadWholeFile(DataDirectory / "Items" / swordFile));
    CHECK_FALSE(std::filesystem::exists(itemDirectory / (swordFile + ".tmp")));
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

    SUBCASE("a name set for another language goes into the database, not into the shown name")
    {
        REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "pl", "Ostrze"));
        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pl") == "Ostrze");
        CHECK(std::wstring(ItemAttribute[BladeType].Name) == L"Blade");
    }

    SUBCASE("a name set for the shown language is shown, also when it is longer than the game shows")
    {
        const std::string longName(60, 'B');
        REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "en", longName));
        CHECK(g_ItemDatabase.Find(BladeType)->names.GetNeutral() == longName);
        CHECK(std::wstring(ItemAttribute[BladeType].Name) == std::wstring(MAX_ITEM_NAME - 1, L'B'));
    }

    SUBCASE("an empty name removes a translation, but not the English name")
    {
        REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "pt", ""));
        CHECK_FALSE(g_ItemDatabase.Find(BladeType)->names.GetTranslations().contains("pt"));
        CHECK_FALSE(g_ItemDataHandler.SetItemName(BladeType, "en", ""));
        CHECK(g_ItemDatabase.Find(BladeType)->names.GetNeutral() == "Blade");
        CHECK_FALSE(g_ItemDataHandler.SetItemName(MakeItemType(0, 500), "en", "Nothing"));
    }

    SUBCASE("a name with the separator is refused, since the save would refuse it")
    {
        CHECK_FALSE(g_ItemDataHandler.SetItemName(BladeType, "pt", "L\xC3\xA2mina||de=Klinge"));
        CHECK_FALSE(g_ItemDataHandler.SetItemName(BladeType, "en", "Bl||ade"));
        CHECK(g_ItemDatabase.Find(BladeType)->names.GetNeutral() == "Blade");
        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pt") == "L\xC3\xA2mina");
    }

    SUBCASE("an item without an English name gets that first, so it cannot lose its last name")
    {
        ItemDefinition untranslated = *g_ItemDatabase.Find(KrisType);
        untranslated.names = Data::LocalizedString{};
        untranslated.names.Set("pt", "Cris");
        g_ItemDatabase.Set(untranslated);

        CHECK_FALSE(g_ItemDataHandler.SetItemName(KrisType, "pt", ""));
        CHECK_FALSE(g_ItemDataHandler.SetItemName(KrisType, "es", "Daga"));
        REQUIRE(g_ItemDatabase.Find(KrisType) != nullptr);
        CHECK(g_ItemDatabase.Find(KrisType)->names.Get("pt") == "Cris");
        CHECK(g_ItemDataHandler.SetItemName(KrisType, "en", "Kris"));
    }

    SUBCASE("a value change does not turn the English fallback into a translation")
    {
        // No item has a Polish name: the English one is shown.
        I18N::SetLocale("pl");
        REQUIRE(client.Name(KrisType) == L"Kris");
        ItemAttribute[KrisType].Width = 2;
        g_ItemDataHandler.OnItemEdited(KrisType);

        CHECK_FALSE(g_ItemDatabase.Find(KrisType)->names.GetTranslations().contains("pl"));
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

    SUBCASE("a new weapon gets its group's weapon kind")
    {
        const int newSpearType = MakeItemType(3, 100);
        REQUIRE(g_ItemDatabase.Find(newSpearType) == nullptr);
        const std::wstring name = L"New Spear";
        std::fill(std::begin(ItemAttribute[newSpearType].Name), std::end(ItemAttribute[newSpearType].Name), L'\0');
        name.copy(ItemAttribute[newSpearType].Name, name.size());
        ItemAttribute[newSpearType].m_byItemSlot = static_cast<BYTE>(ItemSlot::MainHand);
        g_ItemDataHandler.OnItemEdited(newSpearType);

        REQUIRE(g_ItemDatabase.Find(newSpearType) != nullptr);
        CHECK(g_ItemDatabase.GetWeaponKind(newSpearType) == WeaponKind::Spear);
    }

    SUBCASE("a new bow is a bow once it is two-handed and on offHand, in any order")
    {
        const int newBowType = MakeItemType(4, 100);
        REQUIRE(g_ItemDatabase.Find(newBowType) == nullptr);
        const std::wstring name = L"New Bow";
        std::fill(std::begin(ItemAttribute[newBowType].Name), std::end(ItemAttribute[newBowType].Name), L'\0');
        name.copy(ItemAttribute[newBowType].Name, name.size());
        ItemAttribute[newBowType].m_byItemSlot = static_cast<BYTE>(ItemSlot::MainHand);
        g_ItemDataHandler.OnItemEdited(newBowType);
        ItemAttribute[newBowType].TwoHand = true;
        g_ItemDataHandler.OnItemEdited(newBowType);
        CHECK(g_ItemDatabase.GetWeaponKind(newBowType) == WeaponKind::Crossbow);

        ItemAttribute[newBowType].m_byItemSlot = static_cast<BYTE>(ItemSlot::OffHand);
        g_ItemDataHandler.OnItemEdited(newBowType);
        CHECK(g_ItemDatabase.GetWeaponKind(newBowType) == WeaponKind::Bow);

        // The other order: on offHand first, then two-handed.
        const int otherBowType = MakeItemType(4, 101);
        REQUIRE(g_ItemDatabase.Find(otherBowType) == nullptr);
        std::fill(std::begin(ItemAttribute[otherBowType].Name), std::end(ItemAttribute[otherBowType].Name), L'\0');
        name.copy(ItemAttribute[otherBowType].Name, name.size());
        ItemAttribute[otherBowType].m_byItemSlot = static_cast<BYTE>(ItemSlot::OffHand);
        g_ItemDataHandler.OnItemEdited(otherBowType);
        CHECK(g_ItemDatabase.GetWeaponKind(otherBowType) == WeaponKind::None);
        ItemAttribute[otherBowType].TwoHand = true;
        g_ItemDataHandler.OnItemEdited(otherBowType);
        CHECK(g_ItemDatabase.GetWeaponKind(otherBowType) == WeaponKind::Bow);
    }

    SUBCASE("a moved weapon's kind follows its new group")
    {
        const int emptySpearType = MakeItemType(3, 100);
        const int emptySwordType = MakeItemType(0, 100);
        REQUIRE(g_ItemDatabase.Find(emptySpearType) == nullptr);
        REQUIRE(g_ItemDatabase.Find(emptySwordType) == nullptr);

        std::swap(ItemAttribute[KrisType], ItemAttribute[emptySpearType]);
        g_ItemDataHandler.OnItemsSwapped(KrisType, emptySpearType);
        CHECK(g_ItemDatabase.GetWeaponKind(emptySpearType) == WeaponKind::Spear);

        std::swap(ItemAttribute[ITEM_SPEAR], ItemAttribute[emptySwordType]);
        g_ItemDataHandler.OnItemsSwapped(ITEM_SPEAR, emptySwordType);
        CHECK(g_ItemDatabase.GetWeaponKind(emptySwordType) == WeaponKind::Sword);
    }

    SUBCASE("a sword is a glove while only the Rage Fighter may use it")
    {
        REQUIRE(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Sword);
        const ITEM_ATTRIBUTE kris = ItemAttribute[KrisType];
        for (int characterClass = 0; characterClass < MAX_CLASS; ++characterClass)
        {
            ItemAttribute[KrisType].RequireClass[characterClass] = characterClass == CLASS_RAGEFIGHTER ? 1 : 0;
        }
        g_ItemDataHandler.OnItemEdited(KrisType);
        CHECK(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Glove);

        ItemAttribute[KrisType] = kris;
        g_ItemDataHandler.OnItemEdited(KrisType);
        CHECK(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Sword);
    }

    SUBCASE("a staff is a book while it is on offHand")
    {
        REQUIRE(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Staff);
        ItemAttribute[ITEM_STAFF].m_byItemSlot = static_cast<BYTE>(ItemSlot::OffHand);
        g_ItemDataHandler.OnItemEdited(ITEM_STAFF);
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Book);

        ItemAttribute[ITEM_STAFF].m_byItemSlot = static_cast<BYTE>(ItemSlot::MainHand);
        g_ItemDataHandler.OnItemEdited(ITEM_STAFF);
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Staff);
    }

    SUBCASE("an undone edit undoes the weapon kind")
    {
        ItemAttribute[ITEM_BOLT].TwoHand = true;
        g_ItemDataHandler.OnItemEdited(ITEM_BOLT);
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_BOLT) == WeaponKind::Bow);

        ItemAttribute[ITEM_BOLT].TwoHand = false;
        g_ItemDataHandler.OnItemEdited(ITEM_BOLT);
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_BOLT) == WeaponKind::None);
    }

    SUBCASE("a moved shield's stat profile follows its weapon kind")
    {
        const int emptySwordType = MakeItemType(0, 100);
        REQUIRE(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile == "shield");
        std::swap(ItemAttribute[ITEM_SMALL_SHIELD], ItemAttribute[emptySwordType]);
        g_ItemDataHandler.OnItemsSwapped(ITEM_SMALL_SHIELD, emptySwordType);
        CHECK(g_ItemDatabase.GetWeaponKind(emptySwordType) == WeaponKind::None);
        CHECK(g_ItemDatabase.Find(emptySwordType)->statProfile.empty());
    }

    SUBCASE("the Horn of Fenrir keeps the default its file names")
    {
        REQUIRE(g_ItemDatabase.Find(ITEM_HORN_OF_FENRIR)->statProfile == "default");
        ItemAttribute[ITEM_HORN_OF_FENRIR].RequireLevel = 0;
        g_ItemDataHandler.OnItemEdited(ITEM_HORN_OF_FENRIR);
        ItemAttribute[ITEM_HORN_OF_FENRIR].RequireLevel = 300;
        g_ItemDataHandler.OnItemEdited(ITEM_HORN_OF_FENRIR);
        CHECK(g_ItemDatabase.Find(ITEM_HORN_OF_FENRIR)->statProfile == "default");
    }

    SUBCASE("an item without a stat profile takes the one its data gives when edited")
    {
        ItemDefinition shield = *g_ItemDatabase.Find(ITEM_SMALL_SHIELD);
        shield.statProfile.clear();
        g_ItemDatabase.Set(shield);
        g_ItemDataHandler.OnItemEdited(ITEM_SMALL_SHIELD);
        CHECK(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile == "shield");
    }

    SUBCASE("a wing's stat profile follows its slot")
    {
        REQUIRE(g_ItemDatabase.Find(ITEM_WINGS_OF_ELF)->statProfile == "firstTierWing");
        ItemAttribute[ITEM_WINGS_OF_ELF].m_byItemSlot = static_cast<BYTE>(ItemSlot::Helm);
        g_ItemDataHandler.OnItemEdited(ITEM_WINGS_OF_ELF);
        CHECK(g_ItemDatabase.Find(ITEM_WINGS_OF_ELF)->statProfile == "armor");

        ItemAttribute[ITEM_WINGS_OF_ELF].m_byItemSlot = static_cast<BYTE>(ItemSlot::Wings);
        g_ItemDataHandler.OnItemEdited(ITEM_WINGS_OF_ELF);
        CHECK(g_ItemDatabase.Find(ITEM_WINGS_OF_ELF)->statProfile == "firstTierWing");
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

// The item editor moves an item's model with the item, and saving writes the
// model files of the moved models.
TEST_CASE("A moved item takes its model along, and saving writes it [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    // A model file the save must not touch: no model of its group moves.
    const std::filesystem::path axeModels = folder.Directory() / GetItemModelDataDirectory() / GetItemGroupFileName(1);
    std::ofstream(axeModels, std::ios::binary | std::ios::app) << "\n";
    const std::string axeModelsText = TestFiles::ReadWholeFile(axeModels);
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope models;
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE_MESSAGE(g_ItemDataHandler.LoadModels(errorMessage), errorMessage);
    const int emptyType = MakeItemType(0, 100);
    REQUIRE(g_ItemDatabase.Find(emptyType) == nullptr);
    REQUIRE(g_ItemModelDatabase.Find(emptyType) == nullptr);
    REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
    const std::string krisModelFile = g_ItemModelDatabase.Find(KrisType)->file;

    std::swap(ItemAttribute[KrisType], ItemAttribute[emptyType]);
    g_ItemDataHandler.OnItemsSwapped(KrisType, emptyType);
    REQUIRE(g_ItemModelDatabase.Find(emptyType) != nullptr);
    CHECK(g_ItemModelDatabase.Find(emptyType)->file == krisModelFile);
    CHECK(g_ItemModelDatabase.Find(KrisType) == nullptr);

    std::vector<ItemDataIssue> issues;
    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    const ItemModelDataLoadResult saved = LoadItemModelDataDirectory(folder.Directory() / GetItemModelDataDirectory());
    CHECK_FALSE(HasErrors(saved.issues));
    const auto findModel = [&saved](int group, int number) -> const ItemModelDefinition*
    {
        const auto model =
            std::find_if(saved.models.begin(), saved.models.end(), [&](const ItemModelDefinition& candidate)
                         { return candidate.group == group && candidate.number == number; });
        return model != saved.models.end() ? &*model : nullptr;
    };
    REQUIRE(findModel(0, 100) != nullptr);
    CHECK(findModel(0, 100)->file == krisModelFile);
    CHECK(findModel(0, 0) == nullptr);
    CHECK(TestFiles::ReadWholeFile(axeModels) == axeModelsText);
}

// The bmd files have the items at their old numbers, so the import moves the
// models of moved items back as well, also after a save.
TEST_CASE("A bmd import moves the models of moved items back [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope models;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE_MESSAGE(g_ItemDataHandler.LoadModels(errorMessage), errorMessage);
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));
    REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
    const std::string krisModelFile = g_ItemModelDatabase.Find(KrisType)->file;
    const int emptyType = MakeItemType(0, 100);
    std::swap(ItemAttribute[KrisType], ItemAttribute[emptyType]);
    g_ItemDataHandler.OnItemsSwapped(KrisType, emptyType);
    REQUIRE(g_ItemModelDatabase.Find(KrisType) == nullptr);
    std::vector<ItemDataIssue> issues;

    SUBCASE("before a save") {}
    SUBCASE("after a save")
    {
        REQUIRE(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    }

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasErrors(result.issues));
    REQUIRE(g_ItemDatabase.Find(KrisType) != nullptr);
    REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
    CHECK(g_ItemModelDatabase.Find(KrisType)->file == krisModelFile);
    CHECK(g_ItemModelDatabase.Find(emptyType) == nullptr);

    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    const std::string swordFile = GetItemGroupFileName(0);
    CHECK(TestFiles::ReadWholeFile(folder.Directory() / GetItemModelDataDirectory() / swordFile) ==
          TestFiles::ReadWholeFile(DataDirectory / "Items" / "Models" / swordFile));
}

// The names of a language whose item.bmd is missing stay as they are, like
// the names of the languages that have none (German).
TEST_CASE("A bmd import keeps the names of a language whose item.bmd is missing [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));
    REQUIRE(std::filesystem::remove(folder.LegacyItemFilePath(L"Spn")));

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasErrors(result.issues));
    CHECK(result.importedLocales == std::vector<std::string>{"en", "pt"});
    const ItemDefinition* shortSword = g_ItemDatabase.Find(0, 1);
    REQUIRE(shortSword != nullptr);
    CHECK(shortSword->names.Get("es") == "Espada Corta");
    CHECK(shortSword->names.Get("de") == "Kurzschwert");
    CHECK(shortSword->names.Get("pt") == "Espada curta");
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
    // Tags, wing tiers, weapon kinds, potion priorities and rule flags are not in the bmd files;
    // the items keep the ones they have, so the files below match.
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        INFO(GetItemGroupFileName(group));
        CHECK(WriteItemGroupJson(group, g_ItemDatabase.GetAllSlots()) ==
              TestFiles::ReadWholeFile(DataDirectory / "Items" / GetItemGroupFileName(group)));
    }
}

// Export as bmd writes a missing name as the English one; the import must not
// turn it into a name that is the same in that language.
TEST_CASE("A bmd round trip keeps missing names missing and names that are the same [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "pt", ""));
    REQUIRE(g_ItemDataHandler.SetItemName(KrisType, "es", "Kris"));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasErrors(result.issues));
    CHECK_FALSE(HasNameIn(*g_ItemDatabase.Find(BladeType), "pt"));
    CHECK(GetOwnName(*g_ItemDatabase.Find(KrisType), "es") == "Kris");
    CHECK(HasNameIn(*g_ItemDatabase.Find(BladeType), "es"));
}

TEST_CASE("The save warns about an item name in a language the game has no texts for [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "fr", "Lame"));

    std::vector<ItemDataIssue> issues;
    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    CHECK(std::ranges::any_of(issues,
                              [](const ItemDataIssue& issue)
                              {
                                  return issue.severity == ItemDataIssueSeverity::Warning && issue.group == 0 &&
                                         issue.number == 5 && issue.message.find("the fr name") != std::string::npos;
                              }));
}

// The bmd import and the editor give items the profile their data gives; a stat file can lack it
// (renamed or removed). The item then keeps the default with a warning; an item that still names
// the profile is an error on save.
TEST_CASE("A derived stat profile the item stat file does not have keeps the default [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));

    ItemStatsLoadResult stats = LoadItemStatsFile(folder.Directory() / GetItemStatsFile());
    REQUIRE_FALSE(HasErrors(stats.issues));
    std::erase_if(stats.file.profiles, [](const WrittenStatProfile& profile) { return profile.name == "shield"; });
    ItemStatProfilesScope restoreProfiles;
    g_ItemDatabase.SetStatProfiles(ItemStatProfiles(stats.file));
    const auto namesShield = [](ItemDataIssueSeverity severity)
    {
        return [severity](const ItemDataIssue& issue)
        {
            return issue.severity == severity && issue.field == "statProfile" &&
                   issue.message.find("\"shield\"") != std::string::npos;
        };
    };

    SUBCASE("the import")
    {
        const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
        REQUIRE_FALSE(HasErrors(result.issues));
        CHECK_FALSE(HasErrors(result.validationIssues));
        CHECK(std::ranges::any_of(result.validationIssues, namesShield(ItemDataIssueSeverity::Warning)));
        CHECK(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile.empty());
    }

    SUBCASE("an edit")
    {
        g_ItemDataHandler.OnItemEdited(ITEM_SMALL_SHIELD);
        CHECK(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile.empty());
    }

    SUBCASE("the save of items that still name it, with the item checks")
    {
        // An item without an English name is an error of the item checks.
        ItemDefinition untranslated = *g_ItemDatabase.Find(KrisType);
        untranslated.names = Data::LocalizedString{};
        untranslated.names.Set("pt", "Cris");
        g_ItemDatabase.Set(untranslated);

        std::vector<ItemDataIssue> issues;
        CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::InvalidData);
        CHECK(std::ranges::any_of(issues, namesShield(ItemDataIssueSeverity::Error)));
        CHECK(std::ranges::any_of(issues,
                                  [](const ItemDataIssue& issue)
                                  {
                                      return issue.severity == ItemDataIssueSeverity::Error && issue.group == 0 &&
                                             issue.number == 0 && issue.field != "statProfile";
                                  }));
    }
}

TEST_CASE("The bmd exports refuse damage or magic power past 255 and name the items [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const WORD krisDamageMin = ItemAttribute[KrisType].DamageMin;
    const WORD bladeMagicPower = ItemAttribute[BladeType].MagicPower;
    ItemAttribute[KrisType].DamageMin = 300;
    g_ItemDataHandler.OnItemEdited(KrisType);
    ItemAttribute[BladeType].MagicPower = 256;
    g_ItemDataHandler.OnItemEdited(BladeType);
    const std::string named = "Item (0,0) DamageMin is 300; a .bmd item file holds up to 255\n"
                              "Item (0,5) MagicPower is 256; a .bmd item file holds up to 255\n";

    // Every such item is named once, and no file is written.
    std::string changeLog;
    CHECK_FALSE(g_ItemDataHandler.ExportAsBmd(changeLog));
    CHECK(changeLog == named);
    for (const ItemBmdLanguage& language : GetItemBmdLanguages())
    {
        CHECK_FALSE(std::filesystem::exists(folder.LegacyItemFilePath(language.folder)));
    }

    std::wstring legacyFile = (folder.Directory() / "Item_S6E3.bmd").wstring();
    std::string problems;
    CHECK_FALSE(g_ItemDataHandler.ExportAsS6E3(legacyFile.data(), problems));
    CHECK(problems == named);
    CHECK_FALSE(std::filesystem::exists(legacyFile));

    // With the values back, both write.
    ItemAttribute[KrisType].DamageMin = krisDamageMin;
    g_ItemDataHandler.OnItemEdited(KrisType);
    ItemAttribute[BladeType].MagicPower = bladeMagicPower;
    g_ItemDataHandler.OnItemEdited(BladeType);
    std::string savedChangeLog;
    CHECK(g_ItemDataHandler.ExportAsBmd(savedChangeLog));
    problems.clear();
    CHECK(g_ItemDataHandler.ExportAsS6E3(legacyFile.data(), problems));
    CHECK(problems.empty());
    CHECK(std::filesystem::exists(legacyFile));
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
