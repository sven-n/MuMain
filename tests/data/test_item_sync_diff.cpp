#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ClientDataScope.h"

#include "Core/SecretStore.h"
#include "UI/ItemTools/ItemEdits.h"
#include "UI/ItemTools/ItemSyncDiff.h"
#include "UI/ItemTools/ItemSyncServer.h"
#include "UI/ItemTools/MuItemSyncUI.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using namespace MuEditor::Items;
using namespace Data::Items;

// The Item Sync window's diff: the client's data against an exchange file, and the side that
// changes with the checked rows.
namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

constexpr int KrisType = MakeItemType(0, 0);
constexpr int ShortSwordType = MakeItemType(0, 1);
const std::vector<std::string> UiLocales = {"en", "de", "es", "pt"};

// The shipped data as the client's side, and a copy of it as the file.
class Sides
{
public:
    Sides() : m_client(ClientDirectory)
    {
        I18N::SetLocale("en");
        std::string errorMessage;
        REQUIRE(g_ItemDataHandler.Load(errorMessage));
        client = BuildItemExchange(g_ItemDatabase.GetAllSlots(), g_ItemDataHandler.GetStatFile());
        file = client;
        file.source = ItemExchangeSourceOpenMU;
    }

    ItemDefinition& FileItem(int itemType)
    {
        return Find(file, itemType);
    }

    static ItemDefinition& Find(ItemExchangeFile& side, int itemType)
    {
        const auto item = std::ranges::find_if(side.items, [&](const ItemDefinition& entry)
                                               { return MakeItemType(entry.group, entry.number) == itemType; });
        REQUIRE(item != side.items.end());
        return *item;
    }

    ItemSyncDiff Diff(const KnownDifferences* known = nullptr, std::function<bool(int)> isHeld = {}) const
    {
        return BuildItemSyncDiff(client, file, {known, UiLocales, std::move(isHeld)});
    }

    ItemExchangeFile client;
    ItemExchangeFile file;

private:
    ClientDataScope m_client;
};

const SyncItem* FindRow(const ItemSyncDiff& diff, int itemType)
{
    const auto row = std::ranges::find_if(diff.items, [&](const SyncItem& entry)
                                          { return MakeItemType(entry.group, entry.number) == itemType; });
    return row != diff.items.end() ? &*row : nullptr;
}

const SyncField* FindField(const SyncItem& row, const std::string& field)
{
    const auto found = std::ranges::find(row.fields, field, &SyncField::field);
    return found != row.fields.end() ? &*found : nullptr;
}

const SyncFormula* FindFormula(const ItemSyncDiff& diff, const std::string& entry)
{
    const auto found = std::ranges::find(diff.formulas, entry, &SyncFormula::entry);
    return found != diff.formulas.end() ? &*found : nullptr;
}

// A folder of its own for a test's files.
class TemporaryFolder
{
public:
    TemporaryFolder()
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_item_sync_" + std::to_string(std::random_device{}())))
    {
        std::filesystem::create_directories(m_directory);
    }

    ~TemporaryFolder()
    {
        std::error_code ignored;
        std::filesystem::remove_all(m_directory, ignored);
    }

    TemporaryFolder(const TemporaryFolder&) = delete;
    TemporaryFolder& operator=(const TemporaryFolder&) = delete;

    const std::filesystem::path& Path() const
    {
        return m_directory;
    }

private:
    std::filesystem::path m_directory;
};

void WriteFile(const std::filesystem::path& file, const std::string& text)
{
    std::ofstream(file, std::ios::binary) << text;
}
} // namespace

TEST_CASE("The same data on both sides gives no differences [data][items][editor]")
{
    Sides sides;
    CHECK(sides.Diff().IsEmpty());
}

TEST_CASE(
    "An import lists the file's new, changed and removed items and applies the checked ones [data][items][editor]")
{
    Sides sides;
    ItemDefinition& kris = sides.FileItem(KrisType);
    const BYTE width = kris.width;
    ++kris.width;
    kris.names.Set("de", "Dolch");
    ItemDefinition added = kris;
    added.number = 100;
    added.names = Data::LocalizedString::Parse("New Sword");
    sides.file.items.push_back(added);
    std::erase_if(sides.file.items,
                  [](const ItemDefinition& item) { return MakeItemType(item.group, item.number) == ShortSwordType; });

    ItemSyncDiff diff = sides.Diff();
    REQUIRE(diff.items.size() == 3);
    const SyncItem* changed = FindRow(diff, KrisType);
    REQUIRE(changed != nullptr);
    CHECK(changed->change == SyncChange::Changed);
    REQUIRE(changed->fields.size() == 2);
    const SyncField* widthField = FindField(*changed, "width");
    REQUIRE(widthField != nullptr);
    CHECK(widthField->current == std::to_string(width));
    CHECK(widthField->incoming == std::to_string(width + 1));
    CHECK(widthField->checked);
    REQUIRE(FindField(*changed, "name.de") != nullptr);
    const SyncItem* newRow = FindRow(diff, MakeItemType(0, 100));
    REQUIRE(newRow != nullptr);
    CHECK(newRow->change == SyncChange::New);
    CHECK(newRow->checked);
    const SyncItem* removed = FindRow(diff, ShortSwordType);
    REQUIRE(removed != nullptr);
    CHECK(removed->change == SyncChange::Removed);
    CHECK_FALSE(removed->checked);

    // The name is unchecked: only the width comes in.
    for (SyncItem& row : diff.items)
    {
        for (SyncField& field : row.fields)
        {
            field.checked = field.checked && field.field != "name.de";
        }
    }
    std::vector<ItemDataIssue> issues;
    ItemExchangeFile applied = ApplyItemSyncDiff(sides.client, sides.file, diff, issues);
    CHECK(issues.empty());
    CHECK(Sides::Find(applied, KrisType).width == width + 1);
    CHECK(Sides::Find(applied, KrisType).names.Get("de") == Sides::Find(sides.client, KrisType).names.Get("de"));
    CHECK(Sides::Find(applied, MakeItemType(0, 100)).names.GetNeutral() == "New Sword");
    CHECK(Sides::Find(applied, ShortSwordType).Exists());
}

