#include "doctest.h"

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
    glow.level = 8;
    glow.color = {0.5, 0.8, 0.9};
    glow.meshes.only = {0, 2};
    glow.shineColor = {1, 0.5, 0};
    glow.shineWhite = true;
    glow.shineMeshes.hidden = 1;
    glow.ancientColor = {1, 0.7, 0.2};
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

TEST_CASE("Item model glow values are written compactly [data][items]")
{
    const std::vector<ItemModelDefinition> models{MakeGlowingModel()};

    const std::string text = WriteItemModelGroupJson(0, models);

    CHECK(Contains(text, R"("level": 8)"));
    CHECK(Contains(text, R"("color": [0.5, 0.8, 0.9])"));
    CHECK(Contains(text, R"("meshes": [0, 2])"));
    // Whole numbers have no decimals.
    CHECK(Contains(text, R"("shineColor": [1, 0.5, 0])"));
    CHECK(Contains(text, R"("shineHiddenMesh": 1)"));
    CHECK(Contains(text, R"("ancientColor": [1, 0.7, 0.2])"));
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
    CHECK(HasError(R"("glow": {"level": 16})", "glow.level"));
    CHECK(HasError(R"("glow": {"level": -1})", "glow.level"));
    CHECK(HasError(R"("glow": {"level": 2.5})", "glow.level"));
    CHECK(HasError(R"("glow": {"color": [1, 0.5]})", "glow.color"));
    CHECK(HasError(R"("glow": {"color": [1, 0.5, 2]})", "glow.color"));
    CHECK(HasError(R"("glow": {"shineColor": [-0.1, 0, 0]})", "glow.shineColor"));
    CHECK(HasError(R"("glow": {"ancientColor": "gold"})", "glow.ancientColor"));
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
