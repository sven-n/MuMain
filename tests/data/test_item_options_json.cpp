#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ItemData/ItemOptionsJson.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"
#include "Data/GameData/ItemData/OptionRulesJson.h"
#include "Data/GameData/ItemData/OptionTextsJson.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
constexpr const char* Source = "Options/Luck.json";

// A file in the written format: luck as the shipped data has it, and a definition with a value per level.
constexpr const char* SmallFile = R"({
  "formatVersion": 1,
  "definitions": [
    {
      "name": "luck",
      "type": "luck",
      "displayName": {
        "en": "Luck",
        "de": "Glück"
      },
      "openMu": {"id": "00000083-0001-0000-0000-000000000000", "name": "Luck"},
      "server": {"addsRandomly": true, "addChance": 0.25, "maximumOptionsPerItem": 1},
      "options": [
        {"attribute": "criticalDamageChance", "percent": {"values": 5}, "text": ["luckJewelOfSoul", "luckCriticalDamage"]}
      ]
    },
    {
      "name": "luckByLevel",
      "type": "luck",
      "displayName": {
        "en": "Luck by level"
      },
      "options": [
        {"attribute": "criticalDamageChance", "add": {"values": [4, 8, 12, 16]}, "text": "luckCriticalDamage"}
      ]
    }
  ]
}
)";

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}

// The problems of a file with one luck definition whose JSON is `definition`.
std::vector<ItemDataIssue> ReadDefinition(const std::string& definition)
{
    const std::string text = R"({"formatVersion": 1, "definitions": [)" + definition + "]}";
    std::vector<ItemOptionDefinition> definitions;
    std::vector<ItemDataIssue> issues;
    ReadItemOptionsJson(text, Source, "Luck.json", definitions, issues);
    return issues;
}

constexpr const char* LuckOption =
    R"("options": [{"attribute": "criticalDamageChance", "percent": {"values": 5}, "text": "luckCriticalDamage"}])";

std::string Luck(const std::string& fields)
{
    return R"({"name": "luck", "type": "luck", "displayName": {"en": "Luck"}, )" + fields + "}";
}

std::string LuckOptionWith(const std::string& option)
{
    return Luck(R"("options": [)" + option + "]");
}
} // namespace

TEST_CASE("The option definition files read and write back the same [data][items]")
{
    std::vector<ItemOptionDefinition> definitions;
    std::vector<ItemDataIssue> issues;
    ReadItemOptionsJson(SmallFile, Source, "Luck.json", definitions, issues);

    CHECK(issues.empty());
    REQUIRE(definitions.size() == 2);
    const ItemOptionDefinition& luck = definitions[0];
    CHECK(luck.name == "luck");
    CHECK(luck.type == ItemOptionType::Luck);
    CHECK(luck.file == "Luck.json");
    CHECK(luck.displayName.Get("de") == "Glück");
    CHECK(luck.openMu.id == "00000083-0001-0000-0000-000000000000");
    CHECK(luck.openMu.name == "Luck");
    REQUIRE(luck.server.has_value());
    CHECK(luck.server->addsRandomly);
    CHECK(luck.server->addChance == doctest::Approx(0.25));
    CHECK(luck.server->maximumOptionsPerItem == 1);
    REQUIRE(luck.options.size() == 1);
    CHECK(luck.options[0].attribute == "criticalDamageChance");
    CHECK(luck.options[0].add == ItemOptionAdd::Percent);
    CHECK(luck.options[0].value.At(1) == 5);
    CHECK(luck.options[0].texts == std::vector<std::string>{"luckJewelOfSoul", "luckCriticalDamage"});
    CHECK_FALSE(definitions[1].server.has_value());
    CHECK(definitions[1].options[0].texts == std::vector<std::string>{"luckCriticalDamage"});

    CHECK(WriteItemOptionsJson(definitions) == SmallFile);
}

TEST_CASE("An option's value is a constant, or one value per level whose last one counts above the list [data][items]")
{
    const ItemOptionValue constant{{7}, false};
    CHECK(constant.At(0) == 7);
    CHECK(constant.At(1) == 7);
    CHECK(constant.At(9) == 7);

    const ItemOptionValue perLevel{{4, 8, 12, 16}, true};
    CHECK(perLevel.At(0) == 0);
    CHECK(perLevel.At(1) == 4);
    CHECK(perLevel.At(4) == 16);
    CHECK(perLevel.At(7) == 16);
}