TEST_CASE("A name in a language the client does not show, or one the file lacks, is not taken [data][items][editor]")
{
    Sides sides;
    ItemDefinition& kris = sides.FileItem(KrisType);
    Data::LocalizedString withoutSpanish;
    for (const std::string& locale : {"en", "de", "pt"})
    {
        withoutSpanish.Set(locale, kris.names.Get(locale));
    }
    withoutSpanish.Set("zh-CN", "Kris (zh)");
    kris.names = withoutSpanish;
    CHECK(sides.Diff().IsEmpty());
}

TEST_CASE("An item the game holds keeps its size and slot or is not added, and a known difference stays unchecked "
          "[data][items][editor]")
{
    Sides sides;
    ItemDefinition& kris = sides.FileItem(KrisType);
    ++kris.width;
    kris.repairable = !kris.repairable;
    std::erase_if(sides.file.items,
                  [](const ItemDefinition& item) { return MakeItemType(item.group, item.number) == ShortSwordType; });
    // The character holds an item of a type the client has no data for yet.
    ItemDefinition heldNew = kris;
    heldNew.number = 100;
    sides.file.items.push_back(heldNew);
    KnownDifferences known;
    known.fields.insert({0, 0, "repairable"});

    const ItemSyncDiff diff =
        sides.Diff(&known, [](int itemType)
                   { return itemType == KrisType || itemType == ShortSwordType || itemType == MakeItemType(0, 100); });
    const SyncItem* row = FindRow(diff, KrisType);
    REQUIRE(row != nullptr);
    const SyncField* width = FindField(*row, "width");
    REQUIRE(width != nullptr);
    CHECK_FALSE(width->locked.empty());
    CHECK_FALSE(width->checked);
    const SyncField* repairable = FindField(*row, "repairable");
    REQUIRE(repairable != nullptr);
    CHECK(repairable->known);
    CHECK_FALSE(repairable->checked);
    const SyncItem* removed = FindRow(diff, ShortSwordType);
    REQUIRE(removed != nullptr);
    CHECK_FALSE(removed->locked.empty());
    const SyncItem* added = FindRow(diff, MakeItemType(0, 100));
    REQUIRE(added != nullptr);
    CHECK(added->change == SyncChange::New);
    CHECK_FALSE(added->locked.empty());
    CHECK_FALSE(added->checked);

    // Even checked, a locked field and a locked removal stay as they are.
    ItemSyncDiff checkedDiff = diff;
    for (SyncItem& entry : checkedDiff.items)
    {
        entry.checked = true;
        for (SyncField& field : entry.fields)
        {
            field.checked = true;
        }
    }
    std::vector<ItemDataIssue> issues;
    ItemExchangeFile applied = ApplyItemSyncDiff(sides.client, sides.file, checkedDiff, issues);
    CHECK(Sides::Find(applied, KrisType).width == Sides::Find(sides.client, KrisType).width);
    CHECK(Sides::Find(applied, KrisType).repairable == kris.repairable);
    CHECK(Sides::Find(applied, ShortSwordType).Exists());
    CHECK(std::ranges::none_of(applied.items, [](const ItemDefinition& item)
                               { return MakeItemType(item.group, item.number) == MakeItemType(0, 100); }));
}

TEST_CASE("A drop level OpenMU cannot keep is not taken by default [data][items][editor]")
{
    Sides sides;
    REQUIRE(Sides::Find(sides.client, ITEM_HORN_OF_FENRIR).level == 300);
    sides.FileItem(ITEM_HORN_OF_FENRIR).level = 255;
    const ItemSyncDiff diff = sides.Diff();
    const SyncItem* row = FindRow(diff, ITEM_HORN_OF_FENRIR);
    REQUIRE(row != nullptr);
    const SyncField* level = FindField(*row, "level");
    REQUIRE(level != nullptr);
    CHECK_FALSE(level->inexact.empty());
    CHECK_FALSE(level->checked);
}

TEST_CASE("The formulas differ by drop level bonus, table, term and whole profile [data][items][editor]")
{
    Sides sides;
    ItemStatFile& formulas = *sides.file.formulas;
    formulas.excellentDropLevelBonus = 30;
    formulas.tables.front().values[1] = 4;
    const std::string tableEntry = "tables." + formulas.tables.front().name;
    WrittenStatProfile* weapon = nullptr;
    for (WrittenStatProfile& profile : formulas.profiles)
    {
        weapon = profile.name == "weapon" ? &profile : weapon;
    }
    REQUIRE(weapon != nullptr);
    weapon->requirements.strengthPerOptionLevel = 4;
    WrittenStatProfile added;
    added.name = "rune";
    added.requirements.levelPerItemLevel = 2;
    formulas.profiles.push_back(added);

    ItemSyncDiff diff = sides.Diff();
    CHECK(diff.items.empty());
    REQUIRE(diff.formulas.size() == 4);
    REQUIRE(FindFormula(diff, "dropLevelBonus.excellent") != nullptr);
    CHECK(FindFormula(diff, "dropLevelBonus.excellent")->incoming == "30");
    REQUIRE(FindFormula(diff, tableEntry) != nullptr);
    const SyncFormula* term = FindFormula(diff, "profiles.weapon.requirements.strengthPerOptionLevel");
    REQUIRE(term != nullptr);
    CHECK(term->current == "5");
    CHECK(term->incoming == "4");
    const SyncFormula* profile = FindFormula(diff, "profiles.rune");
    REQUIRE(profile != nullptr);
    CHECK_FALSE(profile->current);

    // All but the table.
    for (SyncFormula& row : diff.formulas)
    {
        row.checked = row.entry != tableEntry;
    }
    std::vector<ItemDataIssue> issues;
    const ItemExchangeFile applied = ApplyItemSyncDiff(sides.client, sides.file, diff, issues);
    CHECK(issues.empty());
    REQUIRE(applied.formulas);
    CHECK(applied.formulas->excellentDropLevelBonus == 30);
    CHECK(applied.formulas->tables.front().values == sides.client.formulas->tables.front().values);
    const auto rune = std::ranges::find(applied.formulas->profiles, std::string("rune"), &WrittenStatProfile::name);
    REQUIRE(rune != applied.formulas->profiles.end());
    CHECK(rune->requirements.levelPerItemLevel == 2);
    const auto appliedWeapon =
        std::ranges::find(applied.formulas->profiles, std::string("weapon"), &WrittenStatProfile::name);
    REQUIRE(appliedWeapon != applied.formulas->profiles.end());
    CHECK(appliedWeapon->requirements.strengthPerOptionLevel == 4);
}

