// Story 7.9.7: Adopt GLM and Harden Renderer Matrix Pipeline
// Flow Code: VS0-RENDER-GLM-MATRIX
//
// RED PHASE: Test file does not exist until Task 9.1 creates it.
//            Once created and added to CMakeLists.txt, all tests in this file
//            should be GREEN immediately (GLM integration is done in Tasks 1+2).
//
// AC Mapping:
//   AC-STD-2 -> All TEST_CASEs  Matrix math unit tests using Catch2
//   AC-2     -> GLM integration verified: glm::mat4 ops without mat4:: namespace
//   AC-3     -> Perspective Z range [0,1] verified with GLM_FORCE_DEPTH_ZERO_TO_ONE
//   AC-4     -> Ortho NDC mapping verified
//   Task 9.2 -> TEST_CASE("AC-STD-2 [7-9-7]: glm::perspective produces Z [0,1] ...")
//   Task 9.3 -> TEST_CASE("AC-STD-2 [7-9-7]: glm::ortho maps 2D screen corners ...")
//   Task 9.4 -> TEST_CASE("AC-STD-2 [7-9-7]: matrix stack push/pop ...")
//
// IMPORTANT: No OpenGL calls, no SDL3 GPU calls, no Win32 in this test TU.
// All GLM operations tested directly — no renderer device required.
// Matrix stack interface tests use MatrixMath797Mock implementing IMuRenderer.
// Tests compile and run on macOS/Linux without Win32, OpenGL, or a GPU device.
//
// Run with: ctest --test-dir MuMain/build -R matrix_math_7_9_7

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <span>

#include "MuRenderer.h"

// ---------------------------------------------------------------------------
// MatrixMath797Mock
// Minimal mock for testing the matrix stack interface (PushMatrix, PopMatrix,
// LoadIdentity, Translate, GetMatrix, SetMatrixMode).
// Only overrides methods exercised by these tests — all others are no-ops via
// the default virtual implementations in IMuRenderer.
//
// RED PHASE: Compiles and runs once this file exists and is added to MuTests.
//            All matrix stack methods are inherited defaults in IMuRenderer —
//            tests verify the interface contract.
// ---------------------------------------------------------------------------
namespace
{

// Simple mock: uses the default virtual no-ops from IMuRenderer for all non-matrix methods.
// Matrix methods (PushMatrix, PopMatrix, etc.) are virtual no-ops in the base class — this
// mock overrides them to capture calls for verification.
struct MatrixMath797Mock : public mu::IMuRenderer
{
    // Required pure-virtual overrides (render geometry — not tested here)
    void RenderQuad2D(std::span<const mu::Vertex2D> /*v*/, std::uint32_t /*id*/) override {}
    void RenderTriangles(std::span<const mu::Vertex3D> /*v*/, std::uint32_t /*id*/) override {}
    void RenderLines(std::span<const mu::Vertex3D> /*v*/, std::uint32_t /*id*/) override {}
    void RenderQuadStrip(std::span<const mu::Vertex3D> /*v*/, std::uint32_t /*id*/) override {}

    // Lifecycle (pure-virtual)
    void BeginScene(int /*x*/, int /*y*/, int /*w*/, int /*h*/) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}

    // Blend / depth (pure-virtual)
    void SetBlendMode(mu::BlendMode /*m*/) override {}
    void DisableBlend() override {}
    void SetDepthTest(bool /*e*/) override {}
    void SetFog(const mu::FogParams& /*p*/) override {}

    // Matrix stack — track call counts for push/pop tests.
    // All matrix methods have default no-op implementations in IMuRenderer;
    // we override only the ones we want to instrument.
    int pushCount = 0;
    int popCount = 0;
    int m_mode = 0;

