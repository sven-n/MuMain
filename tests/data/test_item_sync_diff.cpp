#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ClientDataScope.h"

#include "UI/ItemTools/ItemEdits.h"
#include "UI/ItemTools/ItemSyncDiff.h"
#include "UI/ItemTools/MuItemSyncUI.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
#include <string>
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

    ItemSyncDiff Diff(SyncDirection direction, const KnownDifferences* known = nullptr,
                      std::function<bool(int)> isHeld = {}) const
    {
        return BuildItemSyncDiff(client, file, {direction, known, UiLocales, std::move(isHeld)});
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
    CHECK(sides.Diff(SyncDirection::Import).IsEmpty());
    CHECK(sides.Diff(SyncDirection::Export).IsEmpty());
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

    ItemSyncDiff diff = sides.Diff(SyncDirection::Import);
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
    CHECK(sides.Diff(SyncDirection::Import).IsEmpty());
    // An export writes the client's Spanish name into the file, and leaves its other one.
    const ItemSyncDiff exported = sides.Diff(SyncDirection::Export);
    const SyncItem* row = FindRow(exported, KrisType);
    REQUIRE(row != nullptr);
    REQUIRE(row->fields.size() == 1);
    CHECK(row->fields[0].field == "name.es");
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
        sides.Diff(SyncDirection::Import, &known, [](int itemType)
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
    const ItemSyncDiff diff = sides.Diff(SyncDirection::Import);
    const SyncItem* row = FindRow(diff, ITEM_HORN_OF_FENRIR);
    REQUIRE(row != nullptr);
    const SyncField* level = FindField(*row, "level");
    REQUIRE(level != nullptr);
    CHECK_FALSE(level->inexact.empty());
    CHECK_FALSE(level->checked);
}

TEST_CASE("An export writes the checked client values into the file [data][items][editor]")
{
    Sides sides;
    ItemDefinition& clientKris = Sides::Find(sides.client, KrisType);
    clientKris.damageMax = static_cast<BYTE>(clientKris.damageMax + 3);
    ItemDefinition extra = clientKris;
    extra.number = 101;
    sides.file.items.push_back(extra);

    const ItemSyncDiff diff = sides.Diff(SyncDirection::Export);
    const SyncItem* row = FindRow(diff, KrisType);
    REQUIRE(row != nullptr);
    REQUIRE(FindField(*row, "damageMax") != nullptr);
    const SyncItem* onlyInFile = FindRow(diff, MakeItemType(0, 101));
    REQUIRE(onlyInFile != nullptr);
    CHECK(onlyInFile->change == SyncChange::Removed);

    std::vector<ItemDataIssue> issues;
    ItemExchangeFile written = ApplyItemSyncDiff(sides.client, sides.file, diff, issues);
    CHECK(Sides::Find(written, KrisType).damageMax == clientKris.damageMax);
    // The removal is unchecked: the file keeps its item.
    CHECK(Sides::Find(written, MakeItemType(0, 101)).Exists());
    CHECK(written.source == ItemExchangeSourceOpenMU);
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

    ItemSyncDiff diff = sides.Diff(SyncDirection::Import);
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
    ItemSyncDiff diff = sides.Diff(SyncDirection::Import);
    // Only the term, not the table it names.
    for (SyncFormula& row : diff.formulas)
    {
        row.checked = row.entry != "tables.runeTable";
    }
    std::vector<ItemDataIssue> issues;
    const ItemExchangeFile applied = ApplyItemSyncDiff(sides.client, sides.file, diff, issues);
    CHECK(HasErrors(issues));
    CHECK(*applied.formulas == *sides.client.formulas);
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
    std::erase_if(sides.file.items,
                  [](const ItemDefinition& item) { return MakeItemType(item.group, item.number) == ShortSwordType; });
    sides.file.formulas->excellentDropLevelBonus = 30;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));

    g_MuItemSyncUI.Compare(SyncDirection::Import, file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK(g_MuItemSyncUI.GetDiff()->items.size() == 3);
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
    CHECK(g_ItemDataHandler.GetStatFile() == statFile);
    g_ItemEdits.GetHistory().Clear();
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

    g_MuItemSyncUI.Compare(SyncDirection::Import, file);
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

// The window's Export into a file that is not there yet: all items and the formulas.
TEST_CASE("Item Sync exports the client's data into a new file [data][items][editor]")
{
    Sides sides;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    g_MuItemSyncUI.Compare(SyncDirection::Export, file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK(g_MuItemSyncUI.GetDiff()->items.size() == sides.client.items.size());
    CHECK(g_MuItemSyncUI.GetDiff()->comparedItems == static_cast<int>(sides.client.items.size()));
    CHECK(g_MuItemSyncUI.GetDiff()->sameItems == 0);
    REQUIRE(g_MuItemSyncUI.GetDiff()->formulas.size() == 1);
    CHECK(g_MuItemSyncUI.GetDiff()->formulas[0].entry == AllFormulasEntry);
    CHECK(g_MuItemSyncUI.GetApplyBlocker() == CMuItemSyncUI::ApplyBlocker::None);
    REQUIRE(g_MuItemSyncUI.Apply());

    const std::optional<std::string> text = Data::Items::Json::ReadTextFile(file);
    REQUIRE(text);
    std::vector<ItemDataIssue> issues;
    ItemExchangeFile written;
    ReadItemExchangeJson(*text, file.generic_string(), written, issues);
    CHECK(issues.empty());
    CHECK(written.source == ItemExchangeSourceMuMain);
    CHECK(written.items == sides.client.items);
    CHECK(written.formulas == sides.client.formulas);
    // Nothing is left to export, and the window says what was written and that all is the same.
    REQUIRE(g_MuItemSyncUI.GetDiff());
    CHECK(g_MuItemSyncUI.GetDiff()->IsEmpty());
    CHECK_FALSE(g_MuItemSyncUI.GetResult().empty());
    CHECK(g_MuItemSyncUI.GetDiff()->sameItems == static_cast<int>(sides.client.items.size()));
    CHECK(g_MuItemSyncUI.GetDiff()->comparedFormulaEntries > 0);
    CHECK(g_MuItemSyncUI.GetDiff()->sameFormulaEntries == g_MuItemSyncUI.GetDiff()->comparedFormulaEntries);
}

// The write is refused while nothing is checked, and while the client's data changed since the
// comparison: an export would write the data as it is now, not as the diff shows it.
TEST_CASE("Item Sync applies nothing while nothing is checked or the client's data changed [data][items][editor]")
{
    Sides sides;
    g_ItemEdits.GetHistory().Clear();
    ++sides.FileItem(KrisType).width;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));
    const std::optional<std::string> before = Data::Items::Json::ReadTextFile(file);
    REQUIRE(before);

    g_MuItemSyncUI.Compare(SyncDirection::Export, file);
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
    CHECK(Data::Items::Json::ReadTextFile(file) == before);
    g_ItemEdits.GetHistory().Clear();
}

