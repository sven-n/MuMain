#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
// A stat file with one table: 2 at +0, then 3 more per level.
ItemStatProfiles MakeProfiles()
{
    ItemStatFile file;
    LevelBonusTable table;
    table.name = "wingDamage";
    table.values.fill(2);
    table.perLevel = 3;
    file.tables.push_back(table);
    return ItemStatProfiles(file);
}

ItemDefinition MakeJewel(const std::string& english)
{
    ItemDefinition jewel;
    jewel.group = 14;
    jewel.number = 13;
    jewel.names.Set("en", "Jewel of Bless");
    ItemDescriptionLine line;
    line.text.Set("en", english);
    jewel.description = {line};
    return jewel;
}

// The issues of `field`.
std::vector<ItemDataIssue> IssuesOf(const std::vector<ItemDataIssue>& issues, const std::string& field)
{
    std::vector<ItemDataIssue> found;
    std::copy_if(issues.begin(), issues.end(), std::back_inserter(found),
                 [&](const ItemDataIssue& issue) { return issue.field == field; });
    return found;
}

std::vector<ItemDataIssue> CheckSyntax(const ItemDefinition& item)
{
    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{item}, issues);
    return issues;
}

// The issues of the item, without the stat file's.
std::vector<ItemDataIssue> CheckNames(const ItemDefinition& item)
{
    std::vector<ItemDataIssue> issues;
    ValidateDescriptionPlaceholders(std::vector<ItemDefinition>{item}, MakeProfiles(), ItemTooltipFile(),
                                    "ItemStats.json", issues);
    std::erase_if(issues, [](const ItemDataIssue& issue) { return issue.group == ItemDataIssue::NoItem; });
    return issues;
}
} // namespace

TEST_CASE("Description placeholders are read from their braces [data][items]")
{
    CHECK(ScanPlaceholders("Increases damage by 12%").names.empty());
    CHECK(ScanPlaceholders("").names.empty());
    CHECK(ScanPlaceholders("+{level}: {wingDamage}% and {level}").names ==
          std::vector<std::string>{"level", "wingDamage", "level"});
    CHECK_FALSE(ScanPlaceholders("up to {level").error.empty());
    // Item texts have no braces of their own: a doubled one is a mistake too.
    CHECK_FALSE(ScanPlaceholders("{{level}}").error.empty());
    CHECK_FALSE(ScanPlaceholders("{level{wingDamage}").error.empty());
    CHECK_FALSE(ScanPlaceholders("an empty {}").error.empty());
    CHECK_FALSE(ScanPlaceholders("level} alone").error.empty());
    CHECK_FALSE(ScanPlaceholders("{wing damage}").error.empty());
    CHECK_FALSE(ScanPlaceholders("{wing_damage}").error.empty());
}

TEST_CASE("Description placeholders get the item's values [data][items]")
{
    const ItemStatProfiles profiles = MakeProfiles();
    CHECK(GetPlaceholderValue("level", 7, profiles) == 7);
    CHECK(GetPlaceholderValue("wingDamage", 0, profiles) == 2);
    CHECK(GetPlaceholderValue("wingDamage", 9, profiles) == 29);
    CHECK_FALSE(GetPlaceholderValue("wingdamage", 9, profiles));

    CHECK(FillPlaceholders("Increases damage by {wingDamage}% at +{level}", 4, profiles) ==
          "Increases damage by 14% at +4");
    CHECK(FillPlaceholders("no placeholders", 3, profiles) == "no placeholders");
    // What has no value stays as written, from there on.
    CHECK(FillPlaceholders("{level} and {unknown} and {level}", 2, profiles) == "2 and {unknown} and {level}");
    CHECK(FillPlaceholders("{level} and {broken", 2, profiles) == "2 and {broken");
}

TEST_CASE("A typo in a description placeholder gets the nearest name [data][items]")
{
    const std::vector<std::string> known = {"level", "damageIncreaseWeapons", "defenseBonus1stWings"};
    CHECK(SuggestPlaceholder("levl", known) == "level");
    CHECK(SuggestPlaceholder("damgeIncreaseWeapons", known) == "damageIncreaseWeapons");
    CHECK(SuggestPlaceholder("defenseBonus1stWing", known) == "defenseBonus1stWings");
    // Three letters away is another name, not a typo.
    CHECK(SuggestPlaceholder("defenseBonus2ndWings", known).empty());
    CHECK(SuggestPlaceholder("durability", known).empty());
}

TEST_CASE("Broken braces in a description are errors in every language [data][items]")
{
    CHECK(CheckSyntax(MakeJewel("Increases damage by {wingDamage}%")).empty());

    const std::vector<ItemDataIssue> english = IssuesOf(CheckSyntax(MakeJewel("up to {level")), "description[0].text");
    REQUIRE(english.size() == 1);
    CHECK(english.front().severity == ItemDataIssueSeverity::Error);

    ItemDefinition translated = MakeJewel("up to +{level}");
    translated.description[0].text.Set("de", "bis zu +{level");
    const std::vector<ItemDataIssue> german = IssuesOf(CheckSyntax(translated), "description[0].text.de");
    REQUIRE(german.size() == 1);
    CHECK(german.front().severity == ItemDataIssueSeverity::Error);
}

