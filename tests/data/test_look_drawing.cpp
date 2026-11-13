#include "stdafx.h"

#include "doctest.h"

#include "ItemModelDatabaseScope.h"
#include "LogCapture.h"

#include "Core/Globals/_enum.h"
#include "Core/Utilities/Random.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/PartModel.h"
#include "Data/GameData/LookData/LookEffects.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Items/ItemRenderStyles.h"
#include "Render/Items/LookDrawing.h"
#include "Render/Models/DrawLook.h"
#include "Render/Models/ModelDraw.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Sprites/TextureScript.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <cstdint>
#include <span>
#include <string>
#include <tuple>
#include <vector>

// Render styles that are data (Data/Effects/RenderStyles.json): their layers become the model draws the looks in code
// make, and a name in the data is found before the same name in code.
namespace
{
using namespace Data::Looks;
using Render::ModelDrawing::MeshDraw;

constexpr int MeshCount = 4;
constexpr int ImageSize = 2;

class RecordingDrawer final : public Render::ModelDrawing::ModelDrawer
{
public:
    void DrawMesh(const MeshDraw& draw) override
    {
        draws.push_back(draw);
    }

    void DrawShadow(const Render::ModelDrawing::ShadowDraw&) override {}

    std::vector<MeshDraw> draws;
};

struct DrawerScope
{
    explicit DrawerScope(Render::ModelDrawing::ModelDrawer& drawer)
    {
        Render::ModelDrawing::SetModelDrawer(&drawer);
    }
    ~DrawerScope()
    {
        Render::ModelDrawing::SetModelDrawer(nullptr);
    }
};

// The looks of g_LookStore for one test; none afterwards.
struct LookStoreScope
{
    explicit LookStoreScope(std::vector<LookDefinition> looks)
    {
        g_LookStore.SetRenderStyles(std::move(looks));
    }
    ~LookStoreScope()
    {
        g_LookStore.SetRenderStyles({});
    }
    LookStoreScope(const LookStoreScope&) = delete;
    LookStoreScope& operator=(const LookStoreScope&) = delete;
};

// What a mesh draw shows, field by field.
auto Fields(const MeshDraw& draw)
{
    return std::make_tuple(draw.mesh, draw.flags, draw.alpha, draw.blendMesh, draw.blendMeshAlpha, draw.blendMeshU,
                           draw.blendMeshV, draw.texture);
}

std::vector<int> Meshes(const std::vector<MeshDraw>& draws)
{
    std::vector<int> meshes;
    for (const MeshDraw& draw : draws)
    {
        meshes.push_back(draw.mesh);
    }
    return meshes;
}

// A sword (0,0) named `style` whose meshes are: 0 and 1 plain, 2 glowing by its texture script (_R), 3 hidden by its
// texture script (_H).
class TestSword
{
public:
    explicit TestSword(const char* style, std::map<std::string, std::string> values = {})
    {
        Data::Items::ItemModelDefinition entry;
        entry.group = 0;
        entry.number = 0;
        // A model entry needs a file to count (ItemModelDefinition::Exists).
        entry.file = "Data/Item/Sword01.bmd";
        entry.renderStyle = style;
        entry.renderStyleValues = std::move(values);
        g_ItemModelDatabase.Build(std::span(&entry, 1), Data::Effects::GlowColorList{});

        REQUIRE(m_glowing.parsingTScriptA(m_glowingName));
        REQUIRE(m_hidden.parsingTScriptA(m_hiddenName));
        for (int mesh = 0; mesh < MeshCount; ++mesh)
        {
            m_meshes[mesh].NumTriangles = 1;
            m_meshes[mesh].Texture = static_cast<short>(mesh);
            m_meshes[mesh].NoneBlendMesh = false;
        }
        m_meshes[2].m_csTScript = &m_glowing;
        m_meshes[3].m_csTScript = &m_hidden;
        model.NumMeshs = MeshCount;
        model.Meshs = m_meshes;
        model.IndexTexture = m_textures;
        object.Type = MODEL_SWORD;
    }

    ~TestSword()
    {
        for (Mesh_t& mesh : m_meshes)
        {
            mesh.m_csTScript = nullptr;
        }
        model.NumMeshs = 0;
        model.Meshs = nullptr;
        model.IndexTexture = nullptr;
    }

    TestSword(const TestSword&) = delete;
    TestSword& operator=(const TestSword&) = delete;

    // The draws of the item's render style, drawn with `alpha` and `renderType`; `look` gets what it leaves.
    std::vector<MeshDraw> Draw(DrawLook& look, float alpha = 1.f, int renderType = RENDER_TEXTURE)
    {
        RecordingDrawer drawer;
        {
            const DrawerScope scope(drawer);
            CHECK(Render::Items::Styles::Render(&model, &object, look, MODEL_SWORD, alpha, renderType));
        }
        return drawer.draws;
    }

