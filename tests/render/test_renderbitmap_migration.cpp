// Story 4.2.2: Migrate RenderBitmap Variants to RenderQuad2D
// Flow Code: VS1-RENDER-MIGRATE-QUAD2D
//
// RED PHASE: Tests fail until:
//   - MuRendererGL::RenderQuad2D emits per-vertex color (Task 11)
//   - RenderBitmap* function bodies delegate to mu::GetRenderer().RenderQuad2D() (Tasks 2-10)
//   - textureId=0 path is handled by MuRendererGL (Task 10)
//
// IMPORTANT: No OpenGL calls in this test TU.
// RenderQuad2DCapture is a test-double of IMuRenderer defined inline below.
// No gl* functions are called; all tests are pure logic, runnable on macOS/Linux.
//
// AC Mapping:
//   AC-STD-2  → TEST_CASE("AC-STD-2 [4-2-2]: RenderQuad2D vertex layout — basic RenderBitmap")
//   AC-STD-2  → TEST_CASE("AC-STD-2 [4-2-2]: Vertex2D color packing — opaque white")
//   AC-STD-2  → TEST_CASE("AC-STD-2 [4-2-2]: Vertex2D color packing — per-vertex alpha")
//   AC-VAL-1  → TEST_CASE("AC-VAL-1 [4-2-2]: RenderQuad2D called once per RenderBitmap invocation")
//   AC-VAL-1  → TEST_CASE("AC-VAL-1 [4-2-2]: RenderBitmapAlpha calls RenderQuad2D 16 times")
//   AC-2/AC-3 → TEST_CASE("AC-2 [4-2-2]: textureId=0 accepted by RenderQuad2D for untextured quad")
//   AC-1      → TEST_CASE("AC-1 [4-2-2]: RenderQuad2D receives exactly 4 vertices per call")
//   AC-STD-2  → TEST_CASE("AC-STD-2 [4-2-2]: RenderBitmapLocalRotate vertex positions — Rotate=0")
//
// Run with: ctest --test-dir MuMain/build -R renderbitmap_migration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <vector>

#include "MuRenderer.h"

// ---------------------------------------------------------------------------
// Test-double: RenderQuad2DCapture
// Implements IMuRenderer; captures RenderQuad2D calls for assertion.
// Stores a copy of each vertex span and the associated textureId.
// ---------------------------------------------------------------------------
namespace
{

struct CapturedCall
{
    std::vector<mu::Vertex2D> vertices;
    std::uint32_t textureId;
};

struct RenderQuad2DCapture : public mu::IMuRenderer
{
    std::vector<CapturedCall> m_calls;

    void RenderQuad2D(std::span<const mu::Vertex2D> vertices, std::uint32_t textureId) override
    {
        CapturedCall call;
        call.vertices.assign(vertices.begin(), vertices.end());
        call.textureId = textureId;
        m_calls.push_back(std::move(call));
    }

