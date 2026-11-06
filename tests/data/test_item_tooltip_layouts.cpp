#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
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

// The sections of a layout after the name section, in their order.
constexpr const char* SectionsAfterName = R"({"section": "itemCases"}, {"section": "description"},)"
                                          R"({"section": "classes"}, {"section": "options"},)"
                                          R"({"section": "afterOptions"})";

// A tooltip file with the texts `texts` and a default layout of the six sections with `lines` after the name, and the
// layouts `presets` after it (each with a comma before it).
std::string TooltipFile(const std::string& texts, const std::string& lines, const std::string& presets = "")
{
    return R"({"formatVersion": 2, "texts": {)" + texts + R"(}, "layouts": [{"name": "default", "lines": [)" +
           R"({"section": "name"},)" + lines + SectionsAfterName + "]}" + presets + "]}";
}

// The ids of the lines of a built layout, "-" for a line without one and "+" for a line an item adds.
std::vector<std::string> LineIds(const std::vector<BuiltTooltipLine>& lines)
{
    std::vector<std::string> ids;
    for (const BuiltTooltipLine& line : lines)
    {
        if (line.GetSection() == TooltipSection::None)
        {
            ids.push_back(line.item != nullptr ? "+" : line.GetId().empty() ? "-" : std::string(line.GetId()));
        }
    }
    return ids;
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

// An item file with a tag the presets replaced is told what to write instead (ItemJsonFormat): the presets and line
// ids it names are the shipped file's.
TEST_CASE("The shipped tooltip file has what the removed tags' errors name [data][items][tooltip]")
{
    const ItemTooltipLoadResult result = LoadItemTooltipFile(ClientDirectory / GetItemTooltipFile());
    std::vector<BuiltTooltipLine> defaultLines;
    BuildTooltipLayout(result.file, DefaultTooltipLayout, defaultLines);
    REQUIRE_FALSE(defaultLines.empty());
    REQUIRE_FALSE(GetRemovedTooltipTags().empty());
    for (const RemovedTooltipTag& removed : GetRemovedTooltipTags())
    {
        INFO(std::string(removed.tag));
        CHECK(removed.preset.empty() != removed.hide.empty());
        for (const std::string_view preset : {removed.preset, removed.bothPreset})
        {
            if (!preset.empty())
            {
                INFO(std::string(preset));
                CHECK(result.file.FindLayout(preset) != nullptr);
            }
        }
        for (const std::string_view id : removed.hide)
        {
            INFO(std::string(id));
            CHECK(HasTooltipLine(defaultLines, id));
        }
    }
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
        CHECK(Read(TooltipFile(texts, lines)).Has(ItemDataIssueSeverity::Fatal, field, message));
    };
    const std::string defense = R"("defense": "Defense: {defense}")";
    error(defense, R"({"text": "@defence"},)", "layouts[0].lines[1].text", "did you mean \"@defense\"?");
    error(defense, R"({"text": "@defense", "when": ["defence"]},)", "layouts[0].lines[1].when[0]",
          "did you mean defense?");
    error(defense, R"({"text": "@defense", "when": ["!excelent"]},)", "layouts[0].lines[1].when[0]",
          "did you mean excellent?");
    error(defense, R"({"text": "@defense", "when": ["tag:sceptres"]},)", "layouts[0].lines[1].when[0]",
          "names no item tag");
    // A tag the layouts replaced says what replaced it.
    error(defense, R"({"text": "@defense", "when": ["!tag:noDurabilityLine"]},)", "layouts[0].lines[1].when[0]",
          R"(replaced: the items that had it have "tooltipLayout": "noDurabilityLine")");
    error(defense, R"({"text": "@defense", "colors": [{"when": ["tag:noLevelRequirement"], "color": "red"}]},)",
          "layouts[0].lines[1].colors[0].when[0]",
          R"(have "tooltipLayoutChanges": {"hide": ["requiredLevel", "lackingLevel"]})");
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
                    .Has(ItemDataIssueSeverity::Fatal, "layouts[0].lines[1].text", "did you mean"));
    // A translation that drops a placeholder, adds one, or breaks a brace.
    error(R"("defense": {"en": "Defense: {defense}", "de": "Verteidigung"})", R"({"text": "@defense"},)",
          "texts.defense.de", "the English text {defense}");
    error(R"("defense": {"en": "Defense: {defense}", "de": "Verteidigung: {defense} {level}"})",
          R"({"text": "@defense"},)", "texts.defense.de", "the English text {defense}");
    error(R"("defense": {"en": "Defense: {defense}", "de": "Verteidigung: defense}"})", R"({"text": "@defense"},)",
          "texts.defense.de", "without its \"{\"");
}

