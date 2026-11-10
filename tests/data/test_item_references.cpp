#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemNameCases.h"
#include "Data/GameData/ItemData/ItemReferenceNames.h"
#include "Data/Translation/ItemReference.h"
#include "Data/Translation/ItemReferenceChecks.h"
#include "Data/Translation/ItemTextExpansion.h"

#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <vector>

using namespace Data::Items;
using Data::Translation::ItemReference;
using Data::Translation::ParseItemReference;

namespace
{
ItemReference Reference(int group, int number, std::optional<int> level = std::nullopt, std::string form = {})
{
    return {group, number, level, std::move(form)};
}

ItemNameLine LiteralNameLine(const char* names, int level)
{
    ItemNameLine line;
    line.text = Data::LocalizedString::Parse(names);
    line.hasText = true;
    line.minLevel = level;
    line.maxLevel = level;
    return line;
}

// Tear of Elf (14,25) with a Polish name and its accusative, and Rena (14,21), whose level 2 is the Stone of
// Friendship.
std::vector<ItemDefinition> NamedItems()
{
    ItemDefinition tear;
    tear.group = 14;
    tear.number = 25;
    tear.names = Data::LocalizedString::Parse("Tear of Elf||de=Traene der Elfe||pl=Lza Elfa");
    tear.nameForms.Set("pl", "acc", "Lze Elfa");

    ItemDefinition rena;
    rena.group = 14;
    rena.number = 21;
    rena.names = Data::LocalizedString::Parse("Rena||pl=Rena");
    ItemNameLine own;
    own.text = Data::LocalizedString::Parse("{name}");
    own.hasText = true;
    own.minLevel = 0;
    own.maxLevel = 0;
    rena.nameLines.push_back(own);
    ItemNameLine stone = LiteralNameLine("Stone of Friendship||pl=Kamien Przyjazni", 2);
    stone.nameForms.Set("pl", "gen", "Kamienia Przyjazni");
    rena.nameLines.push_back(stone);
    ItemNameLine skill = LiteralNameLine("{skill} Stone", 3);
    rena.nameLines.push_back(skill);
    return {tear, rena};
}

// Polish declines names in every case, as NameCases.json has it.
void DeclinePolish(ItemDatabase& database)
{
    ItemNameCases cases;
    cases.SetCases("pl", {"gen", "dat", "acc", "ins", "loc"});
    database.SetNameCases(cases);
}
} // namespace

TEST_CASE("An item reference is written with its number, level and case [data][items][texts]")
{
    CHECK(ParseItemReference("{item:14,25}") == Reference(14, 25));
    CHECK(ParseItemReference("{item:13,20+3}") == Reference(13, 20, 3));
    CHECK(ParseItemReference("{item:14,25:acc}") == Reference(14, 25, std::nullopt, "acc"));
    CHECK(ParseItemReference("{item:14,21+2:gen}") == Reference(14, 21, 2, "gen"));
    CHECK(Data::Translation::FormatItemReference(Reference(14, 21, 2, "gen")) == "{item:14,21+2:gen}");

    for (const char* malformed : {"{item:14}", "{item:14;25}", "{item:14,25+}", "{item:14,25:abl}", "{item:14,25:}",
                                  "{item:14,12345}", "{item:,25}", "{item:14,25 }"})
    {
        INFO(malformed);
        CHECK_FALSE(ParseItemReference(malformed).has_value());
    }
}

TEST_CASE("The references of a text are found in order, malformed ones too [data][items][texts]")
{
    const std::string text = "Bring {item:14,17} and {item:14;18} and {item:12,15:gen}.";
    const auto matches = Data::Translation::FindItemReferences(text);
    REQUIRE(matches.size() == 3);
    CHECK(text.substr(matches[0].position, matches[0].length) == "{item:14,17}");
    CHECK(matches[0].reference == Reference(14, 17));
    CHECK_FALSE(matches[1].reference.has_value());
    CHECK(matches[2].reference == Reference(12, 15, std::nullopt, "gen"));
    CHECK_FALSE(Data::Translation::HasItemReference("No item here {name}"));

    // An unclosed reference ends where the next one starts.
    const std::string unclosed = "a {item:1 b {item:14,13} c";
    const auto split = Data::Translation::FindItemReferences(unclosed);
    REQUIRE(split.size() == 2);
    CHECK(unclosed.substr(split[0].position, split[0].length) == "{item:1 b ");
    CHECK_FALSE(split[0].reference.has_value());
    CHECK(unclosed.substr(split[1].position, split[1].length) == "{item:14,13}");
    CHECK(split[1].reference == Reference(14, 13));
}

