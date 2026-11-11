#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/LookData/LookJson.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

using namespace Data::Looks;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;

namespace
{
const std::string Source = "RenderStyles.json";

struct ReadResult
{
    std::vector<LookDefinition> looks;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(std::string_view text)
{
    ReadResult result;
    ReadRenderStylesJson(text, Source, result.looks, result.issues);
    return result;
}

std::string File(const std::string& looks)
{
    return R"({ "formatVersion": 1, "kind": "renderStyle", "looks": )" + looks + " }";
}

std::string EffectFile(const std::string& looks)
{
    return R"({ "formatVersion": 1, "kind": "itemEffect", "looks": )" + looks + " }";
}

ReadResult ReadEffects(std::string_view text)
{
    ReadResult result;
    LookFile file;
    ReadLookFile(text, "ItemEffects.json", LookKind::ItemEffect, file, result.issues);
    result.looks = std::move(file.looks);
    return result;
}

// The issue at `field` with `severity`, or nullptr.
const ItemDataIssue* Find(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity,
                          const std::string& field)
{
    const auto it = std::find_if(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                                 { return issue.severity == severity && issue.field == field; });
    return it == issues.end() ? nullptr : &*it;
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
} // namespace

TEST_CASE("The shipped render style file reads without problems and is in the written format [looks][files]")
{
    const auto text = Data::Items::Json::ReadTextFile(std::filesystem::path(MU_TEST_DATA_DIR) / "Effects" /
                                                      std::string(GetRenderStylesFileName()));
    REQUIRE(text);
    const ReadResult result = Read(*text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    CHECK(WriteRenderStylesJson(result.looks) == *text);
}

TEST_CASE("The shipped item effect file reads without problems and is in the written format [looks][files]")
{
    const auto text = Data::Items::Json::ReadTextFile(std::filesystem::path(MU_TEST_DATA_DIR) / "Effects" /
                                                      std::string(GetLookFileName(LookKind::ItemEffect)));
    REQUIRE(text);
    const ReadResult result = ReadEffects(*text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    CHECK(WriteLookFileJson(LookKind::ItemEffect, result.looks) == *text);
}

TEST_CASE("A look's layers read every field, and write back as they were read [looks][files]")
{
    const std::string text = WriteRenderStylesJson({
        {"chromeMesh0",
         {Layer{}, Layer{.kind = LayerKind::Reflection, .blend = Blend::Glow, .meshes = {{0}}, .glowMesh = 0}},
         {}},
        {"albatrossBow",
         {Layer{.meshes = {{1}, true}},
          Layer{.motion = Motion::SlowDrift,
                .meshes = {{2}},
                .texture = "Effect/Chrome01.jpg",
                .opacity = 0.5f,
                .glowMesh = NoGlowMesh{},
                .glowStrength = 1.f,
                .scrollU = 0.25f,
                .scrollV = 0.75f},
          Layer{.kind = LayerKind::Reflection,
                .motion = Motion::StrongSwirl,
                .blend = Blend::AsDrawing,
                .opacity = FromDrawing{}}},
         {.glowMesh = 1, .glowStrength = 1.f, .hiddenMesh = 1}},
    });
    const ReadResult result = Read(text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 2);
    // Sorted by name.
    CHECK(result.looks[0].name == "albatrossBow");
    CHECK(result.looks[1].name == "chromeMesh0");
    const Layer& texture = result.looks[0].layers[1];
    CHECK(texture.kind == LayerKind::Texture);
    CHECK(texture.texture == "Effect/Chrome01.jpg");
    CHECK(texture.opacity == Value{0.5f});
    CHECK(texture.glowMesh == GlowMesh{NoGlowMesh{}});
    CHECK(texture.scrollV == Value{0.75f});
    CHECK(result.looks[0].layers[0].meshes == MeshSet{{1}, true});
    CHECK(result.looks[0].layers[2].motion == Motion::StrongSwirl);
    CHECK(result.looks[0].layers[2].opacity == Value{FromDrawing{}});
    CHECK(result.looks[0].itemValuesAfter.hiddenMesh == 1);
    CHECK(result.looks[1].layers[1] ==
          Layer{.kind = LayerKind::Reflection, .blend = Blend::Glow, .meshes = {{0}}, .glowMesh = 0});
    CHECK(WriteRenderStylesJson(result.looks) == text);
}

TEST_CASE("A render style's Not for and a layer's Only when read every part and write back as they were read "
          "(design L11) [looks][files]")
{
    LookDefinition battleBow{"battleBow", {Layer{.blend = Blend::AsDrawing}}, {}};
    battleBow.notFor = {.doppelganger = true, .outsideMonsterHands = true};
    LookDefinition potion{
        "potion",
        {Layer{.meshes = {{0}}, .onlyWhen = {.hiddenMesh = 1, .withoutBuff = LookBuff::InfinityArrow}},
         Layer{.meshes = {{1}}, .onlyWhen = {.place = Place::InventoryAndGround, .level = std::pair<int, int>{0, 7}}},
         Layer{.onlyWhen = {.place = Place::Worn, .hiddenMesh = -1, .buff = LookBuff::InfinityArrow}}},
        {}};
    const std::string text = WriteRenderStylesJson({battleBow, potion});
    CHECK(text.find(R"("notFor": ["doppelganger", "outsideMonsterHands"])") != std::string::npos);
    CHECK(text.find(R"("onlyWhen": {"place": "inventoryAndGround", "level": [0, 7]})") != std::string::npos);
    CHECK(text.find(R"("onlyWhen": {"hiddenMesh": 1, "withoutBuff": "infinityArrow"})") != std::string::npos);
    CHECK(text.find(R"("onlyWhen": {"place": "worn", "hiddenMesh": "none", "buff": "infinityArrow"})") !=
          std::string::npos);
    const ReadResult result = Read(text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 2);
    CHECK(result.looks[0] == battleBow);
    CHECK(result.looks[1] == potion);
    CHECK(WriteRenderStylesJson(result.looks) == text);
    // A look for every drawing and a layer that always draws write neither.
    const std::string plain = WriteRenderStylesJson({{"plain", {Layer{}}, {}}});
    CHECK(plain.find("notFor") == std::string::npos);
    CHECK(plain.find("onlyWhen") == std::string::npos);
}

TEST_CASE("An item effect sets item values before the drawing, by level, and the object's light, and writes back "
          "as it was read (design L13, L45) [looks][files]")
{
    const ReadResult result = ReadEffects(EffectFile(R"([
        {"name": "hiddenMeshByLevel", "itemValuesBefore": [
            {"onlyWhen": {"level": [0, 0]}, "hiddenMesh": 1},
            {"onlyWhen": {"level": [1, 1]}, "hiddenMesh": 0}]},
        {"name": "devilsEye", "itemValuesBefore": [
            {"glowMesh": 1, "glowStrength": {"shape": "wave", "from": 5.65, "to": 25.65, "seconds": 3.1415927},
             "drawingOpacity": 2}],
         "objectLight": [1, {"shape": "wave", "from": 1.13, "to": 5.13, "seconds": 3.1415927}, 1]},
        {"name": "potion", "itemValuesBefore": [{"onlyWhen": {"level": [1, 15]}, "glowsLikeLevel": 7}]},
        {"name": "shining", "itemValuesBefore": [{"glowMesh": "all"}]}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 4);
    CHECK(result.looks[3].itemValuesBefore[0].values.glowMesh == -2);
    const LookDefinition& byLevel = result.looks[0];
    REQUIRE(byLevel.itemValuesBefore.size() == 2);
    CHECK(byLevel.itemValuesBefore[1].onlyWhen.level == std::pair<int, int>{1, 1});
    CHECK(byLevel.itemValuesBefore[1].values.hiddenMesh == 0);
    const LookDefinition& eye = result.looks[1];
    REQUIRE(eye.itemValuesBefore.size() == 1);
    CHECK(eye.itemValuesBefore[0].drawingOpacity == Number{2.f});
    CHECK(eye.itemValuesBefore[0].values.glowMesh == 1);
    REQUIRE(eye.objectLight);
    CHECK((*eye.objectLight)[0] == Number{1.f});
    CHECK(result.looks[2].itemValuesBefore[0].glowsLikeLevel == 7);

    const std::string text = WriteLookFileJson(LookKind::ItemEffect, result.looks);
    CHECK(text.find(R"("kind": "itemEffect")") != std::string::npos);
    CHECK(text.find(R"({"onlyWhen": {"level": [1, 15]}, "glowsLikeLevel": 7})") != std::string::npos);
    CHECK(text.find(R"("objectLight": [1, {"shape": "wave")") != std::string::npos);
    // Sorted by name, and so again.
    const ReadResult again = ReadEffects(text);
    CHECK(again.issues.empty());
    CHECK(WriteLookFileJson(LookKind::ItemEffect, again.looks) == text);
    REQUIRE(again.looks.size() == 4);
    CHECK(text.find(R"({"glowMesh": "all"})") != std::string::npos);
    CHECK(again.looks[0] == eye);
}

TEST_CASE("A look file has only its kind's parts: render style parts in an item effect and the other way round are "
          "fatal (design L23) [looks][files]")
{
    const struct
    {
        const char* text;
        const char* field;
    } effectCases[] = {
        {R"([{"name": "a", "layers": []}])", "looks[0].layers"},
        {R"([{"name": "a", "notFor": ["doppelganger"]}])", "looks[0].notFor"},
        {R"([{"name": "a", "itemValuesAfter": {"glowMesh": 1}}])", "looks[0].itemValuesAfter"},
        {R"([{"name": "a", "itemValuesBefore": [{"light": [1, 1, 1]}]}])", "looks[0].itemValuesBefore[0].light"},
        {R"([{"name": "a", "itemValuesBefore": [{"glowsLikeLevel": 16}]}])",
         "looks[0].itemValuesBefore[0].glowsLikeLevel"},
        {R"([{"name": "a", "itemValuesBefore": {"glowMesh": 1}}])", "looks[0].itemValuesBefore"},
        {R"([{"name": "a", "objectLight": [1, 1]}])", "looks[0].objectLight"},
    };
    for (const auto& item : effectCases)
    {
        CAPTURE(item.text);
        CHECK(Find(ReadEffects(EffectFile(item.text)).issues, ItemDataIssueSeverity::Fatal, item.field) != nullptr);
    }
    CHECK(Find(Read(File(R"([{"name": "a", "layers": [], "objectLight": [1, 1, 1]}])")).issues,
               ItemDataIssueSeverity::Fatal, "looks[0].objectLight") != nullptr);
    // A file of the other kind.
    CHECK(Find(ReadEffects(File("[]")).issues, ItemDataIssueSeverity::Fatal, "kind") != nullptr);
    CHECK(Find(Read(EffectFile("[]")).issues, ItemDataIssueSeverity::Fatal, "kind") != nullptr);
    // An item effect that does nothing is a warning.
    CHECK(Find(ReadEffects(EffectFile(R"([{"name": "a"}])")).issues, ItemDataIssueSeverity::Warning, "looks[0]") !=
          nullptr);
}

TEST_CASE("An item's values take the place of the fields of its item effect that each item sets (design L12) "
          "[looks][files]")
{
    const ReadResult result = ReadEffects(EffectFile(R"([{"name": "glowing", "itemValuesBefore": [
        {"glowMesh": {"eachItem": "glowMesh", "default": 0}, "glowStrength": 1}]}])"));
    INFO(Describe(result.issues));
    REQUIRE(result.looks.size() == 1);
    CHECK(SetsEachItem(result.looks[0]));
    std::vector<ItemDataIssue> issues;
    const LookDefinition resolved =
        ResolveForItem(result.looks[0], {{"glowMesh", "2"}}, "", issues, LookKind::ItemEffect);
    CHECK(issues.empty());
    REQUIRE(resolved.itemValuesBefore.size() == 1);
    CHECK(resolved.itemValuesBefore[0].values.glowMesh == 2);
    CHECK(WriteLookFileJson(LookKind::ItemEffect, result.looks)
              .find(R"({"glowMesh": {"eachItem": "glowMesh", "default": 0}, "glowStrength": 1})") != std::string::npos);
}

TEST_CASE("A render style can draw alone, which writes after its Not for, and is no part of an item effect (design "
          "L47) [looks][files]")
{
    const ReadResult result =
        Read(File(R"([{"name": "scroll", "drawsAlone": true, "layers": [{"kind": "texture"}]}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    CHECK(result.looks[0].drawsAlone);
    const std::string text = WriteRenderStylesJson(result.looks);
    const std::size_t alonePosition = text.find(R"("drawsAlone": true,)");
    REQUIRE(alonePosition != std::string::npos);
    CHECK(text.find(R"("name": "scroll")") < alonePosition);
    CHECK(alonePosition < text.find(R"("layers")"));
    CHECK(Read(text).looks == result.looks);
    CHECK(WriteRenderStylesJson({{"plain", {Layer{}}, {}}}).find("drawsAlone") == std::string::npos);

    CHECK(Find(Read(File(R"([{"name": "a", "drawsAlone": 1, "layers": []}])")).issues, ItemDataIssueSeverity::Fatal,
               "looks[0].drawsAlone") != nullptr);
    CHECK(Find(ReadEffects(EffectFile(R"([{"name": "a", "drawsAlone": true}])")).issues, ItemDataIssueSeverity::Fatal,
               "looks[0].drawsAlone") != nullptr);
}

TEST_CASE("A layer of no mesh reads and writes as \"none\" (design L12) [looks][files]")
{
    const ReadResult result = Read(File(R"([{"name": "a", "layers": [{"kind": "texture", "meshes": "none"}]}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    CHECK(result.looks[0].layers[0].meshes.none);
    CHECK_FALSE(result.looks[0].layers[0].meshes.IsAll());
    const std::string text = WriteRenderStylesJson(result.looks);
    CHECK(text.find(R"({"kind": "texture", "meshes": "none"})") != std::string::npos);
    CHECK(Read(text).looks == result.looks);
}

TEST_CASE("A reflection under the holes writes its place, and only a reflection can have one (design L8) "
          "[looks][files]")
{
    const ReadResult result = Read(File(R"([{"name": "set", "layers": [{"kind": "texture"},
        {"kind": "reflection", "motion": "slowDrift", "where": "underHoles", "blend": "glow"},
        {"kind": "reflection", "motion": "swirl", "where": "over"}]}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    CHECK(result.looks[0].layers[0].where == Where::Over);
    CHECK(result.looks[0].layers[1].where == Where::UnderHoles);
    CHECK(result.looks[0].layers[2].where == Where::Over);
    const std::string text = WriteRenderStylesJson(result.looks);
    CHECK(text.find(R"("motion": "slowDrift", "where": "underHoles", "blend": "glow")") != std::string::npos);
    // Over the texture is what a reflection does without a place.
    CHECK(text.find(R"("where": "over")") == std::string::npos);
    CHECK(Read(text).looks == result.looks);

    CHECK(Find(Read(File(R"([{"name": "a", "layers": [{"kind": "texture", "where": "underHoles"}]}])")).issues,
               ItemDataIssueSeverity::Fatal, "looks[0].layers[0].where") != nullptr);
    CHECK(Find(Read(File(R"([{"name": "a", "layers": [{"kind": "reflection", "where": "below"}]}])")).issues,
               ItemDataIssueSeverity::Fatal, "looks[0].layers[0].where") != nullptr);
}

TEST_CASE("A layer can be a flat color (design L8) [looks][files]")
{
    const ReadResult result = Read(File(R"([{"name": "backing", "layers": [{"kind": "flatColor", "meshes": [2]}]}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    CHECK(result.looks[0].layers[0].kind == LayerKind::FlatColor);
    CHECK(WriteRenderStylesJson(result.looks).find(R"({"kind": "flatColor", "meshes": [2]})") != std::string::npos);
}

TEST_CASE("A layer's value can be the item's times a shape (design L16) [looks][files]")
{
    const ReadResult result = Read(File(
        R"([{"name": "phoenix", "layers": [{"kind": "texture", "opacity": {"shape": "wave", "from": 0.4, "to": 1, "seconds": 2.0943951, "timesItem": true}}]}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    const Shape* shape = std::get_if<Shape>(&result.looks[0].layers[0].opacity);
    REQUIRE(shape != nullptr);
    CHECK(shape->timesItem);
    const std::string text = WriteRenderStylesJson(result.looks);
    CHECK(text.find(R"("seconds": 2.0943951, "timesItem": true})") != std::string::npos);
    CHECK(Read(text).looks == result.looks);

    // A number that can't be the item's (a light's color) has no item value to multiply: a warning, left out.
    const ReadResult light = Read(File(
        R"([{"name": "red", "layers": [{"kind": "texture", "light": [{"shape": "wave", "from": 0.4, "to": 1, "seconds": 2, "timesItem": true}, 0, 0]}]}])"));
    INFO(Describe(light.issues));
    REQUIRE(light.issues.size() == 1);
    CHECK(light.issues[0].severity == Data::Items::ItemDataIssueSeverity::Warning);
    CHECK(light.issues[0].field == "looks[0].layers[0].light[0].timesItem");
    REQUIRE(light.looks.size() == 1);
    CHECK_FALSE(std::get<Shape>(light.looks[0].layers[0].light->color[0]).timesItem);
}

TEST_CASE("A light can be the model's at a brightness (design L16) [looks][files]")
{
    const ReadResult result =
        Read(File(R"([{"name": "cloak", "layers": [{"kind": "texture", "light": {"brightness": 0.8}}]}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    REQUIRE(result.looks[0].layers[0].light);
    CHECK(result.looks[0].layers[0].light->brightness == Number{0.8f});
    const std::string text = WriteRenderStylesJson(result.looks);
    CHECK(text.find(R"("light": {"brightness": 0.8})") != std::string::npos);
    CHECK(Read(text).looks == result.looks);
}

TEST_CASE("A look can leave a light, a color or a tint, which writes back as it was read (design L46) [looks][files]")
{
    const ReadResult result =
        Read(File(R"([{"name": "gray", "layers": [{"kind": "texture"}], "itemValuesAfter": {"light": [0.7, 0.7, 0.7]}},
        {"name": "tinted", "layers": [{"kind": "texture"}], "itemValuesAfter": {"light": {"tint": [1, 0.5, 0.5]}}}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 2);
    CHECK(result.looks[0].itemValuesAfter.light == LayerLight{.color = {0.7f, 0.7f, 0.7f}});
    CHECK(result.looks[1].itemValuesAfter.light == LayerLight{.tint = true, .color = {1.f, 0.5f, 0.5f}});
    const std::string text = WriteRenderStylesJson(result.looks);
    CHECK(text.find(R"("itemValuesAfter": {"light": [0.7, 0.7, 0.7]})") != std::string::npos);
    CHECK(text.find(R"("itemValuesAfter": {"light": {"tint": [1, 0.5, 0.5]}})") != std::string::npos);
    CHECK(Read(text).looks == result.looks);

    CHECK(Find(Read(File(R"([{"name": "a", "layers": [], "itemValuesAfter": {"light": [1, 1]}}])")).issues,
               ItemDataIssueSeverity::Fatal, "looks[0].itemValuesAfter.light") != nullptr);
}

TEST_CASE("A field each item sets reads its default and keeps its name, and writes back as it was read (design L12) "
          "[looks][files]")
{
    const ReadResult result = Read(File(R"([{"name": "seedSphere", "layers": [
        {"kind": "texture", "meshes": {"eachItem": "glassMeshes", "default": [1]}},
        {"kind": "reflection", "motion": "slowDrift", "blend": "glow", "light": {"eachItem": "color"}}],
        "itemValuesAfter": {"glowMesh": {"eachItem": "glowMesh", "default": 0}}}])"));
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    const LookDefinition& look = result.looks[0];
    CHECK(look.layers[0].meshes.meshes == std::vector<int>{1});
    CHECK(look.layers[0].eachItem == EachItemFields{{"meshes", "glassMeshes"}});
    // Without a default the field is the item's, as when it isn't written.
    CHECK_FALSE(look.layers[1].light);
    CHECK(look.layers[1].eachItem == EachItemFields{{"light", "color"}});
    CHECK(look.itemValuesAfter.glowMesh == 0);
    CHECK(look.itemValuesAfter.eachItem == EachItemFields{{"glowMesh", "glowMesh"}});
    CHECK(SetsEachItem(look));
    const std::string text = WriteRenderStylesJson(result.looks);
    CHECK(text.find(R"("meshes": {"eachItem": "glassMeshes", "default": [1]})") != std::string::npos);
    CHECK(text.find(R"("light": {"eachItem": "color"})") != std::string::npos);
    CHECK(Read(text).looks == result.looks);

    // A field without a default is written at its place.
    const std::string withoutDefault = WriteRenderStylesJson(
        Read(
            File(
                R"([{"name": "b", "layers": [{"kind": "texture", "onlyWhen": {"place": "worn"}, "meshes": {"eachItem": "m"}}]}])"))
            .looks);
    CHECK(withoutDefault.find(R"({"kind": "texture", "meshes": {"eachItem": "m"}, "onlyWhen": {"place": "worn"}})") !=
          std::string::npos);

    CHECK(Find(Read(File(R"([{"name": "a", "layers": [{"kind": "texture", "meshes": {"eachItem": "two words"}}]}])"))
                   .issues,
               ItemDataIssueSeverity::Fatal, "looks[0].layers[0].meshes.eachItem") != nullptr);
}

TEST_CASE("An item's values take the place of the fields each item sets, and a value that can't be the field's is "
          "fatal and leaves the look as it is (design L12) [looks][files]")
{
    LookDefinition look{"seedSphere",
                        {Layer{.meshes = {{1}}, .eachItem = {{"meshes", "glassMeshes"}}},
                         Layer{.kind = LayerKind::Reflection, .blend = Blend::Glow, .eachItem = {{"light", "color"}}}},
                        {}};
    std::vector<ItemDataIssue> issues;
    const LookDefinition fire =
        ResolveForItem(look, {{"color", "[0.9, 0.1, 0.2]"}, {"glassMeshes", "[0, 2]"}}, "Data/Items/Models", issues);
    INFO(Describe(issues));
    CHECK(issues.empty());
    CHECK(fire.layers[0].meshes.meshes == std::vector<int>{0, 2});
    REQUIRE(fire.layers[1].light);
    CHECK(fire.layers[1].light->color == std::array<Number, 3>{0.9f, 0.1f, 0.2f});
    // A value the item doesn't set keeps the look's.
    CHECK(ResolveForItem(look, {{"color", "[1, 1, 1]"}}, "", issues).layers[0].meshes.meshes == std::vector<int>{1});
    // Without values the item draws the look itself.
    CHECK(ResolveForItem(look, {}, "", issues) == look);
    // null leaves the field out: every mesh, as a layer that names none.
    CHECK(ResolveForItem(look, {{"glassMeshes", "null"}}, "", issues).layers[0].meshes.IsAll());
    CHECK(issues.empty());

    issues.clear();
    CHECK(ResolveForItem(look, {{"glassMeshes", R"("crystal")"}}, "", issues) == look);
    CHECK(Data::Items::HasFatal(issues));
}

TEST_CASE("A layer leaves out what is the item's, and a texture layer writes no motion [looks][files]")
{
    const std::string text = WriteRenderStylesJson({{"textured", {Layer{}}, {}}});
    CHECK(text.find(R"({"kind": "texture"})") != std::string::npos);
    CHECK(text.find("motion") == std::string::npos);
    CHECK(text.find("itemValuesAfter") == std::string::npos);
}

TEST_CASE("Problems that make a render style file unusable are fatal [looks][files]")
{
    struct Case
    {
        const char* text;
        const char* field;
    };
    const Case cases[] = {
        {R"([{"name": "a", "layers": [{"kind": "texture"}]}, {"name": "a", "layers": []}])", "looks[1].name"},
        {R"([{"name": "a b", "layers": []}])", "looks[0].name"},
        {R"([{"layers": []}])", "looks[0].name"},
        {R"([{"name": "a"}])", "looks[0].layers"},
        {R"([{"name": "a", "layers": [{}]}])", "looks[0].layers[0].kind"},
        {R"([{"name": "a", "layers": [{"kind": "paint"}]}])", "looks[0].layers[0].kind"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "motion": "swirl"}]}])", "looks[0].layers[0].motion"},
        {R"([{"name": "a", "layers": [{"kind": "reflection", "motion": "spin"}]}])", "looks[0].layers[0].motion"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "blend": "add"}]}])", "looks[0].layers[0].blend"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "meshes": []}]}])", "looks[0].layers[0].meshes"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "meshes": [50]}]}])", "looks[0].layers[0].meshes"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "meshes": {"only": [1]}}]}])", "looks[0].layers[0].meshes"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowMesh": -1}]}])", "looks[0].layers[0].glowMesh"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "opacity": "item"}]}])", "looks[0].layers[0].opacity"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowStrength": "drawing"}]}])",
         "looks[0].layers[0].glowStrength"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "texture": ""}]}])", "looks[0].layers[0].texture"},
        {R"([{"name": "a", "layers": [], "itemValuesAfter": {"hiddenMesh": "all"}}])",
         "looks[0].itemValuesAfter.hiddenMesh"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowMesh": "every"}]}])", "looks[0].layers[0].glowMesh"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "opacity": {"from": 0}}]}])",
         "looks[0].layers[0].opacity.shape"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "opacity": {"shape": "spiral"}}]}])",
         "looks[0].layers[0].opacity.shape"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowStrength": {"shape": "wave", "from": 0, "to": 1}}]}])",
         "looks[0].layers[0].glowStrength.seconds"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowStrength": {"shape": "wave", "from": 0, "to": 1,
            "seconds": 0}}]}])",
         "looks[0].layers[0].glowStrength.seconds"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "scrollV": {"shape": "frames", "count": 0, "step": 0.25,
            "secondsEach": 0.08}}]}])",
         "looks[0].layers[0].scrollV.count"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "scrollU": {"shape": "random", "from": 0, "to": 1,
            "step": 0}}]}])",
         "looks[0].layers[0].scrollU.step"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "scrollU": {"shape": "random", "from": 1, "to": 0,
            "step": 0.1}}]}])",
         "looks[0].layers[0].scrollU.to"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "light": [1, 1]}]}])", "looks[0].layers[0].light"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "light": {"color": [1, 1, 1]}}]}])",
         "looks[0].layers[0].light"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "light": {"tint": [1, 1]}}]}])",
         "looks[0].layers[0].light.tint"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "light": [1, "white", 1]}]}])",
         "looks[0].layers[0].light[1]"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "scrollingMesh": 50}]}])",
         "looks[0].layers[0].scrollingMesh"},
        {R"([{"name": "a", "layers": [], "itemValuesAfter": {"glowStrength": "drawing"}}])",
         "looks[0].itemValuesAfter.glowStrength"},
        {R"([{"name": "a", "layers": [], "itemValuesBefore": [{"hiddenMesh": 1}]}])", "looks[0].itemValuesBefore"},
        {R"([{"name": "a", "notFor": "doppelganger", "layers": []}])", "looks[0].notFor"},
        {R"([{"name": "a", "notFor": ["doppelganger", "inventory"], "layers": []}])", "looks[0].notFor[1]"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "onlyWhen": "worn"}]}])", "looks[0].layers[0].onlyWhen"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "onlyWhen": {"place": "hand"}}]}])",
         "looks[0].layers[0].onlyWhen.place"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "onlyWhen": {"level": [3, 1]}}]}])",
         "looks[0].layers[0].onlyWhen.level"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "onlyWhen": {"level": [0, 16]}}]}])",
         "looks[0].layers[0].onlyWhen.level"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "onlyWhen": {"level": 7}}]}])",
         "looks[0].layers[0].onlyWhen.level"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "onlyWhen": {"hiddenMesh": 50}}]}])",
         "looks[0].layers[0].onlyWhen.hiddenMesh"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "onlyWhen": {"buff": "haste"}}]}])",
         "looks[0].layers[0].onlyWhen.buff"},
    };
    for (const Case& c : cases)
    {
        CAPTURE(c.text);
        const ReadResult result = Read(File(c.text));
        INFO(Describe(result.issues));
        CHECK(Find(result.issues, ItemDataIssueSeverity::Fatal, c.field) != nullptr);
        CHECK(Data::Items::HasFatal(result.issues));
    }

    CHECK(Find(Read(R"({ "formatVersion": 1, "kind": "itemEffect", "looks": [] })").issues,
               ItemDataIssueSeverity::Fatal, "kind") != nullptr);
    CHECK(Find(Read(R"({ "formatVersion": 1, "kind": "renderStyle" })").issues, ItemDataIssueSeverity::Fatal,
               "looks") != nullptr);
    CHECK(Data::Items::HasFatal(Read(R"({ "formatVersion": 2, "kind": "renderStyle", "looks": [] })").issues));
}

