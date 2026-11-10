#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookChecks.h"
#include "Data/GameData/LookData/LookJson.h"

#ifndef _WIN32
#include "Core/Platform/PathResolve.h"
#endif

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

// The render style checks beyond reading the file (design L23, LookChecks.h).
namespace
{
using namespace Data::Looks;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Items::ItemModelDefinition;

const std::string Source = "Data/Effects/RenderStyles.json";

ItemModelDefinition Model(int group, int number, std::string renderStyle)
{
    ItemModelDefinition model;
    model.group = group;
    model.number = number;
    model.file = "Data/Item/Sword01.bmd";
    model.renderStyle = std::move(renderStyle);
    return model;
}

std::vector<ItemDataIssue> Check(const std::vector<LookDefinition>& looks,
                                 const std::vector<ItemModelDefinition>& models,
                                 const std::set<std::string>& textures = {}, int meshes = 2)
{
    std::vector<ItemDataIssue> issues;
    const RenderStyleCheckInput input{.models = models,
                                      .textureExists = [&](const std::string& texture)
                                      { return textures.contains(texture); },
                                      .meshCount = [meshes](int) { return meshes; }};
    CheckRenderStyles(looks, Source, input, issues);
    return issues;
}

std::string Describe(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

int Count(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return static_cast<int>(std::ranges::count_if(issues, [&](const ItemDataIssue& issue)
                                                  { return issue.severity == severity && issue.field == field; }));
}
} // namespace

TEST_CASE("A texture a layer names that the game can't load is an error [looks][checks]")
{
    const std::vector<LookDefinition> looks{
        {"a",
         {Layer{.texture = "Effect/Chrome01.jpg"}, Layer{.texture = "Effect/Missing.tga"},
          Layer{.texture = "Effect/Picture.png"}, Layer{}},
         {}}};
    const std::vector<ItemDataIssue> issues = Check(looks, {Model(0, 1, "a")}, {"Effect/Chrome01.jpg"});
    INFO(Describe(issues));
    CHECK(Count(issues, ItemDataIssueSeverity::Error, "looks[0].layers[0].texture") == 0);
    CHECK(Count(issues, ItemDataIssueSeverity::Error, "looks[0].layers[1].texture") == 1);
    CHECK(Count(issues, ItemDataIssueSeverity::Error, "looks[0].layers[2].texture") == 1);
    CHECK(issues.size() == 2);
    CHECK_FALSE(Data::Items::HasFatal(issues));
}

TEST_CASE("The game reads a texture from its packed file [looks][checks]")
{
    CHECK(GetTextureFilePath("Effect/Chrome01.jpg") == "Data/Effect/Chrome01.OZJ");
    CHECK(GetTextureFilePath("Item/InGameShop/membership_item_red.JPG") ==
          "Data/Item/InGameShop/membership_item_red.OZJ");
    CHECK(GetTextureFilePath("Effect/flare.tga") == "Data/Effect/flare.OZT");
    CHECK(GetTextureFilePath("Effect/flare.png").empty());
    CHECK(GetTextureFilePath("Effect.d/flare").empty());
}

TEST_CASE("A mesh an item's model lacks is a warning about that item, a look no item uses one of the look "
          "[looks][checks]")
{
    const std::vector<LookDefinition> looks{
        {"a",
         {Layer{.meshes = {{0, 3}}, .glowMesh = 2, .scrollingMesh = 1}, Layer{.glowMesh = AllGlowMeshes{}}},
         {.glowMesh = 4, .hiddenMesh = -1}},
        {"unused", {Layer{}}, {}}};
    const std::vector<ItemModelDefinition> models{
        Model(0, 1, "a"), Model(0, 2, "a"), Model(1, 1, "other"),
        ItemModelDefinition{.group = 2, .number = 1, .renderStyle = "unused"}};
    const std::vector<ItemDataIssue> issues = Check(looks, models);
    INFO(Describe(issues));
    // Each of the two items with a two-mesh model: mesh 3 of the layer, its glow mesh 2 and the glow mesh 4 left
    // after; mesh 1, every mesh and "none" are fine.
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].layers[0].meshes") == 2);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].layers[0].glowMesh") == 2);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].layers[0].scrollingMesh") == 0);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].itemValuesAfter.glowMesh") == 2);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].itemValuesAfter.hiddenMesh") == 0);
    const auto aboutItem = std::ranges::find_if(issues, [](const ItemDataIssue& issue) { return issue.number == 2; });
    REQUIRE(aboutItem != issues.end());
    CHECK(aboutItem->group == 0);
    CHECK(aboutItem->message.find("mesh 3 is not in this item's model, which has 2 meshes") != std::string::npos);
    // A model entry that doesn't exist is no user.
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[1].name") == 1);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].name") == 0);
    CHECK(issues.size() == 7);
}

TEST_CASE("A model that is not loaded skips the mesh checks [looks][checks]")
{
    const std::vector<LookDefinition> looks{{"a", {Layer{.meshes = {{9}}}}, {}}};
    CHECK(Check(looks, {Model(0, 1, "a")}, {}, 0).empty());
}

TEST_CASE("The names of a file's looks in file order name what a problem is about [looks][checks]")
{
    CHECK(ReadLookNames(R"({"looks": [{"name": "b"}, {"layers": []}, 3, {"name": "a"}]})") ==
          std::vector<std::string>{"b", "", "", "a"});
    CHECK(ReadLookNames("{ broken").empty());
    CHECK(ReadLookNames(R"({"looks": {}})").empty());
}

TEST_CASE("The shipped render styles name textures that exist, and items use each of them [looks][checks]")
{
    // The textures as the game finds them in the client folder and the shipped item models; the meshes are checked
    // in the editor, which has the models loaded.
    const std::filesystem::path data = MU_TEST_DATA_DIR;
    RenderStylesLoadResult shipped = LoadRenderStyleFile(data / "Effects");
    REQUIRE_FALSE(Data::Items::HasFatal(shipped.issues));
    const Data::Items::ItemModelDataLoadResult models =
        Data::Items::LoadItemModelDataDirectory(data / "Items" / "Models");
    REQUIRE_FALSE(Data::Items::HasFatal(models.issues));
    std::vector<ItemDataIssue> issues;
    const RenderStyleCheckInput input{.models = models.models,
                                      .textureExists =
                                          [&](const std::string& texture)
                                      {
                                          std::string file =
                                              (data.parent_path() / GetTextureFilePath(texture)).string();
#ifndef _WIN32
                                          file = MuResolvePath(file.c_str());
#endif
                                          return std::filesystem::exists(file);
                                      },
                                      .meshCount = [](int) { return 0; }};
    CheckRenderStyles(shipped.looks, Source, input, issues);
    INFO(Describe(issues));
    CHECK(issues.empty());
}
