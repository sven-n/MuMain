#include "stdafx.h"

#include "doctest.h"

#include "ItemModelDatabaseScope.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Render/Items/ItemRenderStyles.h"
#include "Render/Models/DrawLook.h"
#include "Render/Models/ModelDraw.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Sprites/TextureScript.h"

#include <cstdint>
#include <span>
#include <vector>

// The chrome of the Magic Gladiator sets (stormCrow, thunderHawk, hurricane), drawn mesh by mesh: under a texture
// with holes, over the others, with RenderBody's texture scripts.
namespace
{
using Render::ModelDrawing::MeshDraw;

constexpr int MeshCount = 5;
constexpr int ImageSize = 2;
constexpr int ArmorGroup = 8;

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

// Two textures of a 2 x 2 RGBA image, one with a pixel the alpha test drops, one solid; loaded without a GPU.
class TestTextures
{
public:
    TestTextures()
    {
        Render::Sprites::SetGpuUploadsSkipped(true);
        withHoles = Load(L"Data\\Item\\test_set_holes.tga", true);
        solid = Load(L"Data\\Item\\test_set_solid.tga", false);
    }

    ~TestTextures()
    {
        Bitmaps.UnloadImage(withHoles, true);
        Bitmaps.UnloadImage(solid, true);
        Render::Sprites::SetGpuUploadsSkipped(false);
    }

    TestTextures(const TestTextures&) = delete;
    TestTextures& operator=(const TestTextures&) = delete;

    GLuint withHoles = 0;
    GLuint solid = 0;

private:
    static GLuint Load(const wchar_t* name, bool hole)
    {
        std::vector<std::uint8_t> rgba(static_cast<std::size_t>(ImageSize) * ImageSize * 4, 255);
        if (hole)
        {
            rgba[3] = 0;
        }
        return Bitmaps.LoadImagePixels(name, rgba, ImageSize, ImageSize, true);
    }
};

// A Storm Crow armor whose meshes are: 0 with holes, 1 solid with an alpha channel, 2 with holes but glowing by its
// texture script (_R), 3 hidden by its texture script (_H), 4 with holes and the draw's hidden mesh.
class TestSet
{
public:
    explicit TestSet(const TestTextures& textures)
    {
        Data::Items::ItemModelDefinition entry;
        entry.group = ArmorGroup;
        entry.number = ITEM_STORM_CROW_ARMOR - ITEM_ARMOR;
        // A model entry needs a file to count (ItemModelDefinition::Exists).
        entry.file = "Data/Player/ArmorMale16.bmd";
        entry.renderStyle = "stormCrow";
        g_ItemModelDatabase.Build(std::span(&entry, 1), Data::Effects::GlowColorList{});

        REQUIRE(m_glowing.parsingTScriptA(m_glowingName));
        REQUIRE(m_hidden.parsingTScriptA(m_hiddenName));
        for (int mesh = 0; mesh < MeshCount; ++mesh)
        {
            m_meshes[mesh].NumTriangles = 1;
            m_meshes[mesh].Texture = static_cast<short>(mesh);
            m_meshes[mesh].NoneBlendMesh = false;
            m_textures[mesh] = textures.withHoles;
        }
        m_textures[1] = textures.solid;
        m_meshes[2].m_csTScript = &m_glowing;
        m_meshes[3].m_csTScript = &m_hidden;
        model.NumMeshs = MeshCount;
        model.Meshs = m_meshes;
        model.IndexTexture = m_textures;
        object.Type = MODEL_STORM_CROW_ARMOR;
        look.hiddenMesh = 4;
    }

    ~TestSet()
    {
        for (Mesh_t& mesh : m_meshes)
        {
            mesh.m_csTScript = nullptr;
        }
        model.NumMeshs = 0;
        model.Meshs = nullptr;
        model.IndexTexture = nullptr;
    }

    TestSet(const TestSet&) = delete;
    TestSet& operator=(const TestSet&) = delete;

    std::vector<MeshDraw> Draw(int renderType)
    {
        RecordingDrawer drawer;
        {
            const DrawerScope scope(drawer);
            CHECK(Render::Items::Styles::Render(&model, &object, look, MODEL_STORM_CROW_ARMOR, 1.f, renderType));
        }
        return drawer.draws;
    }

    BMD model;
    OBJECT object;
    DrawLook look;

private:
    ItemModelDatabaseScope m_database;
    char m_glowingName[32] = "set_R.tga";
    char m_hiddenName[32] = "set_H.tga";
    TextureScriptParsing m_glowing;
    TextureScriptParsing m_hidden;
    Mesh_t m_meshes[MeshCount]{};
    GLuint m_textures[MeshCount]{};
};

bool IsChrome(const MeshDraw& draw)
{
    return (draw.flags & RENDER_CHROME) != 0;
}
} // namespace

TEST_CASE("The set chrome goes under a texture with holes and over a solid or glowing one [render][items][set_chrome]")
{
    const TestTextures textures;
    REQUIRE(Bitmaps.FindTexture(textures.withHoles) != nullptr);
    CHECK(Bitmaps.FindTexture(textures.withHoles)->hasHoles);
    CHECK_FALSE(Bitmaps.FindTexture(textures.solid)->hasHoles);
    TestSet set(textures);

    const std::vector<MeshDraw> draws = set.Draw(RENDER_TEXTURE);

    // The hidden meshes 3 and 4 are not drawn.
    REQUIRE(draws.size() == 6);
    CHECK(draws[0].mesh == 0);
    CHECK(IsChrome(draws[0]));
    CHECK((draws[0].flags & RENDER_TWOSIDED) != 0);
    CHECK(draws[1].mesh == 0);
    CHECK_FALSE(IsChrome(draws[1]));
    // A solid texture with an alpha channel: the chrome added over it.
    CHECK(draws[2].mesh == 1);
    CHECK_FALSE(IsChrome(draws[2]));
    CHECK(draws[3].mesh == 1);
    CHECK(IsChrome(draws[3]));
    CHECK((draws[3].flags & RENDER_BRIGHT) != 0);
    // A glowing mesh blends itself and gets the chrome added over it.
    CHECK(draws[4].mesh == 2);
    CHECK_FALSE(IsChrome(draws[4]));
    CHECK(draws[4].blendMesh == 2);
    CHECK(draws[5].mesh == 2);
    CHECK(IsChrome(draws[5]));
}

TEST_CASE("A see-through set gets every chrome over its texture, a doppelganger none [render][items][set_chrome]")
{
    const TestTextures textures;
    TestSet set(textures);
    set.look.alpha = 0.5f;

    const std::vector<MeshDraw> draws = set.Draw(RENDER_TEXTURE);

    REQUIRE(draws.size() == 6);
    for (std::size_t pass = 0; pass < draws.size(); pass += 2)
    {
        CHECK_FALSE(IsChrome(draws[pass]));
        CHECK(IsChrome(draws[pass + 1]));
    }

    RecordingDrawer drawer;
    {
        const DrawerScope scope(drawer);
        CHECK_FALSE(Render::Items::Styles::Render(&set.model, &set.object, set.look, MODEL_STORM_CROW_ARMOR, 1.f,
                                                  RENDER_TEXTURE | RENDER_DOPPELGANGER));
    }
    CHECK(drawer.draws.empty());
}