    BMD model;
    OBJECT object;

private:
    ItemModelDatabaseScope m_database;
    char m_glowingName[32] = "sword_R.tga";
    char m_hiddenName[32] = "sword_H.tga";
    TextureScriptParsing m_glowing;
    TextureScriptParsing m_hidden;
    Mesh_t m_meshes[MeshCount]{};
    GLuint m_textures[MeshCount]{};
};
} // namespace

TEST_CASE("A layer of every mesh draws them with the texture scripts, leaving out the item's hidden mesh [looks]")
{
    const LookStoreScope looks({{"testLook", {Layer{}}, {}}});
    TestSword sword("testLook");
    DrawLook look;
    look.hiddenMesh = 1;
    const std::vector<MeshDraw> draws = sword.Draw(look);
    // Mesh 1 is the item's hidden mesh and mesh 3 hidden by its script; mesh 2 glows by its script.
    CHECK(Meshes(draws) == std::vector<int>{0, 2});
    REQUIRE(draws.size() == 2);
    CHECK(draws[0].flags == RENDER_TEXTURE);
    CHECK(draws[1].blendMesh == 2);
}

TEST_CASE("A layer of listed meshes draws each of them in that order, as RenderMesh does [looks]")
{
    const LookStoreScope looks({{"testLook",
                                 {Layer{.kind = LayerKind::Reflection,
                                        .blend = Blend::Glow,
                                        .meshes = {{3, 0}},
                                        .opacity = 0.5f,
                                        .glowMesh = 0,
                                        .glowStrength = 0.25f}},
                                 {}}});
    TestSword sword("testLook");
    DrawLook look;
    const std::vector<MeshDraw> draws = sword.Draw(look);
    // Listed meshes are drawn without the texture scripts: mesh 3 too.
    REQUIRE(Meshes(draws) == std::vector<int>{3, 0});
    for (const MeshDraw& draw : draws)
    {
        CHECK(draw.flags == (RENDER_BRIGHT | RENDER_CHROME));
        CHECK(draw.alpha == 0.5f);
        CHECK(draw.blendMesh == 0);
        CHECK(draw.blendMeshAlpha == 0.25f);
    }
}

TEST_CASE("A layer of all meshes but some leaves those out and keeps the texture scripts [looks]")
{
    const LookStoreScope looks(
        {{"oneOut", {Layer{.meshes = {{0}, true}}}, {}}, {"twoOut", {Layer{.meshes = {{0, 2}, true}}}, {}}});
    DrawLook look;
    look.hiddenMesh = 1;
    {
        TestSword sword("oneOut");
        // As RenderBody leaving out mesh 0: the item's hidden mesh is drawn.
        CHECK(Meshes(sword.Draw(look)) == std::vector<int>{1, 2});
    }
    {
        TestSword sword("twoOut");
        CHECK(Meshes(sword.Draw(look)) == std::vector<int>{1});
    }
}

TEST_CASE("A layer takes the item's values, the drawing's opacity and render type, or its own numbers [looks]")
{
    const LookStoreScope looks(
        {{"testLook",
          {Layer{.meshes = {{0}}},
           Layer{.blend = Blend::AsDrawing, .meshes = {{1}}, .opacity = FromDrawing{}, .glowMesh = NoGlowMesh{}},
           Layer{.kind = LayerKind::Reflection,
                 .motion = Motion::Swirl,
                 .meshes = {{1}},
                 .glowMesh = 1,
                 .glowStrength = 1.f,
                 .scrollU = 0.5f,
                 .scrollV = 0.75f}},
          {}}});
    TestSword sword("testLook");
    DrawLook look;
    look.alpha = 0.8f;
    look.blendMesh = 3;
    look.blendMeshLight = 0.6f;
    look.blendMeshU = 0.1f;
    look.blendMeshV = 0.2f;
    const std::vector<MeshDraw> draws = sword.Draw(look, 0.4f, RENDER_DOPPELGANGER);
    REQUIRE(draws.size() == 3);
    CHECK(Fields(draws[0]) == std::make_tuple(0, RENDER_TEXTURE, 0.8f, 3, 0.6f, 0.1f, 0.2f, -1));
    CHECK(Fields(draws[1]) == std::make_tuple(1, RENDER_DOPPELGANGER, 0.4f, -1, 0.6f, 0.1f, 0.2f, -1));
    CHECK(Fields(draws[2]) == std::make_tuple(1, RENDER_CHROME4, 0.8f, 1, 1.f, 0.5f, 0.75f, -1));
}

