#include "stdafx.h"
#include "doctest.h"

#include "Camera/CameraProjection.h"
#include "App/Control/DrawPurityCheck.h"
#include "Camera/CameraState.h"
#include "Core/Utilities/Random.h"
#include "Engine/Object/ZzzObject.h"
#include "Scenes/SceneCore.h"
#include "Render/Effects/EffectDraw.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Terrain/ZzzLodTerrain.h"

#include "RepeatedDraws.h"

#include <cstdlib>
#include <vector>

extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

namespace
{
// A frame's view over the middle of the map, built as the update builds it; the camera is given back afterwards.
struct ViewScope
{
    const CameraState savedCamera = g_Camera;
    const unsigned int savedWidth = WindowWidth;
    const unsigned int savedHeight = WindowHeight;

    ViewScope()
    {
        WindowWidth = 1280;
        WindowHeight = 720;
        g_Camera = CameraState();
        Vector(-48.5f, 0.f, 33.3f, g_Camera.Angle);
        Vector(12800.f, 12800.f, 900.f, g_Camera.Position);
        CameraProjection::UpdateView(g_Camera, 0, 0, 1280, 720);
        CreateFrustrum(1.f, 1.f);
    }

    ~ViewScope()
    {
        g_Camera = savedCamera;
        WindowWidth = savedWidth;
        WindowHeight = savedHeight;
    }

    // A point `distance` along the view, behind the camera when negative.
    void Ahead(float distance, vec3_t out) const
    {
        for (int i = 0; i < 3; i++)
            out[i] = g_Camera.Position[i] - g_Camera.Matrix[2][i] * distance;
    }
};

void ClearEffects()
{
    for (int i = 0; i < MAX_EFFECTS; ++i)
        Effects[i].Live = false;
}

int CreateEffectAt(const vec3_t position)
{
    vec3_t at;
    VectorCopy(position, at);
    vec3_t angle{0.f, 0.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    CreateEffect(BITMAP_MAGIC, at, angle, light, 8);
    for (int i = MAX_EFFECTS - 1; i >= 0; --i)
    {
        if (Effects[i].Live && VectorCompare(Effects[i].Position, position))
            return i;
    }
    return -1;
}
} // namespace

TEST_CASE(
    "The update culls the effects against the frame's view, and the effect pass only draws what it marked [effects]")
{
    ViewScope view;
    ClearEffects();
    vec3_t ahead;
    vec3_t behind;
    view.Ahead(1000.f, ahead);
    view.Ahead(-1000.f, behind);
    const int shown = CreateEffectAt(ahead);
    const int hidden = CreateEffectAt(behind);
    REQUIRE(shown >= 0);
    REQUIRE(hidden >= 0);

    CullEffects();
    CHECK(Effects[shown].Visible);
    CHECK_FALSE(Effects[hidden].Visible);
    const unsigned int marked = Effects[shown].DrawnFrame;

    CullEffects();
    CHECK(Effects[shown].DrawnFrame == marked + 1);

    ClearEffects();
}

TEST_CASE("Drawing the effect passes again and again changes nothing the next update reads [effects]")
{
    ViewScope view;
    ClearEffects();
    vec3_t ahead;
    view.Ahead(1000.f, ahead);
    REQUIRE(CreateEffectAt(ahead) >= 0);
    CullEffects();

    RepeatedDraws::CheckChangesNothing(
        []
        {
            RenderEffects();
            RenderEffects(true);
        },
        3);

    ClearEffects();
}

namespace
{
// Keeps the lights of the ground marks it is handed.
class DecalLights final : public Render::EffectDrawing::EffectDrawer
{
public:
    std::vector<float> lights;

