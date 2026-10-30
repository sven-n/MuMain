#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"
#include "Data/GameData/MonsterData/MonsterModelsJson.h"

#include <algorithm>
#include <string>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterModelDefinition;

namespace
{
struct ReadResult
{
    std::vector<MonsterModelDefinition> models;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Monsters::ReadMonsterModelsJson(text, "Models.json", result.models, result.issues);
    return result;
}

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field,
                   const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue)
                       {
                           return issue.severity == severity && issue.field == field &&
                                  issue.message.find(message) != std::string::npos;
                       });
}

std::string ModelsFile(const std::string& models)
{
    return R"({ "formatVersion": 1, "models": [ )" + models + " ] }";
}

// One model with the fields that every model has.
std::string Model(const std::string& slot, const std::string& fields = "")
{
    return R"({ "slot": ")" + slot + R"(", "file": "Data/Monster/Monster02.bmd", "textureFolders": ["Monster"])" +
           (fields.empty() ? "" : ", " + fields) + " }";
}

MonsterModelDefinition FullModel()
{
    MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_DARK_KNIGHT";
    model.load.file = "Data/Monster/Monster04.bmd";
    model.load.textureFolders = {"Monster", "NPC/LuckyItem"};
    model.load.speeds = {{MONSTER01_STOP2, 0.2f * 1.2f}, {MONSTER01_WALK, 0.7f}, {12, 0.5f}};
    model.load.holdLastFrame = std::vector<int>{MONSTER01_STOP2, MONSTER01_DIE};
    model.mapTextureFolders = {{{{18}, {53}}, {"NPC"}}};
    model.textureNames = {{0, "xmassanta1.jpg"}};
    model.headBone = 16;
    model.streamMesh = 1;
    model.noneBlendMeshes = {0, 3};
    // Not in frame order: the cues keep the file's order.
    model.cues = {{"step 2", MONSTER01_WALK, {7.0f, 7.5f}},
                  {"step 1", MONSTER01_WALK, {1.0f, 2.0f}},
                  {"swing", MONSTER01_ATTACK1, {3.25f, 4.0f}}};
    model.bitmaps = {{"Monster/bv01_2.jpg", "BITMAP_MONSTER_SKIN", true, true}, {"Monster/iui02.tga", "BITMAP_ROBE+3"}};
    Data::Monsters::MonsterModelPart part;
    part.effect = "totemGolemPart1";
    part.load.file = "Data/Monster/totemhead.bmd";
    part.load.textureFolders = {"Monster"};
    part.load.speeds = {{0, 0.13f}};
    part.load.holdLastFrame = std::vector<int>{};
    part.maps = {{51}};
    Data::Monsters::MonsterModelPart clothes;
    clothes.slot = "MODEL_MERCHANT_FEMALE_HEAD+1";
    clothes.load.file = "Data/NPC/FemaleHead02.bmd";
    clothes.load.textureFolders = {"NPC"};
    model.parts = {part, clothes};
    model.body.boxMin = std::array<float, 3>{-60.0f, -60.5f, 0.0f};
    model.body.boxMax = std::array<float, 3>{50.0f, 50.0f, 150.0f};
    model.body.linkBones = std::vector<int>{43};
    return model;
}
} // namespace

TEST_CASE("Monster models: a written model reads back the same [data][monsters]")
{
    const MonsterModelDefinition model = FullModel();
    const std::string text = Data::Monsters::WriteMonsterModelsJson(std::vector<MonsterModelDefinition>{model});
    const ReadResult read = Read(text);
    INFO(text);
    INFO(Messages(read.issues));
    CHECK(read.issues.empty());
    REQUIRE(read.models.size() == 1);
    CHECK(read.models[0] == model);
}