TEST_CASE("A translation has the placeholders of the English text [data][items]")
{
    ItemDefinition jewel = MakeJewel("Increases damage by {wingDamage}% at +{level}");
    // Another order is fine.
    jewel.description[0].text.Set("de", "Bei +{level}: Schaden +{wingDamage}%");
    // A typo, and a missing one.
    jewel.description[0].text.Set("es", "Aumenta el daño en {wingDamge}% a +{level}");
    jewel.description[0].text.Set("pt", "Aumenta o dano em {wingDamage}%");
    const std::vector<ItemDataIssue> issues = CheckSyntax(jewel);
    CHECK(IssuesOf(issues, "description[0].text.de").empty());
    const std::vector<ItemDataIssue> spanish = IssuesOf(issues, "description[0].text.es");
    REQUIRE(spanish.size() == 1);
    CHECK(spanish.front().severity == ItemDataIssueSeverity::Error);
    CHECK(spanish.front().message.find("{wingDamge}") != std::string::npos);
    CHECK(IssuesOf(issues, "description[0].text.pt").size() == 1);
}

TEST_CASE("A description placeholder is a built-in value or a stat file table [data][items]")
{
    CHECK(CheckNames(MakeJewel("Increases damage by {wingDamage}% at +{level}")).empty());
    CHECK(CheckNames(MakeJewel("No placeholders")).empty());

    const std::vector<ItemDataIssue> typo = CheckNames(MakeJewel("Increases damage by {wingDamge}%, {wingDamge}"));
    REQUIRE(typo.size() == 1);
    CHECK(typo.front().severity == ItemDataIssueSeverity::Error);
    CHECK(typo.front().field == "description[0].text");
    CHECK(typo.front().message == "has the unknown placeholder {wingDamge}; did you mean {wingDamage}?");

    const std::vector<ItemDataIssue> unknown = CheckNames(MakeJewel("Sparkle {sparkle}"));
    REQUIRE(unknown.size() == 1);
    CHECK(unknown.front().message ==
          "has the unknown placeholder {sparkle}; a placeholder is a built-in value (docs/item-data.md), a table of "
          "ItemStats.json, or one of the line's values or the item's levelValues");

    // The stat line values are built in too.
    CHECK(CheckNames(MakeJewel("Durability {durability}/{maxDurability}, {missingStrength} strength missing")).empty());

    // A table no stat uses is for descriptions: a warning when none names it either.
    std::vector<ItemDataIssue> unused;
    ValidateDescriptionPlaceholders(std::vector<ItemDefinition>{MakeJewel("No placeholders")}, MakeProfiles(),
                                    ItemTooltipFile(), "ItemStats.json", unused);
    REQUIRE(unused.size() == 1);
    CHECK(unused.front().severity == ItemDataIssueSeverity::Warning);
    CHECK(unused.front().source == "ItemStats.json");
    CHECK(unused.front().field == "tables.wingDamage");

    // A deleted item (no name) is not checked.
    ItemDefinition deleted = MakeJewel("{sparkle}");
    deleted.names = {};
    CHECK(CheckNames(deleted).empty());
}

TEST_CASE("A description line names the value of its own placeholder [data][items]")
{
    ItemDefinition wing = MakeJewel("Increase {percent}% of Damage");
    wing.description[0].values = {{"percent", "wingDamage"}};
    CHECK(CheckNames(wing).empty());

    // It names a built-in value or a table; a typo suggests the name meant.
    wing.description[0].values = {{"percent", "wingDamge"}};
    const std::vector<ItemDataIssue> typo = CheckNames(wing);
    REQUIRE(typo.size() == 1);
    CHECK(typo.front().field == "description[0].values.percent");
    CHECK(typo.front().message.find("did you mean \"wingDamage\"?") != std::string::npos);

    // A placeholder another line binds is not this line's.
    wing.description.push_back(wing.description[0]);
    wing.description[0].values = {{"percent", "wingDamage"}};
    wing.description[1].values = {};
    const std::vector<ItemDataIssue> unbound = CheckNames(wing);
    REQUIRE(unbound.size() == 1);
    CHECK(unbound.front().message.find("unknown placeholder {percent}") != std::string::npos);
}