// A formula entry only the client has starts unchecked, as a removed item does: taking it away
// would leave the orbs naming a profile that is not there.
TEST_CASE("A formula entry only the client has starts unchecked [data][items][editor]")
{
    Sides sides;
    std::erase_if(sides.file.formulas->profiles,
                  [](const WrittenStatProfile& profile) { return profile.name == "orb"; });
    const ItemSyncDiff diff = sides.Diff();
    const SyncFormula* orb = FindFormula(diff, "profiles.orb");
    REQUIRE(orb != nullptr);
    CHECK(orb->current.has_value());
    CHECK_FALSE(orb->incoming.has_value());
    CHECK_FALSE(orb->checked);
}

TEST_CASE("Formulas the item stat file's checks refuse are not taken [data][items][editor]")
{
    Sides sides;
    // The file's armour profile names a table only the file has.
    ItemStatFile& formulas = *sides.file.formulas;
    LevelBonusTable table = formulas.tables.front();
    table.name = "runeTable";
    formulas.tables.push_back(table);
    for (WrittenStatProfile& profile : formulas.profiles)
    {
        if (profile.name == "armor")
        {
            profile.stats[static_cast<size_t>(ItemStat::Defense)].table = "runeTable";
        }
    }
    ItemSyncDiff diff = sides.Diff();
    // Only the term, not the table it names.
    for (SyncFormula& row : diff.formulas)
    {
        row.checked = row.entry != "tables.runeTable";
    }
    std::vector<ItemDataIssue> issues;
    const ItemExchangeFile applied = ApplyItemSyncDiff(sides.client, sides.file, diff, issues);
    CHECK(HasFatal(issues));
    CHECK(*applied.formulas == *sides.client.formulas);
}

// A refusal by the formulas cannot be traced back to a formula row: the window unchecks them all,
// so the next Apply does not fail the same way.
TEST_CASE("Item Sync unchecks the formula rows when the formulas refuse an import [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    ItemStatFile& formulas = *sides.file.formulas;
    LevelBonusTable table = formulas.tables.front();
    table.name = "runeTable";
    formulas.tables.push_back(table);
    for (WrittenStatProfile& profile : formulas.profiles)
    {
        if (profile.name == "armor")
        {
            profile.stats[static_cast<size_t>(ItemStat::Defense)].table = "runeTable";
        }
    }
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));

    g_MuItemSyncUI.Compare(file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    // The term, not the table it names.
    for (SyncFormula& row : g_MuItemSyncUI.GetDiff()->formulas)
    {
        row.checked = row.entry != "tables.runeTable";
    }
    CHECK_FALSE(g_MuItemSyncUI.Apply());
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK(std::ranges::none_of(g_MuItemSyncUI.GetDiff()->formulas, &SyncFormula::checked));
    CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());
}

// The window's Import: the checked rows change the client's data as one step of the undo
// history, which undoes them all.
TEST_CASE("Item Sync imports the checked rows of a file as one undo step [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    const ItemStatFile statFile = g_ItemDataHandler.GetStatFile();
    ++sides.FileItem(KrisType).width;
    ItemDefinition added = sides.FileItem(KrisType);
    added.number = 100;
    added.names = Data::LocalizedString::Parse("New Sword");
    added.width = 2;
    sides.file.items.push_back(added);
    // A new item whose file names a profile its rules do not give: the import keeps it as a pick.
    ItemDefinition picked = added;
    picked.number = 101;
    picked.names = Data::LocalizedString::Parse("Picked Sword");
    picked.statProfile = "sceptre";
    sides.file.items.push_back(picked);
    std::erase_if(sides.file.items,
                  [](const ItemDefinition& item) { return MakeItemType(item.group, item.number) == ShortSwordType; });
    sides.file.formulas->excellentDropLevelBonus = 30;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));

    g_MuItemSyncUI.Compare(file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK(g_MuItemSyncUI.GetDiff()->items.size() == 4);
    CHECK(g_MuItemSyncUI.GetDiff()->formulas.size() == 1);
    REQUIRE(g_MuItemSyncUI.Apply());

    CHECK(g_ItemDatabase.Find(KrisType)->width == kris.width + 1);
    // Its client-only fields stay.
    CHECK(g_ItemDatabase.Find(KrisType)->weaponKind == kris.weaponKind);
    const ItemDefinition* newSword = g_ItemDatabase.Find(MakeItemType(0, 100));
    REQUIRE(newSword != nullptr);
    CHECK(newSword->twoHanded);
    CHECK(newSword->weaponKind == WeaponKind::Sword);
    CHECK(newSword->statProfile == "weapon");
    const ItemDefinition* pickedSword = g_ItemDatabase.Find(MakeItemType(0, 101));
    REQUIRE(pickedSword != nullptr);
    CHECK(pickedSword->statProfile == "sceptre");
    // The removal was not checked.
    CHECK(g_ItemDatabase.Find(ShortSwordType) != nullptr);
    CHECK(g_ItemDataHandler.GetStatFile().excellentDropLevelBonus == 30);
    CHECK(g_ItemDataHandler.GetStatFile().profileRules == statFile.profileRules);
    CHECK(g_ItemEdits.GetHistory().GetUndoLabel() == I18N::Editor::ImportItems);
    // What is left: the removal.
    REQUIRE(g_MuItemSyncUI.GetDiff());
    REQUIRE(g_MuItemSyncUI.GetDiff()->items.size() == 1);
    CHECK(g_MuItemSyncUI.GetDiff()->items[0].change == SyncChange::Removed);

    REQUIRE(g_ItemEdits.Undo());
    CHECK(g_ItemDatabase.Find(KrisType)->width == kris.width);
    CHECK(g_ItemDatabase.Find(MakeItemType(0, 100)) == nullptr);
    CHECK(g_ItemDatabase.Find(MakeItemType(0, 101)) == nullptr);
    CHECK(g_ItemDataHandler.GetStatFile() == statFile);
    g_ItemEdits.GetHistory().Clear();
}