    void SetState(const Render::EffectDrawing::EffectState&) override {}
    void DrawSprite(const Render::EffectDrawing::SpriteDraw&) override {}
    void DrawQuads(const Render::EffectDrawing::QuadsDraw&) override {}
    void DrawDecal(const Render::EffectDrawing::DecalDraw& draw) override
    {
        lights.insert(lights.end(), draw.light, draw.light + 3);
    }
    void DrawCircle(const Render::EffectDrawing::CircleDraw&) override {}
    void DrawPlane(const Render::EffectDrawing::PlaneDraw&) override {}
};
} // namespace

TEST_CASE("A draw's random value is the same in every pass of a frame and takes nothing from the update's sequences "
          "[effects]")
{
    int key = 0;
    std::srand(914);
    const int next = std::rand();
    std::srand(914);
    const std::mt19937 engine = Random::Snapshot();

    const int first = Random::DrawRand(&key, 1);
    CHECK(Random::DrawRand(&key, 1) == first);
    CHECK(std::rand() == next);
    CHECK(Random::Snapshot() == engine);

    // Over frames it spreads like rand() does.
    int changed = 0;
    long long sum = 0;
    for (int frame = 0; frame < 1000; ++frame)
    {
        Random::NextDrawFrame();
        const int value = Random::DrawRand(&key, 1);
        CHECK(value >= 0);
        CHECK(value <= 32767);
        changed += value != first ? 1 : 0;
        sum += value;
    }
    CHECK(changed > 990);
    CHECK(sum / 1000 > 15000);
    CHECK(sum / 1000 < 17800);
}

TEST_CASE("A draw's frame check passes as often as rand_fps_check does [effects]")
{
    int key = 0;
    int passed = 0;
    for (int frame = 0; frame < 20000; ++frame)
    {
        Random::NextDrawFrame();
        passed += Random::DrawFpsCheck(20, 1.0, &key, 1) ? 1 : 0;
    }
    CHECK(passed > 850);
    CHECK(passed < 1150);
}

TEST_CASE(
    "An effect's flickering ground mark is the same in every pass of a frame, and draws from no sequence [effects]")
{
    OBJECT flame{};
    flame.Live = true;
    flame.Type = BITMAP_FLAME;
    flame.SubType = 0;
    flame.Scale = 1.f;

    DecalLights first;
    DecalLights second;
    Render::EffectDrawing::SetEffectDrawer(&first);
    RepeatedDraws::CheckChangesNothing(
        [&flame] { DescribeEffectShadow(flame, Render::EffectDrawing::ActiveEffectDrawer()); }, 1);
    Render::EffectDrawing::SetEffectDrawer(&second);
    RepeatedDraws::CheckChangesNothing(
        [&flame] { DescribeEffectShadow(flame, Render::EffectDrawing::ActiveEffectDrawer()); }, 1);
    Render::EffectDrawing::SetEffectDrawer(nullptr);

    REQUIRE_FALSE(first.lights.empty());
    CHECK(first.lights == second.lights);
}

TEST_CASE("The update marks the map objects the frame's view shows, and only those [effects]")
{
    ViewScope view;
    // The 2D cull lets everything through outside the world scenes.
    const EGameScene scene = SceneFlag;
    SceneFlag = MAIN_SCENE;
    OBJECT shown{};
    shown.Live = true;
    shown.CollisionRange = 0.f;
    view.Ahead(1000.f, shown.Position);
    OBJECT hidden{};
    hidden.Live = true;
    Vector(200.f, 200.f, 0.f, hidden.Position); // across the map from the camera
    shown.Next = &hidden;
    // The block that covers the shown object: 16 tiles of 100 a side.
    const int column = static_cast<int>(shown.Position[0] / 100.f) / 16;
    const int row = static_cast<int>(shown.Position[1] / 100.f) / 16;
    OBJECT_BLOCK& block = ObjectBlock[column * 16 + row];
    OBJECT* const previousHead = block.Head;
    block.Head = &shown;

    CullObjects();
    const unsigned int marked = shown.DrawnFrame;
    CHECK(shown.Visible);
    CHECK_FALSE(hidden.Visible);
    CHECK(hidden.DrawnFrame != marked);

    CullObjects();
    CHECK(shown.DrawnFrame == marked + 1);

    block.Head = previousHead;
    SceneFlag = scene;
}

#ifdef MU_ENABLE_CONTROL_SOCKET
TEST_CASE("The draw purity check counts a draw pass that draws from the update's random engine [effects]")
{
    namespace Purity = Diagnostics::DrawPurityCheck;
    const auto counted = [](const char* what)
    {
        int draws = 0;
        for (const Purity::Change& change : Purity::Report(draws))
        {
            if (change.what == what)
                return change.count;
        }
        return 0;
    };

    Purity::Start();
    Purity::BeforeDraw();
    Purity::AfterDraw();
    Purity::BeforeDraw();
    (void)Random::Unit();
    Purity::AfterDraw();

    CHECK(counted("random engine") == 1);
}

TEST_CASE("The draw purity check draws nothing from the update's random sequences itself [effects]")
{
    namespace Purity = Diagnostics::DrawPurityCheck;
    std::srand(914);
    const int next = std::rand();
    const std::mt19937 engine = Random::Snapshot();

    std::srand(914);
    Purity::Start();
    for (int pass = 0; pass < 2; ++pass)
    {
        Purity::BeforeDraw();
        Purity::AfterDraw();
    }
    CHECK(std::rand() == next);
    CHECK(Random::Snapshot() == engine);
}
#endif