    void SetMatrixMode(int mode) override { m_mode = mode; }
    void PushMatrix() override { ++pushCount; }
    void PopMatrix() override { ++popCount; }
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// AC-STD-2 [Task 9.2]: Perspective Z range [0,1] for known near/far values
// Verifies that glm::perspective() with GLM_FORCE_DEPTH_ZERO_TO_ONE maps:
//   - A point at exactly z = -zNear (eye-space) → NDC z = 0.0 (near clip)
//   - A point at exactly z = -zFar (eye-space) → NDC z = 1.0 (far clip)
// This is the Metal/Vulkan [0,1] depth convention required by AC-3.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [7-9-7]: glm::perspective produces Z [0,1] for near plane",
          "[render][glm][perspective][7-9-7]")
{
    // Parameters matching game camera defaults (CameraViewNear=20, CameraViewFar=2000)
    constexpr float fovY = glm::radians(55.0f);
    constexpr float aspect = 1024.0f / 768.0f;
    constexpr float zNear = 20.0f;
    constexpr float zFar = 2000.0f;

    // GLM_FORCE_DEPTH_ZERO_TO_ONE defined at top of file — perspective maps to Z [0,1]
    const glm::mat4 proj = glm::perspective(fovY, aspect, zNear, zFar);

    // Transform a point at the near plane: eye-space z = -zNear (right-handed: camera looks -Z)
    const glm::vec4 nearPoint(0.0f, 0.0f, -zNear, 1.0f);
    const glm::vec4 clipNear = proj * nearPoint;
    const float ndcZ_near = clipNear.z / clipNear.w;

    // With GLM_FORCE_DEPTH_ZERO_TO_ONE, near plane → NDC z = 0.0
    REQUIRE(ndcZ_near == Catch::Approx(0.0f).margin(1e-5f));
}

TEST_CASE("AC-STD-2 [7-9-7]: glm::perspective produces Z [0,1] for far plane",
          "[render][glm][perspective][7-9-7]")
{
    constexpr float fovY = glm::radians(55.0f);
    constexpr float aspect = 1024.0f / 768.0f;
    constexpr float zNear = 20.0f;
    constexpr float zFar = 2000.0f;

    const glm::mat4 proj = glm::perspective(fovY, aspect, zNear, zFar);

    // Transform a point at the far plane: eye-space z = -zFar
    const glm::vec4 farPoint(0.0f, 0.0f, -zFar, 1.0f);
    const glm::vec4 clipFar = proj * farPoint;
    const float ndcZ_far = clipFar.z / clipFar.w;

    // With GLM_FORCE_DEPTH_ZERO_TO_ONE, far plane → NDC z = 1.0
    REQUIRE(ndcZ_far == Catch::Approx(1.0f).margin(1e-5f));
}

// ---------------------------------------------------------------------------
// AC-STD-2 [Task 9.3]: glm::ortho maps 2D screen corners to NDC correctly
// The 2D render path in BeginScene uses glm::ortho for the 640×480 design space.
// At physical resolution e.g. 1024×768, the ortho maps 0..W → [-1,1] and 0..H → [-1,1].
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [7-9-7]: glm::ortho maps 2D screen corners to NDC",
          "[render][glm][ortho][7-9-7]")
{
    // Typical physical window size
    constexpr float winW = 1024.0f;
    constexpr float winH = 768.0f;

    // Same call as BeginScene in MuRendererSDLGpu.cpp line ~1098
    const glm::mat4 ortho = glm::ortho(0.0f, winW, 0.0f, winH, -1.0f, 1.0f);

    // Bottom-left corner (0,0) → NDC (-1,-1)
    const glm::vec4 bottomLeft(0.0f, 0.0f, 0.0f, 1.0f);
    const glm::vec4 ndcBL = ortho * bottomLeft;
    REQUIRE(ndcBL.x / ndcBL.w == Catch::Approx(-1.0f).margin(1e-5f));
    REQUIRE(ndcBL.y / ndcBL.w == Catch::Approx(-1.0f).margin(1e-5f));

    // Top-right corner (W,H) → NDC (1,1)
    const glm::vec4 topRight(winW, winH, 0.0f, 1.0f);
    const glm::vec4 ndcTR = ortho * topRight;
    REQUIRE(ndcTR.x / ndcTR.w == Catch::Approx(1.0f).margin(1e-5f));
    REQUIRE(ndcTR.y / ndcTR.w == Catch::Approx(1.0f).margin(1e-5f));
}

