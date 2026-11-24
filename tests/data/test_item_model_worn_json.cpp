#include "doctest.h"

#include "Data/GameData/ItemData/ItemModelJsonFormat.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
const std::string Source = "Group12_Wing.json";

struct ReadResult
{
    std::vector<ItemModelDefinition> models;
    std::vector<ItemDataIssue> issues;
};

ReadResult ReadModel(const std::string& fields)
{
    ReadResult result;
    const std::string text =
        R"({"formatVersion": 1, "group": 12, "models": [{"number": 40, "file": "Data/Item/Wing40.bmd")" +
        (fields.empty() ? std::string() : ", " + fields) + "}]}";
    ReadItemModelGroupJson(text, Source, result.models, result.issues);
    return result;
}

bool HasFatal(const std::vector<ItemDataIssue>& issues, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                       { return issue.severity == ItemDataIssueSeverity::Fatal && issue.field == field; });
}
} // namespace

TEST_CASE("A model without worn values hangs like a wing [data][items][worn]")
{
    const ReadResult result = ReadModel("");

    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].worn.bone == 47);
    CHECK_FALSE(result.models[0].worn.link);
    CHECK(result.models[0].worn.offset == std::array<double, 3>{0.0, 0.0, 15.0});
}

TEST_CASE("A cape's worn values are read and kept through write and read [data][items][worn]")
{
    const ReadResult result = ReadModel(R"("worn": {"bone": 19, "link": true, "offset": [0, 0, 0]})");

    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].worn.bone == 19);
    CHECK(result.models[0].worn.link);
    CHECK(result.models[0].worn.offset == std::array<double, 3>{0.0, 0.0, 0.0});

    ReadResult again;
    ReadItemModelGroupJson(WriteItemModelGroupJson(12, result.models), Source, again.models, again.issues);
    CHECK(again.issues.empty());
    REQUIRE(again.models.size() == 1);
    CHECK(again.models[0] == result.models[0]);
}

TEST_CASE("Invalid worn values are refused [data][items][worn]")
{
    CHECK(HasFatal(ReadModel(R"("worn": 19)").issues, "worn"));
    CHECK(HasFatal(ReadModel(R"("worn": {"bone": 256})").issues, "worn.bone"));
    CHECK(HasFatal(ReadModel(R"("worn": {"link": 1})").issues, "worn.link"));
    CHECK(HasFatal(ReadModel(R"("worn": {"offset": [0, 15]})").issues, "worn.offset"));
}
