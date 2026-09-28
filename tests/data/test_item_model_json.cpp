#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
const std::string Source = "Group13_Helper.json";

struct ReadResult
{
    std::vector<ItemModelDefinition> models;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    ReadItemModelGroupJson(text, Source, result.models, result.issues);
    return result;
}

std::string GroupFile(int group, const std::string& models)
{
    return R"({"formatVersion": 1, "group": )" + std::to_string(group) + R"(, "models": [)" + models + "]}";
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue) {
        return issue.severity == severity && issue.field == field;
    });
}

bool HasError(const std::string& model, const std::string& field)
{
    return HasIssue(Read(GroupFile(13, model)).issues, ItemDataIssueSeverity::Error, field);
}

ItemModelDefinition MakeDarkHorse()
{
    ItemModelDefinition model;
    model.group = 13;
    model.number = 4;
    model.file = "Data/Item/DarkHorseHorn.bmd";
    model.textureFolders = {"Item", "Skill"};
    model.noneBlendMeshes = {1, 3};
    return model;
}
} // namespace

TEST_CASE("Item model JSON keeps every value through write and read [data][items]")
{
    const ItemModelDefinition darkHorse = MakeDarkHorse();
    const std::vector<ItemModelDefinition> models{darkHorse};

    const ReadResult result = Read(WriteItemModelGroupJson(13, models));

    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0] == darkHorse);
}

TEST_CASE("Item model JSON writes short lists on one line and leaves out empty ones [data][items]")
{
    ItemModelDefinition ring;
    ring.group = 13;
    ring.number = 8;
    ring.file = "Data/Item/Ring01.bmd";
    const std::vector<ItemModelDefinition> models{MakeDarkHorse(), ring};

    const std::string text = WriteItemModelGroupJson(13, models);

    CHECK(text.find(R"("textureFolders": ["Item", "Skill"])") != std::string::npos);
    CHECK(text.find(R"("noneBlendMeshes": [1, 3])") != std::string::npos);
    // The ring has no texture folders and no none-blend meshes.
    CHECK(text.find(R"("file": "Data/Item/Ring01.bmd")"
                    "\n    }") != std::string::npos);
}

TEST_CASE("Item model JSON is sorted by number and only has the requested group [data][items]")
{
    ItemModelDefinition second = MakeDarkHorse();
    second.number = 7;
    ItemModelDefinition otherGroup = MakeDarkHorse();
    otherGroup.group = 14;
    const std::vector<ItemModelDefinition> models{second, MakeDarkHorse(), otherGroup};

    const ReadResult result = Read(WriteItemModelGroupJson(13, models));

    REQUIRE(result.models.size() == 2);
    CHECK(result.models[0].number == 4);
    CHECK(result.models[1].number == 7);
}

TEST_CASE("Folder names with spaces stay intact on one line [data][items]")
{
    ItemModelDefinition model = MakeDarkHorse();
    model.textureFolders = {"My Items", "Item"};
    const std::vector<ItemModelDefinition> models{model};

    const std::string text = WriteItemModelGroupJson(13, models);

    CHECK(text.find(R"("textureFolders": ["My Items", "Item"])") != std::string::npos);
    const ReadResult result = Read(text);
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].textureFolders == model.textureFolders);
}

TEST_CASE("A model needs a number and a .bmd file [data][items]")
{
    CHECK(HasError(R"({"file": "Data/Item/Ring01.bmd"})", "number"));
    CHECK(HasError(R"({"number": 512, "file": "Data/Item/Ring01.bmd"})", "number"));
    CHECK(HasError(R"({"number": 8})", "file"));
    CHECK(HasError(R"({"number": 8, "file": ""})", "file"));
    CHECK(HasError(R"({"number": 8, "file": "Data/Item/Ring01.ozj"})", "file"));
    CHECK(HasError(R"({"number": 8, "file": "Data\\Item\\Ring01.bmd"})", "file"));
    CHECK(Read(GroupFile(13, R"({"number": 8, "file": "Data/Item/Ring01.BMD"})")).issues.empty());
}