// OpenMU's export adds its fields one at a time: only the fields a file names are compared and
// taken, and an item only the file has is not added while the file lacks fields.
TEST_CASE("A file with a field list changes only those fields and adds no item [data][items][editor]")
{
    Sides sides;
    sides.file.fields = std::set<std::string, std::less<>>{"height", "width"};
    const ItemDefinition kris = Sides::Find(sides.client, KrisType);
    ItemDefinition& fileKris = sides.FileItem(KrisType);
    fileKris.width = static_cast<BYTE>(kris.width + 1);
    fileKris.damageMin = static_cast<WORD>(kris.damageMin + 7);
    // A file without names.
    fileKris.names = Data::LocalizedString{};
    ItemDefinition added = fileKris;
    added.number = 200;
    sides.file.items.push_back(added);

    const ItemSyncDiff diff = sides.Diff();
    const SyncItem* row = FindRow(diff, KrisType);
    REQUIRE(row != nullptr);
    CHECK(row->name == "Kris");
    REQUIRE(row->fields.size() == 1);
    CHECK(row->fields[0].field == "width");
    CHECK(row->fields[0].checked);
    const SyncItem* addedRow = FindRow(diff, MakeItemType(0, 200));
    REQUIRE(addedRow != nullptr);
    CHECK(addedRow->change == SyncChange::New);
    CHECK_FALSE(addedRow->checked);
    CHECK(addedRow->locked == std::string(I18N::Editor::TheFileDoesNotCarryAllFieldsOfAnItemItIsNotAdded));

    std::vector<ItemDataIssue> issues;
    ItemExchangeFile applied = ApplyItemSyncDiff(sides.client, sides.file, diff, issues);
    CHECK_FALSE(HasFatal(issues));
    const ItemDefinition& appliedKris = Sides::Find(applied, KrisType);
    CHECK(appliedKris.width == kris.width + 1);
    CHECK(appliedKris.damageMin == kris.damageMin);
    CHECK(appliedKris.names == kris.names);
    CHECK(std::ranges::none_of(applied.items, [](const ItemDefinition& item)
                               { return MakeItemType(item.group, item.number) == MakeItemType(0, 200); }));
}

// Import and Update from server list an item only the client has apart from the others, unchecked;
// Check all checks it too (Undo takes back an import), Check none unchecks it.
TEST_CASE("Check all checks the items only the client has too [data][items][editor]")
{
    Sides sides;
    ++sides.FileItem(KrisType).width;
    std::erase_if(sides.file.items,
                  [](const ItemDefinition& item) { return MakeItemType(item.group, item.number) == ShortSwordType; });

    ItemSyncDiff diff = sides.Diff();
    const SyncItem* krisRow = FindRow(diff, KrisType);
    const SyncItem* shortSwordRow = FindRow(diff, ShortSwordType);
    REQUIRE(krisRow != nullptr);
    REQUIRE(shortSwordRow != nullptr);
    SyncItem& kris = diff.items[static_cast<size_t>(krisRow - diff.items.data())];
    SyncItem& shortSword = diff.items[static_cast<size_t>(shortSwordRow - diff.items.data())];
    REQUIRE(kris.fields.size() == 1);
    CHECK_FALSE(IsOnlyInClient(kris));
    CHECK(IsOnlyInClient(shortSword));
    CHECK_FALSE(shortSword.checked);

    SetAllRowsChecked(diff, false);
    CHECK_FALSE(kris.fields[0].checked);
    SetAllRowsChecked(diff, true);
    CHECK(kris.fields[0].checked);
    CHECK(shortSword.checked);

    SetAllRowsChecked(diff, false);
    CHECK_FALSE(shortSword.checked);
    SetRowChecked(shortSword, true);
    CHECK(shortSword.checked);
}

TEST_CASE("Copy as Markdown writes the rows as a table [data][items][editor]")
{
    SyncItem added;
    added.group = 12;
    added.number = 73;
    added.change = SyncChange::New;
    added.name = "Sphere (4)";
    added.fields.resize(2);
    SyncItem removed;
    removed.group = 14;
    removed.number = 15;
    removed.change = SyncChange::Removed;
    removed.name = "Zen | Gold\nCoin";
    const std::vector<const SyncItem*> rows = {&added, &removed};

    const std::string onlyInClient = "Items only in the client (2)\n\n"
                                     "| Group | Number | Item |\n"
                                     "|---|---|---|\n"
                                     "| 12 | 73 | Sphere (4) |\n"
                                     "| 14 | 15 | Zen \\| Gold Coin |\n";
    CHECK(WriteSyncRowsMarkdown(rows, "Items only in the client (2)", "") == onlyInClient);
    CHECK(WriteSyncRowsMarkdown(rows, "Items that differ (2)", "In the client") ==
          "Items that differ (2)\n\n"
          "| In the client | Group | Number | Item | Fields |\n"
          "|---|---|---|---|---|\n"
          "| Added | 12 | 73 | Sphere (4) | 2 |\n"
          "| Removed | 14 | 15 | Zen \\| Gold Coin | 0 |\n");
}

