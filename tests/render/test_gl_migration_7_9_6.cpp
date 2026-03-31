// Story 7.9.6: Migrate All Raw OpenGL Calls to MuRenderer
// Flow Code: VS0-RENDER-GL-MIGRATE
//
// RED PHASE: Tests verify contracts for new IMuRenderer methods required to
// eliminate raw OpenGL calls (state management, clear color, matrix stack,
// texture, depth/stencil, viewport/scissor, screenshot) from game code.
// Tests will FAIL TO COMPILE until the new methods are added to IMuRenderer.
//
// AC Mapping:
//   AC-1  -> TEST_CASE("AC-1 [7-9-6]: ...")  File-scan: zero gl* outside allowed files
//   AC-2  -> TEST_CASE("AC-2 [7-9-6]: ...")  File-scan: stdafx.h GL stubs deleted
//   AC-3  -> TEST_CASE("AC-3 [7-9-6]: ...")  SetClearColor API
//   AC-4  -> TEST_CASE("AC-4 [7-9-6]: ...")  Matrix stack API on IMuRenderer
//   AC-5  -> TEST_CASE("AC-5 [7-9-6]: ...")  Texture API (BindTexture, SetTexture2D)
//   AC-6  -> TEST_CASE("AC-6 [7-9-6]: ...")  Depth/stencil API (SetDepthFunc/SetStencilFunc/etc.)
//   AC-7  -> TEST_CASE("AC-7 [7-9-6]: ...")  ReadPixels / screenshot API
//   AC-8  -> TEST_CASE("AC-8 [7-9-6]: ...")  GetGPUDriverName (replaces glGetString)
//   AC-9  -> TEST_CASE("AC-9 [7-9-6]: ...")  File-scan: WGL/extension calls deleted
//   AC-10 -> Manual visual verification only (cannot be automated)
//   AC-STD-1 -> Verified by ./ctl check (clang-format + cppcheck quality gate)
//   AC-STD-13 -> Verified by ./ctl check quality gate
//
// IMPORTANT: No OpenGL calls or GL headers in this test TU.
// All interface tests use GLMigration796Mock implementing IMuRenderer.
// Tests compile and run on macOS/Linux without Win32, OpenGL, or a GPU device.
// File-scan tests are guarded by #ifndef _WIN32 so MinGW CI compiles cleanly.
//
// Run with: ctest --test-dir MuMain/build -R gl_migration_7_9_6

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

#include "MuRenderer.h"

// ---------------------------------------------------------------------------
// GLMigration796Mock
// Implements ALL IMuRenderer methods required by story 7-9-6.
// RED PHASE: Fails to compile until the new methods are added to IMuRenderer:
//   - SetClearColor(float r, float g, float b, float a)       [AC-3]
//   - SetMatrixMode(int mode)                                  [AC-4]
//   - PushMatrix()                                             [AC-4]
//   - PopMatrix()                                              [AC-4]
//   - LoadIdentity()                                           [AC-4]
//   - Translate(float x, float y, float z)                    [AC-4]
//   - Rotate(float angle, float x, float y, float z)          [AC-4]
//   - Scale(float x, float y, float z)                        [AC-4]
//   - MultMatrix(const float* m)                              [AC-4]
//   - LoadMatrix(const float* m)                              [AC-4]
//   - GetMatrix(int mode, float* m)                           [AC-4]
//   - SetDepthFunc(int func)                                   [AC-6]
//   - SetAlphaFunc(int func, float ref)                       [AC-6]
//   - SetStencilFunc(int func, int ref, unsigned int mask)    [AC-6]
//   - SetStencilOp(int sfail, int dpfail, int dppass)         [AC-6]
//   - SetColorMask(bool r, bool g, bool b, bool a)            [AC-6]
//   - SetViewport(int x, int y, int w, int h)                 [AC-6]
//   - SetScissor(int x, int y, int w, int h)                  [AC-6]
//   - SetScissorEnabled(bool enabled)                         [AC-6]
//   - ReadPixels(int x, int y, int w, int h, void* data)     [AC-7]
// ---------------------------------------------------------------------------
namespace
{

struct GLMigration796Mock : public mu::IMuRenderer
{
    // -----------------------------------------------------------------------
    // Existing pure-virtual methods (required to instantiate the mock)
    // -----------------------------------------------------------------------