TEST_CASE("A tooltip layout has the six sections once each, in their order [data][items][tooltip]")
{
    const auto read = [](const std::string& lines)
    { return Read(R"({"formatVersion": 2, "layouts": [{"name": "default", "lines": [)" + lines + "]}]}"); };
    CHECK(read(std::string(R"({"section": "name"},)") + SectionsAfterName).issues.empty());
    CHECK(read(R"({"section": "name"}, {"section": "itemCases"}, {"section": "classes"}, {"section": "description"},
                  {"section": "options"}, {"section": "afterOptions"})")
              .Has(ItemDataIssueSeverity::Fatal, "layouts[0].lines[3].section", "in the order"));
    CHECK(read(R"({"section": "name"}, {"section": "itemCases"}, {"section": "description"}, {"section": "classes"},
                  {"section": "options"})")
              .Has(ItemDataIssueSeverity::Fatal, "layouts[0].lines", "must have the sections"));
    // Lines may come before the name (the first rows) and after the last section.
    CHECK(read(std::string(R"({"text": ""}, {"section": "name"},)") + SectionsAfterName + R"(, {"text": ""})")
              .issues.empty());
    CHECK(read(R"({"section": "name"}, {"section": "summary"})")
              .Has(ItemDataIssueSeverity::Fatal, "layouts[0].lines[1].section", "must be one of the sections"));
}

// A file of version 1 (before the code lines became layout lines) is refused for its version, not read into errors
// about the sections and lines it lacks.
TEST_CASE("A tooltip file of version 1 is refused for its version [data][items][tooltip]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "layouts": [{"name": "default", "lines": [
        {"section": "name"}, {"section": "description"},
        {"text": "@durability", "when": ["hasDurability", "!tag:noDurabilityLine"]},
        {"section": "classes"}, {"section": "options"}]}]})");
    CHECK(result.Has(ItemDataIssueSeverity::Fatal, "formatVersion", "version 1 is from before"));
    CHECK(result.issues.size() == 1);
    CHECK(Read(R"({"formatVersion": 3, "layouts": []})")
              .Has(ItemDataIssueSeverity::Fatal, "formatVersion", "this client reads up to 2"));
}

TEST_CASE("A tooltip file needs the default layout [data][items][tooltip]")
{
    const ReadResult result = Read(R"({"formatVersion": 2, "layouts": []})");
    CHECK(result.Has(ItemDataIssueSeverity::Fatal, "layouts", "must have the layout \"default\""));

    // A layout of another name doesn't stand in for it, and the default one isn't a preset.
    const ReadResult other = Read(std::string(R"({"formatVersion": 2, "layouts": [{"name": "compact", "lines": [)") +
                                  R"({"section": "name"},)" + SectionsAfterName + "]}]}");
    CHECK(other.Has(ItemDataIssueSeverity::Fatal, "layouts", "must have the layout \"default\""));
    CHECK(Read(R"({"formatVersion": 2, "layouts": [{"name": "default", "base": "compact"}]})")
              .Has(ItemDataIssueSeverity::Fatal, "layouts[0].base", "the default layout has lines of its own"));
}