// Update from server compares the downloaded text as Import compares a file, and Apply compares
// it again afterwards, without a path.
TEST_CASE("Item Sync applies a file from the server [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    sides.file.fields = std::set<std::string, std::less<>>{"height", "width"};
    ++sides.FileItem(KrisType).width;
    const std::string url = "http://localhost:8080/api/items/exchange";

    g_MuItemSyncUI.CompareServerFile(WriteItemExchangeJson(sides.file), url);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    REQUIRE(g_MuItemSyncUI.GetDiff()->items.size() == 1);
    CHECK(g_MuItemSyncUI.GetApplyBlocker() == CMuItemSyncUI::ApplyBlocker::None);
    REQUIRE(g_MuItemSyncUI.Apply());
    CHECK(g_ItemDatabase.Find(KrisType)->width == kris.width + 1);
    CHECK(g_ItemDatabase.Find(KrisType)->damageMin == kris.damageMin);
    // Compared again from the same text: nothing is left.
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK(g_MuItemSyncUI.GetDiff()->items.empty());

    REQUIRE(g_ItemEdits.Undo());
    CHECK(g_ItemDatabase.Find(KrisType)->width == kris.width);
    g_ItemEdits.GetHistory().Clear();
}

// Sphere 4 and 5: the client has their numbers as entries without a name (models, no item), the
// server has the items. Neither side has an item where the server has nothing.
TEST_CASE("Update from server fills the client's entry without a name [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    const int sphere4Type = MakeItemType(12, 73);
    const ItemDefinition before = g_ItemDatabase.GetAllSlots()[sphere4Type];
    REQUIRE(before.entryWithoutName);
    ItemDefinition sphere4;
    sphere4.group = 12;
    sphere4.number = 73;
    sphere4.names = Data::LocalizedString::Parse("Sphere (4)");
    // A size the client's entry does not have (it is 1x1).
    sphere4.width = 2;
    sphere4.height = 2;
    sphere4.statProfile = std::string(StatProfileNames::Default);
    sides.file.items.push_back(sphere4);
    const std::string url = "http://localhost:8080/api/items/exchange";

    // Without fields there is nothing to compare: no row, as for every other item.
    sides.file.fields = std::set<std::string, std::less<>>{};
    g_MuItemSyncUI.CompareServerFile(WriteItemExchangeJson(sides.file), url);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK(g_MuItemSyncUI.GetDiff()->items.empty());

    // Its size is a change of the entry, applied to it: it stays an entry without a name.
    sides.file.fields = std::set<std::string, std::less<>>{"height", "width"};
    g_MuItemSyncUI.CompareServerFile(WriteItemExchangeJson(sides.file), url);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    REQUIRE(g_MuItemSyncUI.GetDiff()->items.size() == 1);
    const SyncItem& row = g_MuItemSyncUI.GetDiff()->items.front();
    CHECK(MakeItemType(row.group, row.number) == sphere4Type);
    CHECK(row.change == SyncChange::Changed);
    CHECK(row.locked.empty());
    CHECK(row.fields.size() == 2);
    REQUIRE(g_MuItemSyncUI.Apply());
    const ItemDefinition& entry = g_ItemDatabase.GetAllSlots()[sphere4Type];
    CHECK(entry.entryWithoutName);
    CHECK(entry.width == 2);
    CHECK(entry.height == 2);
    CHECK(g_ItemDatabase.Find(sphere4Type) == nullptr);

    // Its name makes it the item.
    sides.file.fields = std::set<std::string, std::less<>>{"height", "name", "width"};
    g_MuItemSyncUI.CompareServerFile(WriteItemExchangeJson(sides.file), url);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    REQUIRE(g_MuItemSyncUI.GetDiff()->items.size() == 1);
    REQUIRE(g_MuItemSyncUI.Apply());
    REQUIRE(g_ItemDatabase.Find(sphere4Type) != nullptr);
    CHECK(g_ItemDatabase.Find(sphere4Type)->names.GetNeutral() == "Sphere (4)");
    CHECK_FALSE(g_ItemDatabase.Find(sphere4Type)->entryWithoutName);

    REQUIRE(g_ItemEdits.Undo());
    REQUIRE(g_ItemEdits.Undo());
    CHECK(g_ItemDatabase.GetAllSlots()[sphere4Type] == before);
    g_ItemEdits.GetHistory().Clear();
}

TEST_CASE("The API key is kept per admin panel address [data][items][editor]")
{
    CHECK(GetApiKeySecretName(" https://admin.example.org/ ") == "OpenMU API key https://admin.example.org");
    CHECK(GetApiKeySecretName("http://localhost:8080") == "OpenMU API key http://localhost:8080");
}

// A secret of its own goes into the system's key store and out again. A build without a store,
// or a store that refuses (a locked keychain on a build machine), has nothing to test.
TEST_CASE("The key store keeps, replaces and removes a secret [data][items][editor]")
{
    if (!MuEditor::SecretStore::IsAvailable())
    {
        MESSAGE("this build has no key store");
        return;
    }
    const std::string name = "test " + std::to_string(std::random_device{}());
    if (!MuEditor::SecretStore::Write(name, "first key"))
    {
        MESSAGE("the key store refuses secrets here");
        return;
    }
    CHECK(MuEditor::SecretStore::Read(name) == std::optional<std::string>("first key"));
    CHECK(MuEditor::SecretStore::Write(name, "second key"));
    CHECK(MuEditor::SecretStore::Read(name) == std::optional<std::string>("second key"));
    CHECK(MuEditor::SecretStore::Remove(name));
    CHECK_FALSE(MuEditor::SecretStore::Read(name).has_value());
    CHECK(MuEditor::SecretStore::Remove(name));
}

TEST_CASE("Update from server needs an address and an API key [data][items][editor]")
{
    // The address field is empty in a new editor.
    CHECK_FALSE(g_MuItemSyncUI.UpdateFromServer());
    CHECK(g_MuItemSyncUI.GetServerMessage() == std::string(I18N::Editor::EnterTheAddressOfTheOpenMUAdminPanel));
}