TEST_CASE("Monster models: the writer sorts by slot and puts short values on one line [data][monsters]")
{
    MonsterModelDefinition hound;
    hound.slot = "MONSTER_MODEL_HOUND";
    hound.load.file = "Data/Monster/Monster02.bmd";
    hound.load.textureFolders = {"Monster"};
    hound.load.speeds = {{MONSTER01_WALK, 0.7f}};
    hound.headBone = 5;
    hound.cues = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}}, {"step 2", MONSTER01_WALK, {7.0f, 8.5f}}};
    MonsterModelDefinition smith;
    smith.slot = "MODEL_SMITH";
    smith.load.file = "Data/NPC/Smith01.bmd";
    smith.load.textureFolders = {"NPC"};

    const std::string text = Data::Monsters::WriteMonsterModelsJson(std::vector<MonsterModelDefinition>{smith, hound});
    CHECK(text == R"({
  "formatVersion": 2,
  "models": [
    {
      "slot": "MONSTER_MODEL_HOUND",
      "file": "Data/Monster/Monster02.bmd",
      "textureFolders": ["Monster"],
      "animations": {"walk": 0.7},
      "headBone": 5,
      "cues": [
        {"name": "step 1", "animation": "walk", "frames": [1, 2]},
        {"name": "step 2", "animation": "walk", "frames": [7, 8.5]}
      ]
    },
    {
      "slot": "MODEL_SMITH",
      "file": "Data/NPC/Smith01.bmd",
      "textureFolders": ["NPC"]
    }
  ]
}
)");
}

TEST_CASE("Monster models: the sound fields of a model are fatal, the sounds are in Sounds.json (Q-MN78) "
          "[data][monsters]")
{
    for (const std::string field : {R"("sounds": [{"file": "Data/Sound/a.wav"}])",
                                    R"("soundSlots": {"idle": ["a", "b"], "attack": [null, null], "die": null})",
                                    R"("soundEvents": [{"on": ["hit"], "sounds": ["a"]}])", R"("soundEventMaps": [69])",
                                    R"("soundEventWorlds": [56])"})
    {
        const ReadResult read = Read(ModelsFile(Model("MONSTER_MODEL_HOUND", field)));
        const std::string key = field.substr(1, field.find('"', 1) - 1);
        INFO(field);
        INFO(Messages(read.issues));
        CHECK(read.models.empty());
        CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, "model MONSTER_MODEL_HOUND " + key,
                            "sounds are in the sound sets of Sounds.json"));
        // Not also an unknown key.
        CHECK(read.issues.size() == 1);
    }
}

TEST_CASE("Monster models: a speed is written as the shortest text that reads back the same [data][monsters]")
{
    MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    // OpenMonsterModel's multiplied speeds are not the floats of short texts.
    model.load.speeds = {{0, 0.25f * 1.2f}, {1, 0.2f * 1.2f}, {2, 0.34f * 0.7f}};
    const std::string text = Data::Monsters::WriteMonsterModelsJson(std::vector<MonsterModelDefinition>{model});
    CHECK(text.find(R"("animations": {"stop1": 0.3, "stop2": 0.24000001, "walk": 0.238})") != std::string::npos);
    const ReadResult read = Read(text);
    REQUIRE(read.models.size() == 1);
    CHECK(read.models[0].load.speeds == model.load.speeds);
}

