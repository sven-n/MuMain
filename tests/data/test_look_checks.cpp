#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookChecks.h"
#include "Data/GameData/LookData/LookJson.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterModelsJson.h"

#ifndef _WIN32
#include "Core/Platform/PathResolve.h"
#endif

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
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

TEST_CASE("The values an item sets for its look are checked against the look: one it lacks is a warning, one that "
          "can't be the field's is fatal, both about the item (design L12) [looks][checks]")
{
    const LookDefinition look{"seedSphere", {Layer{.eachItem = {{"light", "color"}}}}, {}};
    ItemModelDefinition fire = Model(12, 60, "seedSphere");
    fire.renderStyleValues = {{"color", "[0.9, 0.1, 0.2]"}, {"shine", "1"}};
    ItemModelDefinition water = Model(12, 61, "seedSphere");
    water.renderStyleValues = {{"color", R"("blue")"}};
    const std::vector<ItemDataIssue> issues = Check({look}, {fire, water});
    INFO(Describe(issues));
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "renderStyleValues.shine") == 1);
    const auto fatal = std::ranges::find_if(issues, [](const ItemDataIssue& issue)
                                            { return issue.severity == ItemDataIssueSeverity::Fatal; });
    REQUIRE(fatal != issues.end());
    CHECK(fatal->group == 12);
    CHECK(fatal->number == 61);
    CHECK(fatal->field.starts_with("looks[0].layers[0].light"));
}

TEST_CASE("A texture an item's values name is checked, about the item (design L12) [looks][checks]")
{
    const LookDefinition look{
        "inventoryTexture",
        {Layer{.meshes = {{0}}, .texture = "Player/Own.tga", .eachItem = {{"texture", "inventoryTexture"}}}},
        {}};
    ItemModelDefinition found = Model(8, 39, "inventoryTexture");
    found.renderStyleValues = {{"inventoryTexture", R"("Player/Found.tga")"}};
    ItemModelDefinition missing = Model(8, 40, "inventoryTexture");
    missing.renderStyleValues = {{"inventoryTexture", R"("Player/Missing.tga")"}};
    const std::vector<ItemDataIssue> issues = Check({look}, {found, missing}, {"Player/Own.tga", "Player/Found.tga"});
    INFO(Describe(issues));
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[0].group == 8);
    CHECK(issues[0].number == 40);
    CHECK(issues[0].field == "looks[0].layers[0].texture");
}

TEST_CASE("The meshes an item's values name are checked on its model, and a mesh of the look the item's values "
          "replace is not (design L12) [looks][checks]")
{
    const LookDefinition look{"inventoryMesh", {Layer{.meshes = {{3}}, .eachItem = {{"meshes", "parts"}}}}, {}};
    ItemModelDefinition own = Model(7, 1, "inventoryMesh");
    own.renderStyleValues = {{"parts", "[5]"}};
    ItemModelDefinition fits = Model(7, 2, "inventoryMesh");
    fits.renderStyleValues = {{"parts", "[1]"}};
    const ItemModelDefinition plain = Model(7, 3, "inventoryMesh");
    const std::vector<ItemDataIssue> issues = Check({look}, {own, fits, plain});
    INFO(Describe(issues));
    REQUIRE(issues.size() == 2);
    CHECK(issues[0].number == 1);
    CHECK(issues[0].message.find("mesh 5 is not in this item's model") != std::string::npos);
    CHECK(issues[0].message.find("(a value of this item)") != std::string::npos);
    CHECK(issues[1].number == 3);
    CHECK(issues[1].message.find("mesh 3 is not in this item's model") != std::string::npos);
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

TEST_CASE("The shipped render styles name textures that exist, and items or monsters use each of them "
          "[looks][checks]")
{
    // The textures as the game finds them in the client folder and the shipped item models; the meshes are checked
    // in the editor, which has the models loaded.
    const std::filesystem::path data = MU_TEST_DATA_DIR;
    RenderStylesLoadResult shipped = LoadRenderStyleFile(data / "Effects");
    REQUIRE_FALSE(Data::Items::HasFatal(shipped.issues));
    const Data::Items::ItemModelDataLoadResult models =
        Data::Items::LoadItemModelDataDirectory(data / "Items" / "Models");
    REQUIRE_FALSE(Data::Items::HasFatal(models.issues));
    // The looks the shipped monsters and monster models name (MN8).
    const Data::Monsters::MonsterDataLoadResult monsters =
        Data::Monsters::LoadMonstersFile(data / "Monsters" / "Monsters.json");
    REQUIRE_FALSE(Data::Items::HasFatal(monsters.issues));
    std::vector<Data::Monsters::MonsterModelDefinition> monsterModels;
    std::vector<ItemDataIssue> modelIssues;
    std::ifstream modelsFile(data / "Monsters" / "Models.json", std::ios::binary);
    const std::string modelsText((std::istreambuf_iterator<char>(modelsFile)), std::istreambuf_iterator<char>());
    Data::Monsters::ReadMonsterModelsJson(modelsText, "Models.json", monsterModels, modelIssues);
    REQUIRE_FALSE(Data::Items::HasFatal(modelIssues));
    std::vector<const Data::Monsters::MonsterDefinition*> monsterEntries;
    for (const Data::Monsters::MonsterDefinition& monster : monsters.monsters)
        monsterEntries.push_back(&monster);
    std::vector<const Data::Monsters::MonsterModelDefinition*> modelEntries;
    for (const Data::Monsters::MonsterModelDefinition& model : monsterModels)
        modelEntries.push_back(&model);
    std::vector<ItemDataIssue> issues;
    const RenderStyleCheckInput input{.models = models.models,
                                      .partModels = models.partModels,
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
                                      .meshCount = [](int) { return 0; },
                                      .monsterUses =
                                          [&](const std::string& name, LookKind kind)
                                      {
                                          return Data::Monsters::CountMonsterLookUses(monsterEntries, modelEntries,
                                                                                         kind == LookKind::Effect)
                                              .contains(name);
                                      }};
    CheckRenderStyles(shipped.looks, Source, input, issues);
    INFO(Describe(issues));
    CHECK(issues.empty());
}

TEST_CASE("Effects are checked against the items that name them: meshes, values, users (design L23) "
          "[looks][checks]")
{
    LookDefinition effect{"pulse"};
    effect.valuesBefore.push_back(
        {.onlyWhen = {.hiddenMesh = 4}, .values = {.glowMesh = 3, .hiddenMesh = 1}, .glowsLikeLevel = 7});
    const LookDefinition unused{"quiet"};
    ItemModelDefinition model = Model(0, 5, "");
    model.effect = "pulse";
    model.effectValues = {{"color", "1"}};
    const std::vector<ItemModelDefinition> models{model};
    std::vector<ItemDataIssue> issues;
    const RenderStyleCheckInput input{.models = models, .meshCount = [](int) { return 2; }};
    CheckItemEffects(std::vector{effect, unused}, Source, input, issues);
    INFO(Describe(issues));
    // Meshes 3 and 4 are not in the item's model of two meshes; mesh 1 is.
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].valuesBefore[0].glowMesh") == 1);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].valuesBefore[0].onlyWhen.hiddenMesh") == 1);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].valuesBefore[0].hiddenMesh") == 0);
    // A value the effect doesn't let each item set, about the item.
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "effectValues.color") == 1);
    // An effect no item uses; a render style of the item's doesn't count.
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[1].name") == 1);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].name") == 0);
}