// Against a running OpenMU: MU_TEST_OPENMU_URL is the exchange file's address (e.g.
// http://127.0.0.1:18080/api/items/exchange) and MU_OPENMU_API_KEY a Viewer key. Without them the
// test does nothing. The file must read without errors, and the diff may only list the fields the
// file names.
TEST_CASE("Update from server reads the file of a running OpenMU [data][items][editor]")
{
    const char* url = std::getenv("MU_TEST_OPENMU_URL");
    if (url == nullptr || ReadOpenMUApiKey().empty())
    {
        MESSAGE("MU_TEST_OPENMU_URL or MU_OPENMU_API_KEY is not set: no server to read");
        return;
    }
    Sides sides;
    CItemSyncDownload download;
    REQUIRE(download.Start(url, ReadOpenMUApiKey()));
    std::optional<ServerDownload> result;
    for (int attempt = 0; attempt < 1200 && !result; ++attempt)
    {
        result = download.TakeResult();
        if (!result)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    REQUIRE(result);
    REQUIRE(result->problem == ServerUpdateProblem::None);

    std::vector<ItemDataIssue> issues;
    ItemExchangeFile file;
    ReadItemExchangeJson(result->body, url, file, issues);
    for (const ItemDataIssue& issue : issues)
    {
        MESSAGE(issue.ToString());
    }
    REQUIRE(CanImportItemExchange(file, issues));
    CHECK(file.source == ItemExchangeSourceOpenMU);

    g_MuItemSyncUI.CompareServerFile(result->body, url);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    for (const SyncItem& row : g_MuItemSyncUI.GetDiff()->items)
    {
        for (const SyncField& field : row.fields)
        {
            CAPTURE(field.field);
            CHECK(CarriesField(file, std::string_view(field.field).substr(0, field.field.find('.'))));
        }
        if (row.change == SyncChange::New && !CarriesEveryField(file))
        {
            CHECK_FALSE(row.checked);
        }
    }
    MESSAGE("rows: " << g_MuItemSyncUI.GetDiff()->items.size()
                     << ", same items: " << g_MuItemSyncUI.GetDiff()->sameItems);
}

// The checks of a save run on the data as it would be: an item that names a profile the
// formulas have not got refuses the import, and nothing changes.
TEST_CASE("Item Sync refuses an import the checks of a save refuse [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    ItemDefinition& fileKris = sides.FileItem(KrisType);
    fileKris.statProfile = "rune";
    ++fileKris.width;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));

    g_MuItemSyncUI.Compare(file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK_FALSE(g_MuItemSyncUI.Apply());
    CHECK(*g_ItemDatabase.Find(KrisType) == kris);
    CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());
    // The item's row is unchecked.
    const SyncItem* row = FindRow(*g_MuItemSyncUI.GetDiff(), KrisType);
    REQUIRE(row != nullptr);
    CHECK(std::ranges::none_of(row->fields, [](const SyncField& field) { return field.checked; }));
    CHECK(g_MuItemSyncUI.GetMessages().size() > 1);
}

// The window's Export writes the client's items and formulas without a diff: into a new file right
// away, into a file that is there after the confirmation that says what changes in it, and not at
// all into a file that has the client's data already.
TEST_CASE("Item Sync exports the client's items and formulas [data][items][editor]")
{
    Sides sides;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    g_MuItemSyncUI.Export(file);
    CHECK_FALSE(g_MuItemSyncUI.GetPendingExport());
    CHECK_FALSE(g_MuItemSyncUI.GetExportResult().empty());
    const std::optional<std::string> exported = Data::Items::Json::ReadTextFile(file);
    REQUIRE(exported);
    std::vector<ItemDataIssue> issues;
    ItemExchangeFile written;
    ReadItemExchangeJson(*exported, file.generic_string(), written, issues);
    CHECK(issues.empty());
    CHECK(written.source == ItemExchangeSourceMuMain);
    CHECK(written.items == sides.client.items);
    CHECK(written.formulas == sides.client.formulas);

    SUBCASE("a file that has the client's data already")
    {
        g_MuItemSyncUI.Export(file);
        CHECK_FALSE(g_MuItemSyncUI.GetPendingExport());
        CHECK_FALSE(g_MuItemSyncUI.GetExportResult().empty());
        CHECK(Data::Items::Json::ReadTextFile(file) == exported);
    }
    SUBCASE("a file that is there asks first, with what changes in it")
    {
        // One item changed, one only in the file, one only in the client, one formula entry.
        ItemExchangeFile other = sides.client;
        ++Sides::Find(other, KrisType).width;
        ItemDefinition extra = Sides::Find(other, KrisType);
        extra.number = 100;
        other.items.push_back(extra);
        std::erase_if(other.items, [](const ItemDefinition& item)
                      { return MakeItemType(item.group, item.number) == ShortSwordType; });
        other.formulas->excellentDropLevelBonus = 30;
        WriteFile(file, WriteItemExchangeJson(other));
        const std::optional<std::string> before = Data::Items::Json::ReadTextFile(file);

        g_MuItemSyncUI.Export(file);
        REQUIRE(g_MuItemSyncUI.GetPendingExport());
        const ItemSyncExportPreview preview = *g_MuItemSyncUI.GetPendingExport();
        CHECK(preview.readable);
        CHECK(preview.added == 1);
        CHECK(preview.changed == 1);
        CHECK(preview.removed == 1);
        CHECK(preview.formulaEntries == 1);
        CHECK(Data::Items::Json::ReadTextFile(file) == before);
        SUBCASE("cancelled")
        {
            g_MuItemSyncUI.CancelExport();
            CHECK_FALSE(g_MuItemSyncUI.ConfirmExport());
            CHECK(Data::Items::Json::ReadTextFile(file) == before);
        }
        SUBCASE("confirmed")
        {
            REQUIRE(g_MuItemSyncUI.ConfirmExport());
            CHECK_FALSE(g_MuItemSyncUI.GetPendingExport());
            CHECK(Data::Items::Json::ReadTextFile(file) == exported);
        }
    }
    SUBCASE("a file that is not an exchange file asks first too")
    {
        WriteFile(file, "not an exchange file");
        g_MuItemSyncUI.Export(file);
        REQUIRE(g_MuItemSyncUI.GetPendingExport());
        CHECK_FALSE(g_MuItemSyncUI.GetPendingExport()->readable);
        REQUIRE(g_MuItemSyncUI.ConfirmExport());
        CHECK(Data::Items::Json::ReadTextFile(file) == exported);
    }
}

