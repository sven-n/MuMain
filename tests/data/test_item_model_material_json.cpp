#include "doctest.h"

#include "Data/GameData/ItemData/ItemModelJsonFormat.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
const std::string Source = "Group08_Armor.json";

struct ReadResult
{
    std::vector<ItemModelDefinition> models;
    std::vector<ItemDataIssue> issues;
};

ReadResult ReadModel(const std::string& fields)
{
    ReadResult result;
    const std::string text =
        R"({"formatVersion": 1, "group": 8, "models": [{"number": 0, "file": "Data/Player/ArmorMale01.bmd")" +
        (fields.empty() ? std::string() : ", " + fields) + "}]}";
    ReadItemModelGroupJson(text, Source, result.models, result.issues);
    return result;
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}
} // namespace

TEST_CASE("Item model specular is read when present [data][items][material]")
{
    const ReadResult result = ReadModel(R"("material": {"specular": 0.6})");

    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].material.specular == doctest::Approx(0.6));
}

TEST_CASE("Item model without material has no specular [data][items][material]")
{
    const ReadResult result = ReadModel("");

    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].material.specular == 0.0);
    CHECK(result.models[0].material == ItemMaterial{});
}

TEST_CASE("Item model specular rejects invalid values and keeps none [data][items][material]")
{
    for (const char* fields : {R"("material": {"specular": "shiny"})", R"("material": {"specular": -0.1})",
                               R"("material": {"specular": 1.5})", R"("material": {"specular": true})"})
    {
        CAPTURE(fields);
        const ReadResult result = ReadModel(fields);
        CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Fatal, "material.specular"));
        CHECK(result.models.empty());
    }
}

TEST_CASE("Item model material must be an object and warns about unknown keys [data][items][material]")
{
    CHECK(HasIssue(ReadModel(R"("material": 0.6)").issues, ItemDataIssueSeverity::Fatal, "material"));

    const ReadResult result = ReadModel(R"("material": {"specular": 0.4, "roughness": 1})");
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "material.roughness"));
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].material.specular == doctest::Approx(0.4));
}

TEST_CASE("Item model specular is kept through write and read, and left out at its default [data][items][material]")
{
    ItemModelDefinition armor;
    armor.group = 8;
    armor.number = 0;
    armor.file = "Data/Player/ArmorMale01.bmd";
    armor.material.specular = 0.6;
    ItemModelDefinition plain = armor;
    plain.number = 1;
    plain.material = {};
    const std::vector<ItemModelDefinition> models{armor, plain};

    const std::string text = WriteItemModelGroupJson(8, models);
    CHECK(text.find("\"material\"") != std::string::npos);
    CHECK(text.find("\"material\"") == text.rfind("\"material\""));

    ReadResult result;
    ReadItemModelGroupJson(text, Source, result.models, result.issues);
    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 2);
    CHECK(result.models[0] == armor);
    CHECK(result.models[1] == plain);
}
