#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "ItemStatProfilesScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "I18N/All.h"
#include "Engine/Object/ItemTooltipValues.h"
#include "Engine/Object/ZzzInventory.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

extern int TextNum;
extern int SkipNum;

using namespace Data::Items;

namespace
{
const std::filesystem::path ShippedClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

// Five units a letter: the description wraps at 200, so 40 letters fit a row.
int FiveUnitsPerLetter(const wchar_t*, size_t length)
{
    return static_cast<int>(length) * 5;
}

ItemDescriptionLine Line(const char* english, ItemTextColor color = ItemTextColor::White)
{
    ItemDescriptionLine line;
    line.text.Set("en", english);
    line.color = color;
    return line;
}

// The tooltip rows the description writes, from an empty list.
std::vector<std::wstring> WriteRows(const ItemDefinition& definition)
{
    TextNum = 0;
    SkipNum = 0;
    int rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(definition, 0, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    return {std::begin(TextList), std::begin(TextList) + TextNum};
}

std::vector<ItemDataIssue> DescriptionIssues(const ItemDefinition& definition, std::span<const char* const> uiLocales)
{
    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{definition}, issues);
    ValidateNameLocales(std::vector<ItemDefinition>{definition}, uiLocales, issues);
    std::erase_if(issues, [](const ItemDataIssue& issue) { return !issue.field.starts_with("description"); });
    return issues;
}

ItemDefinition MakeJewel()
{
    ItemDefinition jewel;
    jewel.group = 14;
    jewel.number = 13;
    jewel.names.Set("en", "Jewel of Bless");
    return jewel;
}
} // namespace

TEST_CASE("A blank line inside a description text is a blank tooltip row, not the tooltip's end [data][items]")
{
    ItemDefinition jewel = MakeJewel();
    jewel.description = {Line("Line one\n\nLine two"), Line("\nLeading break"), Line("")};

    const std::vector<std::wstring> rows = WriteRows(jewel);

    // An empty row would end the tooltip there: a blank one is "\n", as the tooltip writes its own.
    const std::vector<std::wstring> expected = {L"Line one", L"\n", L"Line two", L"\n", L"Leading break", L"\n"};
    CHECK(rows == expected);
    CHECK(std::none_of(rows.begin(), rows.end(), [](const std::wstring& row) { return row.empty(); }));
    CHECK(SkipNum == 3);
}

TEST_CASE("A description wraps to the tooltip's width in its line's color [data][items]")
{
    ItemDefinition jewel = MakeJewel();
    jewel.description = {
        Line("It is used to increase your item level up to 6 with a little more", ItemTextColor::Blue)};

    const std::vector<std::wstring> rows = WriteRows(jewel);

    REQUIRE(rows.size() == 2);
    CHECK(rows[0] == L"It is used to increase your item level");
    CHECK(rows[1] == L"up to 6 with a little more");
    CHECK(TextListColor[0] == TEXT_COLOR_BLUE);
    CHECK(TextListColor[1] == TEXT_COLOR_BLUE);
}

