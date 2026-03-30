// Story 7.9.2: OpenGL Immediate-Mode → MuRenderer Abstraction Migration
// Flow Code: VS0-QUAL-RENDER-GLMIGRATE
//
// RED PHASE: Tests verify contracts for new IMuRenderer methods required to
// eliminate raw OpenGL calls (glBegin/glEnd/glVertex*) from game code.
// Tests will FAIL TO COMPILE until Task 1 adds these methods to IMuRenderer.
//
// AC Mapping:
//   AC-1     -> TEST_CASE("AC-1 [7-9-2]: ...")   BeginScene/EndScene routing
//   AC-2     -> TEST_CASE("AC-2 [7-9-2]: ...")   Begin2DPass/End2DPass routing
//   AC-5     -> TEST_CASE("AC-5 [7-9-2]: ...")   RenderLines interface
//   AC-6     -> TEST_CASE("AC-6 [7-9-2]: ...")   IsFrameActive lifecycle guard
//   AC-7     -> TEST_CASE("AC-7 [7-9-2]: ...")   ClearScreen callable
//   AC-STD-2 -> TEST_CASE("AC-STD-2 [7-9-2]: ...") Extended interface completeness
//
// IMPORTANT: No OpenGL calls in this test TU.
// All tests use a MigrationCaptureMock test double implementing IMuRenderer.
// Tests compile and run on macOS/Linux without Win32, OpenGL, or a GPU device.
//
// Run with: ctest --test-dir MuMain/build -R gl_migration_7_9_2

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "MuRenderer.h"

// ---------------------------------------------------------------------------
// MigrationCaptureMock
// Implements ALL new IMuRenderer methods required by story 7-9-2.
// RED PHASE: Fails to compile until Task 1 adds these methods to IMuRenderer:
//   - BeginScene(int x, int y, int w, int h)
//   - EndScene()
//   - Begin2DPass()
//   - End2DPass()
//   - ClearScreen()
//   - RenderLines(std::span<const Vertex3D>, std::uint32_t)
//   - IsFrameActive() const
// ---------------------------------------------------------------------------
namespace
{

struct MigrationCaptureMock : public mu::IMuRenderer
{
    // --- Existing interface (4.2.1) ----------------------------------------

    std::vector<mu::Vertex2D> m_lastQuad2DVertices;
    std::uint32_t m_lastQuad2DTextureId{0};
    int m_renderQuad2DCallCount{0};

    void RenderQuad2D(std::span<const mu::Vertex2D> v, std::uint32_t id) override
    {
        m_lastQuad2DVertices.assign(v.begin(), v.end());
        m_lastQuad2DTextureId = id;
        ++m_renderQuad2DCallCount;
    }

    void RenderTriangles(std::span<const mu::Vertex3D> /*v*/, std::uint32_t /*id*/) override
    {
    }

    void RenderQuadStrip(std::span<const mu::Vertex3D> /*v*/, std::uint32_t /*id*/) override
    {
    }

    void SetBlendMode(mu::BlendMode /*mode*/) override
    {
    }

    void DisableBlend() override
    {
    }

    void SetDepthTest(bool /*enabled*/) override
    {
    }

    void SetFog(const mu::FogParams& /*params*/) override
    {
    }

    // --- New methods required by 7-9-2 (AC-1) --------------------------------

    int m_beginSceneCallCount{0};
    int m_endSceneCallCount{0};
    int m_lastBeginSceneX{-1};
    int m_lastBeginSceneY{-1};
    int m_lastBeginSceneW{-1};
    int m_lastBeginSceneH{-1};

    void BeginScene(int x, int y, int w, int h) override
    {
        m_lastBeginSceneX = x;
        m_lastBeginSceneY = y;
        m_lastBeginSceneW = w;
        m_lastBeginSceneH = h;
        ++m_beginSceneCallCount;
    }

    void EndScene() override
    {
        ++m_endSceneCallCount;
    }

