#include "stdafx.h"

#include <doctest.h>

#include "Engine/Physics/PhysicsManager.h"
#include "Render/Drawing/DrawCallLog.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Models/ModelDraw.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/TextureScript.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace
{
class RecordingDrawer final : public Render::ModelDrawing::ModelDrawer
{
public:
    void DrawMesh(const Render::ModelDrawing::MeshDraw& draw) override
    {
        draws.push_back(draw);
    }

    void DrawShadow(const Render::ModelDrawing::ShadowDraw& draw) override
    {
        shadows.push_back(draw);
    }

    std::vector<Render::ModelDrawing::MeshDraw> draws;
    std::vector<Render::ModelDrawing::ShadowDraw> shadows;
};

// A cloth of 2 by 3 points placed at x, without a simulation.
class GridCloth final : public CPhysicsCloth
{
public:
    explicit GridCloth(float x)
    {
        m_iNumHor = 2;
        m_iNumVer = 3;
        m_iNumVertices = m_iNumHor * m_iNumVer;
        m_pVertices = new CPhysicsVertex[m_iNumVertices];
        for (int i = 0; i < m_iNumVertices; ++i)
            m_pVertices[i].Init(x + static_cast<float>(i % 2) * 10.f, static_cast<float>(i / 2) * 10.f, 50.f);
    }
    ~GridCloth() override
    {
        Destroy();
    }
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
} // namespace

TEST_CASE("RenderMesh hands the active drawer the call and the model state it reads [render][model_draw]")
{
    auto model = std::make_unique<BMD>();
    Vector(0.8f, 0.6f, 0.4f, model->BodyLight);
    Vector(100.f, 200.f, 300.f, model->BodyOrigin);
    model->BodyScale = 1.25f;
    model->LightEnable = true;
    model->HideSkin = true;
    model->StreamMesh = 2;
    model->ActiveLod = 1;

    RecordingDrawer drawer;
    {
        DrawerScope scope(drawer);
        model->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, 0.5f, 1, 0.75f, 0.1f, 0.2f, 42);
    }

    REQUIRE(drawer.draws.size() == 1);
    const Render::ModelDrawing::MeshDraw& draw = drawer.draws.front();
    CHECK(draw.model == model.get());
    CHECK(draw.mesh == 3);
    CHECK(draw.flags == (RENDER_TEXTURE | RENDER_BRIGHT));
    CHECK(draw.alpha == doctest::Approx(0.5f));
    CHECK(draw.blendMesh == 1);
    CHECK(draw.blendMeshAlpha == doctest::Approx(0.75f));
    CHECK(draw.blendMeshU == doctest::Approx(0.1f));
    CHECK(draw.blendMeshV == doctest::Approx(0.2f));
    CHECK(draw.texture == 42);
    CHECK(draw.bodyLight[1] == doctest::Approx(0.6f));
    CHECK(draw.bodyOrigin[2] == doctest::Approx(300.f));
    CHECK(draw.bodyScale == doctest::Approx(1.25f));
    CHECK(draw.lightEnable);
    CHECK(draw.hideSkin);
    CHECK(draw.streamMesh == 2);
    CHECK(draw.lod == 1);
}

TEST_CASE("A faded bright or chrome pass dims the model's light for it and the meshes after it, before any drawer "
          "sees it [render][model_draw]")
{
    // One drawn mesh, and a second one whose texture is hidden.
    Mesh_t meshes[2]{};
    meshes[0].NumTriangles = 1;
    meshes[0].Texture = 0;
    meshes[0].NoneBlendMesh = false;
    meshes[1].NumTriangles = 1;
    meshes[1].Texture = 1;
    meshes[1].NoneBlendMesh = false;
    GLuint textures[2] = {BITMAP_CHROME, BITMAP_HIDE};
    BMD model;
    model.NumMeshs = 2;
    model.Meshs = meshes;
    model.IndexTexture = textures;
    Vector(0.8f, 0.6f, 0.4f, model.BodyLight);

    RecordingDrawer drawer;
    {
        DrawerScope scope(drawer);
        model.RenderMesh(0, RENDER_TEXTURE, 0.5f);                // not a chrome pass
        model.RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 1.f);  // not faded
        model.RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 0.5f); // draws nothing
        model.RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 0.5f); // dims
        model.RenderMesh(0, RENDER_TEXTURE);                      // keeps the dimmed light
    }

    REQUIRE(drawer.draws.size() == 5);
    for (int i = 0; i < 3; ++i)
        CHECK(drawer.draws[i].bodyLight[0] == doctest::Approx(0.8f));
    CHECK(drawer.draws[3].bodyLight[0] == doctest::Approx(0.4f));
    CHECK(drawer.draws[4].bodyLight[0] == doctest::Approx(0.4f));
    CHECK(model.BodyLight[0] == doctest::Approx(0.4f));

    model.NumMeshs = 0;
    model.Meshs = nullptr;
    model.IndexTexture = nullptr;
}

