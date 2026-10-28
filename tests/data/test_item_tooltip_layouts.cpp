#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ItemTooltipValues.h"

#include <algorithm>
#include <filesystem>
#include <tuple>
#include <string>
#include <vector>

using namespace Data::Items;
using namespace Engine::Items::TooltipValues;

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

constexpr int ScrollOfPoisonType = MakeItemType(15, 0);

// A tooltip file with the texts `texts` and a default layout of the four sections with `lines` after the name.
std::string TooltipFile(const std::string& texts, const std::string& lines)
{
    return R"({"formatVersion": 1, "texts": {)" + texts + R"(}, "layouts": [{"name": "default", "lines": [)" +
           R"({"section": "name"},)" + lines + R"({"section": "description"}, {"section": "classes"},)" +
           R"({"section": "options"}]}]})";
}

struct ReadResult
{
    ItemTooltipFile file;
    std::vector<ItemDataIssue> issues;

    // Whether an issue of that severity has the field and a message with `message` in it.
    bool Has(ItemDataIssueSeverity severity, const std::string& field, const std::string& message) const
    {
        for (const ItemDataIssue& issue : issues)
        {
            if (issue.severity == severity && issue.field == field && issue.message.find(message) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    ReadItemTooltipJson(text, "Tooltip.json", result.file, result.issues);
    return result;
}

const TooltipLayoutLine& FirstLine(const ReadResult& result)
{
    // After the name section.
    return result.file.FindLayout(DefaultTooltipLayout)->lines.at(1);
}
} // namespace

TEST_CASE("The shipped tooltip file loads without problems [data][items][tooltip]")
{
    const ItemTooltipLoadResult result = LoadItemTooltipFile(ClientDirectory / GetItemTooltipFile());
    for (const ItemDataIssue& issue : result.issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
    REQUIRE(result.file.FindLayout(DefaultTooltipLayout) != nullptr);
}

TEST_CASE("A tooltip layout line has its text, conditions, colors, style and values [data][items][tooltip]")
{
    const ReadResult result = Read(TooltipFile(R"("lacking": {"en": "Lacking {missing}", "de": "Fehlt {missing}"})",
                                               R"({"text": "@lacking", "when": ["requiredLevel", "!excellent",
                                                   "tag:sceptre"], "color": "red", "colors": [{"when": ["ancient"],
                                                   "color": "green"}], "bold": true,
                                                   "values": {"missing": "missingLevel"}},)"));
    CHECK(result.issues.empty());
    const TooltipLayoutLine& line = FirstLine(result);
    CHECK(line.textName == "lacking");
    REQUIRE(line.when.size() == 3);
    // Read as the value and the state they name.
    CHECK(line.when[0] == TooltipCondition{TooltipCondition::Kind::Value, TooltipValue::RequiredLevel});
    CHECK(line.when[0].GetName() == "requiredLevel");
    CHECK(line.when[1] == TooltipCondition{TooltipCondition::Kind::State, TooltipValue::Count, TooltipState::Excellent,
                                           ItemTag::Count, true});
    CHECK(line.when[1].GetName() == "excellent");
    CHECK(line.when[2].kind == TooltipCondition::Kind::Tag);
    CHECK(line.when[2].tag == ItemTag::Sceptre);
    CHECK(line.color == ItemTextColor::Red);
    REQUIRE(line.colors.size() == 1);
    CHECK(line.colors[0].color == ItemTextColor::Green);
    CHECK(line.bold);
    REQUIRE(line.values.size() == 1);
    CHECK(line.values[0] == std::pair<std::string, std::string>("missing", "missingLevel"));
    CHECK(result.file.FindText("lacking")->Get("de") == "Fehlt {missing}");
}

TEST_CASE("A tooltip layout line can have its own text or a blank one [data][items][tooltip]")
{
    const ReadResult result = Read(TooltipFile("", R"({"text": {"en": "Defense: {defense}"}}, {"text": ""},)"));
    CHECK(result.issues.empty());
    const auto& lines = result.file.FindLayout(DefaultTooltipLayout)->lines;
    CHECK(lines[1].textName.empty());
    CHECK(lines[1].text.GetNeutral() == "Defense: {defense}");
    CHECK(lines[2].text.IsEmpty());
}

TEST_CASE("A tooltip file's typos are errors with the name meant [data][items][tooltip]")
{
    const auto error =
        [](const std::string& texts, const std::string& lines, const std::string& field, const std::string& message)
    {
        INFO(lines);
        CHECK(Read(TooltipFile(texts, lines)).Has(ItemDataIssueSeverity::Error, field, message));
    };
    const std::string defense = R"("defense": "Defense: {defense}")";
    error(defense, R"({"text": "@defence"},)", "layouts[0].lines[1].text", "did you mean \"@defense\"?");
    error(defense, R"({"text": "@defense", "when": ["defence"]},)", "layouts[0].lines[1].when[0]",
          "did you mean defense?");
    error(defense, R"({"text": "@defense", "when": ["!excelent"]},)", "layouts[0].lines[1].when[0]",
          "did you mean excellent?");
    error(defense, R"({"text": "@defense", "when": ["tag:sceptres"]},)", "layouts[0].lines[1].when[0]",
          "names no item tag");
    error("", R"({"text": "Defense: {defence}"},)", "layouts[0].lines[1].text", "did you mean {defense}?");
    error(R"("lacking": "Lacking {missing}")", R"({"text": "@lacking"},)", "layouts[0].lines[1].text",
          "unknown placeholder {missing}");
    error("", R"({"text": "", "values": {"missing": "missingLevl"}},)", "layouts[0].lines[1].values.missing",
          "must name a built-in value");
    error("", R"({"text": "", "color": "pink"},)", "layouts[0].lines[1].color", "must be a color name");
    error("", R"({"text": "", "colors": [{"when": [], "color": "red"}]},)", "layouts[0].lines[1].colors[0].when",
          "needs a condition");
    error(R"("defense": {"en": "Defense: {defense}", "de": "Verteidigung: {defens}"})", R"({"text": "@defense"},)",
          "texts.defense.de", "the English text {defense}");
    error("", R"({"when": ["defense"]},)", "layouts[0].lines[1]", "must have a \"text\"");
    // A reference that's like no text names none.
    error(defense, R"({"text": "@nothing"},)", "layouts[0].lines[1].text", "\"@nothing\" is not a text of \"texts\"");
    CHECK_FALSE(Read(TooltipFile(defense, R"({"text": "@nothing"},)"))
                    .Has(ItemDataIssueSeverity::Error, "layouts[0].lines[1].text", "did you mean"));
    // A translation that drops a placeholder, adds one, or breaks a brace.
    error(R"("defense": {"en": "Defense: {defense}", "de": "Verteidigung"})", R"({"text": "@defense"},)",
          "texts.defense.de", "the English text {defense}");
    error(R"("defense": {"en": "Defense: {defense}", "de": "Verteidigung: {defense} {level}"})",
          R"({"text": "@defense"},)", "texts.defense.de", "the English text {defense}");
    error(R"("defense": {"en": "Defense: {defense}", "de": "Verteidigung: defense}"})", R"({"text": "@defense"},)",
          "texts.defense.de", "without its \"{\"");
}

TEST_CASE("A tooltip layout has the four sections once each, in their order [data][items][tooltip]")
{
    const auto read = [](const std::string& lines)
    { return Read(R"({"formatVersion": 1, "layouts": [{"name": "default", "lines": [)" + lines + "]}]}"); };
    CHECK(read(R"({"section": "name"}, {"section": "description"}, {"section": "classes"}, {"section": "options"})")
              .issues.empty());
    CHECK(read(R"({"section": "name"}, {"section": "classes"}, {"section": "description"}, {"section": "options"})")
              .Has(ItemDataIssueSeverity::Error, "layouts[0].lines[2].section", "in the order"));
    CHECK(read(R"({"section": "name"}, {"section": "description"}, {"section": "options"})")
              .Has(ItemDataIssueSeverity::Error, "layouts[0].lines", "must have the sections"));
    CHECK(read(R"({"text": ""}, {"section": "name"}, {"section": "description"}, {"section": "classes"},
                  {"section": "options"})")
              .Has(ItemDataIssueSeverity::Error, "layouts[0].lines[1].section", "start after the name"));
    CHECK(read(R"({"section": "name"}, {"section": "description"}, {"section": "classes"}, {"section": "options"},
                  {"text": ""})")
              .Has(ItemDataIssueSeverity::Error, "layouts[0].lines[4]", "no line comes after it"));
    CHECK(read(R"({"section": "name"}, {"section": "summary"})")
              .Has(ItemDataIssueSeverity::Error, "layouts[0].lines[1].section", "must be \"name\""));
}

TEST_CASE("A tooltip file needs the default layout [data][items][tooltip]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "layouts": []})");
    CHECK(result.Has(ItemDataIssueSeverity::Error, "layouts", "must have the layout \"default\""));

