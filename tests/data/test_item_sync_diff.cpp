#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ClientDataScope.h"

#include "UI/ItemTools/ItemSyncDiff.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
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

TEST_CASE(
    "An item the game holds keeps its size and slot, and a known difference stays unchecked [data][items][editor]")
{
    Sides sides;
    ItemDefinition& kris = sides.FileItem(KrisType);
    ++kris.width;
    kris.repairable = !kris.repairable;
    std::erase_if(sides.file.items,
                  [](const ItemDefinition& item) { return MakeItemType(item.group, item.number) == ShortSwordType; });
    KnownDifferences known;
    known.fields.insert({0, 0, "repairable"});

    const ItemSyncDiff diff = sides.Diff(SyncDirection::Import, &known, [](int itemType)
                                         { return itemType == KrisType || itemType == ShortSwordType; });
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
