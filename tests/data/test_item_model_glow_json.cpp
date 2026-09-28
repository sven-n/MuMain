#include "doctest.h"

#include "Data/GameData/EffectData/GlowColors.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
const std::string Source = "Group00_Sword.json";

struct ReadResult
{
    std::vector<ItemModelDefinition> models;
    std::vector<ItemDataIssue> issues;
};

ReadResult ReadModel(const std::string& fields)
{
    ReadResult result;
    const std::string text =
        R"({"formatVersion": 1, "group": 0, "models": [{"number": 0, "file": "Data/Item/Sword01.bmd", )" + fields +
        "}]}";
    ReadItemModelGroupJson(text, Source, result.models, result.issues);
    return result;
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}

bool HasError(const std::string& fields, const std::string& field)
{
    return HasIssue(ReadModel(fields).issues, ItemDataIssueSeverity::Error, field);
}

bool Contains(const std::string& text, const std::string& part)
{
    return text.find(part) != std::string::npos;
}

ItemModelDefinition MakeGlowingModel()
{
    ItemModelDefinition model;
    model.group = 0;
    model.number = 0;
    model.file = "Data/Item/Sword01.bmd";
    ItemGlow& glow = model.glow;
    glow.levels.emplace();
    glow.levels->fill(8);
    glow.color = "ice";
    glow.meshes.only = {0, 2};
    glow.shineColor = "orange";
    glow.shineWhite = true;
    glow.shineMeshes.hidden = 1;
    glow.ancientColor = "gold";
    glow.excellent = false;
    glow.excellentMesh = 1;
    glow.excellentMeshWithoutSkin = 2;
    return model;
}
} // namespace

TEST_CASE("Item model glow values are kept through write and read [data][items]")
{
    const std::vector<ItemModelDefinition> models{MakeGlowingModel()};

    ReadResult result;
    ReadItemModelGroupJson(WriteItemModelGroupJson(0, models), Source, result.models, result.issues);

    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0] == models[0]);
}

TEST_CASE("An item model can glow like another level for each item level [data][items]")
{
    ItemModelDefinition arrows = MakeGlowingModel();
    arrows.glow.levels = {{0, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31}};
    const std::vector<ItemModelDefinition> models{arrows};

    const std::string text = WriteItemModelGroupJson(0, models);
    CHECK(Contains(text, R"("level": [0, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31])"));

    ReadResult result;
    ReadItemModelGroupJson(text, Source, result.models, result.issues);
    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].glow.levels == arrows.glow.levels);
}

TEST_CASE("Item model glow values are written compactly [data][items]")
{
    const std::vector<ItemModelDefinition> models{MakeGlowingModel()};

    const std::string text = WriteItemModelGroupJson(0, models);

    CHECK(Contains(text, R"("level": 8)"));
    CHECK(Contains(text, R"("color": "ice")"));
    CHECK(Contains(text, R"("meshes": [0, 2])"));
    CHECK(Contains(text, R"("shineColor": "orange")"));
    CHECK(Contains(text, R"("shineHiddenMesh": 1)"));
    CHECK(Contains(text, R"("ancientColor": "gold")"));
    CHECK(Contains(text, R"("excellent": false)"));
}

TEST_CASE("Item model glow defaults are left out [data][items]")
{
    ItemModelDefinition plain;
    plain.file = "Data/Item/Sword02.bmd";
    const std::vector<ItemModelDefinition> models{plain};

    CHECK_FALSE(Contains(WriteItemModelGroupJson(0, models), "glow"));

    const ReadResult result = ReadModel(R"("glow": {})");
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].glow == ItemGlow{});
    CHECK(result.models[0].glow.excellent);
    CHECK(result.models[0].glow.meshes.only.empty());
    CHECK_FALSE(result.models[0].glow.meshes.hidden.has_value());
}

TEST_CASE("Item model glow values are checked [data][items]")
{
    CHECK(HasError(R"("glow": [])", "glow"));
    CHECK(HasError(R"("glow": {"level": 32})", "glow.level"));
    CHECK(HasError(R"("glow": {"level": -1})", "glow.level"));
    CHECK(HasError(R"("glow": {"level": 2.5})", "glow.level"));
    CHECK(HasError(R"("glow": {"level": [0, 1, 2]})", "glow.level"));
    CHECK(HasError(R"("glow": {"color": [1, 0.5, 0]})", "glow.color"));
    CHECK(HasError(R"("glow": {"color": ""})", "glow.color"));
    CHECK(HasError(R"("glow": {"shineColor": "light blue"})", "glow.shineColor"));
    CHECK(HasError(R"("glow": {"ancientColor": 3})", "glow.ancientColor"));
    CHECK(HasError(R"("glow": {"meshes": []})", "glow.meshes"));
    CHECK(HasError(R"("glow": {"meshes": [0, -1]})", "glow.meshes"));
    CHECK(HasError(R"("glow": {"meshes": [1], "hiddenMesh": 0})", "glow.meshes"));
    CHECK(HasError(R"("glow": {"shineHiddenMesh": "all"})", "glow.shineHiddenMesh"));
    CHECK(HasError(R"("glow": {"shineWhite": 1})", "glow.shineWhite"));
    CHECK(HasError(R"("glow": {"excellent": "no"})", "glow.excellent"));
    CHECK(HasError(R"("glow": {"excellentMesh": -2})", "glow.excellentMesh"));
    CHECK(ReadModel(R"("glow": {"level": 0, "hiddenMesh": 1, "shineMeshes": [0], "excellent": true})").issues.empty());
}

