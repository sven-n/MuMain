#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemNameCases.h"
#include "Data/GameData/ItemData/ItemReferenceNames.h"
#include "Data/Translation/ItemReference.h"
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

    std::vector<ItemDataIssue> issues;
    ValidateNameForms(items, cases, "NameCases.json", issues);
    const auto has = [&](const std::string& field)
    {
        return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                           { return issue.field == field && issue.severity == ItemDataIssueSeverity::Warning; });
    };
    CHECK(has("nameForms.en"));
    CHECK(has("nameForms.ru"));
    CHECK(has("nameForms.pl.voc"));
    CHECK_FALSE(has("nameForms.de"));
    CHECK(issues.size() == 3);
}