TEST_CASE("A render style leaves its item values for what is drawn after it [looks]")
{
    const LookStoreScope looks(
        {{"testLook", {Layer{}}, {.opacity = 0.5f, .glowMesh = 1, .glowStrength = 1.f, .hiddenMesh = -1}}});
    TestSword sword("testLook");
    DrawLook look;
    look.blendMesh = 2;
    look.blendMeshLight = 0.3f;
    look.blendMeshU = 0.7f;
    look.hiddenMesh = 3;
    sword.Draw(look);
    CHECK(look.alpha == 0.5f);
    CHECK(look.blendMesh == 1);
    CHECK(look.blendMeshLight == 1.f);
    CHECK(look.hiddenMesh == -1);
    // Values the style doesn't name stay as they were.
    CHECK(look.blendMeshU == 0.7f);
}

TEST_CASE("A name in the data is found before the same name in code, which it then replaces [looks]")
{
    // Any look still in code; the data look of its name draws mesh 2 alone.
    const auto all = Render::Items::Styles::GetNames();
    const auto inCode = std::ranges::find_if(all, &Render::Items::Styles::ExistsInCode);
    if (inCode == all.end())
    {
        MESSAGE("No render style is in code any more.");
        return;
    }
    const std::string name(*inCode);
    const LookStoreScope looks({{name, {Layer{.meshes = {{2}}}}, {}}});
    CHECK(Render::Items::Styles::Exists(name));
    const auto names = Render::Items::Styles::GetNames();
    CHECK(std::count(names.begin(), names.end(), name) == 1);
    TestSword sword(name.c_str());
    DrawLook look;
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{2});
}

TEST_CASE("A name only in the data is a render style too [looks]")
{
    CHECK_FALSE(Render::Items::Styles::Exists("onlyInData"));
    const LookStoreScope looks({{"onlyInData", {Layer{}}, {}}});
    CHECK(Render::Items::Styles::Exists("onlyInData"));
    const auto names = Render::Items::Styles::GetNames();
    CHECK(std::is_sorted(names.begin(), names.end()));
    CHECK(std::find(names.begin(), names.end(), "onlyInData") != names.end());
}

TEST_CASE("A layer draws with the texture file it names, and is skipped when the file is missing [looks]")
{
    Render::Sprites::SetGpuUploadsSkipped(true);
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(ImageSize) * ImageSize * 4, 255);
    const GLuint texture =
        Bitmaps.LoadImagePixels(L"Data\\Item\\test_look_layer.jpg", rgba, ImageSize, ImageSize, true);
    {
        const LookStoreScope looks(
            {{"testLook",
              {Layer{.meshes = {{0}}, .texture = "Item/test_look_layer.jpg"},
               Layer{.meshes = {{1}}, .texture = "Item/no_such_texture.jpg"}, Layer{.meshes = {{2}}}},
              {}}});
        Render::Items::Looks::PrepareTextures();
        TestSword sword("testLook");
        DrawLook look;
        const std::vector<MeshDraw> draws = sword.Draw(look);
        REQUIRE(Meshes(draws) == std::vector<int>{0, 2});
        CHECK(draws[0].texture == static_cast<int>(texture));
        CHECK(draws[1].texture == -1);
    }
    Bitmaps.UnloadImage(texture, true);
    Render::Sprites::SetGpuUploadsSkipped(false);
}

TEST_CASE("A layer of every mesh, the item's hidden one too, draws the item's hidden mesh [looks]")
{
    const LookStoreScope looks({{"testLook", {Layer{.meshes = {{}, false, true}}}, {}}});
    TestSword sword("testLook");
    DrawLook look;
    look.hiddenMesh = 1;
    // Mesh 3 stays hidden by its texture script.
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0, 1, 2});
}

namespace
{
// WorldTime for one test, as the clock the shapes read.
struct WorldTimeScope
{
    explicit WorldTimeScope(double time) : m_previous(WorldTime)
    {
        WorldTime = time;
    }
    ~WorldTimeScope()
    {
        WorldTime = m_previous;
    }
    WorldTimeScope(const WorldTimeScope&) = delete;
    WorldTimeScope& operator=(const WorldTimeScope&) = delete;

private:
    double m_previous;
};

constexpr double TestTime = 1234567.25;
} // namespace

