#include "stdafx.h"

#include "doctest.h"

// The parts of the effect browser's preview (FX1.7a) that need neither ImGui
// nor a frame: the camera, the quads, what is shown for a type, the SubTypes,
// the texture size, the item search, the effect's object and the pool guard.
#ifdef _EDITOR
#include "EffectTestData.h"

#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Render/Effects/EffectRegistry.h"
#include "Render/Effects/ZzzEffect.h"
#include "UI/EffectBrowser/EffectPoolGuard.h"
#include "UI/EffectBrowser/EffectPreviewCamera.h"
#include "UI/EffectBrowser/EffectPreviewGeometry.h"
#include "UI/EffectBrowser/EffectPreviewItems.h"
#include "UI/EffectBrowser/EffectPreviewObject.h"
#include "UI/EffectBrowser/EffectPreviewSubject.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace MuEditor::Effects;
using Data::Effects::EffectKind;

namespace
{
constexpr float Tolerance = 1e-3f;

float Dot(const PreviewVector& a, const PreviewVector& b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

PreviewVector Minus(const PreviewVector& a, const PreviewVector& b)
{
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

float Length(const PreviewVector& v)
{
    return std::sqrt(Dot(v, v));
}

PreviewVector PositionOf(const mu::Vertex3D& vertex)
{
    return {vertex.x, vertex.y, vertex.z};
}

// The point `p` in the space of the column-major matrix `m`.
PreviewVector Apply(const std::array<float, 16>& m, const PreviewVector& p)
{
    return {m[0] * p[0] + m[4] * p[1] + m[8] * p[2] + m[12], m[1] * p[0] + m[5] * p[1] + m[9] * p[2] + m[13],
            m[2] * p[0] + m[6] * p[1] + m[10] * p[2] + m[14]};
}
} // namespace

TEST_CASE("The effect preview's camera looks at the type from the thumbnails' direction and turns and zooms within "
          "limits [effects][editor]")
{
    EffectPreviewCamera camera;
    camera.Frame({10.0f, 20.0f, 30.0f}, 100.0f);
    const float framed = camera.Distance();
    const PreviewVector toEye = Minus(camera.Eye(), camera.Center());
    CHECK(Length(toEye) == doctest::Approx(framed).epsilon(Tolerance));
    // The map editor's thumbnails look along (1, -1, 0.8).
    const float diagonal = std::sqrt(1.0f + 1.0f + 0.64f);
    CHECK(toEye[0] / framed == doctest::Approx(1.0f / diagonal).epsilon(Tolerance));
    CHECK(toEye[1] / framed == doctest::Approx(-1.0f / diagonal).epsilon(Tolerance));
    CHECK(toEye[2] / framed == doctest::Approx(0.8f / diagonal).epsilon(Tolerance));

    // The view puts the eye at the origin and the center straight ahead.
    const std::array<float, 16> view = camera.View();
    CHECK(Length(Apply(view, camera.Eye())) == doctest::Approx(0.0f).epsilon(Tolerance));
    const PreviewVector center = Apply(view, camera.Center());
    CHECK(center[2] == doctest::Approx(-framed).epsilon(Tolerance));
    const EffectPreviewCamera::Basis basis = camera.GetBasis();
    CHECK(Dot(basis.right, basis.up) == doctest::Approx(0.0f).epsilon(Tolerance));
    CHECK(Dot(basis.right, basis.forward) == doctest::Approx(0.0f).epsilon(Tolerance));
    CHECK(basis.up[2] > 0.0f);

    // Turning stops short of looking straight down; zooming stays within
    // limits; Reset brings back the first view.
    const float limit = std::sin(89.0f * 3.14159265f / 180.0f);
    camera.Turn(0.0f, 100000.0f);
    const PreviewVector top = Minus(camera.Eye(), camera.Center());
    CHECK(top[2] / framed == doctest::Approx(limit).epsilon(Tolerance));
    // Still on the first side, not over the top.
    CHECK(top[0] > 0.0f);
    camera.Turn(0.0f, -200000.0f);
    CHECK(Minus(camera.Eye(), camera.Center())[2] / framed == doctest::Approx(-limit).epsilon(Tolerance));
    camera.Zoom(1000.0f);
    CHECK(camera.Distance() >= framed * 0.19f);
    camera.Zoom(-1000.0f);
    CHECK(camera.Distance() <= framed * 5.01f);
    camera.Reset();
    CHECK(camera.Distance() == doctest::Approx(framed).epsilon(Tolerance));
    CHECK(Minus(camera.Eye(), camera.Center())[2] / framed == doctest::Approx(0.8f / diagonal).epsilon(Tolerance));
    CHECK(camera.Near() >= 1.0f);
}

TEST_CASE("The effect preview's quads face the camera, lie flat and cover the view [effects][editor]")
{
    EffectPreviewCamera camera;
    camera.Frame({0.0f, 0.0f, 0.0f}, 50.0f);
    const EffectPreviewCamera::Basis basis = camera.GetBasis();

    // As RenderSprite: the first corner at the bottom left, v = 1 there.
    const PreviewGeometry::Quad sprite = PreviewGeometry::Billboard({0.0f, 0.0f, 0.0f}, basis, 40.0f, 20.0f, 0u);
    const PreviewVector across = Minus(PositionOf(sprite[1]), PositionOf(sprite[0]));
    const PreviewVector up = Minus(PositionOf(sprite[3]), PositionOf(sprite[0]));
    CHECK(Length(across) == doctest::Approx(40.0f).epsilon(Tolerance));
    CHECK(Dot(across, basis.right) == doctest::Approx(40.0f).epsilon(Tolerance));
    CHECK(Dot(up, basis.up) == doctest::Approx(20.0f).epsilon(Tolerance));
    CHECK(sprite[0].u == 0.0f);
    CHECK(sprite[0].v == 1.0f);
    CHECK(sprite[2].u == 1.0f);
    CHECK(sprite[2].v == 0.0f);

    const PreviewGeometry::Quad ground = PreviewGeometry::GroundQuad({5.0f, 5.0f, 2.0f}, 30.0f, 45.0f, 0u);
    CHECK(std::all_of(ground.begin(), ground.end(), [](const mu::Vertex3D& vertex) { return vertex.z == 2.0f; }));
    CHECK(Length(Minus(PositionOf(ground[1]), PositionOf(ground[0]))) == doctest::Approx(30.0f).epsilon(Tolerance));
    // As RenderTerrainAlphaBitmap: u grows along x, v along y.
    const PreviewGeometry::Quad flat = PreviewGeometry::GroundQuad({0.0f, 0.0f, 0.0f}, 10.0f, 0.0f, 0u);
    CHECK(flat[1].x > flat[0].x);
    CHECK(flat[1].u > flat[0].u);
    CHECK(flat[3].y > flat[0].y);
    CHECK(flat[3].v > flat[0].v);

    const PreviewGeometry::PlaneQuads plane = PreviewGeometry::Plane(800.0f);
    CHECK(std::all_of(plane.begin(), plane.end(), [](const mu::Vertex3D& vertex)
                      { return vertex.z == 0.0f && std::abs(vertex.x) <= 400.0f && std::abs(vertex.y) <= 400.0f; }));

    const PreviewGeometry::CubeQuads cube = PreviewGeometry::Cube(100.0f);
    const auto [low, high] = std::minmax_element(
        cube.begin(), cube.end(), [](const mu::Vertex3D& a, const mu::Vertex3D& b) { return a.z < b.z; });
    CHECK(low->z == 0.0f);
    CHECK(high->z == 100.0f);

    // The cover lies in front of the eye, along the view.
    const PreviewGeometry::Quad cover = PreviewGeometry::ScreenCover(camera.Eye(), basis, camera.Near(), 1.5f, 0u);
    const PreviewVector middle = {(cover[0].x + cover[2].x) * 0.5f, (cover[0].y + cover[2].y) * 0.5f,
                                  (cover[0].z + cover[2].z) * 0.5f};
    CHECK(Dot(Minus(middle, camera.Eye()), basis.forward) > camera.Near());
}

TEST_CASE("The effect preview shows models, sprites and ground decals by slot and stage, and says why not "
          "[effects][editor]")
{
    EffectStages notDrawn;
    EffectStages asModel;
    asModel.render = RenderStage::DrawnAsModel;
    EffectStages onGround;
    onGround.render = RenderStage::OnGround;
    onGround.drawnOnGround = true;

    const PreviewSubject model =
        DescribePreviewSubject(EffectKind::Effect, MODEL_POISON, EffectAssetSlot::Model, true, asModel);
    CHECK(model.draw == PreviewDraw::Model);
    CHECK(model.notes == 0);
    CHECK(DescribePreviewSubject(EffectKind::Effect, MODEL_POISON, EffectAssetSlot::Model, true, notDrawn).notes ==
          NoteNotDrawnByGame);
    CHECK((DescribePreviewSubject(EffectKind::Effect, MODEL_KALIMA_FALLING_STONE, EffectAssetSlot::Model, true, asModel)
               .notes &
           NoteWorldObjectSlot) != 0);

    const PreviewSubject empty =
        DescribePreviewSubject(EffectKind::Effect, MODEL_POISON, EffectAssetSlot::Model, false, asModel);
    CHECK(empty.draw == PreviewDraw::None);
    CHECK(empty.notes == NoteNothingLoaded);
    CHECK(DescribePreviewSubject(EffectKind::Joint, MODEL_SPEARSKILL, EffectAssetSlot::TextureChosenInCode, true,
                                 notDrawn)
              .notes == NoteTextureChosenInCode);

    CHECK(
        DescribePreviewSubject(EffectKind::Effect, BITMAP_SHOCK_WAVE, EffectAssetSlot::Texture, true, onGround).draw ==
        PreviewDraw::GroundDecal);
    CHECK(DescribePreviewSubject(EffectKind::Effect, BITMAP_SKULL, EffectAssetSlot::Texture, true, notDrawn).draw ==
          PreviewDraw::Sprite);
    const PreviewSubject particle =
        DescribePreviewSubject(EffectKind::Particle, BITMAP_SMOKE, EffectAssetSlot::DefaultTexture, true, notDrawn);
    CHECK(particle.draw == PreviewDraw::Sprite);
    CHECK(particle.notes == NoteCodeMayChooseTexture);
}

TEST_CASE("The effect preview offers one SubType per column of the creation table [effects][editor]")
{
    CHECK(PreviewSubTypes({}) == std::vector<int>{0});
    CHECK(PreviewSubTypes({{1, 2}, {3}}) == std::vector<int>{0, 1, 3});
    CHECK(PreviewSubTypes({{0}}) == std::vector<int>{1, 0});
    CHECK(PreviewSubTypes({{0, 1}, {2}}) == std::vector<int>{3, 0, 2});
}

TEST_CASE("The effect preview's texture grows in steps and stays within limits [effects][editor]")
{
    const auto size = [](float width, float height, float scale)
    {
        const PreviewTextureSize chosen = ChoosePreviewTextureSize(width, height, scale);
        return std::vector<int>{chosen.width, chosen.height};
    };
    CHECK(size(100.0f, 75.0f, 1.0f) == std::vector<int>{128, 96});
    CHECK(size(10.0f, 10.0f, 1.0f) == std::vector<int>{64, 64});
    CHECK(size(5000.0f, 100.0f, 1.0f) == std::vector<int>{2048, 128});
    CHECK(size(100.0f, 75.0f, 2.0f) == std::vector<int>{224, 160});
}

TEST_CASE("The effect preview finds items by a part of their name or their number [effects][editor]")
{
    const std::vector<std::string> names = {"Kris", "", "Short Sword", "Rapier", "Kriss Knife"};
    EffectPreviewItems items;
    items.Build(
        static_cast<int>(names.size()), [&](int itemType) { return names[itemType]; },
        [](int itemType) { return itemType != 3; });
    // Without a name or a model to draw an item is not listed.
    REQUIRE(items.GetItems().size() == 3);
    CHECK(items.Find(2)->name == "Short Sword");
    CHECK(items.Find(3) == nullptr);

    const auto typesOf = [&](const std::vector<int>& found)
    {
        std::vector<int> types;
        for (const int index : found)
            types.push_back(items.GetItems()[index].type);
        return types;
    };
    CHECK(typesOf(items.Filter("kri")) == std::vector<int>{0, 4});
    CHECK(typesOf(items.Filter("SWORD")) == std::vector<int>{2});
    CHECK(typesOf(items.Filter("4")) == std::vector<int>{4});
    CHECK(items.Filter("").size() == 3);
}

TEST_CASE("The effect preview makes an effect as CreateEffect would, without its hook and outside the pools "
          "[effects][editor]")
{
    EffectTestData::BuildShippedRegistry();
    OBJECT o;

    // Without a row: the setup every effect gets.
    CHECK(BuildPreviewEffectObject(o, MODEL_FIRE, 0, nullptr) == 0);
    CHECK(o.Scale == 0.9f);
    CHECK(o.BlendMesh == -1);
    CHECK(o.HiddenMesh == -1);
    CHECK(o.Alpha == 1.0f);
    CHECK(o.Velocity == 0.3f);

    // The row's values, and the variant of the SubType.
    BuildPreviewEffectObject(o, MODEL_CUNDUN_GHOST, 0, Render::Effects::Lookup(MODEL_CUNDUN_GHOST));
    CHECK(o.Scale == 1.8f);
    CHECK(o.BlendMesh == -2);
    BuildPreviewEffectObject(o, BITMAP_SKULL, 2, Render::Effects::Lookup(BITMAP_SKULL));
    CHECK(o.LifeTime == 8);
    BuildPreviewEffectObject(o, BITMAP_SKULL, 0, Render::Effects::Lookup(BITMAP_SKULL));
    CHECK(o.LifeTime == 1000);

    // A row that starts invisible is shown, one that copies the call's scale
    // gets 1, and the position stays at the origin.
    CHECK(BuildPreviewEffectObject(o, MODEL_KENTAUROS_ARROW, 0, Render::Effects::Lookup(MODEL_KENTAUROS_ARROW)) ==
          NoteStartsInvisible);
    CHECK(o.Alpha == 1.0f);
    CHECK(BuildPreviewEffectObject(o, MODEL_DARK_ELF_SKILL, 0, Render::Effects::Lookup(MODEL_DARK_ELF_SKILL)) ==
          NoteStartsInvisible);
    CHECK(o.Scale == 1.0f);
    BuildPreviewEffectObject(o, MODEL_TARGETMON_EFFECT, 0, Render::Effects::Lookup(MODEL_TARGETMON_EFFECT));
    CHECK(o.Scale == 1.0f);
    BuildPreviewEffectObject(o, MODEL_DRAGON, 0, Render::Effects::Lookup(MODEL_DRAGON));
    CHECK(o.Position[2] == 0.0f);

    // MODEL_MAYASTONE4 has only a creation hook, which draws a random lifetime
    // of 32 or more and a scale of 1 or more; the preview never runs it, so
    // the object keeps the setup every effect gets.
    BuildPreviewEffectObject(o, MODEL_MAYASTONE4, 0, Render::Effects::Lookup(MODEL_MAYASTONE4));
    CHECK(o.LifeTime == 0);
    CHECK(o.Scale == 0.9f);
}

TEST_CASE("MoveEffect animates the models of the skill range except some [effects][editor]")
{
    CHECK(IsAnimatedByMoveEffect(MODEL_POISON, 0));
    CHECK_FALSE(IsAnimatedByMoveEffect(MODEL_SKILL_WHEEL1, 0));
    CHECK_FALSE(IsAnimatedByMoveEffect(MODEL_STONE1, 5));
    CHECK(IsAnimatedByMoveEffect(MODEL_STONE1, 0));
    CHECK_FALSE(IsAnimatedByMoveEffect(MODEL_SKILL_END, 0));
    CHECK_FALSE(IsAnimatedByMoveEffect(BITMAP_SKULL, 0));
}

TEST_CASE("The effect preview's pool guard removes what was created while it lived [effects][editor]")
{
    const bool keptBefore = Sprites[10].Live;
    const bool newBefore = Sprites[11].Live;
    const bool particleBefore = Particles[5].Live;
    Sprites[10].Live = true;
    Sprites[11].Live = false;
    Particles[5].Live = false;
    {
        const EffectPoolGuard guard;
        Sprites[11].Live = true;
        Particles[5].Live = true;
    }
    CHECK(Sprites[10].Live);
    CHECK_FALSE(Sprites[11].Live);
    CHECK_FALSE(Particles[5].Live);
    Sprites[10].Live = keptBefore;
    Sprites[11].Live = newBefore;
    Particles[5].Live = particleBefore;
}
#endif // _EDITOR