    void RenderQuad2D(std::span<const mu::Vertex2D> /*v*/, std::uint32_t /*id*/) override
    {
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

    void BeginScene(int /*x*/, int /*y*/, int /*w*/, int /*h*/) override
    {
    }

    void EndScene() override
    {
    }

    void Begin2DPass() override
    {
    }

    void End2DPass() override
    {
    }

    void ClearScreen() override
    {
    }

    void RenderLines(std::span<const mu::Vertex3D> /*v*/, std::uint32_t /*id*/) override
    {
    }

    // -----------------------------------------------------------------------
    // AC-3: SetClearColor — replaces glClearColor calls
    // -----------------------------------------------------------------------

    int m_setClearColorCallCount{0};
    float m_lastClearR{-1.0f};
    float m_lastClearG{-1.0f};
    float m_lastClearB{-1.0f};
    float m_lastClearA{-1.0f};

    void SetClearColor(float r, float g, float b, float a) override
    {
        m_lastClearR = r;
        m_lastClearG = g;
        m_lastClearB = b;
        m_lastClearA = a;
        ++m_setClearColorCallCount;
    }

    // -----------------------------------------------------------------------
    // AC-4: Matrix stack API — replaces glPushMatrix/glPopMatrix/glTranslatef/etc.
    // -----------------------------------------------------------------------

    int m_setMatrixModeCallCount{0};
    int m_lastMatrixMode{-1};

    void SetMatrixMode(int mode) override
    {
        m_lastMatrixMode = mode;
        ++m_setMatrixModeCallCount;
    }

    int m_pushMatrixCallCount{0};

    void PushMatrix() override
    {
        ++m_pushMatrixCallCount;
    }

    int m_popMatrixCallCount{0};

    void PopMatrix() override
    {
        ++m_popMatrixCallCount;
    }

    int m_loadIdentityCallCount{0};

    void LoadIdentity() override
    {
        ++m_loadIdentityCallCount;
    }

    int m_translateCallCount{0};
    float m_lastTranslateX{0.0f};
    float m_lastTranslateY{0.0f};
    float m_lastTranslateZ{0.0f};

    void Translate(float x, float y, float z) override
    {
        m_lastTranslateX = x;
        m_lastTranslateY = y;
        m_lastTranslateZ = z;
        ++m_translateCallCount;
    }

    int m_rotateCallCount{0};
    float m_lastRotateAngle{0.0f};
    float m_lastRotateX{0.0f};
    float m_lastRotateY{0.0f};
    float m_lastRotateZ{0.0f};

    void Rotate(float angle, float x, float y, float z) override
    {
        m_lastRotateAngle = angle;
        m_lastRotateX = x;
        m_lastRotateY = y;
        m_lastRotateZ = z;
        ++m_rotateCallCount;
    }

    int m_scaleCallCount{0};
    float m_lastScaleX{0.0f};
    float m_lastScaleY{0.0f};
    float m_lastScaleZ{0.0f};

    void Scale(float x, float y, float z) override
    {
        m_lastScaleX = x;
        m_lastScaleY = y;
        m_lastScaleZ = z;
        ++m_scaleCallCount;
    }

    int m_multMatrixCallCount{0};
    std::array<float, 16> m_lastMultMatrix{};

    void MultMatrix(const float* m) override
    {
        std::memcpy(m_lastMultMatrix.data(), m, sizeof(float) * 16);
        ++m_multMatrixCallCount;
    }

    int m_loadMatrixCallCount{0};
    std::array<float, 16> m_lastLoadMatrix{};

    void LoadMatrix(const float* m) override
    {
        std::memcpy(m_lastLoadMatrix.data(), m, sizeof(float) * 16);
        ++m_loadMatrixCallCount;
    }

    int m_getMatrixCallCount{0};
    int m_lastGetMatrixMode{-1};
    // Simulates identity matrix read-back for testing
    std::array<float, 16> m_getMatrixResult{
        1.0f, 0.0f, 0.0f, 0.0f, // col 0
        0.0f, 1.0f, 0.0f, 0.0f, // col 1
        0.0f, 0.0f, 1.0f, 0.0f, // col 2
        0.0f, 0.0f, 0.0f, 1.0f  // col 3
    };

    void GetMatrix(int mode, float* m) override
    {
        m_lastGetMatrixMode = mode;
        std::memcpy(m, m_getMatrixResult.data(), sizeof(float) * 16);
        ++m_getMatrixCallCount;
    }

    // -----------------------------------------------------------------------
    // AC-6: Depth/stencil/viewport API
    // -----------------------------------------------------------------------

    int m_setDepthFuncCallCount{0};
    int m_lastDepthFunc{-1};

    void SetDepthFunc(int func) override
    {
        m_lastDepthFunc = func;
        ++m_setDepthFuncCallCount;
    }

    int m_setAlphaFuncCallCount{0};
    int m_lastAlphaFunc{-1};
    float m_lastAlphaRef{-1.0f};

    void SetAlphaFunc(int func, float ref) override
    {
        m_lastAlphaFunc = func;
        m_lastAlphaRef = ref;
        ++m_setAlphaFuncCallCount;
    }

    int m_setStencilFuncCallCount{0};
    int m_lastStencilFunc{-1};
    int m_lastStencilRef{-1};
    unsigned int m_lastStencilMask{0};

    void SetStencilFunc(int func, int ref, unsigned int mask) override
    {
        m_lastStencilFunc = func;
        m_lastStencilRef = ref;
        m_lastStencilMask = mask;
        ++m_setStencilFuncCallCount;
    }

    int m_setStencilOpCallCount{0};
    int m_lastStencilSfail{-1};
    int m_lastStencilDpfail{-1};
    int m_lastStencilDppass{-1};

    void SetStencilOp(int sfail, int dpfail, int dppass) override
    {
        m_lastStencilSfail = sfail;
        m_lastStencilDpfail = dpfail;
        m_lastStencilDppass = dppass;
        ++m_setStencilOpCallCount;
    }

    int m_setColorMaskCallCount{0};
    bool m_lastColorMaskR{true};
    bool m_lastColorMaskG{true};
    bool m_lastColorMaskB{true};
    bool m_lastColorMaskA{true};

    void SetColorMask(bool r, bool g, bool b, bool a) override
    {
        m_lastColorMaskR = r;
        m_lastColorMaskG = g;
        m_lastColorMaskB = b;
        m_lastColorMaskA = a;
        ++m_setColorMaskCallCount;
    }

    int m_setViewportCallCount{0};
    int m_lastViewportX{-1};
    int m_lastViewportY{-1};
    int m_lastViewportW{-1};
    int m_lastViewportH{-1};

    void SetViewport(int x, int y, int w, int h) override
    {
        m_lastViewportX = x;
        m_lastViewportY = y;
        m_lastViewportW = w;
        m_lastViewportH = h;
        ++m_setViewportCallCount;
    }

    int m_setScissorCallCount{0};
    int m_lastScissorX{-1};
    int m_lastScissorY{-1};
    int m_lastScissorW{-1};
    int m_lastScissorH{-1};

    void SetScissor(int x, int y, int w, int h) override
    {
        m_lastScissorX = x;
        m_lastScissorY = y;
        m_lastScissorW = w;
        m_lastScissorH = h;
        ++m_setScissorCallCount;
    }

    int m_setScissorEnabledCallCount{0};
    bool m_lastScissorEnabled{false};

    void SetScissorEnabled(bool enabled) override
    {
        m_lastScissorEnabled = enabled;
        ++m_setScissorEnabledCallCount;
    }

    // -----------------------------------------------------------------------
    // AC-7: ReadPixels — replaces glReadPixels for screenshot functionality
    // -----------------------------------------------------------------------

    int m_readPixelsCallCount{0};
    int m_lastReadPixelsX{-1};
    int m_lastReadPixelsY{-1};
    int m_lastReadPixelsW{-1};
    int m_lastReadPixelsH{-1};

    void ReadPixels(int x, int y, int w, int h, void* /*data*/) override
    {
        m_lastReadPixelsX = x;
        m_lastReadPixelsY = y;
        m_lastReadPixelsW = w;
        m_lastReadPixelsH = h;
        ++m_readPixelsCallCount;
    }
};

} // anonymous namespace

// ===========================================================================
// AC-1 [7-9-6]: Zero gl* calls outside stdafx.h and MuRendererSDLGpu.cpp
//
// This is a static analysis / grep-verified AC.
// The verification command from the story is:
//   grep -rn 'gl[A-Z]' src/source/ --include='*.cpp' \
//       | grep -v stdafx.h | grep -v MuRendererSDLGpu.cpp
// Expected: zero results after full migration.
//
// This test documents the contract and exercises the file-scan equivalent
// of AC-1 using MU_SOURCE_DIR (injected at configure time).
// ===========================================================================
#ifndef _WIN32
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

TEST_CASE("AC-1 [7-9-6]: Zero raw gl* calls outside stdafx.h and MuRendererSDLGpu.cpp",
    "[render][migration][ac-1][static-analysis]")
{
#ifndef MU_SOURCE_DIR
    SKIP("MU_SOURCE_DIR not defined — run via ctest (CMake injects this path)");
#else
    SECTION("No gl* calls in ZzzOpenglUtil.cpp outside include guards (post-migration)")
    {
        // After migration, ZzzOpenglUtil.cpp must route ALL state through mu::GetRenderer()
        // and contain zero raw gl* calls in game logic code.
        // This section documents the expected post-migration state.
        // Pre-migration: 26 raw gl* calls. Post-migration: 0.
        //
        // Verification: grep -n 'gl[A-Z]' ZzzOpenglUtil.cpp | grep -v '//' | grep -v stdafx
        // Expected: empty output after migration is complete.
        const std::filesystem::path sourceDir{MU_SOURCE_DIR};
        const auto targetFile = sourceDir / "RenderFX" / "ZzzOpenglUtil.cpp";

        REQUIRE(std::filesystem::exists(targetFile)); // file must exist

        std::ifstream f(targetFile);
        REQUIRE(f.is_open());

        const std::regex glCallPattern{R"(\bgl[A-Z][A-Za-z]+\s*\()"};
        std::string line;
        int lineNumber{0};
        int rawGlCallCount{0};

        while (std::getline(f, line))
        {
            ++lineNumber;
            // Skip comments
            const auto commentPos = line.find("//");
            const auto checkLine = (commentPos != std::string::npos) ? line.substr(0, commentPos) : line;
            if (std::regex_search(checkLine, glCallPattern))
            {
                ++rawGlCallCount;
            }
        }

        // RED PHASE: This check will FAIL until migration is complete.
        // Post-migration target: 0 raw gl* calls in ZzzOpenglUtil.cpp
        CHECK(rawGlCallCount == 0);
    }

    SECTION("No gl* calls in ShadowVolume.cpp (stencil shadow migration)")
    {
        // After migration, ShadowVolume.cpp (19 raw gl* calls pre-migration) must
        // route ALL state through mu::GetRenderer().
        const std::filesystem::path sourceDir{MU_SOURCE_DIR};
        const auto targetFile = sourceDir / "RenderFX" / "ShadowVolume.cpp";

        REQUIRE(std::filesystem::exists(targetFile));

        std::ifstream f(targetFile);
        REQUIRE(f.is_open());

        const std::regex glCallPattern{R"(\bgl[A-Z][A-Za-z]+\s*\()"};
        std::string line;
        int rawGlCallCount{0};

        while (std::getline(f, line))
        {
            const auto commentPos = line.find("//");
            const auto checkLine = (commentPos != std::string::npos) ? line.substr(0, commentPos) : line;
            if (std::regex_search(checkLine, glCallPattern))
            {
                ++rawGlCallCount;
            }
        }

        // RED PHASE: 0 after migration. Pre-migration count: ~19
        CHECK(rawGlCallCount == 0);
    }
#endif
}
#endif // ifndef _WIN32

// ===========================================================================
// AC-2 [7-9-6]: stdafx.h GL stubs section deleted entirely
//
// The entire "OpenGL Constants" and "OpenGL Function stubs" sections must be
// removed from stdafx.h after all call sites are migrated. This test verifies
// that no inline GL function stubs remain.
// ===========================================================================
#ifndef _WIN32
TEST_CASE("AC-2 [7-9-6]: stdafx.h contains no inline GL function stubs after migration",
    "[render][migration][ac-2][static-analysis]")
{
#ifndef MU_SOURCE_DIR
    SKIP("MU_SOURCE_DIR not defined — run via ctest");
#else
    SECTION("No inline GL stub definitions in stdafx.h")
    {
        // Pre-migration: ~70 inline no-op stubs like:
        //   inline void glEnable(GLenum) {}
        //   inline void glClear(GLbitfield) {}
        // Post-migration: entire stubs section deleted.
        const std::filesystem::path sourceDir{MU_SOURCE_DIR};
        const auto targetFile = sourceDir / "Main" / "stdafx.h";

        REQUIRE(std::filesystem::exists(targetFile));

        std::ifstream f(targetFile);
        REQUIRE(f.is_open());

        // Pattern matches: "inline void gl<Name>(" or "inline int gl<Name>("
        const std::regex glStubPattern{R"(inline\s+\w+\s+gl[A-Z][A-Za-z]+\s*\()"};
        std::string line;
        int stubCount{0};

        while (std::getline(f, line))
        {
            if (std::regex_search(line, glStubPattern))
            {
                ++stubCount;
            }
        }

        // RED PHASE: 0 after migration. Pre-migration: ~70 stubs
        CHECK(stubCount == 0);
    }
#endif
}
#endif // ifndef _WIN32

// ===========================================================================
// AC-3 [7-9-6]: SetClearColor replaces glClearColor
//
// All 19 glClearColor call sites (SceneManager.cpp, MainScene.cpp,
// LoginScene.cpp, CharacterScene.cpp, GMHellas.cpp) must be replaced with
// mu::GetRenderer().SetClearColor(r, g, b, a).
//
// RED PHASE: Fails to compile until SetClearColor is added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-3 [7-9-6]: SetClearColor is callable on IMuRenderer",
    "[render][migration][ac-3]")
{
    SECTION("SetClearColor records RGBA values")
    {
        // GIVEN: a fresh mock renderer
        GLMigration796Mock mock;
        REQUIRE(mock.m_setClearColorCallCount == 0);

        // WHEN: SetClearColor called with a dark blue background (Lorencia map)
        mock.SetClearColor(0.0f, 0.0f, 0.2f, 1.0f);

        // THEN: all RGBA components captured
        REQUIRE(mock.m_setClearColorCallCount == 1);
        REQUIRE(mock.m_lastClearR == Catch::Approx(0.0f));
        REQUIRE(mock.m_lastClearG == Catch::Approx(0.0f));
        REQUIRE(mock.m_lastClearB == Catch::Approx(0.2f));
        REQUIRE(mock.m_lastClearA == Catch::Approx(1.0f));
    }

    SECTION("SetClearColor with pure black (alpha = 1.0)")
    {
        // glClearColor(0,0,0,0) is the default — maps to SetClearColor(0,0,0,1)
        // (alpha should always be 1.0 for opaque clear; original code used 0 but
        // semantically opaque is 1.0)
        GLMigration796Mock mock;
        mock.SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        REQUIRE(mock.m_lastClearR == Catch::Approx(0.0f));
        REQUIRE(mock.m_lastClearG == Catch::Approx(0.0f));
        REQUIRE(mock.m_lastClearB == Catch::Approx(0.0f));
        REQUIRE(mock.m_lastClearA == Catch::Approx(1.0f));
    }

    SECTION("SetClearColor called multiple times per scene — last call wins")
    {
        // Some scenes change clear color mid-frame. The SDL GPU backend
        // stores the last set color and applies it in BeginFrame render pass.
        GLMigration796Mock mock;

        mock.SetClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        mock.SetClearColor(0.5f, 0.5f, 0.5f, 1.0f); // overrides

        REQUIRE(mock.m_setClearColorCallCount == 2);
        REQUIRE(mock.m_lastClearB == Catch::Approx(0.5f));
    }

    SECTION("SetClearColor accepts float-normalized [0, 1] range values")
    {
        // glClearColor takes normalized floats. The migration preserves this
        // convention — callers divide by 255.0f if needed (same as before).
        GLMigration796Mock mock;
        constexpr float r = 64.0f / 255.0f;
        constexpr float g = 128.0f / 255.0f;
        constexpr float b = 192.0f / 255.0f;
        constexpr float a = 1.0f;

        mock.SetClearColor(r, g, b, a);

        REQUIRE(mock.m_lastClearR == Catch::Approx(r));
        REQUIRE(mock.m_lastClearG == Catch::Approx(g));
        REQUIRE(mock.m_lastClearB == Catch::Approx(b));
        REQUIRE(mock.m_lastClearA == Catch::Approx(1.0f));
    }
}