TEST_CASE("Option definition file errors stop the start [data][items]")
{
    using Severity = ItemDataIssueSeverity;
    const auto fatal = [](const std::string& definition, const std::string& field)
    {
        INFO(definition);
        CHECK(HasIssue(ReadDefinition(definition), Severity::Fatal, field));
    };

    fatal(R"({"type": "luck", "displayName": {"en": "Luck"}, )" + std::string(LuckOption) + "}", "definitions[0].name");
    fatal(R"({"name": "luck", "type": "excellent", "displayName": {"en": "Luck"}, )" + std::string(LuckOption) + "}",
          "definitions[0].type");
    fatal(R"({"name": "luck", "type": "luck", )" + std::string(LuckOption) + "}", "definitions[0].displayName");
    fatal(R"({"name": "luck", "type": "luck", "displayName": {"de": "Glück"}, )" + std::string(LuckOption) + "}",
          "definitions[0].displayName");
    fatal(Luck(R"("openMu": {"id": "luck"}, )" + std::string(LuckOption)), "definitions[0].openMu.id");
    fatal(Luck(R"("server": {"addChance": 2}, )" + std::string(LuckOption)), "definitions[0].server.addChance");
    fatal(Luck(R"("server": {"addsRandomly": 1}, )" + std::string(LuckOption)), "definitions[0].server.addsRandomly");
    fatal(Luck(R"("options": [])"), "definitions[0].options");
    fatal(Luck(R"("other": 1)"), "definitions[0].options");
    // A luck definition has one option, without a number.
    fatal(LuckOptionWith(R"({"attribute": "a", "add": {"values": 1}, "text": "t"}, )"
                         R"({"attribute": "b", "add": {"values": 1}, "text": "t"})"),
          "definitions[0].options");
    fatal(LuckOptionWith(R"({"number": 1, "attribute": "a", "add": {"values": 1}, "text": "t"})"),
          "definitions[0].options[0].number");
    fatal(LuckOptionWith(R"({"add": {"values": 1}, "text": "t"})"), "definitions[0].options[0].attribute");
    // Exactly one way to add, its value an object.
    fatal(LuckOptionWith(R"({"attribute": "a", "text": "t"})"), "definitions[0].options[0]");
    fatal(LuckOptionWith(R"({"attribute": "a", "add": {"values": 1}, "percent": {"values": 1}, "text": "t"})"),
          "definitions[0].options[0]");
    fatal(LuckOptionWith(R"({"attribute": "a", "add": 5, "text": "t"})"), "definitions[0].options[0].add");
    fatal(LuckOptionWith(R"({"attribute": "a", "add": {"values": "5"}, "text": "t"})"),
          "definitions[0].options[0].add.values");
    fatal(LuckOptionWith(R"({"attribute": "a", "add": {"values": []}, "text": "t"})"),
          "definitions[0].options[0].add.values");
    fatal(LuckOptionWith(R"({"attribute": "a", "add": {"values": [1, -1]}, "text": "t"})"),
          "definitions[0].options[0].add.values[1]");
    // A value per level is a list, never a step per level.
    fatal(LuckOptionWith(R"({"attribute": "a", "add": {"values": 0, "perLevel": 4}, "text": "t"})"),
          "definitions[0].options[0].add.perLevel");
    // Its one problem, not also an unknown field.
    CHECK(ReadDefinition(LuckOptionWith(R"({"attribute": "a", "add": {"values": 0, "perLevel": 4}, "text": "t"})"))
              .size() == 1);
    fatal(LuckOptionWith(R"({"attribute": "a", "add": {"values": 1}})"), "definitions[0].options[0].text");
    fatal(LuckOptionWith(R"({"attribute": "a", "add": {"values": 1}, "text": ["t", 1]})"),
          "definitions[0].options[0].text");

    std::vector<ItemOptionDefinition> definitions;
    std::vector<ItemDataIssue> issues;
    ReadItemOptionsJson(R"({"formatVersion": 2, "definitions": []})", Source, "Luck.json", definitions, issues);
    CHECK(HasIssue(issues, Severity::Fatal, "formatVersion"));
    issues.clear();
    ReadItemOptionsJson(R"({"formatVersion": 1})", Source, "Luck.json", definitions, issues);
    CHECK(HasIssue(issues, Severity::Fatal, "definitions"));
}

TEST_CASE("Option definition file warnings name the unknown fields [data][items]")
{
    const std::vector<ItemDataIssue> issues = ReadDefinition(
        Luck(R"("color": "blue", "openMu": {"name": "Luck", "guid": 1}, )"
             R"("options": [{"attribute": "a", "add": {"values": 1, "levels": 4}, "text": "t", "bold": true}])"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].color"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].openMu.guid"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].options[0].add.levels"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].options[0].bold"));
    CHECK_FALSE(HasFatal(issues));
}

