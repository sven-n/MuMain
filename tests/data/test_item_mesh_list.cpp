#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "../render/DrawerContract.h"

#include "UI/ItemTools/ItemMeshFields.h"
#include "UI/ItemTools/ItemMeshFocus.h"
#include "UI/ItemTools/ItemMeshPicking.h"
#include "UI/ItemTools/ItemMeshPreviewDrawer.h"
#include "UI/ItemTools/ItemMeshRows.h"

#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Render/Models/ModelDraw.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Sprites/TextureScript.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

using namespace MuEditor::Items;
using Render::ModelDrawing::MeshDraw;

namespace
{
// No texture is loaded under it in the tests.
constexpr GLuint UnloadedTexture = 999999;

// A model whose meshes and textures a test sets. They are taken back before the model goes, so its destructor frees
// nothing of theirs.
class TestModel
{
public:
    explicit TestModel(int meshCount)
        : m_meshes(meshCount), m_textures(meshCount), m_indexes(meshCount, UnloadedTexture),
          m_model(std::make_unique<BMD>())
    {
        m_model->NumMeshs = static_cast<short>(meshCount);
        m_model->Meshs = m_meshes.data();
        m_model->Textures = m_textures.data();
        m_model->IndexTexture = m_indexes.data();
        // Mesh_t's constructor leaves these to the model file reader.
        for (int mesh = 0; mesh < meshCount; ++mesh)
        {
            m_meshes[mesh].Texture = static_cast<short>(mesh);
            m_meshes[mesh].NoneBlendMesh = false;
        }
    }

    ~TestModel()
    {
        for (Mesh_t& mesh : m_meshes)
        {
            mesh.Triangles = nullptr;
            mesh.m_csTScript = nullptr;
        }
        m_model->Meshs = nullptr;
        m_model->Textures = nullptr;
        m_model->IndexTexture = nullptr;
        m_model->NumMeshs = 0;
    }

    TestModel(const TestModel&) = delete;
    TestModel& operator=(const TestModel&) = delete;

    BMD& Model()
    {
        return *m_model;
    }

    Mesh_t& Mesh(int mesh)
    {
        return m_meshes[mesh];
    }

    void SetTexture(int mesh, const char* name)
    {
        std::strncpy(m_textures[mesh].FileName, name, sizeof(m_textures[mesh].FileName) - 1);
    }

    void SetLoadedTexture(int mesh, GLuint texture)
    {
        m_indexes[mesh] = texture;
    }

private:
    std::vector<Mesh_t> m_meshes;
    std::vector<Texture_t> m_textures;
    std::vector<GLuint> m_indexes;
    std::unique_ptr<BMD> m_model;
};

class RecordingDrawer final : public Render::ModelDrawing::ModelDrawer
{
public:
    explicit RecordingDrawer(bool drawsLater = false) : m_drawsLater(drawsLater) {}

    [[nodiscard]] bool DrawsLater() const override
    {
        return m_drawsLater;
    }

    void DrawMesh(const MeshDraw& draw) override
    {
        draws.push_back(draw);
        alphaTests.push_back(glIsEnabled(GL_ALPHA_TEST) != 0);
    }

    void DrawShadow(const Render::ModelDrawing::ShadowDraw& draw) override
    {
        shadows.push_back(draw);
    }

    std::vector<MeshDraw> draws;
    std::vector<Render::ModelDrawing::ShadowDraw> shadows;
    // Whether the alpha test was on for each draw.
    std::vector<bool> alphaTests;

private:
    bool m_drawsLater;
};

// A 2 x 2 RGBA texture with a pixel the alpha test drops, loaded without a GPU.
class TextureWithHoles
{
public:
    static constexpr int Size = 2;

    TextureWithHoles()
    {
        Render::Sprites::SetGpuUploadsSkipped(true);
        std::vector<std::uint8_t> rgba(static_cast<std::size_t>(Size) * Size * 4, 255);
        rgba[3] = 0;
        index = Bitmaps.LoadImagePixels(L"Data\\Item\\test_mesh_list_holes.tga", rgba, Size, Size, true);
    }