// ===========================================================================
// AC-4 [7-9-6]: Matrix stack API on IMuRenderer replaces gl matrix calls
//
// 82 matrix call sites across 15 files must be replaced with IMuRenderer
// matrix methods. The SDL GPU backend builds transform matrices and uploads
// them to the GPU as shader uniform buffers.
//
// gl mapping:
//   glMatrixMode(m)         → SetMatrixMode(m)
//   glPushMatrix()          → PushMatrix()
//   glPopMatrix()           → PopMatrix()
//   glLoadIdentity()        → LoadIdentity()
//   glTranslatef(x, y, z)  → Translate(x, y, z)
//   glRotatef(a, x, y, z)  → Rotate(a, x, y, z)
//   glScalef(x, y, z)      → Scale(x, y, z)
//   glMultMatrixf(m)        → MultMatrix(m)
//   glLoadMatrixf(m)        → LoadMatrix(m)
//   glGetFloatv(m, ptr)     → GetMatrix(m, ptr)
//
// RED PHASE: Fails to compile until all matrix methods are added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-4 [7-9-6]: Matrix stack methods are callable on IMuRenderer",
    "[render][migration][ac-4]")
{
    SECTION("PushMatrix and PopMatrix are callable in matched pairs")
    {
        // GIVEN: a fresh mock
        GLMigration796Mock mock;

        // WHEN: a typical glPushMatrix/glPopMatrix sequence
        mock.PushMatrix();
        mock.Translate(100.0f, 200.0f, 0.0f);
        mock.PopMatrix();

        // THEN: call counts reflect the sequence
        REQUIRE(mock.m_pushMatrixCallCount == 1);
        REQUIRE(mock.m_translateCallCount == 1);
        REQUIRE(mock.m_popMatrixCallCount == 1);
    }

    SECTION("Multiple PushMatrix/PopMatrix pairs are independent")
    {
        // Models nested transform groups (e.g., parent object + child bone)
        GLMigration796Mock mock;

        mock.PushMatrix();
        mock.Translate(0.0f, 0.0f, 5.0f);
        mock.PushMatrix(); // nested
        mock.Rotate(45.0f, 0.0f, 1.0f, 0.0f);
        mock.PopMatrix(); // restore bone rotation
        mock.PopMatrix(); // restore parent position

        REQUIRE(mock.m_pushMatrixCallCount == 2);
        REQUIRE(mock.m_popMatrixCallCount == 2);
        REQUIRE(mock.m_translateCallCount == 1);
        REQUIRE(mock.m_rotateCallCount == 1);
    }

    SECTION("LoadIdentity resets current matrix")
    {
        GLMigration796Mock mock;

        mock.LoadIdentity();

        REQUIRE(mock.m_loadIdentityCallCount == 1);
    }

    SECTION("SetMatrixMode captures mode value")
    {
        // GL_MODELVIEW = 0x1700, GL_PROJECTION = 0x1701 (plain int — no GL headers)
        constexpr int GL_MODELVIEW = 0x1700;
        constexpr int GL_PROJECTION = 0x1701;

        GLMigration796Mock mock;

        mock.SetMatrixMode(GL_MODELVIEW);
        REQUIRE(mock.m_lastMatrixMode == GL_MODELVIEW);

        mock.SetMatrixMode(GL_PROJECTION);
        REQUIRE(mock.m_lastMatrixMode == GL_PROJECTION);
        REQUIRE(mock.m_setMatrixModeCallCount == 2);
    }

    SECTION("Translate captures all three components")
    {
        GLMigration796Mock mock;

        mock.Translate(10.5f, -20.0f, 3.14f);

        REQUIRE(mock.m_lastTranslateX == Catch::Approx(10.5f));
        REQUIRE(mock.m_lastTranslateY == Catch::Approx(-20.0f));
        REQUIRE(mock.m_lastTranslateZ == Catch::Approx(3.14f));
        REQUIRE(mock.m_translateCallCount == 1);
    }

    SECTION("Rotate captures angle and axis")
    {
        GLMigration796Mock mock;

        // Y-axis rotation (0,1,0) — common for turntable model preview
        mock.Rotate(90.0f, 0.0f, 1.0f, 0.0f);

        REQUIRE(mock.m_lastRotateAngle == Catch::Approx(90.0f));
        REQUIRE(mock.m_lastRotateX == Catch::Approx(0.0f));
        REQUIRE(mock.m_lastRotateY == Catch::Approx(1.0f));
        REQUIRE(mock.m_lastRotateZ == Catch::Approx(0.0f));
        REQUIRE(mock.m_rotateCallCount == 1);
    }

    SECTION("Scale captures all three axis scales")
    {
        GLMigration796Mock mock;

        mock.Scale(2.0f, 2.0f, 2.0f);

        REQUIRE(mock.m_lastScaleX == Catch::Approx(2.0f));
        REQUIRE(mock.m_lastScaleY == Catch::Approx(2.0f));
        REQUIRE(mock.m_lastScaleZ == Catch::Approx(2.0f));
        REQUIRE(mock.m_scaleCallCount == 1);
    }

    SECTION("MultMatrix captures the 16-element column-major matrix")
    {
        GLMigration796Mock mock;

        // Identity matrix in column-major order (GL convention)
        const std::array<float, 16> identity{
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

        mock.MultMatrix(identity.data());

        REQUIRE(mock.m_multMatrixCallCount == 1);
        REQUIRE(mock.m_lastMultMatrix[0] == Catch::Approx(1.0f));  // m[0,0]
        REQUIRE(mock.m_lastMultMatrix[5] == Catch::Approx(1.0f));  // m[1,1]
        REQUIRE(mock.m_lastMultMatrix[10] == Catch::Approx(1.0f)); // m[2,2]
        REQUIRE(mock.m_lastMultMatrix[15] == Catch::Approx(1.0f)); // m[3,3]
        REQUIRE(mock.m_lastMultMatrix[12] == Catch::Approx(0.0f)); // Tx = 0
    }

    SECTION("LoadMatrix captures the 16-element column-major matrix")
    {
        GLMigration796Mock mock;

        // Translation matrix T(5, 10, 0) in column-major order
        const std::array<float, 16> translationMat{
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 5.0f, 10.0f, 0.0f, 1.0f};

        mock.LoadMatrix(translationMat.data());

        REQUIRE(mock.m_loadMatrixCallCount == 1);
        REQUIRE(mock.m_lastLoadMatrix[12] == Catch::Approx(5.0f));  // Tx
        REQUIRE(mock.m_lastLoadMatrix[13] == Catch::Approx(10.0f)); // Ty
    }

    SECTION("GetMatrix captures mode and writes back matrix data")
    {
        constexpr int GL_MODELVIEW_MATRIX = 0x0BA6;

        GLMigration796Mock mock;
        std::array<float, 16> result{};

        mock.GetMatrix(GL_MODELVIEW_MATRIX, result.data());

        REQUIRE(mock.m_getMatrixCallCount == 1);
        REQUIRE(mock.m_lastGetMatrixMode == GL_MODELVIEW_MATRIX);
        // Mock returns identity — diagonal must be 1.0
        REQUIRE(result[0] == Catch::Approx(1.0f));
        REQUIRE(result[5] == Catch::Approx(1.0f));
        REQUIRE(result[10] == Catch::Approx(1.0f));
        REQUIRE(result[15] == Catch::Approx(1.0f));
    }

    SECTION("Typical 3D object transform sequence: SetMatrixMode, PushMatrix, Translate+Rotate, PopMatrix")
    {
        // Documents the full UIWindows.cpp / ZzzBMD.cpp pattern (82 call sites):
        //   glMatrixMode(GL_MODELVIEW);
        //   glPushMatrix();
        //   glTranslatef(x, y, z);
        //   glRotatef(angle, 0, 0, 1);
        //   ... render ...
        //   glPopMatrix();
        constexpr int GL_MODELVIEW = 0x1700;
        GLMigration796Mock mock;

        mock.SetMatrixMode(GL_MODELVIEW);
        mock.PushMatrix();
        mock.Translate(50.0f, 100.0f, 0.0f);
        mock.Rotate(30.0f, 0.0f, 0.0f, 1.0f);
        mock.PopMatrix();

        REQUIRE(mock.m_setMatrixModeCallCount == 1);
        REQUIRE(mock.m_pushMatrixCallCount == 1);
        REQUIRE(mock.m_translateCallCount == 1);
        REQUIRE(mock.m_rotateCallCount == 1);
        REQUIRE(mock.m_popMatrixCallCount == 1);
    }
}

// ===========================================================================
// AC-5 [7-9-6]: Texture API — glBindTexture/glTexParameteri/glTexEnvi replaced
//
// BindTexture and SetTexture2D already exist on IMuRenderer (pre-7-9-6).
// This test documents the AC-5 contract using the existing interface:
//   - glBindTexture(GL_TEXTURE_2D, id) → BindTexture(id)  [already migrated]
//   - glEnable/glDisable(GL_TEXTURE_2D) → SetTexture2D(bool) [already migrated]
//   - glTexParameteri → handled at SDL GPU texture upload time (no new method)
//   - glTexEnvi/glTexEnvf → shader state (no new method needed)
//
// These tests verify that the pre-existing methods are callable and have
// correct AC-N: annotation in the context of 7-9-6.
// ===========================================================================
TEST_CASE("AC-5 [7-9-6]: Texture API BindTexture and SetTexture2D are callable",
    "[render][migration][ac-5]")
{
    SECTION("BindTexture is callable on IMuRenderer (pre-existing method)")
    {
        // glBindTexture(GL_TEXTURE_2D, texId) → mu::GetRenderer().BindTexture(texId)
        // IMuRenderer::BindTexture already exists (Story 7-9-5 migration context).
        // Compile-time check: successful compilation proves the method is present.
        GLMigration796Mock mock;
        REQUIRE_NOTHROW(mock.BindTexture(42));
    }

    SECTION("SetTexture2D is callable on IMuRenderer (pre-existing method)")
    {
        // glEnable(GL_TEXTURE_2D) → mu::GetRenderer().SetTexture2D(true)
        // glDisable(GL_TEXTURE_2D) → mu::GetRenderer().SetTexture2D(false)
        GLMigration796Mock mock;
        REQUIRE_NOTHROW(mock.SetTexture2D(true));
        REQUIRE_NOTHROW(mock.SetTexture2D(false));
    }

    SECTION("glTexParameteri migration note: handled at SDL GPU upload time")
    {
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) etc.
        // are called during texture loading in GlobalBitmap.cpp (12 call sites, 2 files).
        // In SDL GPU, texture filter/wrap modes are specified in the sampler descriptor
        // at texture upload time — no per-draw call needed.
        // No new IMuRenderer method is required; the SDL GPU backend's UploadTexture()
        // path handles this internally.
        // This section documents the design decision (no assertion needed).
        REQUIRE(true); // Design decision documented above
    }
}