TEST_CASE("An item text's placeholders may name items, and a translation may add a case [data][items][texts]")
{
    const PlaceholderScan scan = ScanPlaceholders("Combine {item:14,113} for a {item:14,123:acc}");
    CHECK(scan.error.empty());
    REQUIRE(scan.names.size() == 2);
    CHECK(scan.names[0] == "item:14,113");
    CHECK(IsItemReferencePlaceholder(scan.names[1]));
    CHECK(ToParityPlaceholders(scan.names) == std::set<std::string>{"item:14,113", "item:14,123"});

    const PlaceholderScan malformed = ScanPlaceholders("Combine {item:14;113}");
    CHECK(malformed.error.find("{item:group,number}") != std::string::npos);
}

TEST_CASE("A reference names the item in the text's language, its form, or its level's name line [data][items][texts]")
{
    ItemDatabase database;
    database.Build(NamedItems());
    DeclinePolish(database);

    CHECK(ResolveReferencedName(database, Reference(14, 25), "pl").text == "Lza Elfa");
    CHECK(ResolveReferencedName(database, Reference(14, 25), "en").text == "Tear of Elf");
    // A language without a name of its own shows the English name, which has no forms.
    const ReferencedName spanish = ResolveReferencedName(database, Reference(14, 25, std::nullopt, "acc"), "es");
    CHECK(spanish.text == "Tear of Elf");
    CHECK(spanish.problem == ItemReferenceProblem::None);

    CHECK(ResolveReferencedName(database, Reference(14, 25, std::nullopt, "acc"), "pl").text == "Lze Elfa");
    const ReferencedName missing = ResolveReferencedName(database, Reference(14, 25, std::nullopt, "gen"), "pl");
    CHECK(missing.text == "Lza Elfa");
    CHECK(missing.problem == ItemReferenceProblem::MissingForm);

    CHECK(ResolveReferencedName(database, Reference(14, 21, 0), "pl").text == "Rena");
    CHECK(ResolveReferencedName(database, Reference(14, 21, 2), "pl").text == "Kamien Przyjazni");
    CHECK(ResolveReferencedName(database, Reference(14, 21, 2, "gen"), "pl").text == "Kamienia Przyjazni");
    CHECK(ResolveReferencedName(database, Reference(14, 21, 2), "en").text == "Stone of Friendship");

    const ReferencedName standard = ResolveReferencedName(database, Reference(14, 21, 5), "en");
    CHECK(standard.text == "Rena +5");
    CHECK(standard.problem == ItemReferenceProblem::StandardName);
    // At +0 the standard name is the item's name: nothing to warn about.
    const ReferencedName plain = ResolveReferencedName(database, Reference(14, 25, 0), "en");
    CHECK(plain.text == "Tear of Elf");
    CHECK(plain.problem == ItemReferenceProblem::None);
    CHECK(ResolveReferencedName(database, Reference(14, 21, 3), "en").problem == ItemReferenceProblem::UnfillableLine);
    CHECK(ResolveReferencedName(database, Reference(14, 99), "en").problem == ItemReferenceProblem::UnknownItem);
    CHECK(ResolveReferencedName(database, Reference(99, 25), "en").problem == ItemReferenceProblem::UnknownItem);
    // +15 is the highest level an item has.
    CHECK(ResolveReferencedName(database, Reference(14, 21, 15), "en").text == "Rena +15");
    const ReferencedName tooHigh = ResolveReferencedName(database, Reference(14, 21, 16), "en");
    CHECK(tooHigh.text.empty());
    CHECK(tooHigh.problem == ItemReferenceProblem::NoSuchLevel);
}