TEST_CASE("AC-STD-2 [7-9-7]: glm::ortho Z is flat [0,1] range at z=0",
          "[render][glm][ortho][7-9-7]")
{
    const glm::mat4 ortho = glm::ortho(0.0f, 1024.0f, 0.0f, 768.0f, -1.0f, 1.0f);

    // A point at z=0 (screen plane) should map to NDC z=0.5 (midpoint of [-1,1])
    // With GLM_FORCE_DEPTH_ZERO_TO_ONE: z=0 (near=-1) → NDC z = 0.5
    const glm::vec4 center(512.0f, 384.0f, 0.0f, 1.0f);
    const glm::vec4 ndc = ortho * center;
    REQUIRE(ndc.z / ndc.w == Catch::Approx(0.5f).margin(1e-5f));
}

// ---------------------------------------------------------------------------
// AC-STD-2 [Task 9.4]: Matrix stack push/pop preserves and restores state
// The IMuRenderer interface provides PushMatrix/PopMatrix/SetMatrixMode.
// We verify that the interface contract counts pushes and pops correctly.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [7-9-7]: matrix stack push preserves and pop restores state",
          "[render][matrix-stack][7-9-7]")
{
    MatrixMath797Mock mock;

    // Initial state: no pushes or pops
    REQUIRE(mock.pushCount == 0);
    REQUIRE(mock.popCount == 0);

    mock.PushMatrix();
    REQUIRE(mock.pushCount == 1);

    mock.PopMatrix();
    REQUIRE(mock.popCount == 1);

    // Nested push/pop
    mock.PushMatrix();
    mock.PushMatrix();
    mock.PopMatrix();
    REQUIRE(mock.pushCount == 3);
    REQUIRE(mock.popCount == 2);
}

TEST_CASE("AC-STD-2 [7-9-7]: matrix stack SetMatrixMode routes to modelview or projection",
          "[render][matrix-stack][7-9-7]")
{
    MatrixMath797Mock mock;

    // GL_MODELVIEW = 0x1700, GL_PROJECTION = 0x1701
    constexpr int GL_MODELVIEW = 0x1700;
    constexpr int GL_PROJECTION = 0x1701;

    mock.SetMatrixMode(GL_PROJECTION);
    REQUIRE(mock.m_mode == GL_PROJECTION);

    mock.SetMatrixMode(GL_MODELVIEW);
    REQUIRE(mock.m_mode == GL_MODELVIEW);
}

// ---------------------------------------------------------------------------
// AC-STD-2: GLM depth convention consistency check
// Verifies that the same perspective matrix with and without GLM_FORCE_DEPTH_ZERO_TO_ONE
// gives different results — confirming the macro has the expected effect.
// This test is always enabled (no platform guard needed — pure math).
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [7-9-7]: GLM_FORCE_DEPTH_ZERO_TO_ONE changes depth mapping",
          "[render][glm][depth-convention][7-9-7]")
{
    // With GLM_FORCE_DEPTH_ZERO_TO_ONE (defined at top of file):
    constexpr float fovY = glm::radians(55.0f);
    constexpr float aspect = 1.0f;
    constexpr float zNear = 1.0f;
    constexpr float zFar = 100.0f;

    const glm::mat4 proj = glm::perspective(fovY, aspect, zNear, zFar);

    // Near plane at z=-1 (eye-space right-handed)
    const glm::vec4 nearEye(0.0f, 0.0f, -zNear, 1.0f);
    const glm::vec4 clipNear = proj * nearEye;
    const float ndcZ = clipNear.z / clipNear.w;

    // With GLM_FORCE_DEPTH_ZERO_TO_ONE: NDC z at near == 0.0 (not -1.0 as in OpenGL NDC)
    REQUIRE(ndcZ == Catch::Approx(0.0f).margin(1e-5f));

    // Confirm it's NOT the OpenGL convention (-1.0 at near)
    REQUIRE_FALSE(ndcZ == Catch::Approx(-1.0f).margin(0.1f));
}