TEST_CASE("Values that change over time take the clock, as the looks in code wrote them [looks]")
{
    const Shape wave{.kind = ShapeKind::Wave, .from = -0.2, .to = 1.0, .seconds = 6.283185};
    const Shape bounce{.kind = ShapeKind::Bounce, .from = 0.1, .to = 0.4, .seconds = 3.1415925};
    const Shape scroll{.kind = ShapeKind::Scroll, .perSecond = 0.5};
    const Shape ramp{.kind = ShapeKind::Ramp, .from = 0.0, .to = -1.0, .seconds = 2.0};
    const Shape frames{.kind = ShapeKind::Frames, .seconds = 0.08, .step = 0.25, .count = 4};
    const LookStoreScope looks(
        {{"testLook",
          {Layer{.meshes = {{0}}, .opacity = wave, .glowStrength = bounce, .scrollU = scroll, .scrollV = ramp},
           Layer{.meshes = {{1}}, .scrollV = frames}},
          {.glowStrength = wave}}});
    TestSword sword("testLook");
    const WorldTimeScope clock(TestTime);
    DrawLook look;
    const std::vector<MeshDraw> draws = sword.Draw(look);
    REQUIRE(draws.size() == 2);
    // The numbers of the code each shape stands for.
    const float pulse = sinf(static_cast<float>(TestTime * 0.001f)) * 0.6f + 0.4f;
    CHECK(draws[0].alpha == pulse);
    CHECK(draws[0].blendMeshAlpha == 0.1f + fabsf(sinf(static_cast<float>(TestTime * 0.001f))) * 0.3f);
    CHECK(draws[0].blendMeshU == static_cast<float>(TestTime * 0.0005f));
    CHECK(draws[0].blendMeshV == -static_cast<int>(TestTime) % 2000 * 0.0005f);
    CHECK(draws[1].blendMeshV == ((static_cast<int>(TestTime * 0.05) % 16) / 4) * 0.25f);
    CHECK(look.blendMeshLight == pulse);
}

TEST_CASE("A zigzag folds back at its To, a falling one the same way downwards, and one from a value to the same "
          "value stays there [looks]")
{
    const Shape up{.kind = ShapeKind::Zigzag, .from = 0.0, .to = 1.0, .seconds = 2.0};
    const Shape down{.kind = ShapeKind::Zigzag, .from = 0.0, .to = -1.0, .seconds = 2.0};
    const Shape flat{.kind = ShapeKind::Zigzag, .from = 0.5, .to = 0.5, .seconds = 2.0};
    CHECK(Render::Items::Looks::ShapeAtClock(up, 500.0) == doctest::Approx(0.25));
    CHECK(Render::Items::Looks::ShapeAtClock(up, 2500.0) == doctest::Approx(0.75));
    for (const double time : {0.0, 500.0, 1500.0, 2500.0, 3999.0, TestTime})
    {
        CAPTURE(time);
        const float rising = Render::Items::Looks::ShapeAtClock(up, time);
        CHECK(rising >= 0.f);
        CHECK(rising <= 1.f);
        CHECK(Render::Items::Looks::ShapeAtClock(down, time) == -rising);
        CHECK(Render::Items::Looks::ShapeAtClock(flat, time) == 0.5f);
    }
}

TEST_CASE("A random value takes Random::DrawRand of the object, with the next salt for each one [looks]")
{
    const Shape random{.kind = ShapeKind::Random, .from = 0.0, .to = 0.9, .step = 0.1};
    const LookStoreScope looks({{"testLook", {Layer{.meshes = {{0}}, .scrollU = random, .scrollV = random}}, {}}});
    TestSword sword("testLook");
    DrawLook look;
    const std::vector<MeshDraw> draws = sword.Draw(look);
    REQUIRE(draws.size() == 1);
    CHECK(draws[0].blendMeshU == static_cast<float>(Random::DrawRand(&sword.object, 1) % 10) * 0.1f);
    CHECK(draws[0].blendMeshV == static_cast<float>(Random::DrawRand(&sword.object, 2) % 10) * 0.1f);
}

TEST_CASE("A layer's light stays for the layers after it, and a tint takes the model's light [looks]")
{
    const LookStoreScope looks(
        {{"testLook",
          {Layer{.meshes = {{0}}}, Layer{.meshes = {{1}}, .light = LayerLight{.color = {1.f, 0.f, 0.2f}}},
           Layer{.meshes = {{0}}},
           Layer{.meshes = {{1}}, .light = LayerLight{.tint = true, .color = {0.8f, 0.f, 0.8f}}}},
          {}}});
    TestSword sword("testLook");
    Vector(0.5f, 0.5f, 0.5f, sword.model.BodyLight);
    DrawLook look;
    const std::vector<MeshDraw> draws = sword.Draw(look);
    REQUIRE(draws.size() == 4);
    const auto light = [](const MeshDraw& draw)
    { return std::make_tuple(draw.bodyLight[0], draw.bodyLight[1], draw.bodyLight[2]); };
    CHECK(light(draws[0]) == std::make_tuple(0.5f, 0.5f, 0.5f));
    CHECK(light(draws[1]) == std::make_tuple(1.f, 0.f, 0.2f));
    CHECK(light(draws[2]) == std::make_tuple(1.f, 0.f, 0.2f));
    CHECK(light(draws[3]) == std::make_tuple(0.4f, 0.f, 0.4f));
    // The model keeps the light of the last layer.
    CHECK(sword.model.BodyLight[0] == 0.4f);
}