TEST_CASE("The option texts read and write back the same, and a text the game can't show is left out [data][items]")
{
    constexpr const char* Texts = R"json({
  "formatVersion": 1,
  "texts": {
    "luckCriticalDamage": {
      "en": "Luck (critical damage rate +{value}%)",
      "de": "Glück (kritische Schadensrate +{value}%)"
    },
    "luckJewelOfSoul": {
      "en": "Luck (success rate of Jewel of Soul +25%)"
    }
  }
}
)json";
    ItemOptionTexts texts;
    std::vector<ItemDataIssue> issues;
    ReadOptionTextsJson(Texts, OptionTextsFileName, texts, issues);
    CHECK(issues.empty());
    REQUIRE(texts.Find("luckCriticalDamage") != nullptr);
    CHECK(texts.Find("luckCriticalDamage")->Get("de") == "Glück (kritische Schadensrate +{value}%)");
    CHECK(WriteOptionTextsJson(texts) == Texts);

    // No English, a placeholder that is no option's, a translation without the English one's: errors, left out.
    ItemOptionTexts wrong;
    issues.clear();
    ReadOptionTextsJson(R"({"formatVersion": 1, "texts": {
        "noEnglish": {"de": "Glück"},
        "level": {"en": "+{level}"},
        "translation": {"en": "+{value}", "de": "+5"},
        "fine": {"en": "+{value}"}}})",
                        OptionTextsFileName, wrong, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "texts.noEnglish"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "texts.level"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "texts.translation.de"));
    CHECK_FALSE(HasFatal(issues));
    CHECK(wrong.texts.size() == 1);
    CHECK(wrong.Find("fine") != nullptr);

    // A name twice: the reader would keep one of them.
    issues.clear();
    ReadOptionTextsJson(R"({"formatVersion": 1, "texts": {"a": {"en": "A"}, "a": {"en": "B"}}})", OptionTextsFileName,
                        wrong, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "texts.a"));
}

TEST_CASE(
    "The option rules read and write back the same, with item numbers and tags an item must not have [data][items]")
{
    constexpr const char* Rules = R"({
  "formatVersion": 1,
  "rules": [
    {"options": [], "tags": ["ammunition"]},
    {"options": ["luck"], "group": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], "tags": ["!ammunition"]},
    {"options": ["luck"], "group": [12], "numbers": [0, 1, 2, 3, 4, 5, 6, 41, 42, 49]},
    {"options": ["luck"], "slot": ["wings"], "wingTier": ["third"]}
  ]
}
)";
    std::vector<OptionRule> rules;
    std::vector<ItemDataIssue> issues;
    ReadOptionRulesJson(Rules, OptionRulesFileName, rules, issues);
    CHECK(issues.empty());
    REQUIRE(rules.size() == 4);
    CHECK(rules[0].options.empty());
    CHECK(rules[1].conditions.withoutTags.Has(ItemTag::Ammunition));
    CHECK(rules[1].conditions.tags.IsEmpty());
    CHECK(rules[2].conditions.numbers.count() == 10);
    CHECK(WriteOptionRulesJson(rules) == Rules);

    ItemDefinition bolt;
    bolt.group = 4;
    bolt.number = 7;
    bolt.tags.Set(ItemTag::Ammunition);
    ItemDefinition bow = bolt;
    bow.tags = {};
    ItemDefinition wingOfDespair;
    wingOfDespair.group = 12;
    wingOfDespair.number = 42;
    ItemDefinition smallWing = wingOfDespair;
    smallWing.number = 130;
    CHECK(DeriveItemOptions(rules, bolt).empty());
    CHECK(DeriveItemOptions(rules, bow) == std::vector<std::string>{"luck"});
    CHECK(DeriveItemOptions(rules, wingOfDespair) == std::vector<std::string>{"luck"});
    CHECK(DeriveItemOptions(rules, smallWing).empty());
    // Without the first rule, the second still leaves the bolt out: it has the tag the rule's items must not have.
    CHECK(DeriveItemOptions(std::span(rules).subspan(1), bolt).empty());
}

TEST_CASE("Option rule errors stop the start [data][items]")
{
    const auto fatal = [](const std::string& rule, const std::string& field)
    {
        INFO(rule);
        std::vector<OptionRule> rules;
        std::vector<ItemDataIssue> issues;
        ReadOptionRulesJson(R"({"formatVersion": 1, "rules": [)" + rule + "]}", OptionRulesFileName, rules, issues);
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, field));
        CHECK(rules.empty());
    };
    fatal(R"({"group": [0]})", "rules[0].options");
    fatal(R"({"options": ["luck", 1]})", "rules[0].options");
    fatal(R"({"options": ["luck"], "numbers": [1]})", "rules[0].numbers");
    fatal(R"({"options": ["luck"], "group": [0], "numbers": [512]})", "rules[0].numbers[0]");
    fatal(R"({"options": ["luck"], "tags": ["!nothing"]})", "rules[0].tags[0]");
    fatal(R"({"options": ["luck"], "colors": ["blue"]})", "rules[0].colors");
}