// D73: a preset is its base without the lines it hides and with the lines it adds, also a preset of a preset.
TEST_CASE("A tooltip layout preset hides and adds lines of its base [data][items][tooltip]")
{
    const std::string lines = R"({"id": "first", "text": "First"}, {"id": "second", "text": "Second"}, {"text": ""},)";
    const ReadResult result =
        Read(TooltipFile("", lines,
                         R"(, {"name": "short", "base": "default", "hide": ["first"],
                               "add": [{"after": "second", "lines": [{"id": "third", "text": "Third"},
                                                                       {"text": "Fourth"}]}]},
                             {"name": "shorter", "base": "short", "hide": ["third"]})"));
    CHECK(result.issues.empty());
    std::vector<BuiltTooltipLine> built;
    BuildTooltipLayout(result.file, DefaultTooltipLayout, built);
    CHECK(LineIds(built) == std::vector<std::string>{"first", "second", "-"});
    BuildTooltipLayout(result.file, "short", built);
    CHECK(LineIds(built) == std::vector<std::string>{"second", "third", "-", "-"});
    BuildTooltipLayout(result.file, "shorter", built);
    CHECK(LineIds(built) == std::vector<std::string>{"second", "-", "-"});
    // The sections stay in every preset.
    CHECK(std::count_if(built.begin(), built.end(), [](const BuiltTooltipLine& line)
                        { return line.GetSection() != TooltipSection::None; }) == 6);
    BuildTooltipLayout(result.file, "nothing", built);
    CHECK(built.empty());

    const auto error = [&](const std::string& presets, const std::string& field, const std::string& message)
    {
        INFO(presets);
        CHECK(Read(TooltipFile("", lines, presets)).Has(ItemDataIssueSeverity::Fatal, field, message));
    };
    error(R"(, {"name": "short", "base": "nothing"})", "layouts[1].base", "\"nothing\" is not a layout");
    error(R"(, {"name": "one", "base": "two"}, {"name": "two", "base": "one"})", "layouts[1].base",
          "go round in a circle");
    error(R"(, {"name": "short", "base": "default", "hide": ["frist"]})", "layouts[1].hide[0]",
          "has no line with the id \"frist\"");
    error(R"(, {"name": "short", "base": "default", "hide": ["first"], "add": [{"after": "first", "lines": []}]})",
          "layouts[1].add[0].after", "the preset hides the line \"first\" itself");
    // A line a base preset leaves out is named with it.
    error(R"(, {"name": "short", "base": "default", "hide": ["first"]},
             {"name": "shorter", "base": "short", "hide": ["first"], "add": [{"after": "first", "lines": []}]})",
          "layouts[2].hide[0]", R"(the layout "short" has no line with the id "first" (the preset "short" leaves it)");
    error(R"(, {"name": "short", "base": "default", "hide": ["first"]},
             {"name": "shorter", "base": "short", "add": [{"after": "first", "lines": []}]})",
          "layouts[2].add[0].after", R"((the preset "short" leaves it out))");
    error(R"(, {"name": "short", "base": "default",
                "add": [{"after": "second", "lines": [{"id": "first", "text": ""}]}]})",
          "layouts[1].add", "which its layout has already");
    error(R"(, {"name": "short", "base": "default", "add": [{"after": "second", "lines": [{"section": "name"}]}]})",
          "layouts[1].add[0].lines[0]", "can't add a section");
    error(R"(, {"name": "short", "base": "default", "lines": []})", "layouts[1].lines", "has no lines of its own");
    // Its changes are read as an item's are (Json::ReadLineIds, ReadLineAddition): ids and "after" are names.
    error(R"(, {"name": "short", "base": "default", "hide": ["first line"]})", "layouts[1].hide", "list of line ids");
    error(R"(, {"name": "short", "base": "default", "add": [{"after": "first line", "lines": []}]})",
          "layouts[1].add[0]", R"(must be {"after")");
    error(R"(, {"name": "short", "base": "default", "add": {"after": "first", "lines": []}})", "layouts[1].add",
          "must be a list of");
    error(R"(, {"name": "full", "lines": [{"section": "name"}], "hide": ["first"]})", "layouts[1].hide",
          "only a preset");
    CHECK(Read(TooltipFile("", R"({"id": "first", "text": ""}, {"id": "first", "text": ""},)"))
              .Has(ItemDataIssueSeverity::Fatal, "layouts[0].lines", "more than one line with the id \"first\""));
}