TEST_CASE("RenderBodyShadow hands the active drawer the call and the model state it reads [render][model_draw]")
{
    auto model = std::make_unique<BMD>();
    Vector(100.f, 200.f, 300.f, model->BodyOrigin);
    model->BodyScale = 1.25f;
    model->ActiveLod = 2;
    GridCloth clothes[2] = {GridCloth(0.f), GridCloth(100.f)};

    RecordingDrawer drawer;
    {
        DrawerScope scope(drawer);
        model->RenderBodyShadow(2, 3, 4, 5, clothes, 2, 0.5f);
    }

    REQUIRE(drawer.shadows.size() == 1);
    const Render::ModelDrawing::ShadowDraw& draw = drawer.shadows.front();
    CHECK(draw.model == model.get());
    CHECK(draw.blendMesh == 2);
    CHECK(draw.hiddenMesh == 3);
    CHECK(draw.startMesh == 4);
    CHECK(draw.endMesh == 5);
    // The cloth's points as the simulation left them, not the simulation.
    REQUIRE(draw.clothes.size() == 2);
    CHECK(draw.clothes[1].columns == 3);
    CHECK(draw.clothes[1].rows == 2);
    CHECK(draw.clothes[1].points[5][0] == doctest::Approx(110.f));
    CHECK(draw.clothes[1].points[5][1] == doctest::Approx(20.f));
    CHECK(draw.alpha == doctest::Approx(0.5f));
    CHECK(draw.bodyOrigin[2] == doctest::Approx(300.f));
    CHECK(draw.bodyScale == doctest::Approx(1.25f));
    CHECK(draw.lod == 2);
}

TEST_CASE("A shadow draws the casts it carries, whatever the frame's light is by then [render][model_draw]")
{
    auto model = std::make_unique<BMD>();
    GridCloth cloth(0.f);
    RecordingDrawer drawer;
    {
        DrawerScope scope(drawer);
        model->RenderBodyShadow(-1, -1, -1, -1, &cloth, 1, 1.f);
    }
    REQUIRE(drawer.shadows.size() == 1);
    Render::ModelDrawing::ShadowDraw draw = drawer.shadows.front();

    const auto countsOf = [&](const char* state)
    {
        Render::Drawing::DrawCallLog log;
        mu::SetRendererOverride(&log);
        model->DrawShadow(draw);
        mu::SetRendererOverride(nullptr);
        return std::count_if(log.calls.begin(), log.calls.end(), [&](const std::string& call)
                             { return call.starts_with("Triangles") && call.find(state) != std::string::npos; });
    };

    draw.flatStrength = 1.f;
    draw.lightCastCount = 0;
    CHECK(countsOf("groundShadow 10") == 1);
    CHECK(countsOf("groundShadow 01") == 0);

    draw.flatStrength = 0.f;
    draw.lightCastCount = 2;
    draw.lightCasts[0] = {0.f, 0.5f};
    draw.lightCasts[1] = {1.f, 0.25f};
    CHECK(countsOf("groundShadow 10") == 0);
    CHECK(countsOf("groundShadow 01") == 2);
}