TEST_CASE("A look drawn mesh by mesh starts each mesh in the drawing's light, and a layer's light stays for the "
          "layers after it also on a mesh it doesn't draw [looks]")
{
    const LayerLight red{.color = {1.f, 0.f, 0.f}};
    const auto light = [](const MeshDraw& draw)
    { return std::make_tuple(draw.mesh, draw.bodyLight[0], draw.bodyLight[1], draw.bodyLight[2]); };
    SUBCASE("The light a layer leaves on one mesh doesn't reach the first layers of the next")
    {
        const LookStoreScope looks(
            {{"testLook",
              {Layer{.meshes = {{0, 1}}}, Layer{.meshes = {{0}}, .light = red},
               Layer{.kind = LayerKind::Reflection, .where = Where::UnderHoles, .meshes = {{0, 1}}}},
              {}}});
        TestSword sword("testLook");
        Vector(0.5f, 0.5f, 0.5f, sword.model.BodyLight);
        DrawLook look;
        const std::vector<MeshDraw> draws = sword.Draw(look);
        REQUIRE(draws.size() == 5);
        CHECK(light(draws[0]) == std::make_tuple(0, 0.5f, 0.5f, 0.5f));
        CHECK(light(draws[1]) == std::make_tuple(0, 1.f, 0.f, 0.f));
        CHECK(light(draws[2]) == std::make_tuple(0, 1.f, 0.f, 0.f));
        CHECK(light(draws[3]) == std::make_tuple(1, 0.5f, 0.5f, 0.5f));
        CHECK(light(draws[4]) == std::make_tuple(1, 1.f, 0.f, 0.f));
    }
    SUBCASE("A layer's light reaches the layers after it on a mesh it doesn't draw, also when it draws nothing")
    {
        Layer none{.light = red};
        none.meshes.none = true;
        const LookStoreScope looks(
            {{"testLook",
              {none, Layer{.meshes = {{0}}},
               Layer{.kind = LayerKind::Reflection, .where = Where::UnderHoles, .meshes = {{0}}}},
              {}}});
        TestSword sword("testLook");
        Vector(0.5f, 0.5f, 0.5f, sword.model.BodyLight);
        DrawLook look;
        const std::vector<MeshDraw> draws = sword.Draw(look);
        REQUIRE(draws.size() == 2);
        CHECK(light(draws[0]) == std::make_tuple(0, 1.f, 0.f, 0.f));
        CHECK(light(draws[1]) == std::make_tuple(0, 1.f, 0.f, 0.f));
    }
}

TEST_CASE("A layer's scrolling mesh is the model's while it draws, and the item values can leave one [looks]")
{
    const LookStoreScope looks(
        {{"whileDrawn", {Layer{.meshes = {{1}}, .scrollingMesh = 1}, Layer{.meshes = {{0}}}}, {}},
         {"left", {Layer{.meshes = {{0}}}}, {.scrollingMesh = 1}}});
    {
        TestSword sword("whileDrawn");
        sword.model.StreamMesh = -1;
        DrawLook look;
        const std::vector<MeshDraw> draws = sword.Draw(look);
        REQUIRE(draws.size() == 2);
        CHECK(draws[0].streamMesh == 1);
        CHECK(draws[1].streamMesh == -1);
        CHECK(sword.model.StreamMesh == -1);
    }
    {
        TestSword sword("left");
        sword.model.StreamMesh = -1;
        DrawLook look;
        sword.Draw(look);
        CHECK(sword.model.StreamMesh == 1);
    }
}

TEST_CASE("An item effect that is data sets the item values of its entries whose Only when holds, in their order, "
          "and the object's light in the update (design L13, L45) [looks]")
{
    LookDefinition effect{"byLevel", {}, {}};
    effect.itemValuesBefore = {
        {.onlyWhen = {.level = std::pair<int, int>{1, 15}}, .glowsLikeLevel = 7},
        // Reads the level the entry before set.
        {.onlyWhen = {.level = std::pair<int, int>{7, 7}}, .values = {.glowMesh = 1, .glowStrength = 0.5f}},
        {.onlyWhen = {.level = std::pair<int, int>{0, 0}}, .values = {.hiddenMesh = 1}, .drawingOpacity = 2.f}};
    effect.objectLight = std::array<Number, 3>{0.5f, 0.25f, 1.f};
    BMD model;
    OBJECT object;
    {
        DrawLook look;
        float alpha = 1.f;
        int level = 3;
        Render::Items::Looks::ApplyItemEffect(effect, &model, &object, look, alpha, level);
        CHECK(level == 7);
        CHECK(look.level == 7);
        CHECK(look.blendMesh == 1);
        CHECK(look.blendMeshLight == 0.5f);
        CHECK(look.hiddenMesh == -1);
        CHECK(alpha == 1.f);
    }
    {
        DrawLook look;
        float alpha = 1.f;
        int level = 0;
        Render::Items::Looks::ApplyItemEffect(effect, &model, &object, look, alpha, level);
        CHECK(level == 0);
        CHECK(look.hiddenMesh == 1);
        CHECK(alpha == 2.f);
    }
    Render::Items::Looks::SpawnItemEffect(effect, &object);
    CHECK(object.Light[0] == 0.5f);
    CHECK(object.Light[1] == 0.25f);
    CHECK(object.Light[2] == 1.f);
}