    // --- New methods required by 7-9-2 (AC-2) --------------------------------

    int m_begin2DCallCount{0};
    int m_end2DCallCount{0};

    void Begin2DPass() override
    {
        ++m_begin2DCallCount;
    }

    void End2DPass() override
    {
        ++m_end2DCallCount;
    }

    // --- New methods required by 7-9-2 (AC-5 / Task 1.4) --------------------

    int m_renderLinesCallCount{0};
    std::size_t m_lastRenderLinesVertexCount{0};
    std::uint32_t m_lastRenderLinesTextureId{0};

    void RenderLines(std::span<const mu::Vertex3D> vertices, std::uint32_t textureId) override
    {
        m_lastRenderLinesVertexCount = vertices.size();
        m_lastRenderLinesTextureId = textureId;
        ++m_renderLinesCallCount;
    }

    // --- New methods required by 7-9-2 (Task 1.5) ----------------------------

    int m_clearScreenCallCount{0};

    void ClearScreen() override
    {
        ++m_clearScreenCallCount;
    }

    // --- New methods required by 7-9-2 (AC-6) --------------------------------

    bool m_isFrameActiveResult{false};

    [[nodiscard]] bool IsFrameActive() const override
    {
        return m_isFrameActiveResult;
    }
};

} // anonymous namespace

// ===========================================================================
// AC-1 [7-9-2]: BeginScene/EndScene routing through IMuRenderer
//
// Verifies that BeginScene(x, y, w, h) and EndScene() exist on IMuRenderer
// and capture viewport parameters correctly. These replace BeginOpengl() /
// EndOpengl() in ZzzOpenglUtil.cpp which called glMatrixMode/glPushMatrix
// directly.
//
// RED PHASE: Fails to compile until BeginScene/EndScene are added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-1 [7-9-2]: BeginScene records viewport parameters", "[render][migration][ac-1]")
{
    SECTION("BeginScene stores x, y, width, height in tracked fields")
    {
        // GIVEN: a fresh mock renderer
        MigrationCaptureMock mock;

        // WHEN: BeginScene called with a 1024×768 full-viewport
        mock.BeginScene(0, 0, 1024, 768);

        // THEN: all viewport params are captured
        REQUIRE(mock.m_beginSceneCallCount == 1);
        REQUIRE(mock.m_lastBeginSceneX == 0);
        REQUIRE(mock.m_lastBeginSceneY == 0);
        REQUIRE(mock.m_lastBeginSceneW == 1024);
        REQUIRE(mock.m_lastBeginSceneH == 768);
    }

    SECTION("EndScene follows BeginScene in matched pairs")
    {
        // GIVEN: a fresh mock
        MigrationCaptureMock mock;

        // WHEN: one full scene lifecycle
        mock.BeginScene(0, 0, 800, 600);
        mock.EndScene();

        // THEN: both call counts are 1
        REQUIRE(mock.m_beginSceneCallCount == 1);
        REQUIRE(mock.m_endSceneCallCount == 1);
    }

    SECTION("Multiple BeginScene/EndScene pairs are independent")
    {
        // GIVEN: a fresh mock
        MigrationCaptureMock mock;

        // WHEN: two consecutive scene lifecycles
        mock.BeginScene(0, 0, 1024, 768);
        mock.EndScene();
        mock.BeginScene(100, 100, 512, 384);
        mock.EndScene();

        // THEN: call counts are 2; last call's params are recorded
        REQUIRE(mock.m_beginSceneCallCount == 2);
        REQUIRE(mock.m_endSceneCallCount == 2);
        REQUIRE(mock.m_lastBeginSceneX == 100);
        REQUIRE(mock.m_lastBeginSceneY == 100);
        REQUIRE(mock.m_lastBeginSceneW == 512);
        REQUIRE(mock.m_lastBeginSceneH == 384);
    }

    SECTION("BeginScene with sub-viewport (split-screen or mini-map scenario)")
    {
        // Documents that non-zero x,y origin is supported — required for any
        // future viewport-relative 3D rendering (e.g., mini-map or editor panels).
        MigrationCaptureMock mock;

        mock.BeginScene(200, 100, 400, 300);

        REQUIRE(mock.m_lastBeginSceneX == 200);
        REQUIRE(mock.m_lastBeginSceneY == 100);
    }
}