TEST_CASE("Unknown item model glow fields are warnings [data][items]")
{
    const ReadResult result = ReadModel(R"("glow": {"sparkle": 3})");

    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "glow.sparkle"));
    CHECK(result.models.size() == 1);
}

TEST_CASE("The glow color list names colors [data][effects]")
{
    std::vector<Data::Effects::GlowColor> colors;
    std::vector<ItemDataIssue> issues;
    Data::Effects::ReadGlowColorsJson(
        R"({"formatVersion": 1, "colors": {"orange": [1, 0.5, 0], "gold": [1, 0.7, 0.2]}})", "GlowColors.json", colors,
        issues);

    CHECK(issues.empty());
    const std::vector<Data::Effects::GlowColor> expected{{"orange", {1, 0.5, 0}}, {"gold", {1, 0.7, 0.2}}};
    CHECK(colors == expected);
}

TEST_CASE("The glow color list is checked [data][effects]")
{
    const auto errorsOf = [](const std::string& text)
    {
        std::vector<Data::Effects::GlowColor> colors;
        std::vector<ItemDataIssue> issues;
        Data::Effects::ReadGlowColorsJson(text, "GlowColors.json", colors, issues);
        return issues;
    };

    CHECK(HasIssue(errorsOf(R"({"colors": {}})"), ItemDataIssueSeverity::Error, "formatVersion"));
    CHECK(HasIssue(errorsOf(R"({"formatVersion": 1})"), ItemDataIssueSeverity::Error, "colors"));
    CHECK(HasIssue(errorsOf(R"({"formatVersion": 1, "colors": {"light blue": [0, 0, 1]}})"),
                   ItemDataIssueSeverity::Error, "colors.light blue"));
    CHECK(HasIssue(errorsOf(R"({"formatVersion": 1, "colors": {"blue": [0, 0, 2]}})"), ItemDataIssueSeverity::Error,
                   "colors.blue"));
    CHECK(HasIssue(errorsOf(R"({"formatVersion": 1, "colors": {"blue": [0, 1]}})"), ItemDataIssueSeverity::Error,
                   "colors.blue"));
    CHECK(HasIssue(errorsOf(R"({"formatVersion": 1, "colors": {}, "shades": 3})"), ItemDataIssueSeverity::Warning,
                   "shades"));
    // The JSON reader would keep only the last of two equal names.
    CHECK(HasIssue(errorsOf(R"({"formatVersion": 1, "colors": {"gold": [1, 0.7, 0.2], "gold": [1, 1, 0]}})"),
                   ItemDataIssueSeverity::Error, "colors.gold"));
    // Repeated keys elsewhere are no color names.
    const std::vector<ItemDataIssue> notes =
        errorsOf(R"({"formatVersion": 1, "colors": {}, "notes": {"a": 1, "a": 2}})");
    CHECK(HasIssue(notes, ItemDataIssueSeverity::Warning, "notes"));
    CHECK_FALSE(HasErrors(notes));
}

TEST_CASE("Item model glow colors must be in the glow color list [data][items]")
{
    const std::vector<Data::Effects::GlowColor> colors{
        {"orange", {1, 0.5, 0}}, {"white", {1, 1, 1}}, {"azure", {0.1, 0.6, 1}}, {"gold", {1, 0.7, 0.2}}};
    ItemModelDefinition model;
    model.file = "Data/Item/Sword01.bmd";
    model.glow.ancientColor = "gold";

    std::vector<ItemDataIssue> issues;
    ValidateItemModelGlowColors(std::vector<ItemModelDefinition>{model}, colors, issues);
    CHECK(issues.empty());

    model.glow.color = "teal";
    ValidateItemModelGlowColors(std::vector<ItemModelDefinition>{model}, colors, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "glow.color"));

    // The default colors must be in the list.
    issues.clear();
    ValidateItemModelGlowColors({}, std::vector<Data::Effects::GlowColor>{{"orange", {1, 0.5, 0}}}, issues);
    CHECK(issues.size() == 2);
}
