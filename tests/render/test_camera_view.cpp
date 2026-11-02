#include "stdafx.h"
#include "doctest.h"

#include "../data/TestModelSlots.h"

#include "Camera/CameraProjection.h"
#include "Camera/CameraState.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Input/Selection.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Scenes/SceneCore.h"
#include "UI/Social/PhotoViewer.h"
#include "World/MapInfra/MapManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstring>
#include <memory>

extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

namespace
{
// Keeps the matrix the draw loads into the view.
class ViewRenderer final : public mu::IMuRenderer
{
public:
    int mode = 0;
    float loaded[16] = {};
    int loads = 0;

    void RenderQuad2D(std::span<const mu::Vertex2D>, std::uint32_t) override {}
    void RenderTriangles(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void RenderQuad3D(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void RenderQuadStrip(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void RenderLines(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void SetBlendMode(mu::BlendMode) override {}
    void DisableBlend() override {}
    void SetDepthTest(bool) override {}
    void SetDepthMask(bool) override {}
    void SetCullFace(bool) override {}
    void SetAlphaTest(bool) override {}
    void SetTexture2D(bool) override {}
    void SetFogEnabled(bool) override {}
    void BindTexture(int) override {}
    void SetFog(const mu::FogParams&) override {}
    void BeginScene(int, int, int, int) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}
    void SetMatrixMode(int m) override { mode = m; }
    void LoadMatrix(const float* m) override
    {
        if (mode == GL_MODELVIEW)
        {
            std::memcpy(loaded, m, sizeof(loaded));
            ++loads;
        }
    }
};

// The modelview the renderer's matrix stack built from the camera before the update built the view.
glm::mat4 StackView(const CameraState& state)
{
    glm::mat4 m(1.f);
    m = glm::rotate(m, glm::radians(state.Angle[1]), glm::vec3(0.f, 1.f, 0.f));
    if (state.TopViewEnable == false)
        m = glm::rotate(m, glm::radians(state.Angle[0]), glm::vec3(1.f, 0.f, 0.f));
    m = glm::rotate(m, glm::radians(state.Angle[2]), glm::vec3(0.f, 0.f, 1.f));
    return glm::translate(m, glm::vec3(-state.Position[0], -state.Position[1], -state.Position[2]));
}

CameraState Camera(float pitch, float yaw, bool topView)
{
    CameraState state;
    Vector(pitch, 0.f, yaw, state.Angle);
    Vector(12800.f, 12800.f, 900.f, state.Position);
    state.TopViewEnable = topView;
    return state;
}
} // namespace

TEST_CASE("The view the update builds is the modelview the renderer's stack built from the camera [render]")
{
    for (const bool topView : {false, true})
    {
        for (const float yaw : {-45.f, 0.f, 33.3f, 180.f, -271.5f})
        {
            CameraState state = Camera(-48.5f, yaw, topView);
            CameraProjection::UpdateView(state, 0, 0, 1280, 720);
            const glm::mat4 expected = StackView(state);
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 4; j++)
                    CHECK(state.Matrix[i][j] == expected[j][i]);
            }
        }
    }
}

TEST_CASE("The draw loads the view the update built, unchanged [render]")
{
    CameraState state = Camera(-48.5f, 33.3f, false);
    CameraProjection::UpdateView(state, 0, 0, 1280, 720);

    ViewRenderer renderer;
    mu::SetRendererOverride(&renderer);
    CameraProjection::ApplyView(state, 1280, 720);
    mu::SetRendererOverride(nullptr);

    REQUIRE(renderer.loads == 1);
    const glm::mat4 expected = StackView(state);
    CHECK(std::memcmp(renderer.loaded, &expected[0][0], sizeof(renderer.loaded)) == 0);
}

TEST_CASE("The view projects what is straight ahead to the viewport's centre [render]")
{
    CameraState state = Camera(-48.5f, 33.3f, false);
    CameraProjection::UpdateView(state, 100, 50, 1280, 720);
    CHECK(state.ScreenCenterX == 100 + 640);
    CHECK(state.ScreenCenterY == 50 + 360);

    // Straight ahead is the view's -Z axis.
    vec3_t ahead;
    for (int i = 0; i < 3; i++)
        ahead[i] = state.Position[i] - state.Matrix[2][i] * 1000.f;
    float x = 0.f;
    float y = 0.f;
    REQUIRE(CameraProjection::WorldToWindowPixel(state, ahead, &x, &y));
    CHECK(x == doctest::Approx(state.ScreenCenterX).epsilon(0.001));
    CHECK(y == doctest::Approx(state.ScreenCenterY).epsilon(0.001));
}

TEST_CASE("The frustum built from the view holds what is ahead and not what is behind [render]")
{
    const unsigned int windowWidth = WindowWidth;
    const unsigned int windowHeight = WindowHeight;
    WindowWidth = 1280;
    WindowHeight = 720;
    const CameraState saved = g_Camera;

    g_Camera = Camera(-48.5f, 33.3f, false);
    CameraProjection::UpdateView(g_Camera, 0, 0, 1280, 720);
    CreateFrustrum(1.f, 1.f);

    vec3_t ahead;
    vec3_t behind;
    for (int i = 0; i < 3; i++)
    {
        ahead[i] = g_Camera.Position[i] - g_Camera.Matrix[2][i] * 1000.f;
        behind[i] = g_Camera.Position[i] + g_Camera.Matrix[2][i] * 1000.f;
    }
    CHECK(TestFrustrum(ahead, 0.f));
    CHECK_FALSE(TestFrustrum(behind, 0.f));

    g_Camera = saved;
    WindowWidth = windowWidth;
    WindowHeight = windowHeight;
}

TEST_CASE("Picking follows the cursor as it is now, not the ray the last update built [render]")
{
    const unsigned int windowWidth = WindowWidth;
    const unsigned int windowHeight = WindowHeight;
    const CameraState savedCamera = g_Camera;
    const EGameScene savedScene = SceneFlag;
    const int savedWorld = gMapManager.WorldActive;
    const float savedRate[] = {g_fScreenRate_x, g_fScreenRate_y, g_fScreenOffset_x, g_fScreenOffset_y};
    const int savedMouse[] = {MouseX, MouseY};
    CHARACTER* const savedCharacters = CharactersClient;
    WindowWidth = 1280;
    WindowHeight = 720;
    g_fScreenRate_x = g_fScreenRate_y = 1.f;
    g_fScreenOffset_x = g_fScreenOffset_y = 0.f;
    SceneFlag = CHARACTER_SCENE;
    gMapManager.WorldActive = WD_1DUNGEON;

    TestModelSlots models;
    auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT + 1);
    CharactersClient = characters.get();
    g_Camera = Camera(-48.5f, 33.3f, false);
    CameraProjection::UpdateView(g_Camera, 0, 0, 1280, 720);