TEST_CASE("Monster models: wrong values are errors that name the model and the field [data][monsters]")
{
    struct Case
    {
        std::string model;
        std::string field;
        std::string message;
    };
    const std::vector<Case> cases = {
        {R"({ "slot": "MONSTER_MODEL_HOUND", "textureFolders": ["Monster"] })", "model MONSTER_MODEL_HOUND file",
         "missing"},
        {R"({ "slot": "MONSTER_MODEL_HOUND", "file": "Data/Monster/Monster02.txt", "textureFolders": ["Monster"] })",
         "model MONSTER_MODEL_HOUND file", ".bmd"},
        {R"({ "slot": "MONSTER_MODEL_HOUND", "file": "Data\\Monster\\Monster02.bmd", "textureFolders": ["Monster"] })",
         "model MONSTER_MODEL_HOUND file", "/"},
        {R"({ "slot": "MONSTER_MODEL_HOUND", "file": "Data/Monster/Monster02.bmd" })",
         "model MONSTER_MODEL_HOUND textureFolders", "missing"},
        {R"({ "slot": "MONSTER_MODEL_HOUND", "file": "Data/Monster/Monster02.bmd", "textureFolders": [] })",
         "model MONSTER_MODEL_HOUND textureFolders", "one folder"},
        {Model("MONSTER_MODEL_HOUND", R"("animations": {"jump": 0.5})"), "model MONSTER_MODEL_HOUND animations.jump",
         "not an animation"},
        {Model("MONSTER_MODEL_HOUND", R"("animations": {"walk": -1})"), "model MONSTER_MODEL_HOUND animations.walk",
         "0 or more"},
        {Model("MONSTER_MODEL_HOUND", R"("holdLastFrame": ["die", "die"])"), "model MONSTER_MODEL_HOUND holdLastFrame",
         "more than once"},
        {Model("MONSTER_MODEL_HOUND", R"("textureNames": {"0": "a_name_that_is_longer_than_thirty_one.jpg"})"),
         "model MONSTER_MODEL_HOUND textureNames.0", "at most 31"},
        {Model("MONSTER_MODEL_HOUND", R"("bitmaps": [{"file": "Monster/a.jpg", "slot": "X", "wrap": "mirror"}])"),
         "model MONSTER_MODEL_HOUND bitmaps[0].wrap", "repeat or clamp"},
        {Model(
             "MONSTER_MODEL_HOUND",
             R"("parts": [{"slot": "MODEL_BC_BOX", "effect": "x", "file": "Data/NPC/box.bmd", "textureFolders": ["NPC"]}])"),
         "model MONSTER_MODEL_HOUND parts[0]", "not both"},
        {Model("MONSTER_MODEL_HOUND", R"("linkBones": [1, 2, 3])"), "model MONSTER_MODEL_HOUND linkBones",
         "at most two"},
        {Model("MONSTER_MODEL_HOUND", R"("boundingBox": {"min": [0, 0]})"), "model MONSTER_MODEL_HOUND boundingBox.min",
         "three numbers"},
        {Model("MONSTER_MODEL_HOUND", R"("headBone": 1.5)"), "model MONSTER_MODEL_HOUND headBone", "whole number"},
        {Model("MONSTER_MODEL_HOUND", R"("cues": {"step 1": [1, 2]})"), "model MONSTER_MODEL_HOUND cues",
         "list of cues"},
        {Model("MONSTER_MODEL_HOUND", R"("cues": [{"name": "step 1", "frames": [1, 2]}])"),
         "model MONSTER_MODEL_HOUND cues[0]", "name, animation and frames"},
        {Model("MONSTER_MODEL_HOUND", R"("cues": [{"name": "", "animation": "walk", "frames": [1, 2]}])"),
         "model MONSTER_MODEL_HOUND cues[0].name", "must be a text"},
        {Model("MONSTER_MODEL_HOUND", R"("cues": [{"name": "step 1", "animation": "jump", "frames": [1, 2]}])"),
         "model MONSTER_MODEL_HOUND cues[0].animation", "not an animation"},
        {Model("MONSTER_MODEL_HOUND", R"("cues": [{"name": "step 1", "animation": "walk", "frames": [8, 7]}])"),
         "model MONSTER_MODEL_HOUND cues[0].frames", "first below the second"},
        {Model(
             "MONSTER_MODEL_HOUND",
             R"("cues": [{"name": "step", "animation": "walk", "frames": [1, 2]}, {"name": "step", "animation": "walk", "frames": [7, 8]}])"),
         "model MONSTER_MODEL_HOUND cues[1].name", "the name of another cue"},
    };
    for (const Case& test : cases)
    {
        const ReadResult read = Read(ModelsFile(test.model));
        INFO(test.model);
        INFO(Messages(read.issues));
        CHECK(read.models.empty());
        CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, test.field, test.message));
    }
}

TEST_CASE("Monster models: a slot must be a model slot of the monster area and be there once [data][monsters]")
{
    ReadResult read = Read(ModelsFile(Model("MODEL_PLAYER")));
    CHECK(read.models.empty());
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, "models[0].slot", "monster model type"));

    read = Read(ModelsFile(Model("MONSTER_MODEL_HOUND") + ", " + Model("MONSTER_MODEL_HOUND")));
    CHECK(read.models.size() == 1);
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, "model MONSTER_MODEL_HOUND slot", "more than once"));
}

TEST_CASE("Monster models: both weapons may hang on one bone [data][monsters]")
{
    const ReadResult read = Read(ModelsFile(Model("MONSTER_MODEL_ORC_ARCHER", R"("linkBones": [39, 39])")));
    INFO(Messages(read.issues));
    CHECK(read.issues.empty());
    REQUIRE(read.models.size() == 1);
    CHECK(read.models[0].body.linkBones == std::vector<int>{39, 39});
}