// Compare two files lists, read-only, what changes from file A to file B, in every language (also
// one only file A has); nothing can be applied, and an import shows the import again.
TEST_CASE("Item Sync compares two files read-only [data][items][editor]")
{
    Sides sides;
    const ItemDefinition clientKris = *g_ItemDatabase.Find(KrisType);
    TemporaryFolder folder;
    const std::filesystem::path a = folder.Path() / "Client.json";
    const std::filesystem::path b = folder.Path() / "Server.json";
    ItemExchangeFile fileA = sides.client;
    ItemExchangeFile fileB = sides.client;
    fileB.source = ItemExchangeSourceOpenMU;
    ++Sides::Find(fileB, KrisType).width;
    Sides::Find(fileA, ShortSwordType).names.Set("zh-CN", "Short Sword (zh)");
    ItemDefinition added = Sides::Find(fileB, KrisType);
    added.number = 100;
    added.names = Data::LocalizedString::Parse("New Sword");
    fileB.items.push_back(added);
    fileB.formulas->excellentDropLevelBonus = 30;
    // Only file A has (0,101), and file A has (0,3) twice, as an OpenMU export can.
    ItemDefinition onlyInA = Sides::Find(fileA, KrisType);
    onlyInA.number = 101;
    onlyInA.names = Data::LocalizedString::Parse("Old Sword");
    fileA.items.push_back(onlyInA);
    const int twiceType = MakeItemType(0, 3);
    fileA.items.push_back(Sides::Find(fileA, twiceType));
    WriteFile(a, WriteItemExchangeJson(fileA));
    WriteFile(b, WriteItemExchangeJson(fileB));

    g_MuItemSyncUI.CompareFiles(a, b);
    CHECK(g_MuItemSyncUI.IsComparingFiles());
    REQUIRE(g_MuItemSyncUI.GetDiff());
    const ItemSyncDiff& diff = *g_MuItemSyncUI.GetDiff();
    CHECK(diff.items.size() == 5);
    const SyncItem* removed = FindRow(diff, MakeItemType(0, 101));
    REQUIRE(removed != nullptr);
    CHECK(removed->change == SyncChange::Removed);
    const SyncField* removedName = FindField(*removed, "name.en");
    REQUIRE(removedName != nullptr);
    CHECK(removedName->current.has_value());
    CHECK_FALSE(removedName->incoming.has_value());
    // File B has it once: changed, with the name file B has, locked.
    const SyncItem* twice = FindRow(diff, twiceType);
    REQUIRE(twice != nullptr);
    CHECK(twice->change == SyncChange::Changed);
    CHECK_FALSE(twice->name.empty());
    CHECK(twice->locked == std::string(I18N::Editor::ItIsInFileAMoreThanOnce));
    const SyncItem* kris = FindRow(diff, KrisType);
    REQUIRE(kris != nullptr);
    CHECK(kris->change == SyncChange::Changed);
    CHECK(FindField(*kris, "width") != nullptr);
    const SyncItem* shortSword = FindRow(diff, ShortSwordType);
    REQUIRE(shortSword != nullptr);
    const SyncField* name = FindField(*shortSword, "name.zh-CN");
    REQUIRE(name != nullptr);
    CHECK(name->current.has_value());
    CHECK_FALSE(name->incoming.has_value());
    const SyncItem* newSword = FindRow(diff, MakeItemType(0, 100));
    REQUIRE(newSword != nullptr);
    CHECK(newSword->change == SyncChange::New);
    CHECK(FindFormula(diff, "dropLevelBonus.excellent") != nullptr);

    CHECK_FALSE(g_MuItemSyncUI.Apply());
    CHECK(*g_ItemDatabase.Find(KrisType) == clientKris);
    g_MuItemSyncUI.Compare(b);
    CHECK_FALSE(g_MuItemSyncUI.IsComparingFiles());
}

// The import is refused while nothing is checked, and while the client's data changed since the
// comparison: the diff shows the data as it was.
TEST_CASE("Item Sync applies nothing while nothing is checked or the client's data changed [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    ++sides.FileItem(KrisType).width;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));

    g_MuItemSyncUI.Compare(file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK(g_MuItemSyncUI.GetApplyBlocker() == CMuItemSyncUI::ApplyBlocker::None);
    SUBCASE("nothing checked")
    {
        for (SyncItem& row : g_MuItemSyncUI.GetDiff()->items)
        {
            row.checked = false;
            for (SyncField& field : row.fields)
            {
                field.checked = false;
            }
        }
        for (SyncFormula& row : g_MuItemSyncUI.GetDiff()->formulas)
        {
            row.checked = false;
        }
        CHECK(g_MuItemSyncUI.GetApplyBlocker() == CMuItemSyncUI::ApplyBlocker::NothingChecked);
    }
    SUBCASE("the client's data changed")
    {
        ItemDefinition shortSword = *g_ItemDatabase.Find(ShortSwordType);
        shortSword.damageMax = static_cast<BYTE>(shortSword.damageMax + 1);
        REQUIRE(g_ItemEdits.SetItem("test", shortSword));
        CHECK(g_MuItemSyncUI.GetApplyBlocker() == CMuItemSyncUI::ApplyBlocker::DataChanged);
        // Undone, the data is as compared, but it changed twice since: compare again.
        REQUIRE(g_ItemEdits.Undo());
        CHECK(g_MuItemSyncUI.GetApplyBlocker() == CMuItemSyncUI::ApplyBlocker::DataChanged);
    }
    CHECK_FALSE(g_MuItemSyncUI.Apply());
    CHECK(g_MuItemSyncUI.GetResult().empty());
    CHECK(g_ItemDatabase.Find(KrisType)->width == kris.width);
    g_ItemEdits.GetHistory().Clear();
}

// After a partial Apply the window compares again: the rows that were not checked stay listed,
// and unchecked, so a second click does not take them after all. The confirmation counts what
// is checked.
TEST_CASE("Item Sync leaves what was not checked unchecked after an Apply [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    const BYTE krisWidth = g_ItemDatabase.Find(KrisType)->width;
    ++sides.FileItem(KrisType).width;
    ++sides.FileItem(ShortSwordType).width;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));

    g_MuItemSyncUI.Compare(file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    std::vector<SyncItem>& rows = g_MuItemSyncUI.GetDiff()->items;
    const auto shortSword = std::ranges::find_if(rows, [](const SyncItem& row)
                                                 { return MakeItemType(row.group, row.number) == ShortSwordType; });
    REQUIRE(shortSword != rows.end());
    for (SyncField& field : shortSword->fields)
    {
        field.checked = false;
    }
    const CMuItemSyncUI::CheckedChanges changes = g_MuItemSyncUI.CountCheckedChanges();
    CHECK(changes.added == 0);
    CHECK(changes.changed == 1);
    CHECK(changes.removed == 0);
    CHECK(changes.formulaEntries == 0);
    REQUIRE(g_MuItemSyncUI.Apply());
    CHECK(g_ItemDatabase.Find(KrisType)->width == krisWidth + 1);

    REQUIRE(g_MuItemSyncUI.GetDiff());
    REQUIRE(g_MuItemSyncUI.GetDiff()->items.size() == 1);
    const SyncItem& left = g_MuItemSyncUI.GetDiff()->items[0];
    CHECK(MakeItemType(left.group, left.number) == ShortSwordType);
    CHECK_FALSE(left.fields.empty());
    CHECK(std::ranges::none_of(left.fields, &SyncField::checked));
    CHECK(g_MuItemSyncUI.GetApplyBlocker() == CMuItemSyncUI::ApplyBlocker::NothingChecked);
    CHECK_FALSE(g_MuItemSyncUI.GetResult().empty());
    REQUIRE(g_ItemEdits.Undo());
    g_ItemEdits.GetHistory().Clear();
}