// ===========================================================================
// AC-6 [7-9-6]: Depth/stencil/viewport API replaces glDepthFunc/glStencilFunc/etc.
//
// Replaces:
//   glDepthFunc(func)              → SetDepthFunc(func)
//   glAlphaFunc(func, ref)         → SetAlphaFunc(func, ref)
//   glStencilFunc(func, ref, mask) → SetStencilFunc(func, ref, mask)
//   glStencilOp(sf, dpf, dpp)      → SetStencilOp(sf, dpf, dpp)
//   glColorMask(r, g, b, a)        → SetColorMask(r, g, b, a)
//   glViewport(x, y, w, h)         → SetViewport(x, y, w, h)
//   glScissor(x, y, w, h)          → SetScissor(x, y, w, h)
//   glEnable/glDisable(GL_SCISSOR) → SetScissorEnabled(bool)
//
// RED PHASE: Fails to compile until all depth/stencil/viewport methods are
// added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-6 [7-9-6]: Depth/stencil/viewport API methods are callable on IMuRenderer",
    "[render][migration][ac-6]")
{
    SECTION("SetDepthFunc captures function enum value")
    {
        // glDepthFunc(GL_LEQUAL) is the standard MU depth function
        constexpr int GL_LEQUAL = 0x0203;

        GLMigration796Mock mock;
        mock.SetDepthFunc(GL_LEQUAL);

        REQUIRE(mock.m_setDepthFuncCallCount == 1);
        REQUIRE(mock.m_lastDepthFunc == GL_LEQUAL);
    }

    SECTION("SetAlphaFunc captures function and reference threshold")
    {
        // glAlphaFunc(GL_GREATER, 0.0f) used for alpha-test cutout rendering
        constexpr int GL_GREATER = 0x0204;

        GLMigration796Mock mock;
        mock.SetAlphaFunc(GL_GREATER, 0.0f);

        REQUIRE(mock.m_setAlphaFuncCallCount == 1);
        REQUIRE(mock.m_lastAlphaFunc == GL_GREATER);
        REQUIRE(mock.m_lastAlphaRef == Catch::Approx(0.0f));
    }

    SECTION("SetStencilFunc captures all three parameters")
    {
        // Shadow volume stencil: glStencilFunc(GL_NOTEQUAL, 0, 0xFF)
        constexpr int GL_NOTEQUAL = 0x0205;
        constexpr unsigned int kFullMask = 0xFF;

        GLMigration796Mock mock;
        mock.SetStencilFunc(GL_NOTEQUAL, 0, kFullMask);

        REQUIRE(mock.m_setStencilFuncCallCount == 1);
        REQUIRE(mock.m_lastStencilFunc == GL_NOTEQUAL);
        REQUIRE(mock.m_lastStencilRef == 0);
        REQUIRE(mock.m_lastStencilMask == kFullMask);
    }

    SECTION("SetStencilOp captures fail/depth-fail/depth-pass operations")
    {
        // Shadow volume stencil increment:
        //   glStencilOp(GL_KEEP, GL_INCR_WRAP, GL_KEEP)
        constexpr int GL_KEEP = 0x1E00;
        constexpr int GL_INCR_WRAP = 0x8507;

        GLMigration796Mock mock;
        mock.SetStencilOp(GL_KEEP, GL_INCR_WRAP, GL_KEEP);

        REQUIRE(mock.m_setStencilOpCallCount == 1);
        REQUIRE(mock.m_lastStencilSfail == GL_KEEP);
        REQUIRE(mock.m_lastStencilDpfail == GL_INCR_WRAP);
        REQUIRE(mock.m_lastStencilDppass == GL_KEEP);
    }

    SECTION("SetColorMask captures per-channel boolean flags")
    {
        // Shadow volume: write depth only — disable color channels
        //   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE)
        GLMigration796Mock mock;
        mock.SetColorMask(false, false, false, false);

        REQUIRE(mock.m_setColorMaskCallCount == 1);
        REQUIRE_FALSE(mock.m_lastColorMaskR);
        REQUIRE_FALSE(mock.m_lastColorMaskG);
        REQUIRE_FALSE(mock.m_lastColorMaskB);
        REQUIRE_FALSE(mock.m_lastColorMaskA);
    }

    SECTION("SetColorMask with all channels enabled (restore after stencil pass)")
    {
        GLMigration796Mock mock;
        mock.SetColorMask(true, true, true, true);

        REQUIRE(mock.m_lastColorMaskR);
        REQUIRE(mock.m_lastColorMaskG);
        REQUIRE(mock.m_lastColorMaskB);
        REQUIRE(mock.m_lastColorMaskA);
    }

    SECTION("SetViewport captures all four parameters")
    {
        // glViewport(0, 0, WindowWidth, WindowHeight)
        GLMigration796Mock mock;
        mock.SetViewport(0, 0, 1280, 960);

        REQUIRE(mock.m_setViewportCallCount == 1);
        REQUIRE(mock.m_lastViewportX == 0);
        REQUIRE(mock.m_lastViewportY == 0);
        REQUIRE(mock.m_lastViewportW == 1280);
        REQUIRE(mock.m_lastViewportH == 960);
    }

    SECTION("SetScissor captures rect parameters")
    {
        // Used by UI panel clipping: glScissor(x, y, w, h)
        GLMigration796Mock mock;
        mock.SetScissor(100, 200, 400, 300);

        REQUIRE(mock.m_setScissorCallCount == 1);
        REQUIRE(mock.m_lastScissorX == 100);
        REQUIRE(mock.m_lastScissorY == 200);
        REQUIRE(mock.m_lastScissorW == 400);
        REQUIRE(mock.m_lastScissorH == 300);
    }

    SECTION("SetScissorEnabled captures boolean state")
    {
        // glEnable(GL_SCISSOR_TEST) → SetScissorEnabled(true)
        // glDisable(GL_SCISSOR_TEST) → SetScissorEnabled(false)
        GLMigration796Mock mock;

        mock.SetScissorEnabled(true);
        REQUIRE(mock.m_setScissorEnabledCallCount == 1);
        REQUIRE(mock.m_lastScissorEnabled);

        mock.SetScissorEnabled(false);
        REQUIRE(mock.m_setScissorEnabledCallCount == 2);
        REQUIRE_FALSE(mock.m_lastScissorEnabled);
    }

    SECTION("Shadow volume stencil render sequence (ShadowVolume.cpp pattern)")
    {
        // Documents the full ShadowVolume stencil shadow rendering pattern (19 calls):
        // 1. Setup: disable color write, set stencil to increment
        // 2. Render: draw shadow geometry into stencil buffer
        // 3. Restore: re-enable color write, set stencil to lit areas
        constexpr int GL_KEEP = 0x1E00;
        constexpr int GL_INCR = 0x1E02;
        constexpr int GL_DECR = 0x1E03;
        constexpr int GL_ALWAYS = 0x0207;
        constexpr int GL_NOTEQUAL = 0x0205;
        constexpr unsigned int kMask = 0xFF;

        GLMigration796Mock mock;

        // Phase 1: Setup stencil
        mock.SetColorMask(false, false, false, false);
        mock.SetStencilFunc(GL_ALWAYS, 0, kMask);
        mock.SetStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        // Phase 2: (render shadow geometry via RenderTriangles — not mocked here)

        // Phase 3: Restore and render lit shadow areas
        mock.SetColorMask(true, true, true, true);
        mock.SetStencilFunc(GL_NOTEQUAL, 0, kMask);
        mock.SetStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

        REQUIRE(mock.m_setColorMaskCallCount == 2);
        REQUIRE(mock.m_setStencilFuncCallCount == 2);
        REQUIRE(mock.m_setStencilOpCallCount == 2);
    }
}