TEST_CASE("Monster models: unknown fields are warnings and are ignored [data][monsters]")
{
    const ReadResult read = Read(ModelsFile(Model(
        "MONSTER_MODEL_HOUND",
        R"("scale": 1, "cues": [{"name": "step 1", "animation": "walk", "frames": [1, 2], "volume": 1}], "parts": [{"effect": "x", "file": "Data/Monster/p.bmd", "textureFolders": ["Monster"], "speed": 1}])")));
    INFO(Messages(read.issues));
    CHECK_FALSE(HasFatal(read.issues));
    REQUIRE(read.models.size() == 1);
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "model MONSTER_MODEL_HOUND scale", "unknown"));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "model MONSTER_MODEL_HOUND cues[0].volume",
                        "unknown"));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "model MONSTER_MODEL_HOUND parts[0].speed",
                        "unknown"));
}

TEST_CASE("Monster models: the map lists' old names are fatal with their new names (D67) [data][monsters]")
{
    struct Case
    {
        std::string fields;
        std::string field;
        std::string newName;
    };
    const std::vector<Case> cases = {
        {R"("worldTextureFolders": [{"worlds": [18], "folders": ["NPC"]}])",
         "model MONSTER_MODEL_HOUND worldTextureFolders", "mapTextureFolders"},
        {R"("parts": [{"effect": "x", "file": "Data/Monster/p.bmd", "textureFolders": ["Monster"], "worlds": [24]}])",
         "model MONSTER_MODEL_HOUND parts[0].worlds", "maps"},
    };
    for (const Case& test : cases)
    {
        const ReadResult read = Read(ModelsFile(Model("MONSTER_MODEL_HOUND", test.fields)));
        INFO(test.fields);
        INFO(Messages(read.issues));
        CHECK(read.models.empty());
        CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, test.field, "renamed to " + test.newName));
        // Not also an unknown key.
        CHECK(read.issues.size() == 1);
    }
}

TEST_CASE("Monster models: the slots of the monster and NPC ranges have names [data][monsters]")
{
    CHECK(Data::Monsters::FindMonsterModelSlot("MONSTER_MODEL_HOUND") == MODEL_HOUND);
    CHECK(Data::Monsters::FindMonsterModelSlot("MODEL_SMITH") == MODEL_SMITH);
    CHECK(Data::Monsters::FindMonsterModelSlot("MODEL_MERCHANT_FEMALE_HEAD+1") == MODEL_MERCHANT_FEMALE_HEAD + 1);
    CHECK(Data::Monsters::FindMonsterModelSlot("MODEL_LITTLESANTA+7") == MODEL_LITTLESANTA + 7);
    CHECK_FALSE(Data::Monsters::FindMonsterModelSlot("MODEL_PLAYER"));
    CHECK_FALSE(Data::Monsters::FindMonsterModelSlot("MODEL_HOUND"));

    for (int slot = MODEL_MONSTER01; slot < MODEL_MONSTER01 + MONSTER_MODEL_COUNT; ++slot)
    {
        CHECK(Data::Monsters::FindMonsterModelSlot(Data::Monsters::GetMonsterModelSymbol(slot)) == slot);
    }
    for (int slot = MODEL_NPC_BEGIN + 1; slot < MODEL_NPC_END; ++slot)
    {
        CHECK(Data::Monsters::FindMonsterModelSlot(Data::Monsters::GetMonsterModelSymbol(slot)) == slot);
    }
    CHECK(Data::Monsters::GetMonsterModelSymbol(MODEL_PLAYER).empty());
    CHECK(Data::Monsters::GetMonsterModelTypeName(MONSTER_MODEL_COUNT) == "UNKNOWN");
}

TEST_CASE("Monster models: animations have names, later ones go by their number [data][monsters]")
{
    for (int animation = 0; animation < 20; ++animation)
    {
        CHECK(Data::Monsters::FindMonsterAnimation(Data::Monsters::GetMonsterAnimationName(animation)) == animation);
    }
    CHECK(Data::Monsters::GetMonsterAnimationName(MONSTER01_APEAR) == "appear");
    CHECK(Data::Monsters::GetMonsterAnimationName(12) == "12");
    // An animation with a name is not written by its number.
    CHECK_FALSE(Data::Monsters::FindMonsterAnimation("6"));
    CHECK_FALSE(Data::Monsters::FindMonsterAnimation("-1"));
}