// ===========================================================================
// AC-2 [7-9-2]: Begin2DPass/End2DPass routing through IMuRenderer
//
// Verifies that Begin2DPass() and End2DPass() exist on IMuRenderer. These
// replace BeginBitmap() / EndBitmap() in ZzzOpenglUtil.cpp which called
// gluOrtho2D/glDisable directly.
//
// RED PHASE: Fails to compile until Begin2DPass/End2DPass are added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-2 [7-9-2]: Begin2DPass and End2DPass are callable on IMuRenderer",
    "[render][migration][ac-2]")
{
    SECTION("Begin2DPass increments call count")
    {
        // GIVEN: a fresh mock
        MigrationCaptureMock mock;

        // WHEN: 2D pass initiated
        mock.Begin2DPass();

        // THEN: call count is 1
        REQUIRE(mock.m_begin2DCallCount == 1);
    }

    SECTION("End2DPass follows Begin2DPass in matched pairs")
    {
        // GIVEN: a fresh mock
        MigrationCaptureMock mock;

        // WHEN: full 2D pass lifecycle
        mock.Begin2DPass();
        mock.End2DPass();

        // THEN: both counts are 1
        REQUIRE(mock.m_begin2DCallCount == 1);
        REQUIRE(mock.m_end2DCallCount == 1);
    }

    SECTION("2D pass is orthogonal to 3D scene (independent call counts)")
    {
        // Documents the typical frame ordering:
        // BeginScene → [render 3D] → Begin2DPass → [render 2D] → End2DPass → EndScene
        MigrationCaptureMock mock;

        mock.BeginScene(0, 0, 1024, 768);
        mock.Begin2DPass();
        mock.End2DPass();
        mock.EndScene();

        REQUIRE(mock.m_beginSceneCallCount == 1);
        REQUIRE(mock.m_begin2DCallCount == 1);
        REQUIRE(mock.m_end2DCallCount == 1);
        REQUIRE(mock.m_endSceneCallCount == 1);
    }

    SECTION("Multiple 2D pass pairs within a single 3D scene are supported")
    {
        // Some scenes call BeginBitmap/EndBitmap multiple times per frame
        // (e.g., HUD layers). Both pairs must be independent.
        MigrationCaptureMock mock;

        mock.BeginScene(0, 0, 1024, 768);
        mock.Begin2DPass();
        mock.End2DPass();
        mock.Begin2DPass(); // second 2D pass in same frame
        mock.End2DPass();
        mock.EndScene();

        REQUIRE(mock.m_begin2DCallCount == 2);
        REQUIRE(mock.m_end2DCallCount == 2);
    }
}