TEST_CASE("Both draws carry the skinning the model's last Transform set up [render][model_draw]")
{
    extern float BoneScale;
    auto model = std::make_unique<BMD>();
    auto bones = std::make_unique<float[][3][4]>(MAX_BONES);
    vec3_t boxMin{};
    vec3_t boxMax{};
    OBB_t obb{};
    BoneScale = 1.5f;
    model->TransformCheap(bones.get(), boxMin, boxMax, &obb, true, 0.75f);
    BoneScale = 1.f;
    const unsigned int version = g_BoneTransformVersion;

    RecordingDrawer drawer;
    {
        DrawerScope scope(drawer);
        model->RenderMesh(0, RENDER_TEXTURE);
        model->RenderBodyShadow();
    }

    REQUIRE(drawer.draws.size() == 1);
    REQUIRE(drawer.shadows.size() == 1);
    for (const Render::ModelDrawing::ModelSkin& skin : {drawer.draws[0].skin, drawer.shadows[0].skin})
    {
        CHECK(skin.bones == bones.get());
        CHECK(skin.boneVersion == version);
        CHECK(skin.boneScale == doctest::Approx(1.5f));
        CHECK(skin.restPoseScale == doctest::Approx(0.75f));
        CHECK(skin.translate);
    }
}

TEST_CASE("Without a drawer set, the default drawer draws [render][model_draw]")
{
    RecordingDrawer drawer;
    Render::ModelDrawing::SetModelDrawer(&drawer);
    CHECK(&Render::ModelDrawing::ActiveModelDrawer() == &drawer);
    Render::ModelDrawing::SetModelDrawer(nullptr);
    CHECK(&Render::ModelDrawing::ActiveModelDrawer() != &drawer);
}

// RenderBody's rules, which RenderScriptedMesh and IsMeshHidden share with looks that draw mesh by mesh.
TEST_CASE("RenderBody skips hidden meshes, lets a glowing one blend itself and adds the shadow pass [render][model_draw]")
{
    char hiddenName[32] = "body_H.jpg";
    char glowingName[32] = "body_R.jpg";
    char shadowName[32] = "body_DC.jpg";
    TextureScriptParsing hidden;
    TextureScriptParsing glowing;
    TextureScriptParsing shadow;
    REQUIRE(hidden.parsingTScriptA(hiddenName));
    REQUIRE(glowing.parsingTScriptA(glowingName));
    REQUIRE(shadow.parsingTScriptA(shadowName));

    // Mesh 0 plain, 1 hidden by its script, 2 glowing, 3 with a shadow pass, 4 the hidden mesh of the call.
    Mesh_t meshes[5]{};
    for (int i = 0; i < 5; ++i)
    {
        meshes[i].NumTriangles = 1;
        meshes[i].Texture = static_cast<short>(i);
        meshes[i].NoneBlendMesh = false;
    }
    meshes[1].m_csTScript = &hidden;
    meshes[2].m_csTScript = &glowing;
    meshes[3].m_csTScript = &shadow;
    GLuint textures[5] = {BITMAP_CHROME, BITMAP_CHROME, BITMAP_CHROME, BITMAP_CHROME, BITMAP_CHROME};
    BMD model;
    model.NumMeshs = 5;
    model.Meshs = meshes;
    model.IndexTexture = textures;

    CHECK(model.IsMeshHidden(1, -1));
    CHECK(model.IsMeshHidden(4, 4));
    CHECK_FALSE(model.IsMeshHidden(0, 4));
    CHECK(model.IsMeshGlowing(2, -1));
    CHECK(model.IsMeshGlowing(0, 0));
    CHECK_FALSE(model.IsMeshGlowing(3, 0));

    RecordingDrawer drawer;
    {
        DrawerScope scope(drawer);
        model.RenderBody(RENDER_TEXTURE, 1.f, 0, 1.f, 0.f, 0.f, 4);
    }

    REQUIRE(drawer.draws.size() == 4);
    CHECK(drawer.draws[0].mesh == 0);
    CHECK(drawer.draws[0].blendMesh == 0);
    CHECK(drawer.draws[1].mesh == 2);
    CHECK(drawer.draws[1].blendMesh == 2);
    CHECK(drawer.draws[2].mesh == 3);
    CHECK(drawer.draws[2].flags == RENDER_TEXTURE);
    CHECK(drawer.draws[3].mesh == 3);
    CHECK(drawer.draws[3].flags == (RENDER_COLOR | RENDER_SHADOWMAP));

    for (Mesh_t& mesh : meshes)
    {
        mesh.m_csTScript = nullptr;
    }
    model.NumMeshs = 0;
    model.Meshs = nullptr;
    model.IndexTexture = nullptr;
}