TEST_CASE("A form shows only in a case its language declines in [data][items][texts]")
{
    ItemDatabase database;
    database.Build(NamedItems());

    // Without NameCases.json Polish declines in no case: the Tear of Elf's accusative does not show.
    const ReferencedName undeclined = ResolveReferencedName(database, Reference(14, 25, std::nullopt, "acc"), "pl");
    CHECK(undeclined.text == "Lza Elfa");
    CHECK(undeclined.problem == ItemReferenceProblem::MissingForm);
    std::vector<ItemDataIssue> issues;
    Data::Translation::CheckItemReferences(
        database, {"{item:14,25:acc}", "pl", "Game.pl.resx", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "Tear"}, {},
        issues);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].message.find("does not decline names in the case acc") != std::string::npos);

    DeclinePolish(database);
    CHECK(ResolveReferencedName(database, Reference(14, 25, std::nullopt, "acc"), "pl").text == "Lze Elfa");
}

TEST_CASE("Changing the name cases is a change of the item data and of its names [data][items][texts]")
{
    ItemDatabase database;
    database.Build(NamedItems());
    const int version = database.GetVersion();
    const int nameVersion = database.GetNameVersion();
    ItemNameCases cases;
    cases.SetCases("pl", {"gen"});
    database.SetNameCases(cases);
    // Data Checks sees a change of the data by the version, the texts that name items by the name version.
    CHECK(database.GetVersion() != version);
    CHECK(database.GetNameVersion() != nameVersion);
}