namespace
{
// The issues ValidateDescriptionTexts finds for one item.
std::vector<ItemDataIssue> TextIssuesOf(const ItemDefinition& item, const ItemTooltipFile& tooltips)
{
    std::vector<ItemDataIssue> issues;
    ValidateDescriptionTexts(std::vector<ItemDefinition>{item}, tooltips, "Tooltip.json", issues);
    return issues;
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field,
              const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                       {
                           return issue.severity == severity && issue.field == field &&
                                  issue.message.find(message) != std::string::npos;
                       });
}
} // namespace

// D73: an item names its layout and changes it itself; the layout and the ids it names are checked with the file.
TEST_CASE("An item's tooltip layout is its preset with its own changes [data][items][tooltip]")
{
    const ReadResult tooltips =
        Read(TooltipFile("", R"({"id": "first", "text": "First"}, {"id": "second", "text": "Second"},)",
                         R"(, {"name": "short", "base": "default", "hide": ["first"]})"));
    REQUIRE(tooltips.issues.empty());
    ItemDefinition item;
    item.group = 13;
    item.number = 0;
    item.names.Set("en", "Pet");
    item.tooltipLayout = "short";
    ItemDescriptionLine own;
    own.text.Set("en", "Own");
    item.tooltipLayoutChanges.add = {{"second", {own}}};
    std::vector<BuiltTooltipLine> built;
    BuildItemTooltipLayout(tooltips.file, item, built);
    CHECK(LineIds(built) == std::vector<std::string>{"second", "+"});
    item.tooltipLayout.clear();
    item.tooltipLayoutChanges.hide = {"second"};
    item.tooltipLayoutChanges.add = {{"first", {own}}};
    BuildItemTooltipLayout(tooltips.file, item, built);
    CHECK(LineIds(built) == std::vector<std::string>{"first", "+"});
    CHECK(CheckItemTooltipLayout(tooltips.file, item).empty());
    CHECK_FALSE(HasFatal(TextIssuesOf(item, tooltips.file)));

    ItemDefinition typo = item;
    typo.tooltipLayout = "shrot";
    CHECK(HasIssue(TextIssuesOf(typo, tooltips.file), ItemDataIssueSeverity::Fatal, "tooltipLayout",
                   "did you mean \"short\"?"));
    ItemDefinition unknownId = item;
    unknownId.tooltipLayoutChanges.hide = {"third"};
    CHECK(HasIssue(TextIssuesOf(unknownId, tooltips.file), ItemDataIssueSeverity::Fatal,
                   "tooltipLayoutChanges.hide[0]", "has no line with the id \"third\""));
    ItemDefinition hiddenAfter = item;
    hiddenAfter.tooltipLayoutChanges.hide = {"first"};
    CHECK(HasIssue(TextIssuesOf(hiddenAfter, tooltips.file), ItemDataIssueSeverity::Fatal,
                   "tooltipLayoutChanges.add[0].after", "the item hides the line \"first\" itself"));
    // A line its preset leaves out is the preset's doing, not the item's.
    ItemDefinition presetHides = item;
    presetHides.tooltipLayout = "short";
    presetHides.tooltipLayoutChanges.hide = {"first"};
    const std::vector<ItemDataIssue> presetIssues = TextIssuesOf(presetHides, tooltips.file);
    CHECK(HasIssue(presetIssues, ItemDataIssueSeverity::Fatal, "tooltipLayoutChanges.hide[0]",
                   R"(the layout "short" has no line with the id "first" (the preset "short" leaves it out))"));
    CHECK(HasIssue(presetIssues, ItemDataIssueSeverity::Fatal, "tooltipLayoutChanges.add[0].after",
                   R"((the preset "short" leaves it out))"));
    CHECK_FALSE(
        HasIssue(presetIssues, ItemDataIssueSeverity::Fatal, "tooltipLayoutChanges.add[0].after", "the item hides"));
    // The added lines are checked as the item's other lines are.
    ItemDefinition sharedTypo = item;
    sharedTypo.tooltipLayoutChanges.add[0].lines[0].textName = "nothing";
    CHECK(HasIssue(TextIssuesOf(sharedTypo, tooltips.file), ItemDataIssueSeverity::Fatal,
                   "tooltipLayoutChanges.add[0].lines[0].text", "is not a text of Tooltip.json"));

    // Whether the item's own changes fit another layout (Item Tooltip offers only those): its hidden "second" and its
    // line after "first" fit the default layout, not "short", which hides "first" itself.
    CHECK(CheckTooltipLayoutChanges(tooltips.file, DefaultTooltipLayout, item.tooltipLayoutChanges).empty());
    const auto onShort = CheckTooltipLayoutChanges(tooltips.file, "short", item.tooltipLayoutChanges);
    REQUIRE(onShort.size() == 1);
    CHECK(onShort[0].first == "tooltipLayoutChanges.add[0].after");
    CHECK(CheckTooltipLayoutChanges(tooltips.file, "missing", item.tooltipLayoutChanges).empty());
}