TEST_CASE("An item names an item effect that is data before one still in code (design L14) [looks]")
{
    // wingsOfDarkness stays code until its things on bones move (step 3): it spawns and sets no value.
    LookDefinition effect{"wingsOfDarkness", {}, {}};
    effect.itemValuesBefore = {{.values = {.glowStrength = 0.25f}}};
    g_LookStore.SetItemEffects({effect});
    REQUIRE(Render::Items::ItemEffects::ExistsInCode("wingsOfDarkness"));
    ItemModelDatabaseScope database;
    Data::Items::ItemModelDefinition entry;
    entry.group = 0;
    entry.number = 0;
    entry.file = "Data/Item/Sword01.bmd";
    entry.itemEffect = "wingsOfDarkness";
    g_ItemModelDatabase.Build(std::span(&entry, 1), Data::Effects::GlowColorList{});
    BMD model;
    OBJECT object;
    DrawLook look;
    float alpha = 1.f;
    int level = 0;
    CHECK(Render::Items::ItemEffects::Apply(&model, &object, look, MODEL_SWORD, alpha, level) ==
          Render::Items::ItemEffects::Result::Applied);
    CHECK(look.blendMeshLight == 0.25f);
    CHECK_FALSE(Render::Items::ItemEffects::Spawns(MODEL_SWORD));
    g_LookStore.SetItemEffects({});
    look = {};
    look.blendMeshLight = 0.5f;
    Render::Items::ItemEffects::Apply(&model, &object, look, MODEL_SWORD, alpha, level);
    CHECK(look.blendMeshLight == 0.5f);
    CHECK(Render::Items::ItemEffects::Spawns(MODEL_SWORD));
}

TEST_CASE("An item says whether its render style draws alone (design L47) [looks]")
{
    LookDefinition alone{"alone", {Layer{}}, {}};
    alone.drawsAlone = true;
    const LookStoreScope looks({alone, {"withOthers", {Layer{}}, {}}});
    {
        TestSword sword("alone");
        CHECK(Render::Items::Styles::DrawsAlone(MODEL_SWORD));
    }
    {
        TestSword sword("withOthers");
        CHECK_FALSE(Render::Items::Styles::DrawsAlone(MODEL_SWORD));
    }
    CHECK_FALSE(Render::Items::Styles::DrawsAlone(MODEL_PLAYER));

    // A part model's style too (design L28).
    const ItemModelDatabaseScope restoreModels;
    Data::Items::PartModel head;
    head.model.name = "grade15Head";
    head.model.file = "Data/Item/Sword01.bmd";
    head.renderStyle = "alone";
    g_ItemModelDatabase.SetPartModels(std::span(&head, 1));
    CHECK(Render::Items::Styles::DrawsAlone(MODEL_15GRADE_ARMOR_OBJ_HEAD));
    g_ItemModelDatabase.SetPartModels({});
}

TEST_CASE("A layer of no mesh draws nothing until an item's values give it meshes (design L12) [looks]")
{
    const LookStoreScope looks(
        {{"someItems",
          {Layer{.meshes = {{0}}}, Layer{.meshes = {.none = true}, .eachItem = {{"meshes", "more"}}}},
          {}}});
    {
        TestSword sword("someItems");
        DrawLook look;
        CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0});
    }
    {
        TestSword sword("someItems", {{"more", "[1]"}});
        DrawLook look;
        CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0, 1});
    }
}

TEST_CASE("A layer can draw in the model's light at a brightness, a grey of it for a black light (design L16) "
          "[looks]")
{
    const LookStoreScope looks(
        {{"cloak", {Layer{.meshes = {{0}}, .light = LayerLight{.brightness = Number{0.8f}}}}, {}}});
    TestSword sword("cloak");
    Vector(0.5f, 0.25f, 0.f, sword.model.BodyLight);
    DrawLook look;
    std::vector<MeshDraw> draws = sword.Draw(look);
    REQUIRE(draws.size() == 1);
    CHECK(draws[0].bodyLight[0] == 0.5f * (0.8f / 0.5f));
    CHECK(draws[0].bodyLight[1] == 0.25f * (0.8f / 0.5f));
    Vector(0.f, 0.f, 0.f, sword.model.BodyLight);
    draws = sword.Draw(look);
    CHECK(draws[0].bodyLight[2] == 0.8f);
}