    CHARACTER* c = &CharactersClient[3];
    OBJECT* o = &c->Object;
    o->Live = o->Visible = true;
    o->Alpha = 1.f;
    o->Kind = KIND_PLAYER;
    c->m_bIsSelected = true;
    for (int i = 0; i < 3; i++)
        o->Position[i] = g_Camera.Position[i] - g_Camera.Matrix[2][i] * 1500.f;

    float x = 0.f;
    float y = 0.f;
    REQUIRE(CameraProjection::WorldToWindowPixel(g_Camera, o->Position, &x, &y));

    // The last update built its ray with the cursor in a corner, over nothing.
    CameraProjection::ScreenToWorldRay(g_Camera, 5, 5, MouseTarget);
    MouseX = static_cast<int>(x);
    MouseY = static_cast<int>(y);
    Input::Selection::SelectObjects();
    CHECK(SelectedCharacter == 3);

    MouseX = MouseY = 5;
    Input::Selection::SelectObjects();
    CHECK(SelectedCharacter == -1);

    CharactersClient = savedCharacters;
    MouseX = savedMouse[0];
    MouseY = savedMouse[1];
    g_fScreenRate_x = savedRate[0];
    g_fScreenRate_y = savedRate[1];
    g_fScreenOffset_x = savedRate[2];
    g_fScreenOffset_y = savedRate[3];
    gMapManager.WorldActive = savedWorld;
    SceneFlag = savedScene;
    g_Camera = savedCamera;
    WindowWidth = windowWidth;
    WindowHeight = windowHeight;
}

TEST_CASE("Drawing a letter's photo leaves the world camera as the update built it [render]")
{
    const CameraState saved = g_Camera;
    g_Camera = Camera(-48.5f, 33.3f, false);
    CameraProjection::UpdateView(g_Camera, 0, 0, 1280, 720);
    const CameraState world = g_Camera;
    TestModelSlots models;
    ViewRenderer renderer;
    mu::SetRendererOverride(&renderer);
    {
        // The photo target's drawer, as RenderTarget calls it.
        struct Photo : CUIPhotoViewer
        {
            using CUIPhotoViewer::RenderInto;
        } photo;
        photo.RenderInto(256, 256);
    }
    mu::SetRendererOverride(nullptr);

    CHECK(std::memcmp(g_Camera.Matrix, world.Matrix, sizeof(world.Matrix)) == 0);
    CHECK(g_Camera.PerspectiveX == world.PerspectiveX);
    CHECK(g_Camera.PerspectiveY == world.PerspectiveY);
    CHECK(g_Camera.ScreenCenterX == world.ScreenCenterX);
    CHECK(g_Camera.ScreenCenterY == world.ScreenCenterY);
    CHECK(g_Camera.ScreenCenterYFlip == world.ScreenCenterYFlip);
    g_Camera = saved;
}