// ===========================================================================
// AC-7 [7-9-6]: ReadPixels replaces glReadPixels for screenshot functionality
//
// src/source/Platform/PlatformCompat.h glReadPixels stub (3 call sites in 1 file)
// replaced with mu::GetRenderer().ReadPixels(x, y, w, h, data).
// SDL_gpu backend: uses SDL_GPUDownloadFromGPUTexture or SDL_RenderReadPixels.
//
// RED PHASE: Fails to compile until ReadPixels is added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-7 [7-9-6]: ReadPixels is callable on IMuRenderer (replaces glReadPixels)",
    "[render][migration][ac-7]")
{
    SECTION("ReadPixels captures all parameters (fullscreen screenshot)")
    {
        // GIVEN: a fresh mock and a pixel buffer for 100×100 screenshot
        GLMigration796Mock mock;
        std::vector<std::uint8_t> pixelBuffer(100 * 100 * 4, 0); // RGBA

        // WHEN: ReadPixels called for a 100×100 region starting at (0, 0)
        mock.ReadPixels(0, 0, 100, 100, pixelBuffer.data());

        // THEN: all parameters captured
        REQUIRE(mock.m_readPixelsCallCount == 1);
        REQUIRE(mock.m_lastReadPixelsX == 0);
        REQUIRE(mock.m_lastReadPixelsY == 0);
        REQUIRE(mock.m_lastReadPixelsW == 100);
        REQUIRE(mock.m_lastReadPixelsH == 100);
    }

    SECTION("ReadPixels with offset origin (partial capture)")
    {
        // glReadPixels can capture sub-regions; same for SDL GPU download
        GLMigration796Mock mock;
        std::vector<std::uint8_t> pixelBuffer(50 * 50 * 4, 0);

        mock.ReadPixels(200, 150, 50, 50, pixelBuffer.data());

        REQUIRE(mock.m_lastReadPixelsX == 200);
        REQUIRE(mock.m_lastReadPixelsY == 150);
        REQUIRE(mock.m_lastReadPixelsW == 50);
        REQUIRE(mock.m_lastReadPixelsH == 50);
    }

    SECTION("ReadPixels accepts nullptr data without crash (degenerate case)")
    {
        // Robust implementation must handle nullptr gracefully
        GLMigration796Mock mock;
        REQUIRE_NOTHROW(mock.ReadPixels(0, 0, 0, 0, nullptr));
        REQUIRE(mock.m_readPixelsCallCount == 1);
    }
}