TEST_CASE("A layer's value can be the item's times a shape (design L16) [looks]")
{
    const Shape twice{.kind = ShapeKind::Ramp, .from = 2.0, .to = 2.0, .seconds = 1.0, .timesItem = true};
    const LookStoreScope looks({{"times", {Layer{.meshes = {{0}}, .opacity = twice}}, {}}});
    TestSword sword("times");
    DrawLook look;
    look.alpha = 0.25f;
    const std::vector<MeshDraw> draws = sword.Draw(look);
    REQUIRE(draws.size() == 1);
    CHECK(draws[0].alpha == 0.5f);
}

TEST_CASE("A layer can be only for a character without a buff (design L11) [looks]")
{
    const LookStoreScope looks({{"arrows",
                                 {Layer{.meshes = {{0}}, .onlyWhen = {.buff = LookBuff::InfinityArrow}},
                                  Layer{.meshes = {{1}}, .onlyWhen = {.withoutBuff = LookBuff::InfinityArrow}}},
                                 {}}});
    TestSword sword("arrows");
    DrawLook look;
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{1});
    sword.object.m_BuffMap.RegisterBuff(eBuff_InfinityArrow);
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0});
    sword.object.m_BuffMap.UnRegisterBuff(eBuff_InfinityArrow);
}

TEST_CASE("A look can leave a light on the model, a color or a tint of the light it was handed (design L46) "
          "[looks]")
{
    const LookStoreScope looks({{"gray",
                                 {Layer{.meshes = {{0}}, .light = LayerLight{.color = {1.f, 0.7f, 0.4f}}}},
                                 {.light = LayerLight{.color = {0.7f, 0.7f, 0.7f}}}},
                                {"tinted",
                                 {Layer{.meshes = {{0}}, .light = LayerLight{.color = {1.f, 0.7f, 0.4f}}}},
                                 {.light = LayerLight{.tint = true, .color = {1.f, 1.f, 0.5f}}}}});
    {
        TestSword sword("gray");
        Vector(0.5f, 0.5f, 0.5f, sword.model.BodyLight);
        DrawLook look;
        const std::vector<MeshDraw> draws = sword.Draw(look);
        REQUIRE(draws.size() == 1);
        // The layer draws in its own light; the look leaves its gray after it.
        CHECK(draws[0].bodyLight[1] == 0.7f);
        CHECK(draws[0].bodyLight[2] == 0.4f);
        CHECK(sword.model.BodyLight[0] == 0.7f);
        CHECK(sword.model.BodyLight[1] == 0.7f);
        CHECK(sword.model.BodyLight[2] == 0.7f);
    }
    {
        TestSword sword("tinted");
        Vector(0.5f, 0.5f, 0.5f, sword.model.BodyLight);
        DrawLook look;
        sword.Draw(look);
        CHECK(sword.model.BodyLight[0] == 0.5f);
        CHECK(sword.model.BodyLight[1] == 0.5f);
        CHECK(sword.model.BodyLight[2] == 0.25f);
    }
}

TEST_CASE("A glow mesh of every mesh draws each mesh as a glowing one [looks]")
{
    const LookStoreScope looks({{"testLook", {Layer{.meshes = {{0}}, .glowMesh = AllGlowMeshes{}}}, {}}});
    TestSword sword("testLook");
    DrawLook look;
    const std::vector<MeshDraw> draws = sword.Draw(look);
    REQUIRE(draws.size() == 1);
    // BMD::DrawMesh draws every mesh as the glowing one below -1.
    CHECK(draws[0].blendMesh == -2);
}

TEST_CASE("A flowing texture or frames of the gallery move on their part, the glowing mesh of their layer [looks]")
{
    // BMD::DrawMesh moves the texture of the glowing mesh or of the scrolling one only.
    for (const EffectKind kind : {EffectKind::Flowing, EffectKind::Frames})
    {
        CAPTURE(static_cast<int>(kind));
        const LookStoreScope looks({{"testLook", {MakeEffectLayer(kind, {1})}, {}}});
        TestSword sword("testLook");
        // A frame after TestTime, where the frames are at the first one, which moves nothing.
        const WorldTimeScope clock(TestTime + 80.0);
        DrawLook look;
        const std::vector<MeshDraw> draws = sword.Draw(look);
        REQUIRE(draws.size() == 1);
        CHECK(draws[0].mesh == 1);
        CHECK(draws[0].blendMesh == 1);
        CHECK((draws[0].blendMeshU != 0.0f || draws[0].blendMeshV != 0.0f));
    }
}