// ===========================================================================
// AC-5 [7-9-2]: RenderLines via IMuRenderer (line primitive support)
//
// Verifies that RenderLines(std::span<const Vertex3D>, textureId) exists on
// IMuRenderer. Replaces GL_LINES / GL_LINE_STRIP glBegin blocks in:
//   - CameraMove.cpp:490 — waypoint gizmo line strip
//   - ZzzObject.cpp:12240 — collision debug lines
//   - ZzzBMD.cpp:2480 — skeleton debug lines
//
// RED PHASE: Fails to compile until RenderLines is added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-5 [7-9-2]: RenderLines is callable on IMuRenderer", "[render][migration][ac-5]")
{
    SECTION("RenderLines with 2 vertices (single line segment)")
    {
        // GIVEN: a fresh mock and a 2-vertex line
        MigrationCaptureMock mock;

        std::array<mu::Vertex3D, 2> lineVerts{};
        lineVerts[0].x = 0.0f;
        lineVerts[0].y = 0.0f;
        lineVerts[0].z = 0.0f;
        lineVerts[1].x = 100.0f;
        lineVerts[1].y = 100.0f;
        lineVerts[1].z = 0.0f;

        // WHEN: RenderLines called
        mock.RenderLines(std::span<const mu::Vertex3D>(lineVerts), 0u);

        // THEN: vertex count captured
        REQUIRE(mock.m_renderLinesCallCount == 1);
        REQUIRE(mock.m_lastRenderLinesVertexCount == 2);
    }

    SECTION("RenderLines with larger vertex count (waypoint gizmo case)")
    {
        // CameraMove waypoint gizmo may emit 4–8 vertices per gizmo quad
        MigrationCaptureMock mock;

        std::array<mu::Vertex3D, 8> gizmoVerts{};
        mock.RenderLines(std::span<const mu::Vertex3D>(gizmoVerts), 0u);

        REQUIRE(mock.m_lastRenderLinesVertexCount == 8);
    }

    SECTION("RenderLines accepts textureId=0 for untextured lines")
    {
        // Debug line rendering is typically untextured (no texture binding needed)
        MigrationCaptureMock mock;

        std::array<mu::Vertex3D, 2> verts{};
        mock.RenderLines(std::span<const mu::Vertex3D>(verts), 0u);

        REQUIRE(mock.m_lastRenderLinesTextureId == 0u);
    }

    SECTION("RenderLines uses mu::Vertex3D — no OpenGL types in interface")
    {
        // Compile-time check: successful compilation without GL headers proves
        // no OpenGL types (GLenum, GLuint, etc.) leaked into the interface.
        MigrationCaptureMock mock;
        std::array<mu::Vertex3D, 4> verts{};
        mock.RenderLines(std::span<const mu::Vertex3D>(verts), 0u);
        REQUIRE(mock.m_renderLinesCallCount == 1);
    }
}

// ===========================================================================
// AC-6 [7-9-2]: IsFrameActive() lifecycle guard on IMuRenderer
//
// Verifies that IsFrameActive() const exists on IMuRenderer:
//   - Default (OpenGL backend): returns false (GL is immediate, no frame lifecycle)
//   - SDL_gpu backend override: returns true when s_renderPass != nullptr
//
// RenderTitleSceneUI() uses this to self-manage BeginFrame()/EndFrame() when
// called from OpenBasicData() outside the main game loop.
//
// RED PHASE: Fails to compile until IsFrameActive is added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-6 [7-9-2]: IsFrameActive allows conditional frame lifecycle management",
    "[render][migration][ac-6]")
{
    SECTION("OpenGL backend: IsFrameActive returns false (no active frame)")
    {
        // GIVEN: mock simulating OpenGL backend (no active render pass)
        MigrationCaptureMock mock;
        mock.m_isFrameActiveResult = false;

        // THEN: IsFrameActive() returns false — safe to call BeginFrame() if needed
        REQUIRE_FALSE(mock.IsFrameActive());
    }

    SECTION("SDL_gpu backend: IsFrameActive returns true when render pass is open")
    {
        // GIVEN: mock simulating SDL_gpu backend with active render pass
        // (s_renderPass != nullptr — between BeginFrame/EndFrame)
        MigrationCaptureMock mock;
        mock.m_isFrameActiveResult = true;

        // THEN: IsFrameActive() returns true — caller must NOT call BeginFrame() again
        REQUIRE(mock.IsFrameActive());
    }

    SECTION("IsFrameActive is const — callable on const IMuRenderer reference")
    {
        // AC-6 const correctness: RenderTitleSceneUI may receive a const renderer ref
        const MigrationCaptureMock constMock;
        REQUIRE_FALSE(constMock.IsFrameActive());
    }

    SECTION("IsFrameActive state transitions: inactive → active → inactive")
    {
        // Documents the expected lifecycle:
        //   Before BeginFrame(): IsFrameActive() == false
        //   After BeginFrame():  IsFrameActive() == true
        //   After EndFrame():    IsFrameActive() == false
        MigrationCaptureMock mock;
        REQUIRE_FALSE(mock.IsFrameActive()); // initial state

        mock.m_isFrameActiveResult = true;
        REQUIRE(mock.IsFrameActive()); // frame open

        mock.m_isFrameActiveResult = false;
        REQUIRE_FALSE(mock.IsFrameActive()); // frame closed
    }
}

