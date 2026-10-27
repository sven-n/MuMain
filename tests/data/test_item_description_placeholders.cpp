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

// A stat file with one table of `name` that is `value` at every level.
ItemStatProfiles MakeProfilesWithTable(const std::string& name, int value)
{
    ItemStatFile file;
    LevelBonusTable table;
    table.name = name;
    table.values.fill(value);
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

std::vector<ItemDataIssue> CheckNames(const ItemDefinition& item, const ItemStatProfiles& profiles = MakeProfiles())
{
    std::vector<ItemDataIssue> issues;
    ValidateDescriptionPlaceholders(std::vector<ItemDefinition>{item}, profiles, ItemTooltipFile(), "ItemStats.json",
                                    issues);
    return issues;
}
} // namespace

TEST_CASE("Description placeholders are read from their braces [data][items]")
{
    CHECK(ScanPlaceholders("Increases damage by 12%").names.empty());
    CHECK(ScanPlaceholders("").names.empty());
    CHECK(ScanPlaceholders("+{level}: {table:wingDamage}% and {level}").names ==
          std::vector<std::string>{"level", "table:wingDamage", "level"});
    CHECK_FALSE(ScanPlaceholders("up to {level").error.empty());
    // Item texts have no braces of their own: a doubled one is a mistake too.
    CHECK_FALSE(ScanPlaceholders("{{level}}").error.empty());
    CHECK_FALSE(ScanPlaceholders("{level{table:wingDamage}").error.empty());
    CHECK_FALSE(ScanPlaceholders("an empty {}").error.empty());
    CHECK_FALSE(ScanPlaceholders("level} alone").error.empty());
    CHECK_FALSE(ScanPlaceholders("{wing damage}").error.empty());
    CHECK_FALSE(ScanPlaceholders("{wing_damage}").error.empty());
    // "table:" takes a table's name, which is letters and digits too.
    CHECK_FALSE(ScanPlaceholders("{table:}").error.empty());
    CHECK_FALSE(ScanPlaceholders("{table:wing damage}").error.empty());
    CHECK_FALSE(ScanPlaceholders("{stat:wingDamage}").error.empty());
}