TEST_CASE("Things on bones are checked: a type the effect catalogue doesn't have is an error, a bone an item's model "
          "doesn't have a warning about the item (design L55) [looks][checks]")
{
    LookDefinition effect{"wings"};
    OnBones flares;
    flares.places.bones = {3, 12};
    flares.things.push_back({.kind = BoneThingKind::Sprite, .type = "flare"});
    OnBones lightning;
    lightning.together = true;
    lightning.places.between = {{2, 30}};
    lightning.things.push_back({.kind = BoneThingKind::Joint, .type = "jointThunder"});
    lightning.things.push_back({.kind = BoneThingKind::Joint, .type = "noSuchJoint"});
    effect.onBones = {flares, lightning};
    ItemModelDefinition model = Model(12, 6, "");
    model.effect = "wings";
    const std::vector<ItemModelDefinition> models{model};
    std::vector<ItemDataIssue> issues;
    const RenderStyleCheckInput input{
        .models = models,
        .typeExists = [](BoneThingKind kind, const std::string& type)
        { return kind == BoneThingKind::Sprite ? type == "flare" : type == "jointThunder"; },
        .boneCount = [](int) { return 10; }};
    CheckItemEffects(std::vector{effect}, Source, input, issues);
    INFO(Describe(issues));
    REQUIRE(issues.size() == 3);
    CHECK(Count(issues, ItemDataIssueSeverity::Error, "looks[0].onBones[1].things[1].joint") == 1);
    // Bones 12 and 30 are not in the item's model of ten bones; 2 and 3 are.
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].onBones[0].bones") == 1);
    CHECK(Count(issues, ItemDataIssueSeverity::Warning, "looks[0].onBones[1].together.between") == 1);
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity == ItemDataIssueSeverity::Warning)
        {
            CHECK(issue.group == 12);
            CHECK(issue.number == 6);
        }
    }

    // A model that is not loaded skips the bone checks.
    issues.clear();
    const RenderStyleCheckInput unloaded{.models = models, .boneCount = [](int) { return 0; }};
    CheckItemEffects(std::vector{effect}, Source, unloaded, issues);
    CHECK(issues.empty());
}

TEST_CASE("The shipped effects are used by items [looks][checks]")
{
    const std::filesystem::path data = MU_TEST_DATA_DIR;
    RenderStylesLoadResult shipped = LoadLookFile(data / "Effects", LookKind::Effect);
    REQUIRE_FALSE(Data::Items::HasFatal(shipped.issues));
    const Data::Items::ItemModelDataLoadResult models =
        Data::Items::LoadItemModelDataDirectory(data / "Items" / "Models");
    REQUIRE_FALSE(Data::Items::HasFatal(models.issues));
    std::vector<ItemDataIssue> issues;
    const RenderStyleCheckInput input{.models = models.models, .meshCount = [](int) { return 0; }};
    CheckItemEffects(shipped.looks, Source, input, issues);
    INFO(Describe(issues));
    CHECK(issues.empty());
}

TEST_CASE("A look a monster or monster model names is used, one nobody names is a warning (MN8) [looks][checks]")
{
    const LookDefinition look{"monsterGlow", {Layer{}}, {}};
    const LookDefinition looks[] = {look};
    for (const bool named : {true, false})
    {
        CAPTURE(named);
        std::vector<ItemDataIssue> issues;
        const RenderStyleCheckInput input{.meshCount = [](int) { return 0; },
                                          .monsterUses = [&](const std::string& name, LookKind kind)
                                          { return named && name == "monsterGlow" && kind == LookKind::RenderStyle; }};
        CheckRenderStyles(looks, Source, input, issues);
        if (named)
        {
            CHECK(issues.empty());
        }
        else
        {
            REQUIRE(issues.size() == 1);
            CHECK(issues[0].message == "no item or monster uses \"monsterGlow\"");
        }
    }
}