// The kinds the original client gave no class lines (its list, now the preset noClassLines): an item of them whose
// layout has the class lines warns.
TEST_CASE("An item of a kind without class lines warns when its layout has them [data][items][tooltip]")
{
    std::string file = TooltipFile(R"("lines": "Can be equipped by {className}")", "",
                                   R"(, {"name": "noClassLines", "base": "default", "hide": ["classLines"]})");
    const std::string classes = R"({"section": "classes"},)";
    file.replace(file.find(classes), classes.size(),
                 classes + R"({"id": "classLines", "text": "@lines", "forEachClass": true},)");
    const ReadResult tooltips = Read(file);
    REQUIRE(tooltips.issues.empty());
    ItemDefinition elixir;
    elixir.group = 14;
    elixir.number = 0;
    elixir.names.Set("en", "Elixir");
    elixir.tags.Set(ItemTag::Elixir);
    const auto warns = [&](const ItemDefinition& item)
    { return HasIssue(TextIssuesOf(item, tooltips.file), ItemDataIssueSeverity::Warning, "tooltipLayout", "elixir"); };
    CHECK(warns(elixir));
    elixir.tooltipLayout = "noClassLines";
    CHECK_FALSE(warns(elixir));
    // Its own changes leaving them out do too, and its own lines added after them have no id.
    ItemDefinition ownChanges = elixir;
    ownChanges.tooltipLayout.clear();
    ownChanges.tooltipLayoutChanges.hide = {"classLines"};
    CHECK_FALSE(warns(ownChanges));
    ownChanges.tooltipLayoutChanges.hide.clear();
    ownChanges.tooltipLayoutChanges.add = {{"classLines", {ItemDescriptionLine{}}}};
    CHECK(warns(ownChanges));
    // Any other item may show its class lines.
    ItemDefinition sword = elixir;
    sword.tags = {};
    sword.tooltipLayout.clear();
    CHECK_FALSE(warns(sword));
}

TEST_CASE("A tooltip file warns about a line value its text does not have [data][items][tooltip]")
{
    const ReadResult result = Read(
        TooltipFile(R"("used": "Defense: {defense}")", R"({"text": "@used", "values": {"missing": "missingLevel"}},)"));
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(result.Has(ItemDataIssueSeverity::Warning, "layouts[0].lines[1].values.missing", "has no {missing}"));
}

