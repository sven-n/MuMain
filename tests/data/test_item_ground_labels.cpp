#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemGroundLabels.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
const std::string Source = "GroundLabels.json";

struct LabelReadResult
{
    GroundLabelFile file;
    std::vector<ItemDataIssue> issues;
};

LabelReadResult ReadLabels(const std::string& text)
{
    LabelReadResult result;
    ReadGroundLabelsJson(text, Source, result.file, result.issues);
    return result;
}

LabelReadResult ReadStylesAndRules(const std::string& styles, const std::string& standard)
{
    return ReadLabels(R"({"formatVersion": 1, "styles": {)" + styles + R"(}, "standard": [)" + standard + "]}");
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}

const ItemDataIssue* FindIssue(const std::vector<ItemDataIssue>& issues, const std::string& field)
{
    const auto found = std::find_if(issues.begin(), issues.end(),
                                    [&](const ItemDataIssue& issue) { return issue.field == field; });
    return found != issues.end() ? &*found : nullptr;
}

struct ItemReadResult
{
    std::vector<ItemDefinition> items;
    std::vector<ItemDataIssue> issues;
};

ItemReadResult ReadItem(const std::string& fields)
{
    ItemReadResult result;
    const std::string text =
        R"({"formatVersion": 1, "group": 14, "items": [{"number": 32, "name": "Pink Chocolate Box", )" + fields + "}]}";
    ReadItemGroupJson(text, "Group14_Potion.json", result.items, result.issues);
    return result;
}

GroundLabelFile MakeLabels()
{
    GroundLabelFile file;
    file.styles.push_back({"giftBoxPink", {255, 76, 255}, {0, 0, 0}, false});
    file.styles.push_back({"white", {255, 255, 255}, {0, 0, 0}, false});
    file.styles.push_back({"setItem", {0, 255, 0}, {60, 60, 200}, true});
    file.styles.push_back({"level0", {178, 178, 178}, {0, 0, 0}, false});
    GroundLabelRule setRule;
    REQUIRE(ReadStylesAndRules(R"("setItem": {"color": "#00ff00"})", R"({"when": ["setItem"], "style": "setItem"})")
                .file.standard.size() == 1);
    setRule = ReadStylesAndRules(R"("setItem": {"color": "#00ff00"})", R"({"when": ["setItem"], "style": "setItem"})")
                  .file.standard[0];
    file.standard.push_back(setRule);
    file.standard.push_back({{}, "level0"});
    return file;
}

ItemDefinition MakeItem(std::vector<ItemGroundLabel> groundLabels)
{
    ItemDefinition item;
    item.group = 14;
    item.number = 32;
    item.names.Set(Data::LocalizedString::NeutralLocale, "Pink Chocolate Box");
    item.width = 1;
    item.height = 1;
    item.groundLabels = std::move(groundLabels);
    return item;
}
} // namespace

TEST_CASE("The ground label file has the styles and the standard rules [data][items]")
{
    const LabelReadResult read = ReadStylesAndRules(
        R"("ticket": {"color": "#e58721"}, "setItem": {"color": "#00FF00", "background": "#3c3cc8", "bold": true})",
        R"({"when": ["setItem"], "style": "setItem"}, {"when": ["level>=7", "!upperTierWings"], "style": "ticket"}, {"style": "ticket"})");
    CHECK(read.issues.empty());
    REQUIRE(read.file.styles.size() == 2);
    const GroundLabelStyle* ticket = read.file.FindStyle("ticket");
    REQUIRE(ticket != nullptr);
    CHECK(ticket->color == GroundLabelColor{0xe5, 0x87, 0x21});
    // Left out: a black background and normal letters.
    CHECK(ticket->background == GroundLabelColor{0, 0, 0});
    CHECK_FALSE(ticket->bold);
    const GroundLabelStyle* setItem = read.file.FindStyle("setItem");
    REQUIRE(setItem != nullptr);
    CHECK(setItem->color == GroundLabelColor{0, 255, 0});
    CHECK(setItem->background == GroundLabelColor{60, 60, 200});
    CHECK(setItem->bold);
    CHECK(read.file.FindStyle("jewel") == nullptr);

    REQUIRE(read.file.standard.size() == 3);
    CHECK(read.file.standard[0].style == "setItem");
    REQUIRE(read.file.standard[1].when.size() == 2);
    CHECK(WriteTooltipCondition(read.file.standard[1].when[0]) == "level>=7");
    CHECK(WriteTooltipCondition(read.file.standard[1].when[1]) == "!upperTierWings");
    CHECK(read.file.standard[2].when.empty());
}

TEST_CASE("The ground label file is written as it is read, one style and one rule a line [data][items]")
{
    const std::string text = R"({
  "formatVersion": 1,
  "styles": {
    "ticket": {"color": "#e58721"},
    "setItem": {"color": "#00ff00", "background": "#3c3cc8", "bold": true}
  },
  "standard": [
    {"when": ["setItem"], "style": "setItem"},
    {"when": ["excellentFlags", "!upperTierWings"], "style": "ticket"},
    {"style": "ticket"}
  ]
}
)";
    const LabelReadResult read = ReadLabels(text);
    CHECK(read.issues.empty());
    CHECK(WriteGroundLabelsJson(read.file) == text);
    CHECK(ReadLabels(WriteGroundLabelsJson(read.file)).file == read.file);
}