// ===========================================================================
// AC-7 [7-9-2]: ClearScreen callable on IMuRenderer (glClear routing)
//
// Verifies that ClearScreen() exists on IMuRenderer. Replaces raw glClear()
// calls found in scene entry points (WebzenScene, LoadingScene, etc.).
// The SDL_gpu backend no-ops this (SDL_gpu clears in BeginFrame).
// The OpenGL backend calls glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT).
//
// RED PHASE: Fails to compile until ClearScreen is added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-7 [7-9-2]: ClearScreen is callable on IMuRenderer (wraps glClear)",
    "[render][migration][ac-7]")
{
    SECTION("ClearScreen can be called without side effects in test double")
    {
        // GIVEN: a fresh mock
        MigrationCaptureMock mock;

        // WHEN: ClearScreen called
        REQUIRE_NOTHROW(mock.ClearScreen());

        // THEN: call count incremented
        REQUIRE(mock.m_clearScreenCallCount == 1);
    }

    SECTION("ClearScreen can be called multiple times per frame")
    {
        // Some scenes clear multiple times (e.g., layered compositing)
        MigrationCaptureMock mock;

        mock.ClearScreen();
        mock.ClearScreen();

        REQUIRE(mock.m_clearScreenCallCount == 2);
    }
}

// ===========================================================================
// AC-STD-2 [7-9-2]: Extended IMuRenderer interface completeness
//
// Verifies ALL new 7-9-2 methods are present and callable on IMuRenderer.
// Also verifies backward compatibility: pre-existing 4.2.1 methods still work.
// The mere fact this TU compiles proves interface completeness.
//
// RED PHASE: Fails to compile until ALL 7 new methods are added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-STD-2 [7-9-2]: IMuRenderer extended interface — all new methods callable",
    "[render][migration][ac-std-2]")
{
    SECTION("All 7-9-2 new methods callable via test double (compile-time interface check)")
    {
        // GIVEN: a fresh mock implementing all new methods
        MigrationCaptureMock mock;

        // WHEN: all new 7-9-2 methods are called
        mock.BeginScene(0, 0, 1024, 768); // AC-1
        mock.EndScene();                   // AC-1
        mock.Begin2DPass();               // AC-2
        mock.End2DPass();                 // AC-2

        std::array<mu::Vertex3D, 2> lines{};
        mock.RenderLines(std::span<const mu::Vertex3D>(lines), 0u); // AC-5

        mock.ClearScreen();               // Task 1.5

        [[maybe_unused]] const bool active = mock.IsFrameActive(); // AC-6

        // THEN: all call counts reflect single invocations
        REQUIRE(mock.m_beginSceneCallCount == 1);
        REQUIRE(mock.m_endSceneCallCount == 1);
        REQUIRE(mock.m_begin2DCallCount == 1);
        REQUIRE(mock.m_end2DCallCount == 1);
        REQUIRE(mock.m_renderLinesCallCount == 1);
        REQUIRE(mock.m_clearScreenCallCount == 1);
    }

    SECTION("Pre-existing 4.2.1 methods remain callable (non-regression)")
    {
        // Verifies backward compatibility: story 7-9-2 additions must not break
        // the existing IMuRenderer interface from story 4.2.1.
        MigrationCaptureMock mock;

        std::array<mu::Vertex2D, 4> quad2d{};
        mock.RenderQuad2D(std::span<const mu::Vertex2D>(quad2d), 0u);

        std::array<mu::Vertex3D, 3> tris{};
        mock.RenderTriangles(std::span<const mu::Vertex3D>(tris), 0u);

        std::array<mu::Vertex3D, 4> strip{};
        mock.RenderQuadStrip(std::span<const mu::Vertex3D>(strip), 0u);

        mock.SetBlendMode(mu::BlendMode::Alpha);
        mock.DisableBlend();
        mock.SetDepthTest(true);

        mu::FogParams fog{};
        mock.SetFog(fog);

        // Verify pre-existing calls didn't accidentally trigger 7-9-2 counters
        REQUIRE(mock.m_beginSceneCallCount == 0);
        REQUIRE(mock.m_endSceneCallCount == 0);
        REQUIRE(mock.m_begin2DCallCount == 0);
        REQUIRE(mock.m_end2DCallCount == 0);
        REQUIRE(mock.m_renderLinesCallCount == 0);
        REQUIRE(mock.m_clearScreenCallCount == 0);
    }

    SECTION("IMuRenderer has no OpenGL types in extended interface (cross-platform compile check)")
    {
        // This TU includes MuRenderer.h WITHOUT any OpenGL headers.
        // Successful compilation proves no GL types (GLenum, GLuint, GLfloat,
        // GLsizei, etc.) are present in the new or existing IMuRenderer methods.
        MigrationCaptureMock mock;
        mock.SetBlendMode(mu::BlendMode::Additive);

        // Compilation is the real assertion; verify no side effects on 7-9-2 counters
        REQUIRE(mock.m_beginSceneCallCount == 0);
    }
}