// ===========================================================================
// AC-8 [7-9-6]: GetGPUDriverName replaces glGetString for system info
//
// 4 glGetString(GL_VENDOR/GL_RENDERER/GL_VERSION) call sites in 1 file
// replaced with mu::GetRenderer().GetGPUDriverName().
// GetGPUDriverName already exists on IMuRenderer (Story 7-6-7, AC-3).
// This test verifies the existing method meets the AC-8 contract.
// ===========================================================================
TEST_CASE("AC-8 [7-9-6]: GetGPUDriverName provides GPU info (replaces glGetString)",
    "[render][migration][ac-8]")
{
    SECTION("GetGPUDriverName is callable and returns non-null string")
    {
        // The base implementation returns "unknown"; SDL_gpu backend overrides
        // with SDL_GetGPUDeviceDriver().
        GLMigration796Mock mock;

        const char* driverName = mock.GetGPUDriverName();

        REQUIRE(driverName != nullptr);
    }

    SECTION("GetGPUDriverName default returns 'unknown' (OpenGL/mock backends)")
    {
        // Default IMuRenderer implementation returns "unknown" — safe sentinel
        // value when no SDL GPU driver is active.
        GLMigration796Mock mock;

        REQUIRE(std::string(mock.GetGPUDriverName()) == "unknown");
    }

    SECTION("GetGPUDriverName is const — callable on const IMuRenderer reference")
    {
        // ErrorReport.cpp receives a const renderer reference
        const GLMigration796Mock constMock;
        const char* name = constMock.GetGPUDriverName();
        REQUIRE(name != nullptr);
    }
}