TEST_CASE("Description placeholders get the item's values [data][items]")
{
    const ItemStatProfiles profiles = MakeProfiles();
    CHECK(GetPlaceholderValue("level", 7, profiles) == 7);
    CHECK(GetPlaceholderValue("table:wingDamage", 0, profiles) == 2);
    CHECK(GetPlaceholderValue("table:wingDamage", 9, profiles) == 29);
    CHECK_FALSE(GetPlaceholderValue("table:wingdamage", 9, profiles));
    // A table is named only with "table:", so it never stands for a built-in value or the other way round.
    CHECK_FALSE(GetPlaceholderValue("wingDamage", 9, profiles));
    CHECK_FALSE(GetPlaceholderValue("table:level", 9, profiles));

    CHECK(FillPlaceholders("Increases damage by {table:wingDamage} at +{level}", 4, profiles) ==
          "Increases damage by 14 at +4");
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

TEST_CASE("A table's placeholders are found and renamed [data][items]")
{
    CHECK(MakeTablePlaceholder("wingDamage") == "table:wingDamage");
    CHECK(GetPlaceholderTable("table:wingDamage") == "wingDamage");
    CHECK(GetPlaceholderTable("level").empty());

    CHECK(NamesTable("+{table:wingDamage} at +{level}", "wingDamage"));
    CHECK_FALSE(NamesTable("+{table:wingDamage2} at +{level}", "wingDamage"));
    CHECK_FALSE(NamesTable("+{wingDamage}", "wingDamage"));

    CHECK(RenameTablePlaceholders("{table:wingDamage}, {table:wingDamage2} and {table:wingDamage}", "wingDamage",
                                  "wingDamageIncrease") ==
          "{table:wingDamageIncrease}, {table:wingDamage2} and {table:wingDamageIncrease}");
    CHECK(RenameTablePlaceholders("no placeholders", "wingDamage", "other") == "no placeholders");
}

TEST_CASE("Broken braces in a description are errors in every language [data][items]")
{
    CHECK(CheckSyntax(MakeJewel("Increases damage by {table:wingDamage}")).empty());

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
    ItemDefinition jewel = MakeJewel("Increases damage by {table:wingDamage} at +{level}");
    // Another order is fine.
    jewel.description[0].text.Set("de", "Bei +{level}: Schaden +{table:wingDamage}");
    // A typo, and a missing one.
    jewel.description[0].text.Set("es", "Aumenta el daño en {table:wingDamge} a +{level}");
    jewel.description[0].text.Set("pt", "Aumenta o dano em {table:wingDamage}");
    const std::vector<ItemDataIssue> issues = CheckSyntax(jewel);
    CHECK(IssuesOf(issues, "description[0].text.de").empty());
    const std::vector<ItemDataIssue> spanish = IssuesOf(issues, "description[0].text.es");
    REQUIRE(spanish.size() == 1);
    CHECK(spanish.front().severity == ItemDataIssueSeverity::Error);
    CHECK(spanish.front().message.find("{table:wingDamge}") != std::string::npos);
    CHECK(IssuesOf(issues, "description[0].text.pt").size() == 1);
}

TEST_CASE("A table may be named like a built-in value [data][items]")
{
    // {level} stays the item's + level; {table:level} is the table.
    const ItemStatProfiles profiles = MakeProfilesWithTable("level", 5);
    CHECK(GetPlaceholderValue("level", 9, profiles) == 9);
    CHECK(GetPlaceholderValue("table:level", 9, profiles) == 5);
    CHECK(FillPlaceholders("+{level}: {table:level}", 9, profiles) == "+9: 5");
    CHECK(CheckNames(MakeJewel("+{level}: {table:level}"), profiles).empty());
}

TEST_CASE("A description placeholder is a built-in value or a stat file table [data][items]")
{
    CHECK(CheckNames(MakeJewel("Increases damage by {table:wingDamage} at +{level}")).empty());
    CHECK(CheckNames(MakeJewel("No placeholders")).empty());

    const std::vector<ItemDataIssue> typo =
        CheckNames(MakeJewel("Increases damage by {table:wingDamge}, {table:wingDamge}"));
    REQUIRE(typo.size() == 1);
    CHECK(typo.front().severity == ItemDataIssueSeverity::Error);
    CHECK(typo.front().field == "description[0].text");
    CHECK(typo.front().message == "has the unknown placeholder {table:wingDamge}; did you mean {table:wingDamage}?");

    // A table without "table:" is a typo of its placeholder.
    const std::vector<ItemDataIssue> bare = CheckNames(MakeJewel("Increases damage by {wingDamage}"));
    REQUIRE(bare.size() == 1);
    CHECK(bare.front().message.find("{table:wingDamage}") != std::string::npos);
    // Also when the table's name is close to a built-in value: {levels} is the table "levels", not {level}.
    const std::vector<ItemDataIssue> nearBuiltIn =
        CheckNames(MakeJewel("+{levels}"), MakeProfilesWithTable("levels", 5));
    REQUIRE(nearBuiltIn.size() == 1);
    CHECK(nearBuiltIn.front().message == "has the unknown placeholder {levels}; did you mean {table:levels}?");

    const std::vector<ItemDataIssue> unknown = CheckNames(MakeJewel("Sparkle {sparkle}"));
    REQUIRE(unknown.size() == 1);
    CHECK(unknown.front().message ==
          "has the unknown placeholder {sparkle}; a placeholder is a built-in value (docs/item-data.md), a table of "
          "ItemStats.json ({table:name}), or one of the line's values or the item's levelValues");

    // The stat line values are built in too.
    CHECK(CheckNames(MakeJewel("Durability {durability}/{maxDurability}, {missingStrength} strength missing")).empty());

    // A deleted item (no name) is not checked.
    ItemDefinition deleted = MakeJewel("{sparkle}");
    deleted.names = {};
    CHECK(CheckNames(deleted).empty());
}

TEST_CASE("A description line names the value of its own placeholder [data][items]")
{
    ItemDefinition wing = MakeJewel("Increase {percent}% of Damage");
    wing.description[0].values = {{"percent", "table:wingDamage"}};
    CHECK(CheckNames(wing).empty());

    // It names a built-in value or a table; a typo suggests the name meant, and so does a table without "table:".
    wing.description[0].values = {{"percent", "table:wingDamge"}};
    const std::vector<ItemDataIssue> typo = CheckNames(wing);
    REQUIRE(typo.size() == 1);
    CHECK(typo.front().field == "description[0].values.percent");
    CHECK(typo.front().message.find("did you mean \"table:wingDamage\"?") != std::string::npos);
    wing.description[0].values = {{"percent", "wingDamage"}};
    const std::vector<ItemDataIssue> bare = CheckNames(wing);
    REQUIRE(bare.size() == 1);
    CHECK(bare.front().message.find("did you mean \"table:wingDamage\"?") != std::string::npos);

    // A placeholder another line binds is not this line's.
    wing.description.push_back(wing.description[0]);
    wing.description[0].values = {{"percent", "table:wingDamage"}};
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

    // A level value named like a built-in value is the item's own: its lines show it.
    fruits.levelValues[0].name = "level";
    fruits.description[0].text.Set("en", "{level} Increases 1~3 stat points");
    CHECK(CheckNames(fruits).empty());

    // One no line shows is a warning.
    fruits.description[0].text.Set("en", "No placeholder here");
    const std::vector<ItemDataIssue> unshown = CheckNames(fruits);
    REQUIRE(unshown.size() == 1);
    CHECK(unshown[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(unshown[0].field == "levelValues.level");
    CHECK(unshown[0].message == "no line of the description shows it");
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

    // A number that is no skill's.
    weapon.description[0].values = {{"mana", "skillMana:9999"}};
    const std::vector<ItemDataIssue> unknown = CheckNames(weapon);
    REQUIRE(unknown.size() == 1);
    CHECK(unknown.front().field == "description[0].values.mana");
    CHECK(unknown.front().message.find("names no skill") != std::string::npos);
}

namespace
{
std::vector<ItemDataIssue> CheckTableUses(const std::vector<ItemDefinition>& items)
{
    std::vector<ItemDataIssue> issues;
    ValidateTableUses(items, MakeProfiles(), ItemTooltipFile(), "ItemStats.json", issues);
    return issues;
}

ItemDefinition MakeWing(short number, WingTier tier, const std::string& english)
{
    ItemDefinition wing = MakeJewel(english);
    wing.group = 12;
    wing.number = number;
    wing.names.Set("en", "Wing " + std::to_string(number));
    wing.wingTier = tier;
    return wing;
}
} // namespace

TEST_CASE("A table that neither a stat nor a description names is a warning [data][items]")
{
    // A table no stat uses is for descriptions: a warning when none names it either.
    const std::vector<ItemDataIssue> unused = CheckTableUses({MakeJewel("No placeholders")});
    REQUIRE(unused.size() == 1);
    CHECK(unused.front().severity == ItemDataIssueSeverity::Warning);
    CHECK(unused.front().source == "ItemStats.json");
    CHECK(unused.front().field == "tables.wingDamage");
    CHECK(CheckTableUses({MakeJewel("Increase {table:wingDamage}% of Damage")}).empty());
}

TEST_CASE("A wing has the lines the other wings of its tier have [data][items]")
{
    const std::string lines = "Increase {table:wingDamage}% of Damage";
    const auto third = [&](short number) { return MakeWing(number, WingTier::Third, lines); };
    std::vector<ItemDefinition> wings = {third(36), third(37), third(38)};
    CHECK(CheckTableUses(wings).empty());

    // A new third-tier wing without the lines.
    wings.push_back(MakeWing(200, WingTier::Third, "Increase speed"));
    const std::vector<ItemDataIssue> missing = CheckTableUses(wings);
    REQUIRE(missing.size() == 1);
    CHECK(missing.front().severity == ItemDataIssueSeverity::Warning);
    CHECK(missing.front().number == 200);
    CHECK(missing.front().field == "description");
    CHECK(missing.front().message == "its wing lines are not those of the other third-tier wings of group 12: it has 1 "
                                     "line naming no table, they have 1 line naming {table:wingDamage}");

    // A wing moved to another tier keeps the tables of its old one.
    wings.back() = MakeWing(200, WingTier::Third, "Increase {table:otherDamage}% of Damage");
    REQUIRE(CheckTableUses(wings).size() == 1);

    // One that gains or loses a line without a table.
    wings.back() = third(200);
    wings.back().description.push_back(wings.back().description.front());
    wings.back().description.back().text.Set("en", "Increase speed");
    const std::vector<ItemDataIssue> extra = CheckTableUses(wings);
    REQUIRE(extra.size() == 1);
    CHECK(extra.front().message.find("it has 2 lines naming {table:wingDamage}") != std::string::npos);
}

TEST_CASE("Wings of a tier that split evenly are told they disagree, none of them blamed [data][items]")
{
    const auto wing = [](short number, const char* text) { return MakeWing(number, WingTier::Third, text); };
    // A pair, one wrong; and two against two.
    for (const std::vector<ItemDefinition>& wings :
         {std::vector<ItemDefinition>{wing(40, "Increase {table:wingDamage}%"), wing(50, "Increase {table:other}%")},
          std::vector<ItemDefinition>{wing(36, "Increase {table:wingDamage}%"),
                                      wing(37, "Increase {table:wingDamage}%"), wing(38, "Increase {table:other}%"),
                                      wing(39, "Increase {table:other}%")}})
    {
        const std::vector<ItemDataIssue> issues = CheckTableUses(wings);
        REQUIRE(issues.size() == 1);
        CHECK(issues.front().message.starts_with("the third-tier wings of group 12 don't agree on their wing lines: "));
    }

    // Wings without lines don't take a side in a tie of the others.
    std::vector<ItemDefinition> withLineless = {wing(36, "Increase {table:wingDamage}%"),
                                                wing(37, "Increase {table:other}%"), wing(200, "x")};
    withLineless.back().description.clear();
    const std::vector<ItemDataIssue> tie = CheckTableUses(withLineless);
    REQUIRE(tie.size() == 1);
    CHECK(tie.front().message.starts_with("the third-tier wings of group 12 don't agree on their wing lines: "));
}

TEST_CASE("Wings without lines never set the lines of their tier [data][items]")
{
    // Two new second-tier capes without lines, next to Cape of Fighter with them: the new ones are pointed out, though
    // they are more.
    auto cape = [](short number, const char* text)
    {
        ItemDefinition item = MakeWing(number, WingTier::Second, text);
        item.tags.Set(ItemTag::Cape);
        return item;
    };
    std::vector<ItemDefinition> capes = {cape(49, "Absorb {table:wingDamage}%"), cape(201, "x"), cape(202, "x")};
    capes[1].description.clear();
    capes[2].description.clear();
    const std::vector<ItemDataIssue> capeIssues = CheckTableUses(capes);
    REQUIRE(capeIssues.size() == 2);
    CHECK(capeIssues[0].number == 201);
    CHECK(capeIssues[1].number == 202);

    // The same for a whole batch: three new third-tier wings without lines next to two with them.
    const std::string lines = "Increase {table:wingDamage}% of Damage";
    std::vector<ItemDefinition> wings = {MakeWing(36, WingTier::Third, lines), MakeWing(37, WingTier::Third, lines)};
    for (short number = 300; number < 303; ++number)
    {
        wings.push_back(MakeWing(number, WingTier::Third, "x"));
        wings.back().description.clear();
    }
    const std::vector<ItemDataIssue> issues = CheckTableUses(wings);
    REQUIRE(issues.size() == 3);
    for (const ItemDataIssue& issue : issues)
    {
        CHECK(issue.number >= 300);
        CHECK(issue.message.find("it has 0 lines naming no table") != std::string::npos);
    }
}

TEST_CASE("A wing alone in its tier and group is compared with the other groups' wings of its tier [data][items]")
{
    const std::string lines = "Increase {table:wingDamage}% of Damage";
    std::vector<ItemDefinition> wings = {MakeWing(36, WingTier::Third, lines), MakeWing(37, WingTier::Third, lines)};
    // A third-tier wing of group 13, without the lines.
    wings.push_back(MakeWing(200, WingTier::Third, "Increase speed"));
    wings.back().group = 13;
    const std::vector<ItemDataIssue> issues = CheckTableUses(wings);
    REQUIRE(issues.size() == 1);
    CHECK(issues.front().group == 13);
    CHECK(
        issues.front().message.starts_with("its wing lines are not those of the third-tier wings of the other groups"));

    // Two wings alone in their groups, as Cape of Fighter (12) and Cape of Lord (13), aren't compared with each other.
    std::vector<ItemDefinition> capes = {MakeWing(49, WingTier::Second, "Absorb {table:wingDamage}%"),
                                         MakeWing(30, WingTier::Second, "Absorb {table:other}%")};
    capes[1].group = 13;
    CHECK(CheckTableUses(capes).empty());

    // A cape is compared with capes only.
    wings.back() = MakeWing(200, WingTier::Third, "Increase {table:otherDamage}% of Damage");
    wings.back().tags.Set(ItemTag::Cape);
    CHECK(CheckTableUses(wings).empty());
}