// ===========================================================================
// AC-3 [7-9-2]: CSprite coordinate conversion — 640×480 → screen pixels
//
// Validates the coordinate conversion formula used by CSprite::Render() to
// transform OpenGL bottom-up 640×480 logical coordinates to screen-space
// pixels. The formula is:
//   screenX = scrX * scaleX * (WindowWidth / 640.0f)
//   screenY = WindowHeight - (scrY * scaleY * (WindowHeight / 480.0f))
//
// Also validates ABGR color packing and full Vertex2D quad construction.
// These tests exercise the SAME math that CSprite::Render() uses, without
// requiring the full game engine (no WindowWidth/WindowHeight globals needed).
// ===========================================================================
TEST_CASE("AC-3 [7-9-2]: CSprite coordinate conversion — 640x480 to screen pixels",
    "[render][migration][ac-3]")
{
    SECTION("Center of 640x480 maps to center of screen")
    {
        // Scenario: 1024×768 display, scale = 1.0
        constexpr float windowWidth = 1024.0f;
        constexpr float windowHeight = 768.0f;
        constexpr float scaleX = 1.0f;
        constexpr float scaleY = 1.0f;

        // Logical center of 640×480 space
        constexpr float logicalX = 320.0f;
        constexpr float logicalY = 240.0f;

        const float screenX = logicalX * scaleX * (windowWidth / 640.0f);
        const float screenY = windowHeight - (logicalY * scaleY * (windowHeight / 480.0f));

        // Center of 640×480 maps to center of 1024×768
        REQUIRE(screenX == Catch::Approx(512.0f));
        REQUIRE(screenY == Catch::Approx(384.0f));
    }

    SECTION("Y-axis inversion: OpenGL bottom-up to screen top-down")
    {
        constexpr float windowHeight = 768.0f;
        constexpr float scaleY = 1.0f;

        // OpenGL Y=0 (bottom) → screen Y=WindowHeight (bottom of screen)
        const float screenY_bottom = windowHeight - (0.0f * scaleY * (windowHeight / 480.0f));
        REQUIRE(screenY_bottom == Catch::Approx(768.0f));

        // OpenGL Y=480 (top) → screen Y=0 (top of screen)
        const float screenY_top = windowHeight - (480.0f * scaleY * (windowHeight / 480.0f));
        REQUIRE(screenY_top == Catch::Approx(0.0f));
    }

    SECTION("Origin maps to bottom-left of screen")
    {
        constexpr float windowWidth = 1280.0f;
        constexpr float windowHeight = 960.0f;
        constexpr float scaleX = 1.0f;
        constexpr float scaleY = 1.0f;

        const float screenX = 0.0f * scaleX * (windowWidth / 640.0f);
        const float screenY = windowHeight - (0.0f * scaleY * (windowHeight / 480.0f));

        REQUIRE(screenX == Catch::Approx(0.0f));
        REQUIRE(screenY == Catch::Approx(960.0f)); // bottom of screen
    }

    SECTION("ABGR color packing: alpha=255, blue=128, green=64, red=32")
    {
        // CSprite::Render packs: (alpha << 24) | (blue << 16) | (green << 8) | red
        const auto alpha = static_cast<std::uint32_t>(255);
        const auto blue = static_cast<std::uint32_t>(128);
        const auto green = static_cast<std::uint32_t>(64);
        const auto red = static_cast<std::uint32_t>(32);

        const std::uint32_t color = (alpha << 24) | (blue << 16) | (green << 8) | red;

        REQUIRE(color == 0xFF804020u);
    }

    SECTION("Full Vertex2D quad construction with coordinate conversion")
    {
        // Simulates the CSprite::Render() loop that builds Vertex2D[4]
        constexpr float windowWidth = 1280.0f;
        constexpr float windowHeight = 960.0f;
        constexpr float scaleX = 1.0f;
        constexpr float scaleY = 1.0f;

        // Sprite corners in 640×480 space: (100,100) to (200,200)
        struct ScrCoord
        {
            float fX, fY;
        };
        struct TexCoord
        {
            float fTU, fTV;
        };
        constexpr int kQuadVerts = 4;
        const ScrCoord scrCoords[kQuadVerts] = {{100, 100}, {100, 200}, {200, 200}, {200, 100}};
        const TexCoord texCoords[kQuadVerts] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
        const std::uint32_t color = 0xFFFFFFFFu;

        mu::Vertex2D vertices[kQuadVerts];
        for (int i = 0; i < kQuadVerts; ++i)
        {
            vertices[i].x = scrCoords[i].fX * scaleX * (windowWidth / 640.0f);
            vertices[i].y = windowHeight - (scrCoords[i].fY * scaleY * (windowHeight / 480.0f));
            vertices[i].u = texCoords[i].fTU;
            vertices[i].v = texCoords[i].fTV;
            vertices[i].color = color;
        }

        // Verify coordinate conversion: 100 * (1280/640) = 200
        REQUIRE(vertices[0].x == Catch::Approx(200.0f));
        // 960 - 100*(960/480) = 960 - 200 = 760
        REQUIRE(vertices[0].y == Catch::Approx(760.0f));

        // Verify opposite corner: 200 * (1280/640) = 400
        REQUIRE(vertices[2].x == Catch::Approx(400.0f));
        // 960 - 200*(960/480) = 960 - 400 = 560
        REQUIRE(vertices[2].y == Catch::Approx(560.0f));

        // Verify texture coords preserved
        REQUIRE(vertices[0].u == Catch::Approx(0.0f));
        REQUIRE(vertices[0].v == Catch::Approx(0.0f));
        REQUIRE(vertices[2].u == Catch::Approx(1.0f));
        REQUIRE(vertices[2].v == Catch::Approx(1.0f));

        // Verify color
        REQUIRE(vertices[0].color == 0xFFFFFFFFu);
    }

    SECTION("Untextured sprite: texture coords are zero when textureId < 0")
    {
        // CSprite::Render uses textureId = 0 sentinel for m_nTexID == -1
        constexpr int textureId = -1;
        mu::Vertex2D vert{};
        vert.u = (textureId >= 0) ? 0.5f : 0.0f;
        vert.v = (textureId >= 0) ? 0.5f : 0.0f;

        REQUIRE(vert.u == Catch::Approx(0.0f));
        REQUIRE(vert.v == Catch::Approx(0.0f));
    }

    SECTION("RenderQuad2D receives correct vertex data via mock")
    {
        // Verify the complete pipeline: build vertices → call RenderQuad2D → capture data
        MigrationCaptureMock mock;
        constexpr int kQuadVerts = 4;
        std::array<mu::Vertex2D, kQuadVerts> verts{};

        // Set up a simple sprite quad at 640×480 origin
        verts[0] = {0.0f, 768.0f, 0.0f, 0.0f, 0xFFFFFFFFu};
        verts[1] = {0.0f, 0.0f, 0.0f, 1.0f, 0xFFFFFFFFu};
        verts[2] = {1024.0f, 0.0f, 1.0f, 1.0f, 0xFFFFFFFFu};
        verts[3] = {1024.0f, 768.0f, 1.0f, 0.0f, 0xFFFFFFFFu};

        const std::uint32_t textureId = 42u;
        mock.RenderQuad2D(std::span<const mu::Vertex2D>(verts), textureId);

        // Verify mock captured the vertex data (Finding #9 fix)
        REQUIRE(mock.m_renderQuad2DCallCount == 1);
        REQUIRE(mock.m_lastQuad2DTextureId == 42u);
        REQUIRE(mock.m_lastQuad2DVertices.size() == 4);
        REQUIRE(mock.m_lastQuad2DVertices[0].x == Catch::Approx(0.0f));
        REQUIRE(mock.m_lastQuad2DVertices[0].y == Catch::Approx(768.0f));
        REQUIRE(mock.m_lastQuad2DVertices[2].x == Catch::Approx(1024.0f));
        REQUIRE(mock.m_lastQuad2DVertices[2].color == 0xFFFFFFFFu);

        // Verify no cross-contamination with other 7-9-2 counters
        REQUIRE(mock.m_beginSceneCallCount == 0);
        REQUIRE(mock.m_renderLinesCallCount == 0);
        REQUIRE(mock.m_clearScreenCallCount == 0);
    }
}