TEST_CASE("A texture missing when its look was prepared is found when the look is prepared again after its file came "
          "[looks]")
{
    Render::Sprites::SetGpuUploadsSkipped(true);
    const LookDefinition later{"testLook", {Layer{.meshes = {{0}}, .texture = "Item/test_look_later.jpg"}}, {}};
    const LookStoreScope looks({later});
    TestSword sword("testLook");
    {
        const LogCapture log("data");
        Render::Items::Looks::PrepareTextures(later);
        CHECK_FALSE(log.LineWith("test_look_later.jpg").empty());
        DrawLook look;
        CHECK(sword.Draw(look).empty());
    }
    {
        // Still missing at the next change: looked for again, logged once.
        const LogCapture log("data");
        Render::Items::Looks::PrepareTextures(later);
        CHECK(log.LineWith("test_look_later.jpg").empty());
    }
    // The file is copied into Data while the game runs; the look is prepared again when it changes.
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(ImageSize) * ImageSize * 4, 255);
    const GLuint texture =
        Bitmaps.LoadImagePixels(L"Data\\Item\\test_look_later.jpg", rgba, ImageSize, ImageSize, true);
    {
        const LogCapture log("data");
        Render::Items::Looks::PrepareTextures(later);
        CHECK(log.LineWith("test_look_later.jpg").empty());
        DrawLook look;
        const std::vector<MeshDraw> draws = sword.Draw(look);
        REQUIRE(draws.size() == 1);
        CHECK(draws[0].texture == static_cast<int>(texture));
    }
    Bitmaps.UnloadImage(texture, true);
    Render::Sprites::SetGpuUploadsSkipped(false);
}

TEST_CASE("A render style not for a drawing leaves the item to the plain drawing, and one only for a monster's hands "
          "draws without the mark (design L11) [looks]")
{
    LookDefinition notForCopies{"notForCopies", {Layer{}}, {}};
    notForCopies.notFor.doppelganger = true;
    LookDefinition inHands{"inHands", {Layer{.blend = Blend::AsDrawing}}, {}};
    inHands.notFor.outsideMonsterHands = true;
    const LookStoreScope looks({notForCopies, inHands});
    const auto drawsOf = [](TestSword& sword, int renderType, bool& drawn)
    {
        RecordingDrawer drawer;
        DrawLook look;
        {
            const DrawerScope scope(drawer);
            drawn = Render::Items::Styles::Render(&sword.model, &sword.object, look, MODEL_SWORD, 1.f, renderType);
        }
        return drawer.draws;
    };
    bool drawn = false;
    {
        TestSword sword("notForCopies");
        CHECK_FALSE(drawsOf(sword, RENDER_TEXTURE, drawn).empty());
        CHECK(drawn);
        CHECK(drawsOf(sword, RENDER_TEXTURE | RENDER_DOPPELGANGER, drawn).empty());
        CHECK_FALSE(drawn);
    }
    {
        TestSword sword("inHands");
        CHECK(drawsOf(sword, RENDER_TEXTURE, drawn).empty());
        CHECK_FALSE(drawn);
        const std::vector<MeshDraw> draws = drawsOf(sword, RENDER_TEXTURE | RENDER_EXTRA, drawn);
        CHECK(drawn);
        REQUIRE_FALSE(draws.empty());
        for (const MeshDraw& draw : draws)
        {
            CHECK((draw.flags & RENDER_EXTRA) == 0);
        }
    }
}

TEST_CASE("A layer draws only while its Only when holds: the place, the level, the item's hidden mesh and a buff "
          "(design L11) [looks]")
{
    const LookStoreScope looks({{"conditions",
                                 {Layer{.meshes = {{0}}, .onlyWhen = {.place = Place::Worn}},
                                  Layer{.meshes = {{1}}, .onlyWhen = {.place = Place::InventoryAndGround}},
                                  Layer{.meshes = {{0}}, .onlyWhen = {.level = std::pair<int, int>{7, 9}}},
                                  Layer{.meshes = {{1}}, .onlyWhen = {.hiddenMesh = 1}},
                                  Layer{.meshes = {{0}}, .onlyWhen = {.buff = LookBuff::InfinityArrow}}},
                                 {}}});
    TestSword sword("conditions");
    DrawLook look;
    // Worn, at +0, without a hidden mesh or a buff.
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0});
    sword.model.HideSkin = true;
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{1});
    sword.model.HideSkin = false;
    look.level = 8;
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0, 0});
    look.level = 10;
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0});
    look.level = 0;
    look.hiddenMesh = 1;
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0, 1});
    look.hiddenMesh = -1;
    sword.object.m_BuffMap.RegisterBuff(eBuff_InfinityArrow);
    CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0, 0});
    sword.object.m_BuffMap.UnRegisterBuff(eBuff_InfinityArrow);
}

TEST_CASE("An item that sets values of its look draws the look with them, another item with the look's own (design "
          "L12) [looks]")
{
    const LookStoreScope looks({{"partsPerItem", {Layer{.meshes = {{0}}, .eachItem = {{"meshes", "parts"}}}}, {}}});
    {
        TestSword sword("partsPerItem", {{"parts", "[1, 2]"}});
        DrawLook look;
        CHECK(Meshes(sword.Draw(look)) == std::vector<int>{1, 2});
    }
    {
        TestSword sword("partsPerItem");
        DrawLook look;
        CHECK(Meshes(sword.Draw(look)) == std::vector<int>{0});
    }
}