TEST_CASE("Unknown fields, a look without layers and empty item values are warnings [looks][files]")
{
    const ReadResult result = Read(File(R"([{"name": "a", "layers": [], "colour": 1, "itemValuesAfter": {}},
        {"name": "b", "layers": [{"kind": "texture", "shine": true}, {"kind": "texture", "onlyWhen": {}},
                                 {"kind": "texture", "onlyWhen": {"worn": true}}]}])"));
    INFO(Describe(result.issues));
    CHECK_FALSE(Data::Items::HasFatal(result.issues));
    // A condition that sets nothing, and one the file doesn't know: the layer always draws.
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[1].layers[1].onlyWhen") != nullptr);
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[1].layers[2].onlyWhen.worn") != nullptr);
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[0].layers") != nullptr);
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[0].colour") != nullptr);
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[0].itemValuesAfter") != nullptr);
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[1].layers[0].shine") != nullptr);
    CHECK(result.looks.size() == 2);
}

TEST_CASE("The Style Editor's folders are the editor's: the file reads without a warning about them [looks][files]")
{
    const ReadResult result =
        Read(R"({ "formatVersion": 1, "kind": "renderStyle", "lookFolders": [{"name": "Tickets"}], "looks": [] })");
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
}

TEST_CASE("The Style Editor's folders write each on a line before the looks, and read back [looks][files]")
{
    const std::vector<LookDefinition> looks{{"b", {Layer{}}, {}}, {"a", {Layer{}}, {}}};
    const std::vector<Data::Colors::ColorFolder> folders{
        {"Tickets", {{"Old", {}, {"a"}}}, {"b", "gone"}},
    };
    const std::string text = WriteRenderStylesJson(looks, folders);
    CHECK(text.find("\"lookFolders\"") < text.find("\"looks\": [\n"));
    CHECK(text.find(R"({"name": "Tickets", "folders": [{"name": "Old", "looks": ["a"]}], "looks": ["b"]})") !=
          std::string::npos);
    RenderStylesFile file;
    std::vector<ItemDataIssue> issues;
    ReadRenderStylesFile(text, Source, file, issues);
    INFO(Describe(issues));
    CHECK(issues.empty());
    // The look that is gone is left out.
    CHECK(file.folders == std::vector<Data::Colors::ColorFolder>{{"Tickets", {{"Old", {}, {"a"}}}, {"b"}}});
    REQUIRE(file.looks.size() == 2);
    CHECK(file.looks[0].name == "a");
}