TEST_CASE("The item stat file's profile rules have neither item numbers nor tags an item must not have [data][items]")
{
    const auto statFile = [](const std::string& rule)
    {
        return R"({"formatVersion": 1, "dropLevelBonus": {"excellent": 25, "ancient": 30}, "tables": [],
            "default": {}, "profileRules": [)" +
               rule + "]}";
    };
    ItemStatFile file;
    std::vector<ItemDataIssue> issues;
    ReadItemStatsJson(statFile(R"({"profile": "default", "group": [0], "numbers": [1]})"), "ItemStats.json", file,
                      issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "profileRules[0].numbers"));
    issues.clear();
    ReadItemStatsJson(statFile(R"({"profile": "default", "tags": ["!ammunition"]})"), "ItemStats.json", file, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "profileRules[0].tags[0]"));
}

// An additional option definition: its options numbered (OpenMU's ItemOption.Number, which the option type the server
// sends picks), several of them in one definition, each with a value per level.
TEST_CASE("Option definitions of type option read their numbered options and write them back [data][items]")
{
    constexpr const char* OptionFile = R"({
  "formatVersion": 1,
  "definitions": [
    {
      "name": "wingOption",
      "type": "option",
      "displayName": {
        "en": "Wing option"
      },
      "options": [
        {"number": 0, "attribute": "healthRecoveryMultiplier", "percent": {"values": [1, 2, 3, 4]}, "text": "automaticHpRecovery"},
        {"number": 2, "attribute": "physicalBaseDmg", "add": {"values": [4, 8, 12, 16]}, "text": "additionalDamage"}
      ]
    }
  ]
}
)";
    std::vector<ItemOptionDefinition> definitions;
    std::vector<ItemDataIssue> issues;
    ReadItemOptionsJson(OptionFile, "Options/Option.json", "Option.json", definitions, issues);
    CHECK(issues.empty());
    REQUIRE(definitions.size() == 1);
    const ItemOptionDefinition& wing = definitions[0];
    CHECK(wing.type == ItemOptionType::Option);
    REQUIRE(wing.options.size() == 2);
    CHECK(wing.options[0].number == 0);
    CHECK(wing.options[0].add == ItemOptionAdd::Percent);
    CHECK(wing.options[0].value.isList);
    CHECK(wing.options[0].value.At(3) == 3);
    CHECK(wing.options[1].number == 2);
    CHECK(wing.options[1].attribute == "physicalBaseDmg");
    CHECK(wing.options[1].value.At(4) == 16);
    CHECK(wing.options[1].value.At(7) == 16);
    CHECK(WriteItemOptionsJson(definitions) == OptionFile);

    const auto option = [](const std::string& options)
    {
        return R"({"name": "o", "type": "option", "displayName": {"en": "O"}, "options": [)" + options + "]}";
    };
    // Each option has a number, and no two have the same.
    CHECK(HasIssue(ReadDefinition(option(R"({"attribute": "a", "add": {"values": [1]}, "text": "t"})")),
                   ItemDataIssueSeverity::Fatal, "definitions[0].options[0].number"));
    CHECK(HasIssue(ReadDefinition(option(R"({"number": 0, "attribute": "a", "add": {"values": [1]}, "text": "t"}, )"
                                         R"({"number": 0, "attribute": "b", "add": {"values": [1]}, "text": "t"})")),
                   ItemDataIssueSeverity::Fatal, "definitions[0].options[1].number"));
    CHECK(HasIssue(ReadDefinition(option(R"({"number": -1, "attribute": "a", "add": {"values": [1]}, "text": "t"})")),
                   ItemDataIssueSeverity::Fatal, "definitions[0].options[0].number"));
    // The number is the option type the server sends in four bits: 15 at most.
    CHECK_FALSE(
        HasFatal(ReadDefinition(option(R"({"number": 15, "attribute": "a", "add": {"values": [1]}, "text": "t"})"))));
    CHECK(HasIssue(ReadDefinition(option(R"({"number": 16, "attribute": "a", "add": {"values": [1]}, "text": "t"})")),
                   ItemDataIssueSeverity::Fatal, "definitions[0].options[0].number"));
}