TEST_CASE("Ground label colors are #rrggbb [data][items]")
{
    GroundLabelColor color{};
    CHECK(ReadGroundLabelColor("#0a1B2c", color));
    CHECK(color == GroundLabelColor{0x0a, 0x1b, 0x2c});
    CHECK(WriteGroundLabelColor(color) == "#0a1b2c");
    for (const char* wrong : {"0a1b2c", "#0a1b2", "#0a1b2c0", "#0a1b2g", "red", ""})
    {
        CAPTURE(wrong);
        CHECK_FALSE(ReadGroundLabelColor(wrong, color));
    }
}

TEST_CASE("The ground label file is checked [data][items]")
{
    SUBCASE("a style needs a color of the right form")
    {
        const LabelReadResult read = ReadStylesAndRules(
            R"("a": {"bold": true}, "b": {"color": "orange"}, "c": {"color": "#ffffff", "background": 3}, "d": {"color": "#ffffff", "bold": "yes"})",
            R"({"style": "a"})");
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "styles.a"));
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "styles.b.color"));
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "styles.c.background"));
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "styles.d.bold"));
        CHECK(read.file.styles.empty());
    }
    SUBCASE("a style name has letters and digits only, and is in the list once")
    {
        const LabelReadResult read = ReadLabels(
            R"({"formatVersion": 1, "styles": {"gift box": {"color": "#ffffff"}, "ticket": {"color": "#ffffff"}, "ticket": {"color": "#000000"}}, "standard": []})");
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "styles.gift box"));
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "styles.ticket"));
    }
    SUBCASE("a rule names a style and has known conditions")
    {
        const LabelReadResult read = ReadStylesAndRules(R"("ticket": {"color": "#ffffff"})",
                                                        R"({"when": ["excelent"], "style": "ticket"}, {"when": []}, 3)");
        const ItemDataIssue* condition = FindIssue(read.issues, "standard[0].when[0]");
        REQUIRE(condition != nullptr);
        CHECK(condition->severity == ItemDataIssueSeverity::Fatal);
        CHECK(condition->message.find("excellent") != std::string::npos);
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "standard[1]"));
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "standard[2]"));
    }
    SUBCASE("rules after one without conditions are never used")
    {
        const LabelReadResult read = ReadStylesAndRules(R"("ticket": {"color": "#ffffff"})",
                                                        R"({"style": "ticket"}, {"when": ["level=0"], "style": "ticket"})");
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Warning, "standard[0]"));
        CHECK(read.file.standard.size() == 2);
    }
    SUBCASE("styles and standard are needed, other fields are warnings")
    {
        const LabelReadResult read =
            ReadLabels(R"({"formatVersion": 1, "colors": {}, "styles": {"t": {"color": "#ffffff", "size": 2}}})");
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "standard"));
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Warning, "colors"));
        CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Warning, "styles.t.size"));
    }
}