TEST_CASE("A folder naming a look the file doesn't have is a warning that leaves it out [looks][files]")
{
    RenderStylesFile file;
    std::vector<ItemDataIssue> issues;
    ReadRenderStylesFile(File(R"([{"name": "a", "layers": [{"kind": "texture"}]}], "lookFolders": [
        {"name": "Tickets", "looks": ["a", "missing"], "colors": ["a"]}])"),
                         Source, file, issues);
    INFO(Describe(issues));
    CHECK(Find(issues, ItemDataIssueSeverity::Warning, "lookFolders[0].looks[1]") != nullptr);
    CHECK(Find(issues, ItemDataIssueSeverity::Warning, "lookFolders[0].colors") != nullptr);
    CHECK_FALSE(Data::Items::HasFatal(issues));
    CHECK(file.folders == std::vector<Data::Colors::ColorFolder>{{"Tickets", {}, {"a"}}});
}

TEST_CASE("A layer of every mesh, the item's hidden one too, reads and writes as allEvenHidden [looks][files]")
{
    const std::string text = WriteRenderStylesJson({{"texturedAllMeshes", {Layer{.meshes = {{}, false, true}}}, {}}});
    CHECK(text.find(R"("meshes": "allEvenHidden")") != std::string::npos);
    const ReadResult result = Read(text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    CHECK(result.looks[0].layers[0].meshes.evenHidden);
    CHECK(result.looks[0].layers[0].meshes.IsAll());
}

TEST_CASE("Shapes, a layer's light, its scrolling mesh and the glow of every mesh write back as they were read "
          "[looks][files]")
{
    const Shape wave{.kind = ShapeKind::Wave, .from = -0.2, .to = 1.2, .seconds = 7.853982};
    const Shape bounce{.kind = ShapeKind::Bounce, .from = 0.1, .to = 0.4, .seconds = 3.141593};
    const Shape scroll{.kind = ShapeKind::Scroll, .perSecond = -0.1};
    const Shape ramp{.kind = ShapeKind::Ramp, .from = 0.0, .to = -1.0, .seconds = 2.0};
    CHECK(
        Read(
            File(
                R"([{"name": "z", "layers": [{"kind": "texture", "glowStrength": {"shape": "zigzag", "from": 0.01, "to": 1, "seconds": 3.96}}]}])"))
            .looks.at(0)
            .layers.at(0)
            .glowStrength == Value{Shape{.kind = ShapeKind::Zigzag, .from = 0.01, .to = 1.0, .seconds = 3.96}});
    const Shape frames{.kind = ShapeKind::Frames, .seconds = 0.08, .step = 0.25, .count = 4};
    const Shape random{.kind = ShapeKind::Random, .from = 0.0, .to = 0.9, .step = 0.1};
    const std::vector<LookDefinition> looks{
        {"timed",
         {Layer{.meshes = {{1}},
                .opacity = wave,
                .glowMesh = AllGlowMeshes{},
                .glowStrength = bounce,
                .scrollU = scroll,
                .scrollV = ramp,
                .light = LayerLight{.color = {bounce, bounce, 1.f}},
                .scrollingMesh = 1},
          Layer{.scrollU = random, .scrollV = frames, .light = LayerLight{.tint = true, .color = {0.8f, 0.f, 0.8f}}}},
         {.glowStrength = wave, .scrollU = scroll, .scrollingMesh = -1}},
    };
    const std::string text = WriteRenderStylesJson(looks);
    INFO(text);
    // Each layer stays on one line, its shapes in it, with the numbers of the file.
    CHECK(text.find(R"("opacity": {"shape": "wave", "from": -0.2, "to": 1.2, "seconds": 7.853982})") !=
          std::string::npos);
    CHECK(text.find(R"("glowMesh": "all")") != std::string::npos);
    CHECK(text.find(R"("light": {"tint": [0.8, 0, 0.8]})") != std::string::npos);
    CHECK(text.find(R"("scrollV": {"shape": "frames", "count": 4, "step": 0.25, "secondsEach": 0.08})") !=
          std::string::npos);
    CHECK(text.find(R"("scrollingMesh": "none")") != std::string::npos);
    const ReadResult result = Read(text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    CHECK(result.looks == looks);
    CHECK(WriteRenderStylesJson(result.looks) == text);
}

TEST_CASE("A field of another shape is a warning [looks][files]")
{
    const ReadResult result = Read(File(R"([{"name": "a", "layers": [{"kind": "texture",
        "glowStrength": {"shape": "scroll", "perSecond": 0.5, "seconds": 2}}]}])"));
    INFO(Describe(result.issues));
    CHECK_FALSE(Data::Items::HasFatal(result.issues));
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[0].layers[0].glowStrength.seconds") != nullptr);
}