// After a partial Apply the window compares again: the rows that were not checked stay listed,
// and unchecked, so a second click does not take them after all.
TEST_CASE("Item Sync leaves what was not checked unchecked after an Apply [data][items][editor]")
{
    Sides sides;
    ++sides.FileItem(KrisType).width;
    ++sides.FileItem(ShortSwordType).width;
    TemporaryFolder folder;
    const std::filesystem::path file = folder.Path() / "ItemExchange.json";
    WriteFile(file, WriteItemExchangeJson(sides.file));

    g_MuItemSyncUI.Compare(SyncDirection::Export, file);
    REQUIRE(g_MuItemSyncUI.GetDiff());
    std::vector<SyncItem>& rows = g_MuItemSyncUI.GetDiff()->items;
    const auto shortSword = std::ranges::find_if(rows, [](const SyncItem& row)
                                                 { return MakeItemType(row.group, row.number) == ShortSwordType; });
    REQUIRE(shortSword != rows.end());
    for (SyncField& field : shortSword->fields)
    {
        field.checked = false;
    }
    REQUIRE(g_MuItemSyncUI.Apply());

    REQUIRE(g_MuItemSyncUI.GetDiff());
    REQUIRE(g_MuItemSyncUI.GetDiff()->items.size() == 1);
    const SyncItem& left = g_MuItemSyncUI.GetDiff()->items[0];
    CHECK(MakeItemType(left.group, left.number) == ShortSwordType);
    CHECK_FALSE(left.fields.empty());
    CHECK(std::ranges::none_of(left.fields, &SyncField::checked));
    CHECK(g_MuItemSyncUI.GetApplyBlocker() == CMuItemSyncUI::ApplyBlocker::NothingChecked);
    CHECK_FALSE(g_MuItemSyncUI.GetResult().empty());
}

// A new item whose client data lost a field after the comparison (e.g. a value set back to its
// default) is written without that field instead of failing.
TEST_CASE("An export takes a new item whose client data lost a field since the comparison [data][items][editor]")
{
    Sides sides;
    std::erase_if(sides.file.items,
                  [](const ItemDefinition& item) { return MakeItemType(item.group, item.number) == KrisType; });
    const ItemSyncDiff diff = sides.Diff(SyncDirection::Export);
    const SyncItem* row = FindRow(diff, KrisType);
    REQUIRE(row != nullptr);
    REQUIRE(row->change == SyncChange::New);
    REQUIRE(FindField(*row, "durability") != nullptr);
    Sides::Find(sides.client, KrisType).durability = 0;

    std::vector<ItemDataIssue> issues;
    ItemExchangeFile written;
    CHECK_NOTHROW(written = ApplyItemSyncDiff(sides.client, sides.file, diff, issues));
    CHECK(Sides::Find(written, KrisType).durability == 0);
}

// OpenMU does not keep (group, number) unique: an item a file has twice is a locked row that
// the import leaves as it is, while the rest of the file is imported. An export does not write
// into such a file, as both copies would be lost.
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

    g_MuItemSyncUI.Compare(SyncDirection::Import, file);
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
    g_MuItemSyncUI.Compare(SyncDirection::Export, file);
    CHECK_FALSE(g_MuItemSyncUI.GetDiff());
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