TEST_CASE("A name with a '%' is kept out of a UI text, a printf format, and not out of other texts [data][items][texts]")
{
    std::vector<ItemDefinition> items = NamedItems();
    ItemDefinition elixir;
    elixir.group = 14;
    elixir.number = 40;
    elixir.names = Data::LocalizedString::Parse("Elixir 10%||pl=Eliksir {x}");
    items.push_back(elixir);
    ItemDatabase database;
    database.Build(items);

    const ReferencedName percent = Data::Items::ResolveUiTextName(database, Reference(14, 40), "en");
    CHECK(percent.text.empty());
    CHECK(percent.problem == ItemReferenceProblem::UnshowableName);
    // Braces are no printf conversion; the item data's texts (the tooltip) take the '%' too.
    CHECK(Data::Items::ResolveUiTextName(database, Reference(14, 40), "pl").text == "Eliksir {x}");
    CHECK(ResolveReferencedName(database, Reference(14, 40), "en").text == "Elixir 10%");

    const auto check = [&](bool uiText)
    {
        std::vector<ItemDataIssue> issues;
        Data::Translation::CheckedText text{"Drink {item:14,40}", "en", "x", ItemDataIssue::NoItem,
                                            ItemDataIssue::NoItem, "Drink", uiText};
        Data::Translation::CheckItemReferences(database, text, {}, issues);
        return issues;
    };
    const std::vector<ItemDataIssue> uiIssues = check(true);
    REQUIRE(uiIssues.size() == 1);
    CHECK(uiIssues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(check(false).empty());
}

TEST_CASE("A UI translation that spells names English writes by number is a warning where items have names "
          "[data][items][texts]")
{
    ItemDatabase database;
    database.Build(NamedItems());
    DeclinePolish(database);
    const auto check = [&](const std::string& translation, std::string_view locale)
    {
        std::vector<ItemDataIssue> issues;
        Data::Translation::CheckItemReferences(
            database, {translation, locale, "Dialog.x.resx", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "Find"},
            "Find the '{item:14,25}'", issues);
        return issues;
    };

    // The Tear of Elf has a Polish name: the Polish text could name it by number.
    const std::vector<ItemDataIssue> polish = check("Znajdz 'Lze Elfa'", "pl");
    REQUIRE(polish.size() == 1);
    CHECK(polish[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(polish[0].group == 14);
    CHECK(polish[0].number == 25);
    // No Spanish or Russian name: those texts spell it on purpose.
    CHECK(check("Busca la 'Lagrima'", "es").empty());
    CHECK(check("Naidi 'Slezu'", "ru").empty());
    // A Polish text worded without the name has nothing to name; nor one that spells it otherwise.
    CHECK(check("Pojde jej szukac!", "pl").empty());
    CHECK(check("Znajdz 'LZA ELFA'", "pl").empty());
    // The name itself, not a form, is found too.
    CHECK(check("Znajdz 'Lza Elfa'", "pl").size() == 1);
}

TEST_CASE("A spelled name counts only as words of its own, and at a level in the name line's forms "
          "[data][items][texts]")
{
    ItemDatabase database;
    database.Build(NamedItems());
    DeclinePolish(database);
    const auto check = [&](const std::string& translation, const std::string& english)
    {
        std::vector<ItemDataIssue> issues;
        Data::Translation::CheckItemReferences(
            database, {translation, "pl", "Dialog.x.resx", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "Find"},
            english, issues);
        return issues;
    };

    // "Rena" inside "Arena" or "Renaty" is another word.
    CHECK(check("Idz na Arena", "Go to {item:14,21}").empty());
    CHECK(check("Zapytaj Renaty", "Ask for {item:14,21}").empty());
    CHECK(check("Daj mi Rena!", "Give me {item:14,21}!").size() == 1);
    // Quotes and other punctuation end a word, also outside ASCII.
    CHECK(check("Znajdz „Lze Elfa”", "Find '{item:14,25}'").size() == 1);
    // At a level, the name line's forms.
    CHECK(check("Bez Kamienia Przyjazni", "Without {item:14,21+2}").size() == 1);
}

TEST_CASE("At a level without a name of its own, a missing form is reported too [data][items][texts]")
{
    ItemDatabase database;
    database.Build(NamedItems());
    DeclinePolish(database);
    const auto describe = [&](const std::string& text)
    {
        std::vector<ItemDataIssue> issues;
        Data::Translation::CheckItemReferences(
            database, {text, "pl", "Game.pl.resx", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "Tear"}, {}, issues);
        REQUIRE(issues.size() == 1);
        return issues[0].message;
    };

    // The Tear of Elf has no name line at +3, and no Polish genitive.
    const std::string genitive = describe("{item:14,25+3:gen}");
    CHECK(genitive.find("no name of its own at +3") != std::string::npos);
    CHECK(genitive.find("(14,25) has no pl gen form") != std::string::npos);
    // Its accusative is there: only the standard name is a warning.
    CHECK(describe("{item:14,25+3:acc}").find("form") == std::string::npos);
}

TEST_CASE("A reference to a level no item has is an error in Data Checks [data][items][texts]")
{
    ItemDatabase database;
    database.Build(NamedItems());
    std::vector<ItemDataIssue> issues;
    Data::Translation::CheckItemReferences(
        database, {"{item:14,25+99}", "en", "Game.en.resx", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "Tear"}, {},
        issues);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[0].message.find("no item has level +99") != std::string::npos);
}

TEST_CASE("A reference at a level takes the name line the tooltip takes [data][items][texts]")
{
    // A color for every level, then a name of +3: the tooltip takes the first line, so +3 shows the standard name.
    ItemDefinition ring;
    ring.group = 13;
    ring.number = 20;
    ring.names = Data::LocalizedString::Parse("Wizards Ring");
    ItemNameLine colorOnly;
    colorOnly.hasColor = true;
    ring.nameLines.push_back(colorOnly);
    ring.nameLines.push_back(LiteralNameLine("Ring of glory", 3));
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{ring});

    const ReferencedName name = ResolveReferencedName(database, Reference(13, 20, 3), "en");
    CHECK(name.text == "Wizards Ring +3");
    CHECK(name.problem == ItemReferenceProblem::StandardName);
}

TEST_CASE("A level's name line with an empty text gives the standard name, as the tooltip does [data][items][texts]")
{
    ItemDefinition ring;
    ring.group = 13;
    ring.number = 20;
    ring.names = Data::LocalizedString::Parse("Wizards Ring");
    // "text": "" at +3, and a line at +4 whose placeholder fills in to nothing.
    ring.nameLines.push_back(LiteralNameLine("", 3));
    ring.nameLines.push_back(LiteralNameLine("{shine}", 4));
    ItemLevelValues shine;
    shine.name = "shine";
    shine.values.resize(ItemLevelCount);
    ring.levelValues.push_back(shine);
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{ring});

    for (const int level : {3, 4})
    {
        INFO("+" << level);
        const ReferencedName name = ResolveReferencedName(database, Reference(13, 20, level), "en");
        CHECK(name.text == "Wizards Ring +" + std::to_string(level));
        CHECK(name.problem == ItemReferenceProblem::StandardName);
    }
}