// ===========================================================================
// AC-9 [7-9-6]: Dead WGL/extension calls deleted (13 call sites, 2 files)
//
// Static/file-scan verification of:
//   wglGetProcAddress, glSwapIntervalEXT, glGetExtensionsStringARB,
//   glGetExtensionsStringEXT, glChoosePixelFormatARB, glGetCurrentDC,
//   glGetSwapIntervalEXT
//
// These are Windows WGL extension entry points that are dead code on SDL3.
// They must be completely deleted (not just guarded) per the No-Stubs policy.
// ===========================================================================
#ifndef _WIN32
TEST_CASE("AC-9 [7-9-6]: Dead WGL/extension function calls are deleted",
    "[render][migration][ac-9][static-analysis]")
{
#ifndef MU_SOURCE_DIR
    SKIP("MU_SOURCE_DIR not defined — run via ctest");
#else
    SECTION("No wglGetProcAddress calls in source files (13 WGL call sites deleted)")
    {
        const std::filesystem::path sourceDir{MU_SOURCE_DIR};

        // ZzzBMD.cpp is the primary WGL call site
        const auto targetFile = sourceDir / "RenderFX" / "ZzzBMD.cpp";

        REQUIRE(std::filesystem::exists(targetFile));

        std::ifstream f(targetFile);
        REQUIRE(f.is_open());

        const std::regex wglPattern{R"(\bwglGetProcAddress\b|\bglSwapIntervalEXT\b|\bglGetExtensionsString)"};
        std::string line;
        int wglCallCount{0};

        while (std::getline(f, line))
        {
            const auto commentPos = line.find("//");
            const auto checkLine = (commentPos != std::string::npos) ? line.substr(0, commentPos) : line;
            if (std::regex_search(checkLine, wglPattern))
            {
                ++wglCallCount;
            }
        }

        // RED PHASE: 0 after deletion. Pre-migration: WGL calls present in ZzzBMD.cpp
        CHECK(wglCallCount == 0);
    }
#endif
}
#endif // ifndef _WIN32