// ===========================================================================
// AC-STD-1 [7-9-2]: No #ifdef rendering guards in game code
//
// This test documents the design contract: all rendering must go through
// IMuRenderer unconditionally. No #ifdef _WIN32 or #ifdef MU_ENABLE_SDL3
// may appear at call sites.
//
// Verification method: python3 MuMain/scripts/check-win32-guards.py (AC-9)
// This test serves as a companion regression check for the interface contract.
// ===========================================================================
TEST_CASE("AC-STD-1 [7-9-2]: IMuRenderer call sites are platform-unconditional",
    "[render][migration][ac-std-1]")
{
    SECTION("New IMuRenderer methods callable without #ifdef guards")
    {
        // Game code calls all renderer methods unconditionally.
        // The OpenGL and SDL_gpu backends provide different implementations
        // but the call site is always the same.
        MigrationCaptureMock mock;

        // Simulate what BeginOpengl() will become (AC-1):
        //   mu::GetRenderer().BeginScene(x, y, w, h);
        mock.BeginScene(0, 0, 1024, 768);

        // Simulate what BeginBitmap() will become (AC-2):
        //   mu::GetRenderer().Begin2DPass();
        mock.Begin2DPass();

        // Simulate what scene code will call (AC-7):
        //   mu::GetRenderer().ClearScreen();
        mock.ClearScreen();

        // All calls made without any conditionals — contract verified
        REQUIRE(mock.m_beginSceneCallCount == 1);
        REQUIRE(mock.m_begin2DCallCount == 1);
        REQUIRE(mock.m_clearScreenCallCount == 1);
    }
}