TEST_CASE("A level's name line may name another item, in the language the line is written in [data][items][texts]")
{
    std::vector<ItemDefinition> items = NamedItems();
    // A bundle whose name line names the Tear of Elf, in English and in Polish with its accusative.
    ItemDefinition bundle;
    bundle.group = 12;
    bundle.number = 30;
    bundle.names = Data::LocalizedString::Parse("Bundle||pl=Paczka");
    bundle.nameLines.push_back(LiteralNameLine("Bundle of {item:14,25}||pl=Paczka {item:14,25:acc}", 0));
    // A line that names its own level never ends.
    ItemDefinition loop;
    loop.group = 12;
    loop.number = 31;
    loop.names = Data::LocalizedString::Parse("Loop");
    loop.nameLines.push_back(LiteralNameLine("{item:12,31+0}", 0));
    items.push_back(bundle);
    items.push_back(loop);
    ItemDatabase database;
    database.Build(items);
    DeclinePolish(database);

    CHECK(ResolveReferencedName(database, Reference(12, 30, 0), "en").text == "Bundle of Tear of Elf");
    CHECK(ResolveReferencedName(database, Reference(12, 30, 0), "pl").text == "Paczka Lze Elfa");
    // German has no text of its own for the line: the English line, with the English name.
    CHECK(ResolveReferencedName(database, Reference(12, 30, 0), "de").text == "Bundle of Tear of Elf");
    CHECK(ResolveReferencedName(database, Reference(12, 31, 0), "en").problem == ItemReferenceProblem::LineNamesItself);

    // The bundle's line leaves out {name}: a case asked of it has no form, whatever forms the bundle gets.
    const ReferencedName genitive = ResolveReferencedName(database, Reference(12, 30, 0, "gen"), "pl");
    CHECK(genitive.text == "Paczka Lze Elfa");
    CHECK(genitive.problem == ItemReferenceProblem::UndeclinedLine);
    // A line written as {name} declines the item's name: its missing form is the item's.
    CHECK(ResolveReferencedName(database, Reference(14, 21, 0, "gen"), "pl").problem ==
          ItemReferenceProblem::MissingForm);
}

TEST_CASE("A name line that cannot be filled says why [data][items][texts]")
{
    std::vector<ItemDefinition> items = NamedItems();
    // Rena's lines at +6 and +7: a shared text the tooltip file lacks, and a line naming an item no one has.
    ItemNameLine missing;
    missing.hasText = true;
    missing.textName = "noSuchText";
    missing.minLevel = 6;
    missing.maxLevel = 6;
    items[1].nameLines.push_back(missing);
    items[1].nameLines.push_back(LiteralNameLine("Bundle of {item:14,500}", 7));
    ItemDatabase database;
    database.Build(items);

    // A skill's name only the game has.
    CHECK(ResolveReferencedName(database, Reference(14, 21, 3), "en").problem == ItemReferenceProblem::UnfillableLine);
    CHECK(ResolveReferencedName(database, Reference(14, 21, 6), "en").problem == ItemReferenceProblem::MissingLineText);
    CHECK(ResolveReferencedName(database, Reference(14, 21, 7), "en").problem == ItemReferenceProblem::LineNamesNoItem);
    for (const int level : {3, 6, 7})
    {
        INFO("+" << level);
        CHECK(ResolveReferencedName(database, Reference(14, 21, level), "en").text.empty());
        std::vector<ItemDataIssue> issues;
        const std::string text = "{item:14,21+" + std::to_string(level) + "}";
        Data::Translation::CheckItemReferences(
            database, {text, "en", "Game.en.resx", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "Rena"}, {}, issues);
        REQUIRE(issues.size() == 1);
        CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
        // Each says its own reason.
        CHECK((issues[0].message.find("only the game has") != std::string::npos) == (level == 3));
    }
}