    ~TextureWithHoles()
    {
        Bitmaps.UnloadImage(index, true);
        Render::Sprites::SetGpuUploadsSkipped(false);
    }

    TextureWithHoles(const TextureWithHoles&) = delete;
    TextureWithHoles& operator=(const TextureWithHoles&) = delete;

    GLuint index = 0;
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

MeshDraw DrawOf(const BMD& model, int mesh, int flags)
{
    MeshDraw draw;
    draw.model = &model;
    draw.mesh = mesh;
    draw.flags = flags;
    draw.blendMeshAlpha = 1.0f;
    for (float& light : draw.bodyLight)
    {
        light = 1.0f;
    }
    return draw;
}

// A valid CPU triangle for the real model drawer; storage stays alive until replay finishes.
struct TriangleGeometry
{
    float positions[3][3] = {{0.f, 0.f, 0.f}, {10.f, 0.f, 0.f}, {0.f, 10.f, 0.f}};
    std::array<Vertex_t, 3> vertices{};
    std::array<TexCoord_t, 3> texCoords{};
    Normal_t normal{};
    Triangle_t triangle{};

    TriangleGeometry()
    {
        triangle.Polygon = 3;
        normal.Normal[2] = 1.f;
        for (int i = 0; i < 3; ++i)
        {
            VectorCopy(positions[i], vertices[i].Position);
            triangle.VertexIndex[i] = static_cast<short>(i);
            triangle.TexCoordIndex[i] = static_cast<short>(i);
        }
    }

    void Attach(TestModel& model, int index)
    {
        Mesh_t& mesh = model.Mesh(index);
        mesh.NumVertices = 3;
        mesh.NumNormals = 1;
        mesh.NumTexCoords = 3;
        mesh.NumTriangles = 1;
        mesh.Vertices = vertices.data();
        mesh.Normals = &normal;
        mesh.TexCoords = texCoords.data();
        mesh.Triangles = &triangle;
    }