TEST_CASE("An item's level values fill its description's placeholders at the levels they have [data][items]")
{
    ItemDefinition fruits = MakeJewel("{stat} Increases 1~3 stat points");
    fruits.description[0].minLevel = 0;
    fruits.description[0].maxLevel = 1;
    ItemLevelValues stat;
    stat.name = "stat";
    stat.values.resize(2);
    stat.values[0].text.Set("en", "ENG");
    stat.values[1].text.Set("en", "STA");
    fruits.levelValues = {stat};
    CHECK(CheckNames(fruits).empty());

    // A line shown at a level without a value.
    fruits.description[0].maxLevel = 4;
    const std::vector<ItemDataIssue> beyond = CheckNames(fruits);
    REQUIRE(beyond.size() == 1);
    CHECK(beyond.front().field == "description[0].levels");
    CHECK(beyond.front().message.find("only up to +1") != std::string::npos);
    fruits.description[0].maxLevel = 1;

    // A level value needs its own name, and one no line shows is a warning.
    fruits.levelValues[0].name = "level";
    fruits.description[0].text.Set("en", "No placeholder here");
    const std::vector<ItemDataIssue> builtIn = CheckNames(fruits);
    REQUIRE(builtIn.size() == 2);
    CHECK(builtIn[0].severity == ItemDataIssueSeverity::Error);
    CHECK(builtIn[0].field == "levelValues.level");
    CHECK(builtIn[1].severity == ItemDataIssueSeverity::Warning);
    CHECK(builtIn[1].message == "no line of the description shows it");
}

TEST_CASE("A level value is a text without placeholders, or a shared text the tooltip file has [data][items]")
{
    ItemTooltipFile tooltips;
    tooltips.texts["energy"].Set("en", "ENG");
    tooltips.texts["withPlaceholder"].Set("en", "+{level}");
    ItemDefinition fruits = MakeJewel("{stat}");
    ItemLevelValues stat;
    stat.name = "stat";
    stat.values.resize(4);
    stat.values[0].textName = "energy";
    stat.values[1].textName = "enrgy";
    stat.values[2].textName = "withPlaceholder";
    stat.values[3].text.Set("en", "{level}");
    fruits.levelValues = {stat};
    std::vector<ItemDataIssue> issues;
    ValidateDescriptionTexts(std::vector<ItemDefinition>{fruits}, tooltips, "Tooltip.json", issues);
    std::erase_if(issues, [](const ItemDataIssue& issue) { return issue.group == ItemDataIssue::NoItem; });
    REQUIRE(issues.size() == 3);
    CHECK(issues[0].field == "levelValues.stat[1]");
    CHECK(issues[0].message.find("did you mean \"@energy\"?") != std::string::npos);
    CHECK(issues[1].field == "levelValues.stat[2]");
    CHECK(issues[2].field == "levelValues.stat[3]");
    CHECK(issues[2].message == "has placeholders, which a level value cannot have");
}

TEST_CASE("A description line can show a skill's mana [data][items]")
{
    ItemDefinition weapon = MakeJewel("Cyclone Cutting skill (Mana:{mana})");
    weapon.description[0].values = {{"mana", "skillMana:22"}};
    CHECK(CheckNames(weapon).empty());
    const std::optional<GameDataValue> mana = ReadGameDataValue("skillMana:22");
    REQUIRE(mana.has_value());
    CHECK(mana->kind == GameDataValueKind::SkillMana);
    CHECK(mana->number == 22);
    CHECK_FALSE(ReadGameDataValue("skillMana:").has_value());
    CHECK_FALSE(ReadGameDataValue("skillMana:cyclone").has_value());
    CHECK(ReadGameDataValue("skillName:30")->kind == GameDataValueKind::SkillName);
    CHECK(ReadGameDataValue("monsterName:41")->kind == GameDataValueKind::MonsterName);
    CHECK_FALSE(ReadGameDataValue("monsterName:99999").has_value());
    CHECK(HasGameDataValuePrefix("monsterName:x"));
    CHECK_FALSE(HasGameDataValuePrefix("monster"));

    // A number that is no skill's.
    weapon.description[0].values = {{"mana", "skillMana:9999"}};
    const std::vector<ItemDataIssue> unknownMana = CheckNames(weapon);
    REQUIRE(unknownMana.size() == 1);
    weapon.description[0].values = {{"mana", "skillMana:22"}};

    // Name lines are checked as description lines are: their placeholders, and their texts' braces.
    ItemNameLine ring;
    ring.hasText = true;
    ring.text.Set("en", "{monster} Transformation Ring");
    ring.values = {{"monster", "monsterName:2"}};
    weapon.nameLines = {ring};
    CHECK(CheckNames(weapon).empty());
    weapon.nameLines[0].values = {{"monster", "monsterName:9999"}};
    const std::vector<ItemDataIssue> noMonster = CheckNames(weapon);
    REQUIRE(noMonster.size() == 1);
    CHECK(noMonster.front().field == "nameLines[0].values.monster");
    CHECK(noMonster.front().message.find("names no skill or monster") != std::string::npos);
    weapon.nameLines[0].text.Set("en", "{name +{level}");
    CHECK_FALSE(IssuesOf(CheckSyntax(weapon), "nameLines[0].text").empty());
    weapon.nameLines.clear();

    weapon.description[0].values = {{"mana", "skillMana:9999"}};
    const std::vector<ItemDataIssue> unknown = CheckNames(weapon);
    REQUIRE(unknown.size() == 1);
    CHECK(unknown.front().field == "description[0].values.mana");
    CHECK(unknown.front().message.find("names no skill") != std::string::npos);
}