TEST_CASE("A UI translation that names other items than English is a warning on the item it names [data][items][texts]")
{
    ItemDatabase database;
    database.Build(NamedItems());
    std::vector<ItemDataIssue> issues;
    const std::string polish = "Znajdz {item:14,21}";
    Data::Translation::CheckItemReferences(
        database, {polish, "pl", "Game.pl.resx", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "Find"},
        "Find {item:14,25}", issues);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Warning);
    // Data Checks selects the item a click on it opens.
    CHECK(issues[0].group == 14);
    CHECK(issues[0].number == 21);
}

TEST_CASE("The texts that name an item are listed from the UI texts and every item text [data][items][texts]")
{
    std::vector<ItemDefinition> items = NamedItems();
    ItemDescriptionLine line;
    line.text = Data::LocalizedString::Parse("Combine with {item:14,25}");
    items[1].description.push_back(line);
    items[1].tooltipLayoutChanges.add.push_back({"classLines", {line}});
    ItemDatabase database;
    database.Build(items);

    const std::vector<std::string> texts = Data::Translation::FindTextsNamingItem(database, 14, 25);
    const auto has = [&](std::string_view suffix)
    {
        return std::any_of(texts.begin(), texts.end(), [&](const std::string& text) { return text.ends_with(suffix); });
    };
    CHECK(has("Dialog: Answer_65_Slot_0"));
    CHECK(has("(14,21): description[0]"));
    CHECK(has("(14,21): tooltipLayoutChanges.add[0].lines[0]"));
    CHECK(Data::Translation::FindTextsNamingItem(database, 14, 21).empty());
}

TEST_CASE("The tooltip file's layout lines are checked and listed as the other item texts are [data][items][texts]")
{
    TooltipLayoutLine named;
    named.text = Data::LocalizedString::Parse("Combine with {item:14,25}||pl=Polacz z {item:14,25:acc}");
    TooltipLayout layout;
    layout.name = "default";
    layout.lines.push_back(named);
    TooltipLayoutLine unknown;
    unknown.text = Data::LocalizedString::Parse("Made of {item:14,99}");
    TooltipLayout preset;
    preset.name = "jewel";
    preset.base = "default";
    preset.add.push_back({"", {unknown}});
    ItemTooltipFile file;
    file.layouts = {layout, preset};
    ItemDatabase database;
    database.SetTooltipFile(file);
    database.Build(NamedItems());
    DeclinePolish(database);

    const std::vector<std::string> texts = Data::Translation::FindTextsNamingItem(database, 14, 25);
    CHECK(std::ranges::find(texts, "Tooltip.json: layouts[0].lines[0]") != texts.end());

    std::vector<ItemDataIssue> issues;
    Data::Translation::CheckItemDataTexts(database, "Tooltip.json", issues);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[0].source == "Tooltip.json");
    CHECK(issues[0].field == "layouts[1].add[0].lines[0].text");
}

TEST_CASE(
    "A text's references are expanded with the resolver's names, and without a name they stay [data][items][texts]")
{
    Data::Translation::ItemTexts::SetNameResolver(
        [](const ItemReference& reference, std::string_view locale) -> std::optional<std::string>
        {
            if (reference.number != 25)
            {
                return std::nullopt;
            }
            return std::string(locale == "pl" ? "Lze Elfa" : "Tear of Elf");
        });
    CHECK(Data::Translation::ItemTexts::ExpandText("Find the '{item:14,25}'!", "en") == "Find the 'Tear of Elf'!");
    CHECK(Data::Translation::ItemTexts::ExpandText("'{item:14,25:acc}' {item:14,99}", "pl") ==
          "'Lze Elfa' {item:14,99}");
    CHECK(Data::Translation::ItemTexts::ExpandText("{item:14;25}", "en") == "{item:14;25}");
    // An unclosed reference stays as written, once, and the next one is still expanded.
    CHECK(Data::Translation::ItemTexts::ExpandText("a {item:1 b {item:14,25} c", "en") == "a {item:1 b Tear of Elf c");
    Data::Translation::ItemTexts::SetNameResolver(nullptr);
    CHECK(Data::Translation::ItemTexts::ExpandText("{item:14,25}", "en") == "{item:14,25}");
}

