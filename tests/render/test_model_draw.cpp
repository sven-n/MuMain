#include "stdafx.h"

#include <doctest.h>

#include "Render/Models/ModelDraw.h"
#include "Render/Models/ZzzBMD.h"

#include <memory>
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
    int clothes = 0;

    RecordingDrawer drawer;
    {
        DrawerScope scope(drawer);
        model->RenderBodyShadow(2, 3, 4, 5, &clothes, 6, 0.5f);
    }

    REQUIRE(drawer.shadows.size() == 1);
    const Render::ModelDrawing::ShadowDraw& draw = drawer.shadows.front();
    CHECK(draw.model == model.get());
    CHECK(draw.blendMesh == 2);
    CHECK(draw.hiddenMesh == 3);
    CHECK(draw.startMesh == 4);
    CHECK(draw.endMesh == 5);
    CHECK(draw.clothes == &clothes);
    CHECK(draw.clothesCount == 6);
    CHECK(draw.alpha == doctest::Approx(0.5f));
    CHECK(draw.bodyOrigin[2] == doctest::Approx(300.f));
    CHECK(draw.bodyScale == doctest::Approx(1.25f));
    CHECK(draw.lod == 2);
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
