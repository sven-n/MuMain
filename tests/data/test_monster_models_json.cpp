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

using Data::Items::HasErrors;
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
    model.worldTextureFolders = {{{18, 53}, {"NPC"}}};
    model.textureNames = {{0, "xmassanta1.jpg"}};
    model.headBone = 16;
    model.streamMesh = 1;
    model.noneBlendMeshes = {0, 3};
    model.sounds = {{"Data/Sound/mDarkKnight1.wav", "SOUND_MONSTER_DARKKNIGHT1", 2, true},
                    {"Data/Sound/mDarkKnightDie.wav", "", 1, false}};
    Data::Monsters::MonsterSoundSlots slots;
    slots.first = {"SOUND_MONSTER_DARKKNIGHT1", "SOUND_MONSTER_DARKKNIGHT1", "", "", "Data/Sound/mDarkKnightDie.wav"};
    slots.second = Data::Monsters::MonsterSoundSlotSet{"a", "b", "c", "d", "e"};
    model.soundSlots = slots;
    Data::Monsters::MonsterSoundEvent walk;
    walk.animations = {MONSTER01_WALK};
    walk.frames = std::pair<float, float>{7.0f, 8.0f};
    walk.sounds = {"SOUND_MONSTER_DARKKNIGHT1"};
    Data::Monsters::MonsterSoundEvent breath;
    breath.animations = {MONSTER01_STOP1, MONSTER01_STOP2};
    breath.chance = Data::Monsters::MonsterSoundChance{100, Data::Monsters::MonsterSoundDraw::Rand};
    breath.pick = Data::Monsters::MonsterSoundDraw::RandFps;
    breath.sounds = {"SOUND_MONSTER_DARKKNIGHT1", "Data/Sound/mDarkKnightDie.wav"};
    Data::Monsters::MonsterSoundEvent die;
    die.animations = {MONSTER01_DIE};
    die.deathEffectArmed = true;
    die.sounds = {"Data/Sound/mDarkKnightDie.wav", "SOUND_MONSTER_DARKKNIGHT1"};
    model.soundEvents = {walk, breath, die};
    model.soundEventWorlds = {69, 70};
    model.bitmaps = {{"Monster/bv01_2.jpg", "BITMAP_MONSTER_SKIN", true, true}, {"Monster/iui02.tga", "BITMAP_ROBE+3"}};
    Data::Monsters::MonsterModelPart part;
    part.effect = "totemGolemPart1";
    part.load.file = "Data/Monster/totemhead.bmd";
    part.load.textureFolders = {"Monster"};
    part.load.speeds = {{0, 0.13f}};
    part.load.holdLastFrame = std::vector<int>{};
    part.worlds = {51};
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
    hound.sounds = {{"Data/Sound/mHound1.wav", "SOUND_MONSTER_HOUND1", 2, true}};
    Data::Monsters::MonsterSoundSlots slots;
    slots.first = {"SOUND_MONSTER_HOUND1", "SOUND_MONSTER_HOUND1", "", "", ""};
    hound.soundSlots = slots;
    MonsterModelDefinition smith;
    smith.slot = "MODEL_SMITH";
    smith.load.file = "Data/NPC/Smith01.bmd";
    smith.load.textureFolders = {"NPC"};

    const std::string text = Data::Monsters::WriteMonsterModelsJson(std::vector<MonsterModelDefinition>{smith, hound});
    CHECK(text == R"({
  "formatVersion": 1,
  "models": [
    {
      "slot": "MONSTER_MODEL_HOUND",
      "file": "Data/Monster/Monster02.bmd",
      "textureFolders": ["Monster"],
      "animations": {"walk": 0.7},
      "headBone": 5,
      "sounds": [
        {"file": "Data/Sound/mHound1.wav", "id": "SOUND_MONSTER_HOUND1", "channels": 2, "spatial": true}
      ],
      "soundSlots": {
        "idle": ["SOUND_MONSTER_HOUND1", "SOUND_MONSTER_HOUND1"],
        "attack": [null, null],
        "die": null
      }
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

TEST_CASE("Monster models: each sound event is written on a line of its own [data][monsters]")
{
    MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    Data::Monsters::MonsterSoundEvent walk;
    walk.animations = {MONSTER01_WALK};
    walk.frames = std::pair<float, float>{1.0f, 2.5f};
    walk.sounds = {"SOUND_KARUTAN_ORCUS_MOVE1"};
    Data::Monsters::MonsterSoundEvent attack;
    attack.animations = {MONSTER01_ATTACK1, MONSTER01_ATTACK2};
    attack.chance = Data::Monsters::MonsterSoundChance{3, Data::Monsters::MonsterSoundDraw::RandFps};
    attack.pick = Data::Monsters::MonsterSoundDraw::Rand;
    attack.sounds = {"SOUND_KARUTAN_ORCUS_ATTACK1", "SOUND_KARUTAN_ORCUS_ATTACK2"};
    model.soundEvents = {walk, attack};
    model.soundEventWorlds = {80, 81};

    const std::string text = Data::Monsters::WriteMonsterModelsJson(std::vector<MonsterModelDefinition>{model});
    CHECK(text.find(R"(      "soundEvents": [
        {"animations": ["walk"], "frames": [1, 2.5], "sounds": ["SOUND_KARUTAN_ORCUS_MOVE1"]},
        {"animations": ["attack1", "attack2"], "chance": {"oneIn": 3, "draw": "randFps"}, "pick": "rand", "sounds": ["SOUND_KARUTAN_ORCUS_ATTACK1", "SOUND_KARUTAN_ORCUS_ATTACK2"]}
      ],
      "soundEventWorlds": [80, 81])") != std::string::npos);
    INFO(text);
    const ReadResult read = Read(text);
    INFO(Messages(read.issues));
    CHECK(read.issues.empty());
    REQUIRE(read.models.size() == 1);
    CHECK(read.models[0] == model);
}

TEST_CASE("Monster models: a sound event names a sound of the model or a SOUND_ id [data][monsters]")
{
    MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    model.sounds = {{"Data/Sound/mHound1.wav", "", 2, true}};
    Data::Monsters::MonsterSoundEvent event;
    event.animations = {MONSTER01_ATTACK1};
    // The model's sound by its file, an id the data can name, and neither.
    event.sounds = {"Data/Sound/mHound1.wav", "SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01", "Data/Sound/mNobody.wav"};
    model.soundEvents = {event};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterModelNames(std::vector<MonsterModelDefinition>{model}, "Models.json", issues);

    INFO(Messages(issues));
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[0].field == "model MONSTER_MODEL_HOUND soundEvents[0].sounds[2]");
    CHECK(issues[0].message.find("is no sound of the model") != std::string::npos);
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
        {Model("MONSTER_MODEL_HOUND", R"("sounds": [{"file": "Data/Sound/a.wav", "channels": 5}])"),
         "model MONSTER_MODEL_HOUND sounds[0].channels", "1 to 4"},
        {Model("MONSTER_MODEL_HOUND", R"("soundSlots": {"idle": ["a"], "attack": [null, null], "die": null})"),
         "model MONSTER_MODEL_HOUND soundSlots.idle", "two sounds"},
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
        {Model("MONSTER_MODEL_HOUND", R"("soundEvents": [{"sounds": ["a"]}])"),
         "model MONSTER_MODEL_HOUND soundEvents[0]", "animations and sounds"},
        {Model("MONSTER_MODEL_HOUND", R"("soundEvents": [{"animations": [], "sounds": ["a"]}])"),
         "model MONSTER_MODEL_HOUND soundEvents[0].animations", "one animation or more"},
        {Model("MONSTER_MODEL_HOUND", R"("soundEvents": [{"animations": ["walk"], "sounds": []}])"),
         "model MONSTER_MODEL_HOUND soundEvents[0].sounds", "one sound or more"},
        {Model("MONSTER_MODEL_HOUND",
               R"("soundEvents": [{"animations": ["walk"], "frames": [8, 7], "sounds": ["a"]}])"),
         "model MONSTER_MODEL_HOUND soundEvents[0].frames", "first below the second"},
        {Model("MONSTER_MODEL_HOUND",
               R"("soundEvents": [{"animations": ["walk"], "chance": {"oneIn": 0, "draw": "rand"}, "sounds": ["a"]}])"),
         "model MONSTER_MODEL_HOUND soundEvents[0].chance.oneIn", "1 to 100000"},
        {Model("MONSTER_MODEL_HOUND",
               R"("soundEvents": [{"animations": ["walk"], "chance": {"oneIn": 2, "draw": "dice"}, "sounds": ["a"]}])"),
         "model MONSTER_MODEL_HOUND soundEvents[0].chance.draw", "randFps or rand"},
        {Model("MONSTER_MODEL_HOUND", R"("soundEvents": [{"animations": ["walk"], "pick": "rand", "sounds": ["a"]}])"),
         "model MONSTER_MODEL_HOUND soundEvents[0].sounds", "two sounds with pick"},
        {Model("MONSTER_MODEL_HOUND",
               R"("soundEvents": [{"animations": ["die"], "deathEffectArmed": 1, "sounds": ["a"]}])"),
         "model MONSTER_MODEL_HOUND soundEvents[0].deathEffectArmed", "true or false"},
        {Model("MONSTER_MODEL_HOUND", R"("soundEventWorlds": [69, -1])"),
         "model MONSTER_MODEL_HOUND soundEventWorlds[1]", "whole number"},
    };
    for (const Case& test : cases)
    {
        const ReadResult read = Read(ModelsFile(test.model));
        INFO(test.model);
        INFO(Messages(read.issues));
        CHECK(read.models.empty());
        CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Error, test.field, test.message));
    }
}

TEST_CASE("Monster models: a slot must be a model slot of the monster area and be there once [data][monsters]")
{
    ReadResult read = Read(ModelsFile(Model("MODEL_PLAYER")));
    CHECK(read.models.empty());
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Error, "models[0].slot", "monster model type"));

    read = Read(ModelsFile(Model("MONSTER_MODEL_HOUND") + ", " + Model("MONSTER_MODEL_HOUND")));
    CHECK(read.models.size() == 1);
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Error, "model MONSTER_MODEL_HOUND slot", "more than once"));
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
        R"("scale": 1, "sounds": [{"file": "Data/Sound/a.wav", "volume": 1}], "parts": [{"effect": "x", "file": "Data/Monster/p.bmd", "textureFolders": ["Monster"], "speed": 1}])")));
    INFO(Messages(read.issues));
    CHECK_FALSE(HasErrors(read.issues));
    REQUIRE(read.models.size() == 1);
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "model MONSTER_MODEL_HOUND scale", "unknown"));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "model MONSTER_MODEL_HOUND sounds[0].volume",
                        "unknown"));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "model MONSTER_MODEL_HOUND parts[0].speed",
                        "unknown"));
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