TEST_CASE("Name forms are read and written back after the name and a name line's text [data][items][texts]")
{
    std::vector<ItemDefinition> items = NamedItems();
    const std::string written = WriteItemGroupJson(14, items);
    const size_t forms = written.find("\"nameForms\"");
    REQUIRE(forms != std::string::npos);
    CHECK(written.find("\"name\"") < forms);
    CHECK(written.find("Lze Elfa") > forms);

    std::vector<ItemDefinition> read;
    std::vector<ItemDataIssue> issues;
    ReadItemGroupJson(written, "Group14_Potion.json", read, issues);
    CHECK(issues.empty());
    // The writer sorts the items by number: Rena (14,21) first.
    REQUIRE(read.size() == 2);
    REQUIRE(read[0].number == 21);
    REQUIRE(read[0].nameLines.size() == 3);
    CHECK(read[0].nameLines[1].nameForms == items[1].nameLines[1].nameForms);
    CHECK(read[0].nameLines[1].nameForms.Find("pl", "gen") == "Kamienia Przyjazni");
    CHECK(read[1].nameForms.Find("pl", "acc") == "Lze Elfa");
    CHECK(WriteItemGroupJson(14, read) == written);
}

TEST_CASE("A form with an unknown case or a language that is no code is an error [data][items][texts]")
{
    const std::string text = R"({"formatVersion": 1, "group": 14, "items": [{"number": 25, "name": "Tear of Elf",
        "nameForms": {"pl": {"abl": "x", "acc": ""}, "p l": {"gen": "y"}}}]})";
    std::vector<ItemDefinition> read;
    std::vector<ItemDataIssue> issues;
    ReadItemGroupJson(text, "Group14_Potion.json", read, issues);
    const auto has = [&](const std::string& field)
    {
        return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                           { return issue.field == field && issue.severity == ItemDataIssueSeverity::Fatal; });
    };
    CHECK(has("nameForms.pl.abl"));
    CHECK(has("nameForms.pl.acc"));
    CHECK(has("nameForms.p l"));
}

TEST_CASE("The name cases file lists each language's cases in the case order [data][items][texts]")
{
    ItemNameCases cases;
    std::vector<ItemDataIssue> issues;
    ReadItemNameCasesJson(R"({"formatVersion": 1, "languages": {"pl": ["acc", "gen", "abl"], "de": ["gen"]}})",
                          "NameCases.json", cases, issues);
    REQUIRE(issues.size() == 1);
    CHECK(issues.front().field == "languages.pl");
    CHECK(cases.GetCases("pl") == std::vector<std::string>{"gen", "acc"});
    CHECK(cases.Uses("de", "gen"));
    CHECK_FALSE(cases.Uses("de", "acc"));
    CHECK(cases.GetCases("ru").empty());
    CHECK(WriteItemNameCasesJson(cases) == "{\n  \"formatVersion\": 1,\n  \"languages\": {\n    \"de\": [\"gen\"],\n   "
                                           " \"pl\": [\"gen\", \"acc\"]\n  }\n}\n");

    cases.SetCases("de", {});
    CHECK(cases.GetLanguages().size() == 1);
}

TEST_CASE("English in the name cases file is a warning and declines nothing [data][items][texts]")
{
    ItemNameCases cases;
    std::vector<ItemDataIssue> issues;
    ReadItemNameCasesJson(R"({"formatVersion": 1, "languages": {"en": ["gen"], "pl": ["gen"]}})", "NameCases.json",
                          cases, issues);
    REQUIRE(issues.size() == 1);
    CHECK(issues.front().severity == ItemDataIssueSeverity::Warning);
    CHECK(issues.front().field == "languages.en");
    CHECK(cases.GetCases("en").empty());
    CHECK(cases.Uses("pl", "gen"));
}