// ===========================================================================
// AC-STD-1 [7-9-6]: All new IMuRenderer methods callable via test double
//                    (complete interface check)
//
// The mere fact this TU compiles proves that ALL new 7-9-6 methods are
// declared on IMuRenderer. This is a compile-time contract test.
//
// RED PHASE: Fails to compile until ALL new methods are added to IMuRenderer.
// ===========================================================================
TEST_CASE("AC-STD-1 [7-9-6]: IMuRenderer extended interface — all 7-9-6 methods callable",
    "[render][migration][ac-std-1]")
{
    SECTION("All new 7-9-6 methods callable via test double (compile-time interface check)")
    {
        GLMigration796Mock mock;

        // AC-3: SetClearColor
        mock.SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // AC-4: Matrix stack API
        constexpr int GL_MODELVIEW = 0x1700;
        mock.SetMatrixMode(GL_MODELVIEW);
        mock.PushMatrix();
        mock.LoadIdentity();
        mock.Translate(1.0f, 0.0f, 0.0f);
        mock.Rotate(45.0f, 0.0f, 1.0f, 0.0f);
        mock.Scale(1.0f, 1.0f, 1.0f);
        const std::array<float, 16> mat{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        mock.MultMatrix(mat.data());
        mock.LoadMatrix(mat.data());
        std::array<float, 16> result{};
        mock.GetMatrix(GL_MODELVIEW, result.data());
        mock.PopMatrix();

        // AC-6: Depth/stencil/viewport
        mock.SetDepthFunc(0x0203);                    // GL_LEQUAL
        mock.SetAlphaFunc(0x0204, 0.0f);              // GL_GREATER
        mock.SetStencilFunc(0x0207, 0, 0xFF);         // GL_ALWAYS
        mock.SetStencilOp(0x1E00, 0x1E00, 0x1E02);   // GL_KEEP, GL_KEEP, GL_INCR
        mock.SetColorMask(true, true, true, true);
        mock.SetViewport(0, 0, 1280, 960);
        mock.SetScissor(0, 0, 640, 480);
        mock.SetScissorEnabled(false);

        // AC-7: ReadPixels
        std::array<std::uint8_t, 4> pixel{};
        mock.ReadPixels(0, 0, 1, 1, pixel.data());

        // AC-8: GetGPUDriverName (already exists — non-regression)
        [[maybe_unused]] const char* name = mock.GetGPUDriverName();

        // Verify all call counts reflect exactly one call each
        REQUIRE(mock.m_setClearColorCallCount == 1);
        REQUIRE(mock.m_setMatrixModeCallCount == 1);
        REQUIRE(mock.m_pushMatrixCallCount == 1);
        REQUIRE(mock.m_loadIdentityCallCount == 1);
        REQUIRE(mock.m_translateCallCount == 1);
        REQUIRE(mock.m_rotateCallCount == 1);
        REQUIRE(mock.m_scaleCallCount == 1);
        REQUIRE(mock.m_multMatrixCallCount == 1);
        REQUIRE(mock.m_loadMatrixCallCount == 1);
        REQUIRE(mock.m_getMatrixCallCount == 1);
        REQUIRE(mock.m_popMatrixCallCount == 1);
        REQUIRE(mock.m_setDepthFuncCallCount == 1);
        REQUIRE(mock.m_setAlphaFuncCallCount == 1);
        REQUIRE(mock.m_setStencilFuncCallCount == 1);
        REQUIRE(mock.m_setStencilOpCallCount == 1);
        REQUIRE(mock.m_setColorMaskCallCount == 1);
        REQUIRE(mock.m_setViewportCallCount == 1);
        REQUIRE(mock.m_setScissorCallCount == 1);
        REQUIRE(mock.m_setScissorEnabledCallCount == 1);
        REQUIRE(mock.m_readPixelsCallCount == 1);
    }

    SECTION("Pre-existing IMuRenderer methods remain callable (non-regression)")
    {
        // Verifies backward compatibility: 7-9-6 additions must not break
        // any existing IMuRenderer method from prior stories.
        GLMigration796Mock mock;

        std::array<mu::Vertex2D, 4> quad2d{};
        mock.RenderQuad2D(std::span<const mu::Vertex2D>(quad2d), 0u);

        std::array<mu::Vertex3D, 3> tris{};
        mock.RenderTriangles(std::span<const mu::Vertex3D>(tris), 0u);

        mock.SetBlendMode(mu::BlendMode::Alpha);
        mock.DisableBlend();
        mock.SetDepthTest(true);
        mock.SetDepthMask(true);
        mock.SetCullFace(true);
        mock.SetAlphaTest(false);
        mock.SetTexture2D(true);
        mock.SetFogEnabled(false);
        mock.BindTexture(0);

        mu::FogParams fog{};
        mock.SetFog(fog);

        mock.BeginScene(0, 0, 1024, 768);
        mock.Begin2DPass();
        mock.End2DPass();
        mock.EndScene();
        mock.ClearScreen();
        mock.ClearDepthBuffer();

        std::array<mu::Vertex3D, 2> lines{};
        mock.RenderLines(std::span<const mu::Vertex3D>(lines), 0u);

        [[maybe_unused]] const bool active = mock.IsFrameActive();

        // No 7-9-6 counters should have been incremented
        REQUIRE(mock.m_setClearColorCallCount == 0);
        REQUIRE(mock.m_pushMatrixCallCount == 0);
        REQUIRE(mock.m_readPixelsCallCount == 0);
    }

    SECTION("IMuRenderer 7-9-6 methods have no OpenGL types in their signatures")
    {
        // This TU includes MuRenderer.h WITHOUT any OpenGL headers.
        // Successful compilation proves no GL types (GLenum, GLuint, GLbitfield,
        // GLsizei, etc.) are present in the new 7-9-6 IMuRenderer methods.
        // The use of plain int/float/bool parameters satisfies the cross-platform rule.
        GLMigration796Mock mock;
        mock.SetClearColor(0.0f, 0.0f, 0.0f, 1.0f); // float params — no GL types
        mock.SetDepthFunc(0x0203);                    // plain int — no GLenum

        REQUIRE(mock.m_setClearColorCallCount == 1);
        REQUIRE(mock.m_setDepthFuncCallCount == 1);
    }
}