TEST_CASE("Model paths stay inside the game folder [data][items]")
{
    CHECK(HasError(R"({"number": 8, "file": "/Data/Item/Ring01.bmd"})", "file"));
    CHECK(HasError(R"({"number": 8, "file": "C:/Mu/Data/Item/Ring01.bmd"})", "file"));
    CHECK(HasError(R"({"number": 8, "file": "Data/../../Ring01.bmd"})", "file"));
    CHECK(HasError(R"({"number": 8, "file": "Data//Item/Ring01.bmd"})", "file"));
    CHECK(
        HasError(R"({"number": 8, "file": "Data/Item/Ring01.bmd", "textureFolders": ["../Item"]})", "textureFolders"));
    CHECK(HasError(R"({"number": 8, "file": "Data/Item/Ring01.bmd", "textureFolders": ["/Item"]})", "textureFolders"));
    CHECK(Read(GroupFile(13, R"({"number": 8, "file": "Data/Item/..Ring01.bmd", "textureFolders": ["Item/xmas"]})"))
              .issues.empty());
}

TEST_CASE("Texture folders and none-blend meshes are checked [data][items]")
{
    CHECK(HasError(R"({"number": 8, "file": "Data/Item/Ring01.bmd", "textureFolders": "Item"})", "textureFolders"));
    CHECK(HasError(R"({"number": 8, "file": "Data/Item/Ring01.bmd", "textureFolders": [""]})", "textureFolders"));
    CHECK(HasError(R"({"number": 8, "file": "Data/Item/Ring01.bmd", "textureFolders": ["Item/"]})",
                   "textureFolders"));
    CHECK(HasError(R"({"number": 8, "file": "Data/Item/Ring01.bmd", "textureFolders": ["Item\\xmas"]})",
                   "textureFolders"));
    CHECK(HasError(R"({"number": 8, "file": "Data/Item/Ring01.bmd", "noneBlendMeshes": [-1]})", "noneBlendMeshes"));
    CHECK(HasError(R"({"number": 8, "file": "Data/Item/Ring01.bmd", "noneBlendMeshes": ["1"]})", "noneBlendMeshes"));

    const ReadResult result = Read(GroupFile(13, R"({"number": 8, "file": "Data/Item/Ring01.bmd"})"));
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].textureFolders.empty());
    CHECK(result.models[0].noneBlendMeshes.empty());
}

TEST_CASE("A model with errors is not read [data][items]")
{
    const ReadResult result =
        Read(GroupFile(13, R"({"number": 8, "file": "Data/Item/Ring01.bmd", "noneBlendMeshes": [-1]},
                              {"number": 9, "file": "Data/Item/Ring02.bmd"})"));

    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].number == 9);
}

TEST_CASE("Unknown model fields are warnings [data][items]")
{
    const ReadResult result = Read(GroupFile(13, R"({"number": 8, "file": "Data/Item/Ring01.bmd", "scale": 2})"));

    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "scale"));
    CHECK_FALSE(HasErrors(result.issues));
    CHECK(result.models.size() == 1);
}

TEST_CASE("Invalid item model files are reported [data][items]")
{
    CHECK(HasErrors(Read("{").issues));
    CHECK(HasErrors(Read(R"({"formatVersion": 2, "group": 13, "models": []})").issues));
    CHECK(HasErrors(Read(R"({"formatVersion": 1, "group": 16, "models": []})").issues));
    CHECK(HasIssue(Read(R"({"formatVersion": 1, "group": 13})").issues, ItemDataIssueSeverity::Error, "models"));
}

TEST_CASE("Item model validation finds items with two models [data][items]")
{
    const std::vector<ItemModelDefinition> models{MakeDarkHorse(), MakeDarkHorse()};
    std::vector<ItemDataIssue> issues;

    ValidateItemModels(models, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].group == 13);
    CHECK(issues[0].number == 4);
}

TEST_CASE("The item model database finds models by item type [data][items]")
{
    ItemModelDefinition invalid = MakeDarkHorse();
    invalid.number = MAX_ITEM_INDEX;
    const std::vector<ItemModelDefinition> models{MakeDarkHorse(), invalid};
    ItemModelDatabase database;
    Data::Effects::GlowColorList glowColors;
    const std::vector<Data::Effects::GlowColor> colors{
        {"orange", {1, 0.5, 0}}, {"white", {1, 1, 1}}, {"azure", {0.1, 0.6, 1}}};
    glowColors.Build(colors);

    database.Build(models, glowColors);

    CHECK(database.GetModelCount() == 1);
    // The glow colors are looked up in the list.
    REQUIRE(database.FindGlowColors(MakeItemType(13, 4)) != nullptr);
    CHECK(database.FindGlowColors(MakeItemType(13, 4))->color == std::array<float, 3>{1.0f, 0.5f, 0.0f});
    CHECK(database.FindGlowColors(MakeItemType(13, 5)) == nullptr);
    REQUIRE(database.Find(MakeItemType(13, 4)) != nullptr);
    CHECK(database.Find(MakeItemType(13, 4))->file == "Data/Item/DarkHorseHorn.bmd");
    CHECK(database.Find(MakeItemType(13, 5)) == nullptr);
    CHECK(database.Find(-1) == nullptr);
    CHECK(database.Find(MAX_ITEM) == nullptr);

    database.Build({}, glowColors);
    CHECK(database.GetModelCount() == 0);
    CHECK(database.Find(MakeItemType(13, 4)) == nullptr);
}