    MeshDraw Draw(const BMD& model, int mesh) const
    {
        MeshDraw draw = DrawOf(model, mesh, RENDER_TEXTURE);
        draw.cpu = {.vertices = positions, .vertexCount = 3};
        return draw;
    }
};

MeshPicking::Ray Downward(float x, float y, float z)
{
    return {{x, y, z}, {0.0f, 0.0f, -1.0f}};
}
} // namespace

TEST_CASE("The mesh list reads each mesh's texture, marks and counts [editor][items][mesh_list]")
{
    TestModel test(3);
    test.SetTexture(0, "Sword01.jpg");
    test.Mesh(0).NumVertices = 10;
    test.Mesh(0).NumTriangles = 8;
    char brightName[32] = "Sword01_R.tga";
    TextureScriptParsing bright;
    REQUIRE(bright.parsingTScriptA(brightName));
    test.SetTexture(1, brightName);
    test.Mesh(1).m_csTScript = &bright;
    test.Mesh(1).NoneBlendMesh = true;
    test.SetTexture(2, "hide_blade.jpg");

    const std::vector<ItemMeshRow> rows = DescribeMeshes(test.Model());

    REQUIRE(rows.size() == 3);
    CHECK(rows[0].mesh == 0);
    CHECK(rows[0].texture == "Sword01.jpg");
    CHECK(rows[0].vertices == 10);
    CHECK(rows[0].triangles == 8);
    CHECK_FALSE(rows[0].hidden);
    CHECK_FALSE(rows[0].glows);
    CHECK_FALSE(rows[0].noChrome);
    // No texture is loaded in the tests.
    CHECK(rows[0].textureFile.empty());
    CHECK(rows[0].textureWidth == 0);
    CHECK(rows[1].texture == "Sword01_R.tga");
    CHECK(rows[1].glows);
    CHECK(rows[1].noChrome);
    CHECK_FALSE(rows[1].scrolls);
    CHECK(rows[2].hidden);
}

TEST_CASE("A model that is not loaded has no mesh rows [editor][items][mesh_list]")
{
    const auto model = std::make_unique<BMD>();
    model->Meshs = nullptr;
    model->NumMeshs = 0;
    CHECK(DescribeMeshes(*model).empty());
}

TEST_CASE("The mesh list names the model entry's fields that name a mesh [editor][items][mesh_list]")
{
    Data::Items::ItemModelDefinition entry;
    entry.noneBlendMeshes = {1};
    entry.glow.meshes.only = {0, 2};
    entry.glow.meshes.hidden = 1;
    entry.glow.shineMeshes.hidden = 0;
    entry.glow.excellentMesh = 2;
    entry.glow.excellentMeshWithoutSkin = 1;

    CHECK(FindMeshUses(entry, 0) == (MeshUse::Glow | MeshUse::ShineAllBut));
    CHECK(FindMeshUses(entry, 1) == (MeshUse::NoneBlend | MeshUse::GlowAllBut | MeshUse::ExcellentWithoutSkin));
    CHECK(FindMeshUses(entry, 2) == (MeshUse::Glow | MeshUse::Excellent));
    CHECK(FindMeshUses(entry, 3) == 0);
}

TEST_CASE("A mesh field's number under the mouse is the one whose digits are there [editor][items][mesh_list]")
{
    CHECK(MeshFields::NumberAt("1, 12, 3", 0) == 1);
    CHECK_FALSE(MeshFields::NumberAt("1, 12, 3", 1));
    CHECK_FALSE(MeshFields::NumberAt("1, 12, 3", 2));
    CHECK(MeshFields::NumberAt("1, 12, 3", 3) == 12);
    CHECK(MeshFields::NumberAt("1, 12, 3", 4) == 12);
    CHECK(MeshFields::NumberAt("1, 12, 3", 7) == 3);
    CHECK_FALSE(MeshFields::NumberAt("1, 12, 3", 8));
    CHECK_FALSE(MeshFields::NumberAt("", 0));
}

TEST_CASE("The preview shows the mesh pointed at in the frame before, else the kept one [editor][items][mesh_list]")
{
    CItemMeshFocus focus;
    focus.BeginFrame(100);
    CHECK_FALSE(focus.Shown());

    // The preview of the same frame was drawn before the row was pointed at.
    focus.Point(2);
    CHECK_FALSE(focus.Shown());
    focus.BeginFrame(100);
    CHECK(focus.Shown() == 2);
    focus.BeginFrame(100);
    CHECK_FALSE(focus.Shown());

    focus.ToggleKept(1);
    CHECK(focus.Kept() == 1);
    CHECK(focus.Shown() == 1);
    focus.Point(3);
    focus.BeginFrame(100);
    CHECK(focus.Shown() == 3);
    focus.BeginFrame(100);
    CHECK(focus.Shown() == 1);
    focus.ToggleKept(1);
    CHECK_FALSE(focus.Kept());
}

TEST_CASE("A click on the preview keeps the mesh it hit, or lets go [editor][items][mesh_list]")
{
    CItemMeshFocus focus;
    focus.BeginFrame(100);
    focus.Pick(4);
    CHECK(focus.Kept() == 4);
    CHECK(focus.TakePicked());
    CHECK_FALSE(focus.TakePicked());

    focus.Pick(std::nullopt);
    CHECK_FALSE(focus.Kept());
    CHECK_FALSE(focus.TakePicked());
}

TEST_CASE("A double click on the preview takes back the pick of its first click [editor][items][mesh_list]")
{
    CItemMeshFocus focus;
    focus.BeginFrame(100);
    focus.ToggleKept(1);
    focus.Pick(4);
    CHECK(focus.TakePicked());
    focus.TakeBackPick();
    CHECK(focus.Kept() == 1);
    // Its row is scrolled to again.
    CHECK(focus.TakePicked());
    // Only the last pick is taken back.
    focus.TakeBackPick();
    CHECK(focus.Kept() == 1);

    // A first click on nothing let the kept mesh go.
    focus.Pick(std::nullopt);
    CHECK_FALSE(focus.Kept());
    focus.TakeBackPick();
    CHECK(focus.Kept() == 1);

    // A row clicked since the pick stays kept.
    focus.Pick(2);
    focus.ToggleKept(3);
    focus.TakeBackPick();
    CHECK(focus.Kept() == 3);

    // A mesh the model lost is not taken back.
    focus.ToggleKept(3);
    focus.ToggleKept(6);
    focus.Pick(2);
    focus.ForgetMeshesFrom(5);
    focus.TakeBackPick();
    CHECK_FALSE(focus.Kept());
}

TEST_CASE("A mesh the model no longer has, or another item, is forgotten [editor][items][mesh_list]")
{
    CItemMeshFocus focus;
    focus.BeginFrame(100);
    focus.ToggleKept(4);
    focus.ForgetMeshesFrom(5);
    CHECK(focus.Kept() == 4);
    focus.ForgetMeshesFrom(3);
    CHECK_FALSE(focus.Kept());

    focus.ToggleKept(1);
    focus.Point(2);
    focus.BeginFrame(101);
    CHECK_FALSE(focus.Kept());
    CHECK_FALSE(focus.Shown());
}

TEST_CASE("A pick ray hits a triangle from either side and misses beside or behind it [editor][items][mesh_list]")
{
    const float a[3] = {0.0f, 0.0f, 0.0f};
    const float b[3] = {10.0f, 0.0f, 0.0f};
    const float c[3] = {0.0f, 10.0f, 0.0f};

    CHECK(MeshPicking::HitTriangle(Downward(2.0f, 2.0f, 5.0f), a, b, c) == doctest::Approx(5.0f));
    const MeshPicking::Ray fromBelow{{2.0f, 2.0f, -3.0f}, {0.0f, 0.0f, 1.0f}};
    CHECK(MeshPicking::HitTriangle(fromBelow, a, b, c) == doctest::Approx(3.0f));
    CHECK_FALSE(MeshPicking::HitTriangle(Downward(20.0f, 20.0f, 5.0f), a, b, c));
    const MeshPicking::Ray away{{2.0f, 2.0f, 5.0f}, {0.0f, 0.0f, 1.0f}};
    CHECK_FALSE(MeshPicking::HitTriangle(away, a, b, c));
    const MeshPicking::Ray along{{2.0f, 2.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};
    CHECK_FALSE(MeshPicking::HitTriangle(along, a, b, c));
}

TEST_CASE("A pick ray finds a mesh's nearest triangle, a quad as two triangles [editor][items][mesh_list]")
{
    TestModel test(1);
    // A square quad at height 0 and a triangle at height 4 over its upper left corner.
    float vertices[7][3] = {{0, 0, 0}, {10, 0, 0}, {10, 10, 0}, {0, 10, 0}, {0, 5, 4}, {5, 10, 4}, {0, 10, 4}};
    std::array<Triangle_t, 2> triangles{};
    triangles[0].Polygon = 4;
    triangles[0].VertexIndex[0] = 0;
    triangles[0].VertexIndex[1] = 1;
    triangles[0].VertexIndex[2] = 2;
    triangles[0].VertexIndex[3] = 3;
    triangles[1].Polygon = 3;
    triangles[1].VertexIndex[0] = 4;
    triangles[1].VertexIndex[1] = 5;
    triangles[1].VertexIndex[2] = 6;
    test.Mesh(0).NumVertices = 7;
    test.Mesh(0).NumTriangles = 2;
    test.Mesh(0).Triangles = triangles.data();

    // Only the quad's second triangle (corners 0, 2, 3) is below (2, 6).
    CHECK(MeshPicking::HitMesh(Downward(2.0f, 6.0f, 10.0f), test.Model(), 0, vertices) == doctest::Approx(10.0f));
    // The triangle above it is nearer.
    CHECK(MeshPicking::HitMesh(Downward(1.0f, 9.0f, 10.0f), test.Model(), 0, vertices) == doctest::Approx(6.0f));
    CHECK_FALSE(MeshPicking::HitMesh(Downward(20.0f, 20.0f, 10.0f), test.Model(), 0, vertices));
    CHECK_FALSE(MeshPicking::HitMesh(Downward(2.0f, 6.0f, 10.0f), test.Model(), 1, vertices));
}

TEST_CASE("The preview's drawer draws the other meshes of the item's model see-through [editor][items][mesh_list]")
{
    TestModel item(3);
    TestModel other(1);
    RecordingDrawer recorder;
    const DrawerScope scope(recorder);
    {
        CItemMeshPreviewDrawer drawer(&item.Model(), 1, std::nullopt);
        CHECK(&Render::ModelDrawing::ActiveModelDrawer() == &drawer);
        drawer.DrawMesh(DrawOf(item.Model(), 1, RENDER_TEXTURE));
        drawer.DrawMesh(DrawOf(item.Model(), 0, RENDER_TEXTURE | RENDER_DARK));
        drawer.DrawMesh(DrawOf(other.Model(), 0, RENDER_TEXTURE));
    }
    CHECK(&Render::ModelDrawing::ActiveModelDrawer() == &recorder);

    REQUIRE(recorder.draws.size() == 3);
    // The shown mesh as it is.
    CHECK(recorder.draws[0].flags == RENDER_TEXTURE);
    CHECK(recorder.draws[0].bodyLight[0] == doctest::Approx(1.0f));
    // Another mesh added dimly, without darkening.
    CHECK(recorder.draws[1].flags == (RENDER_TEXTURE | RENDER_BRIGHT));
    CHECK(recorder.draws[1].bodyLight[0] == doctest::Approx(CItemMeshPreviewDrawer::SeeThroughLight));
    CHECK(recorder.draws[1].blendMeshAlpha == doctest::Approx(CItemMeshPreviewDrawer::SeeThroughLight));
    // Another model untouched.
    CHECK(recorder.draws[2].model == &other.Model());
    CHECK(recorder.draws[2].flags == RENDER_TEXTURE);
}

TEST_CASE("A see-through mesh keeps the holes of its texture [editor][items][mesh_list]")
{
    const TextureWithHoles holes;
    REQUIRE(Bitmaps.FindTexture(holes.index) != nullptr);
    REQUIRE(Bitmaps.FindTexture(holes.index)->hasHoles);
    // Meshes 0 and 1 with holes, 2 solid (no texture is loaded under it); 1 is shown.
    TestModel item(3);
    for (int mesh = 0; mesh < 3; ++mesh)
    {
        item.Mesh(mesh).NumTriangles = 1;
    }
    item.SetLoadedTexture(0, holes.index);
    item.SetLoadedTexture(1, holes.index);
    DisableAlphaTestRaw();
    RecordingDrawer recorder;
    const DrawerScope scope(recorder);
    {
        CItemMeshPreviewDrawer drawer(&item.Model(), 1, std::nullopt);
        drawer.DrawMesh(DrawOf(item.Model(), 0, RENDER_TEXTURE));
        drawer.DrawMesh(DrawOf(item.Model(), 1, RENDER_TEXTURE));
        drawer.DrawMesh(DrawOf(item.Model(), 2, RENDER_TEXTURE));
        drawer.DrawMesh(DrawOf(item.Model(), 0, RENDER_COLOR | RENDER_BRIGHT));
    }

    REQUIRE(recorder.alphaTests.size() == 4);
    // Added light with the alpha test on.
    CHECK(recorder.alphaTests[0]);
    // The shown mesh as it is, its own draw sets the test.
    CHECK_FALSE(recorder.alphaTests[1]);
    CHECK_FALSE(recorder.alphaTests[2]);
    // A color pass draws no texture.
    CHECK_FALSE(recorder.alphaTests[3]);
    CHECK_FALSE(glIsEnabled(GL_ALPHA_TEST));
}

TEST_CASE("A click on the preview finds no mesh past MAX_MESH [editor][items][mesh_list]")
{
    // A triangle under the ray, at the first mesh number VertexTransform has no room for.
    constexpr int Mesh = MAX_MESH;
    TestModel item(Mesh + 1);
    std::array<Triangle_t, 1> triangles{};
    triangles[0].Polygon = 3;
    triangles[0].VertexIndex[1] = 1;
    triangles[0].VertexIndex[2] = 2;
    item.Mesh(Mesh).NumVertices = 3;
    item.Mesh(Mesh).NumTriangles = 1;
    item.Mesh(Mesh).Triangles = triangles.data();
    RecordingDrawer recorder;
    const DrawerScope scope(recorder);
    {
        CItemMeshPreviewDrawer drawer(&item.Model(), std::nullopt, Downward(1.0f, 1.0f, 10.0f));
        drawer.DrawMesh(DrawOf(item.Model(), Mesh, RENDER_TEXTURE));
        CHECK_FALSE(drawer.HitMesh());
    }
    // Still drawn.
    CHECK(recorder.draws.size() == 1);
}

TEST_CASE("The preview's drawer draws later when the drawer it replaced does [editor][items][mesh_list]")
{
    TestModel item(1);
    for (const bool drawsLater : {false, true})
    {
        RecordingDrawer recorder(drawsLater);
        const DrawerScope scope(recorder);
        const CItemMeshPreviewDrawer drawer(&item.Model(), 0, std::nullopt);
        CHECK(drawer.DrawsLater() == drawsLater);
    }
}

TEST_CASE("Without a shown mesh the preview's drawer draws the item as it is [editor][items][mesh_list]")
{
    TestModel item(2);
    RecordingDrawer recorder;
    const DrawerScope scope(recorder);
    {
        CItemMeshPreviewDrawer drawer(&item.Model(), std::nullopt, std::nullopt);
        drawer.DrawMesh(DrawOf(item.Model(), 0, RENDER_TEXTURE));
        drawer.DrawMesh(DrawOf(item.Model(), 1, RENDER_TEXTURE | RENDER_CHROME));
        CHECK_FALSE(drawer.HitMesh());
    }
    REQUIRE(recorder.draws.size() == 2);
    CHECK(recorder.draws[0].flags == RENDER_TEXTURE);
    CHECK(recorder.draws[1].flags == (RENDER_TEXTURE | RENDER_CHROME));
}

TEST_CASE("Preview highlighting preserves cutouts and render state through real replay [drawer-contract]")
{
    const TextureWithHoles holes;
    TestModel item(2);
    TriangleGeometry geometry;
    for (int mesh = 0; mesh < 2; ++mesh)
    {
        geometry.Attach(item, mesh);
        item.SetLoadedTexture(mesh, holes.index);
    }
    const auto scene = [&]
    {
        {
            CItemMeshPreviewDrawer preview(&item.Model(), 1, std::nullopt);
            Render::ModelDrawing::ActiveModelDrawer().DrawMesh(geometry.Draw(item.Model(), 0));
            Render::ModelDrawing::ActiveModelDrawer().DrawMesh(geometry.Draw(item.Model(), 1));
        }
        // A following draw sees the same state too, including the alpha threshold and depth writes.
        Render::ModelDrawing::ActiveModelDrawer().DrawMesh(geometry.Draw(item.Model(), 1));
    };
    DrawerContract::CheckDrawnLaterAsAtOnce(scene, "Triangles");
    const auto calls = DrawerContract::CallsOf(scene, false, false);
    REQUIRE(calls.size() >= 3);
    CHECK(calls[0].find("alphaTest 1") != std::string::npos);
    CHECK(calls[0].find("depth 10") != std::string::npos);
}

TEST_CASE("Preview picking consumes the described pose after the shared model changes [drawer-contract]")
{
    TestModel item(1);
    TriangleGeometry geometry;
    geometry.Attach(item, 0);
    float captured[MAX_BONES][3][4]{};
    float live[MAX_BONES][3][4]{};
    for (int axis = 0; axis < 3; ++axis)
    {
        captured[0][axis][axis] = live[0][axis][axis] = 1.f;
    }
    live[0][0][3] = 100.f;
    MeshDraw draw = geometry.Draw(item.Model(), 0);
    MeshPicking::Ray atCaptured = Downward(1.f, 1.f, 10.f);
    SUBCASE("CPU vertices remain authoritative") {}
    SUBCASE("skin, scale and body origin remain authoritative")
    {
        draw.cpu = {};
        draw.skin.bones = captured;
        draw.skin.translate = true;
        draw.bodyScale = 2.f;
        draw.bodyOrigin[0] = 20.f;
        atCaptured = Downward(22.f, 2.f, 10.f);
    }
    BMD& model = item.Model();
    model.BodyScale = 1.f;
    Vector(0.f, 0.f, 0.f, model.BodyOrigin);
    vec3_t min{}, max{};
    OBB_t bounds{};
    const auto savedBones = g_pActiveBoneTransform;
    model.TransformCheap(live, min, max, &bounds, true);
    SetActiveBoneTransform(savedBones);
    const auto before = Render::Drawing::Hash(geometry.positions, sizeof(geometry.positions));
    RecordingDrawer recorder;
    const DrawerScope scope(recorder);
    {
        CItemMeshPreviewDrawer preview(&model, std::nullopt, atCaptured);
        preview.DrawMesh(draw);
        CHECK(preview.HitMesh() == 0);
    }
    {
        CItemMeshPreviewDrawer preview(&model, std::nullopt, Downward(101.f, 1.f, 10.f));
        preview.DrawMesh(draw);
        CHECK_FALSE(preview.HitMesh());
    }
    CHECK(Render::Drawing::Hash(geometry.positions, sizeof(geometry.positions)) == before);
    CHECK(captured[0][0][3] == 0.f);
    CHECK(live[0][0][3] == 100.f);
}

TEST_CASE("Preview drawers forward other models and shadows without changing their descriptions [drawer-contract]")
{
    TestModel item(1), other(1);
    RecordingDrawer recorder;
    const DrawerScope scope(recorder);
    MeshDraw mesh = DrawOf(other.Model(), 0, RENDER_TEXTURE | RENDER_DARK);
    mesh.alpha = 0.4f;
    mesh.bodyOrigin[0] = 123.f;
    const Render::ModelDrawing::ShadowDraw shadow{.model = &item.Model(), .alpha = 0.6f};
    {
        CItemMeshPreviewDrawer preview(&item.Model(), 0, std::nullopt);
        preview.DrawMesh(mesh);
        preview.DrawShadow(shadow);
    }
    REQUIRE(recorder.draws.size() == 1);
    CHECK(recorder.draws[0].model == mesh.model);
    CHECK(recorder.draws[0].flags == mesh.flags);
    CHECK(recorder.draws[0].alpha == mesh.alpha);
    CHECK(recorder.draws[0].bodyOrigin[0] == mesh.bodyOrigin[0]);
    REQUIRE(recorder.shadows.size() == 1);
    CHECK(recorder.shadows[0].model == shadow.model);
    CHECK(recorder.shadows[0].alpha == shadow.alpha);
}

TEST_CASE("Nested preview drawers restore their caller on normal and exceptional exits [drawer-contract]")
{
    TestModel item(1), other(1);
    RecordingDrawer recorder;
    const DrawerScope scope(recorder);
    {
        CItemMeshPreviewDrawer outer(&item.Model(), 0, std::nullopt);
        CHECK_THROWS_AS(
            [&]
            {
                CItemMeshPreviewDrawer inner(&other.Model(), 0, std::nullopt);
                CHECK(&Render::ModelDrawing::ActiveModelDrawer() == &inner);
                throw std::runtime_error("test scope exit");
            }(),
            std::runtime_error);
        CHECK(&Render::ModelDrawing::ActiveModelDrawer() == &outer);
    }
    CHECK(&Render::ModelDrawing::ActiveModelDrawer() == &recorder);
}

TEST_CASE("Picking rejects descriptions without a complete pose [drawer-contract]")
{
    TestModel item(1);
    TriangleGeometry geometry;
    geometry.Attach(item, 0);
    MeshDraw draw = geometry.Draw(item.Model(), 0);
    const auto ray = Downward(1.f, 1.f, 10.f);
    draw.cpu.vertexCount = 2;
    CHECK_FALSE(MeshPicking::HitMesh(ray, draw));
    draw.cpu = {};
    CHECK_FALSE(MeshPicking::HitMesh(ray, draw));
    draw.model = nullptr;
    CHECK_FALSE(MeshPicking::HitMesh(ray, draw));
}

#endif // _EDITOR