    // A layout of another name doesn't stand in for it.
    const ReadResult other = Read(R"({"formatVersion": 1, "layouts": [{"name": "compact", "lines": [
        {"section": "name"}, {"section": "description"}, {"section": "classes"}, {"section": "options"}]}]})");
    CHECK(other.Has(ItemDataIssueSeverity::Error, "layouts", "must have the layout \"default\""));
    CHECK(other.Has(ItemDataIssueSeverity::Warning, "layouts[0].name", "only the \"default\" layout is used"));
}

TEST_CASE("A tooltip file warns about a line value its text does not have [data][items][tooltip]")
{
    const ReadResult result = Read(
        TooltipFile(R"("used": "Defense: {defense}")", R"({"text": "@used", "values": {"missing": "missingLevel"}},)"));
    CHECK_FALSE(HasErrors(result.issues));
    CHECK(result.Has(ItemDataIssueSeverity::Warning, "layouts[0].lines[1].values.missing", "has no {missing}"));
}

TEST_CASE("A shared text is used by a layout or an item description [data][items][tooltip]")
{
    const ReadResult tooltips = Read(
        TooltipFile(R"("unused": "Nothing shows this", "layout": "Defense: {defense}", "life": "Life: {durability}",)"
                    R"("table": "Up {notATable}", "stats": "Up {table:capeDamageIncrease}", "bound": "Up {percent}%")",
                    R"({"text": "@layout"},)"));
    ItemDefinition pet;
    pet.group = 13;
    pet.number = 0;
    pet.names.Set("en", "Pet");
    ItemDescriptionLine life;
    life.textName = "life";
    ItemDescriptionLine typo;
    typo.textName = "lif";
    ItemDescriptionLine table;
    table.textName = "table";
    ItemDescriptionLine stats;
    stats.textName = "stats";
    // The line's "values" names the table instead, and has a value its text doesn't show.
    ItemDescriptionLine bound;
    bound.textName = "bound";
    bound.values = {{"percent", "table:capeDamageIncrease"}, {"missing", "missingLevel"}};
    pet.description = {life, typo, table, stats, bound};
    const std::vector<ItemDefinition> items = {pet};
    std::vector<ItemDataIssue> issues;
    ItemStatFile statFile;
    LevelBonusTable cape;
    cape.name = "capeDamageIncrease";
    statFile.tables.push_back(cape);
    ValidateDescriptionPlaceholders(items, ItemStatProfiles(statFile), tooltips.file, "ItemStats.json", issues);
    ValidateDescriptionTexts(items, tooltips.file, "Tooltip.json", issues);
    const auto has = [&](ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    {
        return std::any_of(issues.begin(), issues.end(),
                           [&](const ItemDataIssue& issue)
                           {
                               return issue.severity == severity && issue.field == field &&
                                      issue.message.find(message) != std::string::npos;
                           });
    };
    CHECK(has(ItemDataIssueSeverity::Error, "description[1].text", "did you mean \"@life\"?"));
    CHECK(has(ItemDataIssueSeverity::Error, "description[2].text", "{notATable}"));
    // A shared text names a table only through the line's values, also one the stat file has: the editor's table
    // counts and rename read an item's own data.
    CHECK(has(ItemDataIssueSeverity::Error, "description[3].text", "names the table {table:capeDamageIncrease}"));
    CHECK_FALSE(has(ItemDataIssueSeverity::Error, "description[4].text", ""));
    CHECK_FALSE(has(ItemDataIssueSeverity::Error, "description[4].values.percent", ""));
    CHECK(has(ItemDataIssueSeverity::Warning, "description[4].values.missing", "has no {missing}"));
    CHECK(has(ItemDataIssueSeverity::Warning, "texts.unused", "neither a layout nor an item description"));
    CHECK_FALSE(has(ItemDataIssueSeverity::Warning, "texts.life", ""));
    CHECK_FALSE(has(ItemDataIssueSeverity::Warning, "texts.layout", ""));
}

TEST_CASE("A condition compares a value with a whole number [data][items][tooltip]")
{
    const auto read = [](std::string_view text)
    {
        TooltipCondition condition;
        std::string error;
        const bool read = ReadTooltipCondition(text, condition, error);
        return std::make_tuple(read, condition, error);
    };
    const auto [equal, durabilityIsTwo, noError] = read("durability=2");
    REQUIRE(equal);
    CHECK(durabilityIsTwo.comparison == TooltipCondition::Comparison::Equal);
    CHECK(durabilityIsTwo.number == 2);
    CHECK(WriteTooltipCondition(durabilityIsTwo) == "durability=2");
    for (const std::string_view text :
         {"level!=1", "level<=3", "level>=-1", "!durability>4", "level<10", "durability>=2147483647", "level>-2147483648"})
    {
        INFO(text);
        const auto [read_, condition, error] = read(text);
        CHECK(read_);
        CHECK(WriteTooltipCondition(condition) == text);
    }
    CHECK(std::get<2>(read("timed=1")).find("compares a state") != std::string::npos);
    CHECK(std::get<2>(read("durability=two")).find("no comparison with a whole number") != std::string::npos);
    // Past int's range, signs other than one "-", spaces and anything after the number.
    for (const std::string_view text : {"durability=2147483648", "durability>-2147483649", "durability=99999999999",
                                        "durability=+2", "durability=--2", "durability=-", "durability=", "durability= 2",
                                        "durability=2 ", "durability=2x", "durability=1.5"})
    {
        INFO(text);
        CHECK(std::get<2>(read(text)).find("no comparison with a whole number") != std::string::npos);
    }
    CHECK(std::get<2>(read("durabilty=2")).find("compares no built-in value") != std::string::npos);

    TooltipCondition condition = durabilityIsTwo;
    CHECK(CompareTooltipValue(condition, 2));
    CHECK_FALSE(CompareTooltipValue(condition, 3));
    condition.comparison = TooltipCondition::Comparison::GreaterOrEqual;
    CHECK(CompareTooltipValue(condition, 3));
    CHECK_FALSE(CompareTooltipValue(condition, 1));
}

TEST_CASE("A tooltip line's conditions all hold, each turned around by \"!\" [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    ItemTooltipValues values;
    values.defense = 12;
    values.excellent = true;
    // Conditions as a file writes them, read.
    const auto met = [&](std::vector<std::string_view> texts, int type = ScrollOfPoisonType, int level = 0)
    {
        std::vector<TooltipCondition> conditions(texts.size());
        for (size_t index = 0; index < texts.size(); ++index)
        {
            std::string error;
            REQUIRE(ReadTooltipCondition(texts[index], conditions[index], error));
        }
        return AreTooltipConditionsMet(conditions, type, level, values);
    };
    CHECK(met({}));
    CHECK(met({"defense", "excellent"}));
    CHECK_FALSE(met({"defense", "ancient"}));
    CHECK(met({"!ancient"}));
    CHECK_FALSE(met({"magicDefense"}));
    // {level} is a value too.
    CHECK_FALSE(met({"level"}));
    CHECK(met({"level"}, ScrollOfPoisonType, 3));
    // A tag, and a tag turned around: the Battle Scepter is a sceptre, the Kris is not.
    CHECK(met({"tag:sceptre"}, MakeItemType(2, 8)));
    CHECK_FALSE(met({"tag:sceptre"}, MakeItemType(0, 0)));
    CHECK(met({"!tag:sceptre"}, MakeItemType(0, 0)));
    // A value compared with a number.
    values.durability = 2;
    CHECK(met({"durability=2"}));
    CHECK_FALSE(met({"durability=1"}));
    CHECK(met({"!durability=1"}));
    CHECK(met({"level!=1"}, ScrollOfPoisonType, 2));
}

TEST_CASE("A tooltip line takes the color of its first color rule that holds [data][items][tooltip]")
{
    ItemTooltipValues values;
    values.excellent = true;
    values.damageHarmonyBonus = true;
    // Conditions as a file has them: read, so the state each names is resolved.
    const auto read = [](std::string_view text)
    {
        TooltipCondition condition;
        std::string error;
        REQUIRE(ReadTooltipCondition(text, condition, error));
        return condition;
    };
    TooltipLayoutLine line;
    line.color = ItemTextColor::Gray;
    line.colors = {{{read("ancient")}, ItemTextColor::Green},
                   {{read("damageHarmonyBonus")}, ItemTextColor::Yellow},
                   {{read("excellent")}, ItemTextColor::Blue}};
    CHECK(GetTooltipLineColor(line, ScrollOfPoisonType, 0, values) == ItemTextColor::Yellow);
    values.damageHarmonyBonus = false;
    CHECK(GetTooltipLineColor(line, ScrollOfPoisonType, 0, values) == ItemTextColor::Blue);
    values.excellent = false;
    CHECK(GetTooltipLineColor(line, ScrollOfPoisonType, 0, values) == ItemTextColor::Gray);
}

TEST_CASE("Every tooltip state is one the game sets [data][items][tooltip]")
{
    ItemTooltipValues values;
    values.hasDamage = values.skillBook = values.twoHanded = values.excellent = values.ancient = true;
    values.damageHarmonyBonus = values.defenseHarmonyBonus = values.armour = values.wizardryWeapon = true;
    values.strength.reduced = values.dexterity.reduced = values.hasDurability = values.timed = values.hasOptions = true;
    values.strength.asked = values.dexterity.asked = true;
    for (const std::string_view state : GetTooltipStates())
    {
        INFO(state);
        CHECK(GetItemTooltipState(values, state));
    }
    CHECK_FALSE(GetItemTooltipState(values, "unknownState"));
}

TEST_CASE("A tooltip file's texts are checked like item names [data][items][tooltip]")
{
    const ReadResult result = Read(
        TooltipFile(R"("onlyGerman": {"de": "Nur Deutsch"}, "badCode": {"en": "A text", "d e": "Ein Text"},)"
                    R"("unknown": {"en": "A text", "xx": "Ein Text"}, "blank": "")",
                    R"({"text": "@onlyGerman"}, {"text": "@badCode"}, {"text": "@unknown"}, {"text": "@blank"},)"));

    CHECK(result.Has(ItemDataIssueSeverity::Error, "texts.onlyGerman", "the English text is missing"));
    CHECK(result.Has(ItemDataIssueSeverity::Error, "texts.badCode", "\"d e\" is not a language code"));
    CHECK_FALSE(result.Has(ItemDataIssueSeverity::Error, "texts.blank", "English"));

    const char* uiLocales[] = {"en", "de"};
    std::vector<ItemDataIssue> issues;
    ValidateTooltipTextLocales(result.file, uiLocales, "Tooltip.json", issues);
    REQUIRE(issues.size() == 2);
    CHECK(issues[0].field == "texts.badCode");
    CHECK(issues[1].field == "texts.unknown");
    CHECK(issues[1].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[1].message == "the xx text is never shown: the game has no texts in that language");
}