TEST_CASE("An item's ground label rules are kept through write and read [data][items]")
{
    const ItemDefinition item = MakeItem({{"giftBoxPink", true, 0, 1}, {"white"}});
    const std::string text = WriteItemGroupJson(14, std::vector<ItemDefinition>{item});
    // On one line, as the tags are.
    CHECK(text.find(R"("groundLabels": [{"style": "giftBoxPink", "levels": [0, 1]}, {"style": "white"}])") !=
          std::string::npos);
    std::vector<ItemDefinition> read;
    std::vector<ItemDataIssue> issues;
    ReadItemGroupJson(text, "Group14_Potion.json", read, issues);
    CHECK(issues.empty());
    REQUIRE(read.size() == 1);
    CHECK(read[0].groundLabels == item.groundLabels);
    // An item without rules writes none.
    CHECK(WriteItemGroupJson(14, std::vector<ItemDefinition>{MakeItem({})}).find("groundLabels") == std::string::npos);
}

TEST_CASE("An item's ground label rule holds at its levels, without levels at every level [data][items]")
{
    const ItemGroundLabel some{"giftBoxPink", true, 0, 1};
    CHECK(some.HoldsAt(0));
    CHECK(some.HoldsAt(1));
    CHECK_FALSE(some.HoldsAt(2));
    const ItemGroundLabel every{"zen"};
    CHECK(every.HoldsAt(0));
    CHECK(every.HoldsAt(15));
    // Zen's amount is its level.
    CHECK(every.HoldsAt(2000000000));
}

TEST_CASE("An item's ground label rules are checked when read [data][items]")
{
    const ItemReadResult read =
        ReadItem(R"("groundLabels": [{"style": "gift box"}, {"style": "white", "levels": [3, 1]}, {"levels": [0, 0]}, {"style": "white", "size": 1}])");
    CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "groundLabels[0]"));
    CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "groundLabels[1].levels"));
    CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Fatal, "groundLabels[2]"));
    CHECK(HasIssue(read.issues, ItemDataIssueSeverity::Warning, "groundLabels[3].size"));
    CHECK(HasIssue(ReadItem(R"("groundLabels": {"style": "white"})").issues, ItemDataIssueSeverity::Fatal,
                   "groundLabels"));
}

TEST_CASE("The styles the items and the standard rules name are in the ground label file [data][items]")
{
    GroundLabelFile labels = MakeLabels();
    SUBCASE("every style named and used: no issue")
    {
        std::vector<ItemDataIssue> issues;
        ValidateGroundLabels(std::vector<ItemDefinition>{MakeItem({{"giftBoxPink", true, 0, 1}, {"white"}})}, labels,
                             Source, issues);
        CHECK(issues.empty());
    }
    SUBCASE("a style the file doesn't have, with the name meant for a typo")
    {
        std::vector<ItemDataIssue> issues;
        ValidateGroundLabels(std::vector<ItemDefinition>{MakeItem({{"giftBoxPnk"}})}, labels, Source, issues);
        const ItemDataIssue* issue = FindIssue(issues, "groundLabels[0]");
        REQUIRE(issue != nullptr);
        CHECK(issue->severity == ItemDataIssueSeverity::Fatal);
        CHECK(issue->group == 14);
        CHECK(issue->number == 32);
        CHECK(issue->message.find("did you mean \"giftBoxPink\"") != std::string::npos);
        // The unused styles are warnings.
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "styles.white"));
    }
    SUBCASE("a standard rule naming a missing style")
    {
        labels.standard.push_back({{}, "level1"});
        std::vector<ItemDataIssue> issues;
        ValidateGroundLabels(std::vector<ItemDefinition>{MakeItem({{"giftBoxPink", true, 0, 1}, {"white"}})}, labels,
                             Source, issues);
        const ItemDataIssue* issue = FindIssue(issues, "standard[2].style");
        REQUIRE(issue != nullptr);
        CHECK(issue->severity == ItemDataIssueSeverity::Fatal);
        CHECK(issue->source == Source);
    }
    SUBCASE("an item's rule after one for every level is never used")
    {
        std::vector<ItemDataIssue> issues;
        ValidateGroundLabels(std::vector<ItemDefinition>{MakeItem({{"white"}, {"giftBoxPink", true, 0, 1}})}, labels,
                             Source, issues);
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "groundLabels[1]"));
    }
}