// A new item whose fields changed after the comparison (e.g. a value set back to its default) is
// taken without that field instead of failing.
TEST_CASE("Applying a diff takes a new item that lost a field since the comparison [data][items][editor]")
{
    Sides sides;
    ItemDefinition added = sides.FileItem(KrisType);
    added.number = 100;
    added.names = Data::LocalizedString::Parse("New Sword");
    sides.file.items.push_back(added);
    const int addedType = MakeItemType(0, 100);
    const ItemSyncDiff diff = sides.Diff();
    const SyncItem* row = FindRow(diff, addedType);
    REQUIRE(row != nullptr);
    REQUIRE(row->change == SyncChange::New);
    REQUIRE(FindField(*row, "durability") != nullptr);
    Sides::Find(sides.file, addedType).durability = 0;

    std::vector<ItemDataIssue> issues;
    ItemExchangeFile applied;
    CHECK_NOTHROW(applied = ApplyItemSyncDiff(sides.client, sides.file, diff, issues));
    CHECK(Sides::Find(applied, addedType).durability == 0);
}

// A changed row whose item the client no longer has (its data changed since the diff) is left out
// instead of failing.
TEST_CASE("Applying a diff leaves out a changed item the client no longer has [data][items][editor]")
{
    Sides sides;
    ++sides.FileItem(KrisType).width;
    const ItemSyncDiff diff = sides.Diff();
    REQUIRE(FindRow(diff, KrisType) != nullptr);
    std::erase_if(sides.client.items,
                  [](const ItemDefinition& item) { return MakeItemType(item.group, item.number) == KrisType; });

    std::vector<ItemDataIssue> issues;
    ItemExchangeFile applied;
    CHECK_NOTHROW(applied = ApplyItemSyncDiff(sides.client, sides.file, diff, issues));
    CHECK(std::ranges::none_of(applied.items, [](const ItemDefinition& item)
                               { return MakeItemType(item.group, item.number) == KrisType; }));
}

// OpenMU does not keep (group, number) unique: an item a file has twice is a locked row that
// the import leaves as it is, while the rest of the file is imported.
TEST_CASE("An item the file has twice is a locked row of the import [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    const BYTE shortSwordWidth = g_ItemDatabase.Find(ShortSwordType)->width;
    ItemDefinition copy = sides.FileItem(KrisType);
    ++copy.width;
    sides.file.items.push_back(copy);
    ++sides.FileItem(ShortSwordType).width;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));

    g_MuItemSyncUI.Compare(file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    const SyncItem* row = FindRow(*g_MuItemSyncUI.GetDiff(), KrisType);
    REQUIRE(row != nullptr);
    CHECK(row->change == SyncChange::Changed);
    CHECK(row->fields.empty());
    CHECK_FALSE(row->locked.empty());
    CHECK(g_MuItemSyncUI.GetMessages().size() == 1);
    REQUIRE(g_MuItemSyncUI.Apply());
    CHECK(*g_ItemDatabase.Find(KrisType) == kris);
    CHECK(g_ItemDatabase.Find(ShortSwordType)->width == shortSwordWidth + 1);

    REQUIRE(g_ItemEdits.Undo());
    g_ItemEdits.GetHistory().Clear();
}

TEST_CASE("The known differences are written and read back [data][items][editor]")
{
    KnownDifferences known;
    known.fields.insert({13, 4, "repairable"});
    known.fields.insert({13, 4, ""});
    known.fields.insert({7, 62, ""});
    known.formulas.insert("default.requirements.leadership");
    const std::string text = WriteKnownDifferencesJson(known);
    const std::filesystem::path directory = std::filesystem::temp_directory_path() /
                                            ("mu_test_known_differences_" + std::to_string(std::random_device{}()));
    std::filesystem::create_directories(directory);
    const std::filesystem::path file = directory / "KnownDifferences.json";
    std::ofstream(file, std::ios::binary) << text;
    std::vector<ItemDataIssue> issues;
    CHECK(LoadKnownDifferences(file, issues) == known);
    CHECK(issues.empty());
    CHECK(LoadKnownDifferences(directory / "missing.json", issues) == KnownDifferences{});
    CHECK(issues.empty());
    std::error_code ignored;
    std::filesystem::remove_all(directory, ignored);
}

#endif // _EDITOR
