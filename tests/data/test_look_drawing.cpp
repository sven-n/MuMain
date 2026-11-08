#include "stdafx.h"

#include "doctest.h"

#include "ItemModelDatabaseScope.h"
#include "LogCapture.h"

#include "Core/Globals/_enum.h"
#include "Core/Utilities/Random.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/LookData/LookEffects.h"
#include "Data/GameData/LookData/LookStore.h"
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
    explicit TestSword(const char* style)
    {
        Data::Items::ItemModelDefinition entry;
        entry.group = 0;
        entry.number = 0;
        // A model entry needs a file to count (ItemModelDefinition::Exists).
        entry.file = "Data/Item/Sword01.bmd";
        entry.renderStyle = style;
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