TEST_CASE("A description's placeholders are filled in for the item's level before the line wraps [data][items]")
{
    ItemStatFile file;
    LevelBonusTable table;
    table.name = "b";
    table.values[15] = 2000000000;
    file.tables.push_back(table);
    const ItemStatProfilesScope profiles;
    g_ItemDatabase.SetStatProfiles(ItemStatProfiles(file));

    ItemDefinition jewel;
    jewel.names.Set("en", "Jewel");
    jewel.description = {Line("+{level}: {table:b} more")};
    TextNum = 0;
    SkipNum = 0;
    int rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(jewel, 15, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    REQUIRE(TextNum == 1);
    CHECK(std::wstring(TextList[0]) == L"+15: 2000000000 more");

    // 40 letters fit a row. As written the line has 40; with the value at +15 it has 41 and wraps, at +0 it has 32.
    const std::string text = "{table:b} " + std::string(30, 'a');
    jewel.description = {Line(text.c_str())};
    TextNum = 0;
    rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(jewel, 15, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    CHECK(TextNum == 2);
    TextNum = 0;
    rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(jewel, 0, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    CHECK(TextNum == 1);
}

TEST_CASE("A description line is written only at the item levels it is shown at [data][items]")
{
    ItemDefinition scroll = MakeJewel();
    ItemDescriptionLine warp = Line("Warp after 3 seconds");
    warp.minLevel = 1;
    warp.maxLevel = 8;
    scroll.description = {Line("Every level"), warp};

    TextNum = 0;
    SkipNum = 0;
    int rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(scroll, 0, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    REQUIRE(TextNum == 1);
    CHECK(std::wstring(TextList[0]) == L"Every level");

    for (const int level : {1, 8})
    {
        INFO("+" << level);
        TextNum = 0;
        rowsLeft = MaxItemDescriptionRows;
        WriteItemDescriptionLines(scroll, level, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
        REQUIRE(TextNum == 2);
        CHECK(std::wstring(TextList[1]) == L"Warp after 3 seconds");
    }

    TextNum = 0;
    rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(scroll, 9, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    CHECK(TextNum == 1);
}

TEST_CASE("A description writes at most its own rows, leaving the tooltip's rows to the rest [data][items]")
{
    ItemDefinition jewel = MakeJewel();
    jewel.description.assign(MaxItemDescriptionRows + 10, Line("A line"));

    TextNum = 5;
    int rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(jewel, 0, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    CHECK(TextNum == 5 + MaxItemDescriptionRows);
    CHECK(rowsLeft == 0);

    // Near the end of the list it stops at the list's end.
    TextNum = MaxTextListRows - 3;
    rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(jewel, 0, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    CHECK(TextNum == MaxTextListRows);
}

TEST_CASE("A description's parts before and after the options share its rows [data][items]")
{
    ItemDefinition ring = MakeJewel();
    ItemDescriptionLine after = Line("After the options");
    after.afterOptions = true;
    ring.description.assign(20, Line("Before the options"));
    ring.description.insert(ring.description.end(), 20, after);

    TextNum = 0;
    int rowsLeft = MaxItemDescriptionRows;
    WriteItemDescriptionLines(ring, 0, ItemTooltipValues{}, false, rowsLeft, FiveUnitsPerLetter);
    CHECK(TextNum == 20);
    CHECK(rowsLeft == MaxItemDescriptionRows - 20);
    WriteItemDescriptionLines(ring, 0, ItemTooltipValues{}, true, rowsLeft, FiveUnitsPerLetter);
    CHECK(TextNum == MaxItemDescriptionRows);
    CHECK(rowsLeft == 0);
    CHECK(std::wstring(TextList[MaxItemDescriptionRows - 1]) == L"After the options");
}

TEST_CASE("Description texts are checked like item names [data][items]")
{
    const char* uiLocales[] = {"en", "de"};

    ItemDefinition jewel = MakeJewel();
    ItemDescriptionLine noEnglish;
    noEnglish.text.Set("de", "Nur Deutsch");
    ItemDescriptionLine badCode = Line("A text");
    badCode.text.Set("d e", "Ein Text");
    ItemDescriptionLine unknownLanguage = Line("A text");
    unknownLanguage.text.Set("xx", "Ein Text");
    jewel.description = {Line(""), noEnglish, badCode, unknownLanguage};

    const std::vector<ItemDataIssue> issues = DescriptionIssues(jewel, uiLocales);

    // The blank line has none; "d e" is no language code and, like "xx", no language the game shows.
    REQUIRE(issues.size() == 4);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[0].field == "description[1].text");
    CHECK(issues[0].message == "the English text is missing");
    CHECK(issues[1].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[1].field == "description[2].text");
    CHECK(issues[2].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[2].field == "description[2].text");
    CHECK(issues[3].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[3].field == "description[3].text");
}

TEST_CASE("Level values written in place are checked like description texts [data][items]")
{
    const char* uiLocales[] = {"en", "de"};

    ItemDefinition fruit = MakeJewel();
    ItemLevelValues stat;
    stat.name = "stat";
    ItemLevelValue english;
    english.text.Set("en", "ENG");
    ItemLevelValue noEnglish;
    noEnglish.text.Set("de", "ENE");
    ItemLevelValue unknownLanguage;
    unknownLanguage.text.Set("en", "STR");
    unknownLanguage.text.Set("xx", "FUE");
    stat.values = {english, noEnglish, unknownLanguage};
    fruit.levelValues = {stat};

    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{fruit}, issues);
    ValidateNameLocales(std::vector<ItemDefinition>{fruit}, uiLocales, issues);
    std::erase_if(issues, [](const ItemDataIssue& issue) { return !issue.field.starts_with("levelValues"); });

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[0].field == "levelValues.stat[1]");
    CHECK(issues[0].message == "the English text is missing");
    CHECK(issues[1].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[1].field == "levelValues.stat[2]");
}

TEST_CASE("Name line texts written in place are checked like description texts [data][items]")
{
    const char* uiLocales[] = {"en", "de"};

    ItemDefinition ring = MakeJewel();
    ItemNameLine noEnglish;
    noEnglish.text.Set("de", "Ring");
    noEnglish.hasText = true;
    ItemNameLine unknownLanguage;
    unknownLanguage.text.Set("en", "{name}");
    unknownLanguage.text.Set("xx", "{name}");
    unknownLanguage.hasText = true;
    ItemNameLine colorOnly;
    colorOnly.color = ItemTextColor::Yellow;
    colorOnly.hasColor = true;
    ring.nameLines = {noEnglish, unknownLanguage, colorOnly};

    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{ring}, issues);
    ValidateNameLocales(std::vector<ItemDefinition>{ring}, uiLocales, issues);
    std::erase_if(issues, [](const ItemDataIssue& issue) { return !issue.field.starts_with("nameLines"); });

    // A line without a text keeps the standard name and has nothing to check.
    REQUIRE(issues.size() == 2);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[0].field == "nameLines[0].text");
    CHECK(issues[1].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[1].field == "nameLines[1].text");
}

TEST_CASE("A description is in the UI's language also when the item names stay English [data][items]")
{
    ItemDefinition jewel = MakeJewel();
    ItemDescriptionLine line = Line("It is used to increase your item level up to 6");
    line.text.Set("de", "Wird verwendet, um das Gegenstandslevel auf bis zu 6 zu erhöhen");
    jewel.description = {line};

    // Once a test has loaded the item data, switching the UI language refills ItemAttribute, so the switch needs an
    // item table of its own.
    ClientDataScope client(ShippedClientDirectory);

    // "Translate item names" off: the item names are English, the UI German. The option keeps the names English
    // also when the loaded item data follows the language switch.
    const bool translateItemNames = g_ItemDataHandler.GetTranslateItemNames();
    const std::string itemNameLocale = g_ItemDatabase.GetDisplayLocale();
    const std::string uiLocale = I18N::GetCurrentLocale();
    g_ItemDataHandler.SetTranslateItemNames(false);
    g_ItemDatabase.SetDisplayLocale("en");
    I18N::SetLocale("de");
    const std::string namesLocale = g_ItemDatabase.GetDisplayLocale();
    const std::vector<std::wstring> rows = WriteRows(jewel);
    I18N::SetLocale(uiLocale.c_str());
    g_ItemDataHandler.SetTranslateItemNames(translateItemNames);
    g_ItemDatabase.SetDisplayLocale(itemNameLocale);

    CHECK(namesLocale == "en");
    REQUIRE_FALSE(rows.empty());
    CHECK(rows[0].starts_with(L"Wird verwendet"));
}

TEST_CASE("A level value is in the UI's language also when the item names stay English [data][items]")
{
    ItemDefinition fruit = MakeJewel();
    ItemLevelValues stat;
    stat.name = "stat";
    ItemLevelValue energy;
    energy.text.Set("en", "ENG");
    energy.text.Set("de", "ENE");
    stat.values = {energy};
    fruit.levelValues = {stat};
    ItemDescriptionLine line = Line("{stat} +1");
    line.text.Set("de", "{stat} +1");
    fruit.description = {line};

    // Once a test has loaded the item data, switching the UI language refills ItemAttribute, so the switch needs an
    // item table of its own.
    ClientDataScope client(ShippedClientDirectory);
    const std::string itemNameLocale = g_ItemDatabase.GetDisplayLocale();
    const std::string uiLocale = I18N::GetCurrentLocale();
    g_ItemDatabase.SetDisplayLocale("en");
    I18N::SetLocale("de");
    const std::vector<std::wstring> rows = WriteRows(fruit);
    I18N::SetLocale(uiLocale.c_str());
    g_ItemDatabase.SetDisplayLocale(itemNameLocale);

    REQUIRE(rows.size() == 1);
    CHECK(rows[0] == L"ENE +1");
}