TEST_CASE("Forms a text can never show are warnings [data][items][texts]")
{
    std::vector<ItemDefinition> items = NamedItems();
    items[0].nameForms.Set("en", "gen", "Tear of Elf's");
    items[0].nameForms.Set("ru", "gen", "x");
    items[0].nameForms.Set("pl", "voc", "x");
    items[0].nameForms.Set("de", "gen", "Traene der Elfe");
    ItemNameCases cases;
    cases.SetCases("pl", {"gen", "acc"});
    cases.SetCases("de", {"gen"});
    // Rena's lines 3 and 4: a text naming the item at those levels fills in the item's forms, or takes another line.
    ItemNameLine placeholders = LiteralNameLine("Rena +{level}||pl=Rena +{level}", 4);
    placeholders.nameForms.Set("pl", "gen", "x");
    items[1].nameLines.push_back(placeholders);
    ItemNameLine conditional = LiteralNameLine("Rena of the elves||pl=Rena elfow", 5);
    conditional.when.push_back(TooltipCondition{});
    conditional.nameForms.Set("pl", "gen", "x");
    items[1].nameLines.push_back(conditional);

    std::vector<ItemDataIssue> issues;
    ValidateNameForms(items, ItemTooltipFile{}, cases, "NameCases.json", issues);
    const auto has = [&](const std::string& field)
    {
        return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                           { return issue.field == field && issue.severity == ItemDataIssueSeverity::Warning; });
    };
    CHECK(has("nameForms.en"));
    CHECK(has("nameForms.ru"));
    CHECK(has("nameForms.pl.voc"));
    CHECK_FALSE(has("nameForms.de"));
    CHECK(has("nameLines[3].nameForms.pl"));
    CHECK(has("nameLines[4].nameForms"));
    // The Stone of Friendship's literal line keeps its forms.
    CHECK_FALSE(has("nameLines[1].nameForms.pl"));
    CHECK(issues.size() == 5);
}

TEST_CASE("A language of the name cases file the game has no texts in is a warning [data][items][texts]")
{
    ItemNameCases cases;
    cases.SetCases("pl", {"gen"});
    cases.SetCases("pl-PL", {"gen"});
    const std::vector<const char*> uiLocales = {"en", "de", "pl"};

    std::vector<ItemDataIssue> issues;
    ValidateNameCaseLocales(cases, uiLocales, "NameCases.json", issues);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[0].source == "NameCases.json");
    CHECK(issues[0].field == "languages.pl-PL");
}

TEST_CASE("A name line's shared text gives its forms, and its placeholders are a warning [data][items][texts]")
{
    ItemTooltipFile tooltips;
    tooltips.texts.emplace("stone", Data::LocalizedString::Parse("Stone of Friendship||pl=Kamien Przyjazni"));
    tooltips.texts.emplace("namedStone", Data::LocalizedString::Parse("{name} stone||pl={name} kamien"));
    std::vector<ItemDefinition> items = NamedItems();
    // Rena's lines 3 and 4, at +6 and +7: "@stone" and "@namedStone", both with a Polish genitive.
    ItemNameLine shared;
    shared.hasText = true;
    shared.textName = "stone";
    shared.minLevel = 6;
    shared.maxLevel = 6;
    shared.nameForms.Set("pl", "gen", "Kamienia Przyjazni");
    ItemNameLine placeholders = shared;
    placeholders.textName = "namedStone";
    placeholders.minLevel = 7;
    placeholders.maxLevel = 7;
    items[1].nameLines.push_back(shared);
    items[1].nameLines.push_back(placeholders);
    ItemNameCases cases;
    cases.SetCases("pl", {"gen", "acc"});

    std::vector<ItemDataIssue> issues;
    ValidateNameForms(items, tooltips, cases, "NameCases.json", issues);
    const auto has = [&](const std::string& field)
    { return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue) { return issue.field == field; }); };
    CHECK_FALSE(has("nameLines[3].nameForms.pl"));
    // A text that names Rena at +7 fills in Rena's own forms.
    CHECK(has("nameLines[4].nameForms.pl"));

    ItemDatabase database;
    database.SetTooltipFile(tooltips);
    database.Build(items);
    database.SetNameCases(cases);
    CHECK(ResolveReferencedName(database, Reference(14, 21, 6, "gen"), "pl").text == "Kamienia Przyjazni");
}