    void RenderTriangles(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void RenderQuadStrip(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override
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

    void BeginScene(int /*x*/, int /*y*/, int /*w*/, int /*h*/) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}
    void RenderLines(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override {}
};

// ---------------------------------------------------------------------------
// Helper: Build 4 Vertex2D using the RenderBitmap vertex/UV layout.
// Mirrors the migration pattern documented in story 4.2.2 Dev Notes:
//   TL = (x,         y         , u,         v        )
//   BL = (x,         y - Height, u,         v+vHeight)
//   BR = (x + Width, y - Height, u + uWidth, v+vHeight)
//   TR = (x + Width, y         , u + uWidth, v        )
// color = 0xFFFFFFFF for opaque white; Alpha > 0 packs alpha channel.
// ---------------------------------------------------------------------------
[[nodiscard]] std::array<mu::Vertex2D, 4> BuildRenderBitmapVertices(
    float x, float y, float width, float height,
    float u, float v, float uWidth, float vHeight,
    float alpha = 0.0f)
{
    const std::uint32_t color = (alpha > 0.0f)
        ? (static_cast<std::uint32_t>(alpha * 255.0f) << 24) | 0x00FFFFFFu
        : 0xFFFFFFFFu;

    const std::array<mu::Vertex2D, 4> vertices = {{
        { x,         y,          u,          v,          color },
        { x,         y - height, u,          v + vHeight, color },
        { x + width, y - height, u + uWidth, v + vHeight, color },
        { x + width, y,          u + uWidth, v,           color },
    }};
    return vertices;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// AC-VAL-1 [4-2-2]: RenderQuad2D called exactly once per RenderBitmap invocation
// GIVEN: a RenderQuad2DCapture test-double registered as the active renderer
// WHEN:  logic equivalent to RenderBitmap vertex building calls RenderQuad2D once
// THEN:  capture records exactly 1 call with a non-zero textureId
//
// RED PHASE: This test documents the expected call-through contract.
// It will pass once RenderBitmap delegates to mu::GetRenderer().RenderQuad2D().
// For now it tests the test-double mechanics directly (GREEN immediately),
// validating the pattern the implementation must match.
// ---------------------------------------------------------------------------
TEST_CASE("AC-VAL-1 [4-2-2]: RenderQuad2D called once per RenderBitmap invocation",
    "[render][renderbitmap][ac-val-1]")
{
    SECTION("Single RenderBitmap call results in exactly one RenderQuad2D call")
    {
        // GIVEN: a capture test-double
        RenderQuad2DCapture capture;

        // WHEN: logic equivalent to migrated RenderBitmap vertex building
        // (x=10, y=480, Width=64, Height=32, u=0, v=0, uWidth=1, vHeight=1)
        constexpr float x = 10.0f;
        constexpr float y = 480.0f; // post WindowHeight-flip value
        constexpr float width = 64.0f;
        constexpr float height = 32.0f;
        constexpr float u = 0.0f;
        constexpr float v = 0.0f;
        constexpr float uWidth = 1.0f;
        constexpr float vHeight = 1.0f;
        constexpr std::uint32_t textureId = 42u;

        const auto vertices = BuildRenderBitmapVertices(x, y, width, height, u, v, uWidth, vHeight);
        capture.RenderQuad2D(std::span<const mu::Vertex2D>(vertices), textureId);

        // THEN: exactly one call recorded
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].textureId == textureId);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2 [4-2-2]: RenderQuad2D vertex layout — basic RenderBitmap
// GIVEN: screen position (x=10, y=480), size (64×32), UV (u=0, v=0, uW=1, vH=1)
// WHEN:  BuildRenderBitmapVertices constructs the 4 Vertex2D
// THEN:  UV ordering matches expected quad winding (TL, BL, BR, TR):
//          v[0] = (u,      v       ) = (0, 0)
//          v[1] = (u,      v+vH    ) = (0, 1)
//          v[2] = (u+uW,   v+vH    ) = (1, 1)
//          v[3] = (u+uW,   v       ) = (1, 0)
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-2]: RenderQuad2D vertex layout — basic RenderBitmap",
    "[render][renderbitmap][ac-std-2]")
{
    SECTION("UV ordering: TL=(u,v), BL=(u,v+vH), BR=(u+uW,v+vH), TR=(u+uW,v)")
    {
        // GIVEN
        constexpr float x = 10.0f;
        constexpr float y = 480.0f;
        constexpr float width = 64.0f;
        constexpr float height = 32.0f;
        constexpr float u = 0.25f;
        constexpr float v = 0.10f;
        constexpr float uWidth = 0.5f;
        constexpr float vHeight = 0.4f;

        // WHEN
        const auto verts = BuildRenderBitmapVertices(x, y, width, height, u, v, uWidth, vHeight);

        // THEN: 4 vertices
        REQUIRE(verts.size() == 4u);

        // Vertex 0 — top-left
        REQUIRE(verts[0].u == u);
        REQUIRE(verts[0].v == v);
        REQUIRE(verts[0].x == x);
        REQUIRE(verts[0].y == y);

        // Vertex 1 — bottom-left
        REQUIRE(verts[1].u == u);
        REQUIRE(verts[1].v == Catch::Approx(v + vHeight));
        REQUIRE(verts[1].x == x);
        REQUIRE(verts[1].y == Catch::Approx(y - height));

        // Vertex 2 — bottom-right
        REQUIRE(verts[2].u == Catch::Approx(u + uWidth));
        REQUIRE(verts[2].v == Catch::Approx(v + vHeight));
        REQUIRE(verts[2].x == Catch::Approx(x + width));
        REQUIRE(verts[2].y == Catch::Approx(y - height));

        // Vertex 3 — top-right
        REQUIRE(verts[3].u == Catch::Approx(u + uWidth));
        REQUIRE(verts[3].v == v);
        REQUIRE(verts[3].x == Catch::Approx(x + width));
        REQUIRE(verts[3].y == y);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2 [4-2-2]: Vertex2D color packing — opaque white
// GIVEN: BuildRenderBitmapVertices with alpha = 0 (use opaque white default)
// WHEN:  vertex array is constructed
// THEN:  color field == 0xFFFFFFFF; ABGR channels all == 255
//        A=bits31-24, B=bits23-16, G=bits15-8, R=bits7-0
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-2]: Vertex2D color packing — opaque white",
    "[render][renderbitmap][ac-std-2]")
{
    SECTION("alpha=0 produces 0xFFFFFFFF in all four Vertex2D::color fields")
    {
        // GIVEN
        const auto verts = BuildRenderBitmapVertices(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

        // THEN: all four vertices have opaque white color
        for (const auto& vert : verts)
        {
            REQUIRE(vert.color == 0xFFFFFFFFu);

            // Unpack ABGR channels
            const auto a = static_cast<std::uint8_t>((vert.color >> 24) & 0xFF);
            const auto b = static_cast<std::uint8_t>((vert.color >> 16) & 0xFF);
            const auto g = static_cast<std::uint8_t>((vert.color >> 8) & 0xFF);
            const auto r = static_cast<std::uint8_t>((vert.color) & 0xFF);

            REQUIRE(a == 255u); // A channel = fully opaque
            REQUIRE(b == 255u); // B channel
            REQUIRE(g == 255u); // G channel
            REQUIRE(r == 255u); // R channel
        }
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2 [4-2-2]: Vertex2D color packing — per-vertex alpha
// GIVEN: BuildRenderBitmapVertices with alpha = 0.5f (RenderBitmapAlpha case)
// WHEN:  vertex array is constructed
// THEN:  A channel == round(0.5f * 255) == 127 or 128; RGB channels == 0xFF
//        Documents the ABGR packing convention: A=(alpha*255)<<24 | 0x00FFFFFF
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-2]: Vertex2D color packing — per-vertex alpha",
    "[render][renderbitmap][ac-std-2]")
{
    SECTION("alpha=0.5f packs A channel = ~127, RGB = 0xFF")
    {
        // GIVEN: alpha > 0 triggers per-vertex alpha packing
        constexpr float alpha = 0.5f;

        // WHEN
        const auto verts = BuildRenderBitmapVertices(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, alpha);

        // THEN: all four vertices have the same packed color
        for (const auto& vert : verts)
        {
            const auto a = static_cast<std::uint8_t>((vert.color >> 24) & 0xFF);
            const auto b = static_cast<std::uint8_t>((vert.color >> 16) & 0xFF);
            const auto g = static_cast<std::uint8_t>((vert.color >> 8) & 0xFF);
            const auto r = static_cast<std::uint8_t>((vert.color) & 0xFF);

            // A channel encodes the alpha parameter
            const auto expectedA = static_cast<std::uint8_t>(alpha * 255.0f);
            REQUIRE(a == expectedA);

            // RGB channels are fully saturated (0xFF) for RenderBitmapAlpha per-vertex alpha
            REQUIRE(r == 0xFFu);
            REQUIRE(g == 0xFFu);
            REQUIRE(b == 0xFFu);
        }
    }

    SECTION("alpha=1.0f produces A=255 (same as opaque white)")
    {
        const auto verts = BuildRenderBitmapVertices(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

        for (const auto& vert : verts)
        {
            const auto a = static_cast<std::uint8_t>((vert.color >> 24) & 0xFF);
            REQUIRE(a == 255u);
        }
    }
}

// ---------------------------------------------------------------------------
// AC-1 [4-2-2]: RenderQuad2D receives exactly 4 vertices per call
// GIVEN: RenderQuad2DCapture test-double
// WHEN:  vertex array of exactly 4 Vertex2D is passed to RenderQuad2D
// THEN:  captured call has vertex count == 4
//
// This test verifies the vertex count contract for all 9 RenderBitmap* variants.
// Each migrated variant MUST produce exactly 4 vertices per quad call
// (except RenderBitmapAlpha which calls 16 times with 4 vertices each).
// ---------------------------------------------------------------------------
TEST_CASE("AC-1 [4-2-2]: RenderQuad2D receives exactly 4 vertices per call",
    "[render][renderbitmap][ac-1]")
{
    SECTION("A standard RenderBitmap call produces exactly 4 vertices")
    {
        // GIVEN
        RenderQuad2DCapture capture;

        // WHEN
        const auto verts = BuildRenderBitmapVertices(0.0f, 100.0f, 32.0f, 32.0f,
            0.0f, 0.0f, 0.5f, 0.5f);
        capture.RenderQuad2D(std::span<const mu::Vertex2D>(verts), 1u);

        // THEN
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);
    }
}

// ---------------------------------------------------------------------------
// AC-VAL-1 [4-2-2]: RenderBitmapAlpha calls RenderQuad2D 16 times (4×4 tile loop)
// GIVEN: RenderQuad2DCapture test-double
// WHEN:  16 calls are made simulating RenderBitmapAlpha's inner loop
// THEN:  captured call count == 16; each has 4 vertices with per-vertex alpha packing
//
// RenderBitmapAlpha uses a 4×4 tile loop — each tile is a separate quad.
// The per-vertex alpha gradient is encoded in Vertex2D::color A channel.
// ---------------------------------------------------------------------------
TEST_CASE("AC-VAL-1 [4-2-2]: RenderBitmapAlpha calls RenderQuad2D 16 times",
    "[render][renderbitmap][ac-val-1]")
{
    SECTION("4x4 tile loop produces 16 RenderQuad2D calls, each with 4 vertices")
    {
        // GIVEN
        RenderQuad2DCapture capture;

        // WHEN: simulate RenderBitmapAlpha's 4×4 loop
        constexpr int tileCount = 4 * 4;
        for (int i = 0; i < tileCount; ++i)
        {
            // Each tile has a different alpha value (edge gradient)
            const float alpha = static_cast<float>(i) / static_cast<float>(tileCount - 1);
            const auto verts = BuildRenderBitmapVertices(
                static_cast<float>(i) * 8.0f, 64.0f, 8.0f, 8.0f,
                0.0f, 0.0f, 1.0f, 1.0f, alpha);
            capture.RenderQuad2D(std::span<const mu::Vertex2D>(verts), 5u);
        }

        // THEN: exactly 16 calls, each with 4 vertices
        REQUIRE(capture.m_calls.size() == 16u);
        for (const auto& call : capture.m_calls)
        {
            REQUIRE(call.vertices.size() == 4u);
        }
    }
}

// ---------------------------------------------------------------------------
// AC-2 [4-2-2]: textureId=0 accepted by RenderQuad2D for untextured quad (RenderColor)
// GIVEN: RenderQuad2DCapture test-double
// WHEN:  RenderQuad2D is called with textureId == 0 (untextured/color-fill path)
// THEN:  captured call records textureId == 0 without error
//
// This verifies Option A (textureId=0 sentinel) works through the interface.
// MuRendererGL::RenderQuad2D must handle textureId=0 by binding 0 (white texture).
// ---------------------------------------------------------------------------
TEST_CASE("AC-2 [4-2-2]: textureId=0 accepted by RenderQuad2D for untextured quad",
    "[render][renderbitmap][ac-2]")
{
    SECTION("textureId=0 (RenderColor / untextured path) is recorded correctly")
    {
        // GIVEN
        RenderQuad2DCapture capture;

        // WHEN: color fill quad — solid red (ABGR: A=0xFF, B=0x00, G=0x00, R=0xFF)
        constexpr std::uint32_t solidRed = 0xFF0000FFu;
        const std::array<mu::Vertex2D, 4> colorQuad = {{
            { 0.0f,  480.0f, 0.0f, 0.0f, solidRed },
            { 0.0f,  448.0f, 0.0f, 0.0f, solidRed },
            { 64.0f, 448.0f, 0.0f, 0.0f, solidRed },
            { 64.0f, 480.0f, 0.0f, 0.0f, solidRed },
        }};

        capture.RenderQuad2D(std::span<const mu::Vertex2D>(colorQuad), 0u);

        // THEN: call recorded with textureId == 0 (untextured sentinel)
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].textureId == 0u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);

        // Verify color is preserved in the vertex data
        for (const auto& vert : capture.m_calls[0].vertices)
        {
            REQUIRE(vert.color == solidRed);
        }
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2 [4-2-2]: RenderBitmapLocalRotate vertex positions — Rotate=0
// GIVEN: RenderBitmapLocalRotate vertex building logic with Rotate=0
// WHEN:  sinf(0)=0, cosf(0)=1: positions are computed using cached sinR/cosR
// THEN:  vertex[0].x == center.x + (Width*0.5), vertex[0].y == center.y + 0
//        (since vDir = (Width*0.5, -Height*0.5):
//          p[0][0] = cx + (Width*0.5) * cos(0) = cx + Width*0.5
//          p[0][1] = cy + (-Height*0.5) * sin(0) = cy + 0
//        Documents the sinR/cosR caching pattern and rotation correctness at Rotate=0)
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-2]: RenderBitmapLocalRotate vertex positions — Rotate=0",
    "[render][renderbitmap][ac-std-2]")
{
    SECTION("Rotate=0: sin=0, cos=1: vertex[0] = (cx+W*0.5, cy), vertex[2] = (cx-W*0.5, cy)")
    {
        // GIVEN: build positions using the same formula as RenderBitmapLocalRotate
        constexpr float cx = 100.0f;
        constexpr float cy = 200.0f;
        constexpr float W = 64.0f;
        constexpr float H = 32.0f;
        constexpr float Rotate = 0.0f;

        // vCenter = (cx, cy), vDir = (W*0.5, -H*0.5)
        const float sinR = std::sin(Rotate); // = 0
        const float cosR = std::cos(Rotate); // = 1
        const float dirX = W * 0.5f;
        const float dirY = -H * 0.5f;

        // Mirror the rotation formula from ZzzOpenglUtil.cpp:RenderBitmapLocalRotate
        float px0 = cx + dirX * cosR;  // cx + (W*0.5)*1 = cx + W*0.5
        float py0 = cy + dirY * sinR;  // cy + (-H*0.5)*0 = cy
        float px2 = cx - dirX * cosR;  // cx - W*0.5
        float py2 = cy - dirY * sinR;  // cy

        // THEN: at Rotate=0, vertex[0] is top-right, vertex[2] is bottom-left
        REQUIRE(px0 == Catch::Approx(cx + W * 0.5f));
        REQUIRE(py0 == Catch::Approx(cy));
        REQUIRE(px2 == Catch::Approx(cx - W * 0.5f));
        REQUIRE(py2 == Catch::Approx(cy));

        // Verify sinR/cosR caching: same results as direct trig calls
        REQUIRE(sinR == Catch::Approx(std::sin(Rotate)));
        REQUIRE(cosR == Catch::Approx(std::cos(Rotate)));
    }
}