// {className} and classUsable are a class line's ("forEachClass"): on another line, a layout's or an item's own, the
// one is always empty and the other never holds.
TEST_CASE("A line without forEachClass warns about className and classUsable [data][items][tooltip]")
{
    const std::string texts = R"("equipped": "Can be equipped by {className}")";
    const auto warns = [&](const std::string& line)
    {
        INFO(line);
        const ReadResult result = Read(TooltipFile(texts, line + ","));
        CHECK_FALSE(HasFatal(result.issues));
        return result.Has(ItemDataIssueSeverity::Warning, "layouts[0].lines[1]", "only a layout line with") ||
               result.Has(ItemDataIssueSeverity::Warning, "layouts[0].lines[1]", "holds only on a layout line with");
    };
    CHECK(warns(R"({"text": "Class: {className}"})"));
    CHECK(warns(R"({"text": "@equipped"})"));
    CHECK(warns(R"({"text": "Class: {who}", "values": {"who": "className"}})"));
    CHECK(warns(R"({"text": "", "when": ["!classUsable"]})"));
    CHECK(warns(R"({"text": "", "colors": [{"when": ["classUsable"], "color": "red"}]})"));
    CHECK_FALSE(warns(R"({"text": "@equipped", "forEachClass": true, "when": ["!classUsable"]})"));
    // A placeholder of that name that its "values" points at another value shows that one.
    CHECK_FALSE(warns(R"({"text": "{className}", "values": {"className": "defense"}})"));

    // An item's own lines never have "forEachClass".
    const ReadResult tooltips = Read(TooltipFile(texts, ""));
    REQUIRE_FALSE(HasFatal(tooltips.issues));
    ItemDefinition item;
    item.group = 0;
    item.number = 1;
    item.names.Set("en", "Kris");
    ItemDescriptionLine own;
    own.textName = "equipped";
    item.description = {own};
    ItemNameLine name;
    name.when = {TooltipCondition{}};
    std::string error;
    REQUIRE(ReadTooltipCondition("classUsable", name.when[0], error));
    item.nameLines = {name};
    ItemDescriptionLine added;
    added.text.Set("en", "{className}");
    item.tooltipLayoutChanges.add = {{"first", {added}}};
    const std::vector<ItemDataIssue> issues = TextIssuesOf(item, tooltips.file);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "description[0]", "it is empty here"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "nameLines[0]", "it never holds here"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "tooltipLayoutChanges.add[0].lines[0]", "it is empty here"));
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
    CHECK(has(ItemDataIssueSeverity::Fatal, "description[1].text", "did you mean \"@life\"?"));
    CHECK(has(ItemDataIssueSeverity::Fatal, "description[2].text", "{notATable}"));
    // A shared text names a table only through the line's values, also one the stat file has: the editor's table
    // counts and rename read an item's own data.
    CHECK(has(ItemDataIssueSeverity::Fatal, "description[3].text", "names the table {table:capeDamageIncrease}"));
    CHECK_FALSE(has(ItemDataIssueSeverity::Fatal, "description[4].text", ""));
    CHECK_FALSE(has(ItemDataIssueSeverity::Fatal, "description[4].values.percent", ""));
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
    values.socketItem = values.upperTierWings = values.skillLuckOrOption = values.setItem = true;
    values.allClasses = values.classUsable = values.boots = values.gloves = values.expired = true;
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

    CHECK(result.Has(ItemDataIssueSeverity::Fatal, "texts.onlyGerman", "the English text is missing"));
    CHECK(result.Has(ItemDataIssueSeverity::Fatal, "texts.badCode", "\"d e\" is not a language code"));
    CHECK_FALSE(result.Has(ItemDataIssueSeverity::Fatal, "texts.blank", "English"));

    const char* uiLocales[] = {"en", "de"};
    std::vector<ItemDataIssue> issues;
    ValidateTooltipTextLocales(result.file, uiLocales, "Tooltip.json", issues);
    REQUIRE(issues.size() == 2);
    CHECK(issues[0].field == "texts.badCode");
    CHECK(issues[1].field == "texts.unknown");
    CHECK(issues[1].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[1].message == "the xx text is never shown: the game has no texts in that language");
}
