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

ItemModelDefinition MakeKris()
{
    ItemModelDefinition kris;
    kris.group = 0;
    kris.number = 0;
    kris.file = "Data/Item/Sword01.bmd";
    kris.inventory.anchor = {0.8, 0.85};
    kris.inventory.offset = {-0.02, 0.03, 0.0};
    kris.inventory.rotation = {180, 270, 15};
    kris.inventory.scale = 0.0039;
    kris.inventory.bodyHeight = -160;
    kris.ground.rotation = {60, 0, -45};
    kris.ground.scale = 0.7;
    kris.ground.bodyHeight = -100;
    kris.cloth = true;
    return kris;
}
} // namespace

TEST_CASE("Item model display values are kept through write and read [data][items]")
{
    ItemModelDefinition kris = MakeKris();
    kris.inventory.offset[2] = 0.02;
    const std::vector<ItemModelDefinition> models{kris};

    ReadResult result;
    ReadItemModelGroupJson(WriteItemModelGroupJson(0, models), Source, result.models, result.issues);

    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0] == kris);
}

TEST_CASE("Item model display values are written compactly [data][items]")
{
    const std::vector<ItemModelDefinition> models{MakeKris()};

    const std::string text = WriteItemModelGroupJson(0, models);

    CHECK(text.find(R"("anchor": [0.8, 0.85])") != std::string::npos);
    // Without depth the offset has two values; whole numbers have no decimals.
    CHECK(text.find(R"("offset": [-0.02, 0.03])") != std::string::npos);
    CHECK(text.find(R"("rotation": [180, 270, 15])") != std::string::npos);
    CHECK(text.find(R"("bodyHeight": -160)") != std::string::npos);
    CHECK(text.find(R"("cloth": true)") != std::string::npos);
}

TEST_CASE("Item model display defaults are left out [data][items]")
{
    ItemModelDefinition plain;
    plain.file = "Data/Item/Sword02.bmd";
    const std::vector<ItemModelDefinition> models{plain};

    const std::string text = WriteItemModelGroupJson(0, models);

    CHECK(text.find("inventory") == std::string::npos);
    CHECK(text.find("ground") == std::string::npos);
    CHECK(text.find("cloth") == std::string::npos);

    const ReadResult result = ReadModel(R"("inventory": {}, "ground": {})");
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].inventory == ItemInventoryDisplay{});
    CHECK(result.models[0].ground == ItemGroundDisplay{});
    CHECK_FALSE(result.models[0].ground.scale.has_value());
    CHECK_FALSE(result.models[0].cloth);
}

TEST_CASE("Item model display values are checked [data][items]")
{
    CHECK(HasError(R"("inventory": [])", "inventory"));
    CHECK(HasError(R"("inventory": {"anchor": [0.5]})", "inventory.anchor"));
    CHECK(HasError(R"("inventory": {"offset": [1, 2, 3, 4]})", "inventory.offset"));
    CHECK(HasError(R"("inventory": {"rotation": [1, 2]})", "inventory.rotation"));
    CHECK(HasError(R"("inventory": {"rotation": [1, "2", 3]})", "inventory.rotation"));
    CHECK(HasError(R"("inventory": {"scale": 0})", "inventory.scale"));
    CHECK(HasError(R"("inventory": {"bodyHeight": "high"})", "inventory.bodyHeight"));
    CHECK(HasError(R"("ground": {"scale": -1})", "ground.scale"));
    CHECK(HasError(R"("cloth": 1)", "cloth"));
    CHECK(ReadModel(R"("inventory": {"offset": [1, 2]}, "ground": {"scale": 0.5}, "cloth": false)").issues.empty());
}

TEST_CASE("Unknown item model display fields are warnings [data][items]")
{
    const ReadResult result = ReadModel(R"("inventory": {"tilt": 3}, "ground": {"spin": true})");

    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "inventory.tilt"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "ground.spin"));
    CHECK(result.models.size() == 1);
}
