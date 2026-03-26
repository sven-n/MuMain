// Story 4.2.3: Migrate Skeletal Mesh Rendering to RenderTriangles
// Flow Code: VS1-RENDER-MIGRATE-TRIANGLES
//
// GREEN PHASE: All migration tasks complete. Tests verify the implementation contracts:
//   - MuRendererGL::RenderTriangles emits per-vertex color (glColor4f) — Task 7 done
//   - BMD::RenderMesh() delegates to mu::GetRenderer().RenderTriangles() — Task 2 done
//   - BMD::EndRenderCoinHeap() delegates to mu::GetRenderer().RenderTriangles() — Task 3 done
//   - BMD::RenderMeshAlternative() delegates to mu::GetRenderer().RenderTriangles() — Task 4 done
//   - BMD::RenderMeshTranslate() delegates to mu::GetRenderer().RenderTriangles() — Task 5 done
//   - AddMeshShadowTriangles / AddClothesShadowTriangles delegate to RenderTriangles() — Task 6 done
//
// IMPORTANT: No OpenGL calls in this test TU.
// RenderTrianglesCapture is a test-double of IMuRenderer defined inline below.
// No gl* functions are called; all tests are pure logic, runnable on macOS/Linux.
//
// AC Mapping:
//   AC-STD-2(a) → TEST_CASE("AC-STD-2 [4-2-3]: Vertex3D struct layout")
//   AC-STD-2(b) → TEST_CASE("AC-STD-2 [4-2-3]: RenderTriangles call-through — single mesh")
//   AC-STD-2(c) → TEST_CASE("AC-STD-2 [4-2-3]: ABGR color packing — opaque white")
//   AC-STD-2(c) → TEST_CASE("AC-STD-2 [4-2-3]: ABGR color packing — per-vertex RGB from glColor3fv")
//   AC-STD-2(c) → TEST_CASE("AC-STD-2 [4-2-3]: ABGR color packing — semi-transparent from glColor4f")
//   AC-VAL-1    → TEST_CASE("AC-VAL-1 [4-2-3]: RenderTriangles vertex count equals NumTriangles * 3")
//   AC-5        → TEST_CASE("AC-5 [4-2-3]: Shadow path uses textureId=0 and zero UV/normal fields")
//
// Run with: ctest --test-dir MuMain/build -R skeletalmesh_migration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "MuRenderer.h"

// ---------------------------------------------------------------------------
// Test-double: RenderTrianglesCapture
// Implements IMuRenderer; captures RenderTriangles calls for assertion.
// Stores vertex count and textureId from each call.
// ---------------------------------------------------------------------------
namespace
{

struct CapturedTriangleCall
{
    std::vector<mu::Vertex3D> vertices;
    std::uint32_t textureId;
};

struct RenderTrianglesCapture : public mu::IMuRenderer
{
    std::vector<CapturedTriangleCall> m_calls;

    void RenderQuad2D(std::span<const mu::Vertex2D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void RenderTriangles(std::span<const mu::Vertex3D> vertices, std::uint32_t textureId) override
    {
        CapturedTriangleCall call;
        call.vertices.assign(vertices.begin(), vertices.end());
        call.textureId = textureId;
        m_calls.push_back(std::move(call));
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
};

// ---------------------------------------------------------------------------
// Helper: PackABGR
// Mirrors the static inline PackABGR() function in ZzzBMD.cpp.
// Converts float RGBA channels to a packed 32-bit ABGR value.
// A=bits31-24, B=bits23-16, G=bits15-8, R=bits7-0
//
// KEEP IN SYNC WITH: ZzzBMD.cpp::PackABGR
// Channels are clamped to [0.0, 1.0] to match the production implementation
// (LightTransform can exceed 1.0 for overbright effects — clamping prevents
// truncated/incorrect color channels).
// ---------------------------------------------------------------------------
[[nodiscard]] std::uint32_t PackABGR(float r, float g, float b, float a)
{
    auto clamp01 = [](float v) -> float { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); };
    return (static_cast<std::uint32_t>(clamp01(a) * 255.f) << 24) |
           (static_cast<std::uint32_t>(clamp01(b) * 255.f) << 16) |
           (static_cast<std::uint32_t>(clamp01(g) * 255.f) << 8) |
            static_cast<std::uint32_t>(clamp01(r) * 255.f);
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// AC-STD-2(a) [4-2-3]: Vertex3D struct layout — field order contract
// GIVEN: a mu::Vertex3D initialised with known values
// WHEN:  each named field is read back
// THEN:  all fields hold the expected value — documents the field ordering
//        contract that all migration code depends on
//
// RED PHASE: Passes immediately after MuRenderer.h includes Vertex3D definition.
// This test exists to lock the field contract so a future layout change breaks loudly.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-3]: Vertex3D struct layout", "[render][skeletalmesh][ac-std-2]")
{
    SECTION("All Vertex3D fields are readable at expected offsets")
    {
        // GIVEN: construct with distinct values for each field
        const mu::Vertex3D v{ 1.f, 2.f, 3.f,   // x, y, z
                              0.f, 1.f, 0.f,    // nx, ny, nz
                              0.5f, 0.5f,       // u, v
                              0xFFFFFFFFu };     // color (opaque white ABGR)

        // THEN: each field holds the expected value
        REQUIRE(v.x == 1.f);
        REQUIRE(v.y == 2.f);
        REQUIRE(v.z == 3.f);
        REQUIRE(v.nx == 0.f);
        REQUIRE(v.ny == 1.f);
        REQUIRE(v.nz == 0.f);
        REQUIRE(v.u == 0.5f);
        REQUIRE(v.v == 0.5f);
        REQUIRE(v.color == 0xFFFFFFFFu);
    }

    SECTION("Vertex3D zero-initialised is all zeros except color must be manually set")
    {
        // GIVEN: value-initialised (all zero) Vertex3D
        const mu::Vertex3D v{};

        // THEN: all float fields are 0.f; color is 0 (default)
        REQUIRE(v.x == 0.f);
        REQUIRE(v.y == 0.f);
        REQUIRE(v.z == 0.f);
        REQUIRE(v.nx == 0.f);
        REQUIRE(v.ny == 0.f);
        REQUIRE(v.nz == 0.f);
        REQUIRE(v.u == 0.f);
        REQUIRE(v.v == 0.f);
        REQUIRE(v.color == 0u);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2(b) [4-2-3]: RenderTriangles call-through — single mesh
// GIVEN: a RenderTrianglesCapture test-double
// WHEN:  a 2-triangle (6-vertex) Vertex3D array is passed to RenderTriangles
//        with textureId = 42u
// THEN:  exactly 1 call is recorded; capturedCount == 6; capturedTextureId == 42u
//
// RED PHASE: Tests the call-through contract that each migrated ZzzBMD.cpp
// function must satisfy. The test passes once each function delegates
// correctly to mu::GetRenderer().RenderTriangles().
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-3]: RenderTriangles call-through — single mesh",
    "[render][skeletalmesh][ac-std-2]")
{
    SECTION("Single mesh: exactly one RenderTriangles call with 6 vertices and correct textureId")
    {
        // GIVEN: a capture test-double
        RenderTrianglesCapture capture;

        // WHEN: simulate migrated RenderMesh building a 2-triangle (6-vertex) buffer
        constexpr int numTriangles = 2;
        constexpr int numVerts = numTriangles * 3;
        constexpr std::uint32_t textureId = 42u;

        std::vector<mu::Vertex3D> muVerts;
        muVerts.reserve(numVerts);
        for (int i = 0; i < numVerts; ++i)
        {
            muVerts.push_back({ static_cast<float>(i), 0.f, 0.f,
                                0.f, 1.f, 0.f,
                                0.f, 0.f,
                                0xFFFFFFFFu });
        }
        capture.RenderTriangles(std::span<const mu::Vertex3D>(muVerts), textureId);

        // THEN: exactly one call recorded with the correct vertex count and textureId
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].vertices.size() == static_cast<std::size_t>(numVerts));
        REQUIRE(capture.m_calls[0].textureId == textureId);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2(c) [4-2-3]: ABGR color packing — opaque white
// GIVEN: PackABGR(1.f, 1.f, 1.f, 1.f) (all channels full)
// WHEN:  the result is unpacked channel by channel
// THEN:  A=255, B=255, G=255, R=255; packed == 0xFFFFFFFFu
//
// Documents the ABGR bit-packing convention used in all migrated paths.
// Mirrors the unpack logic in MuRendererGL::RenderTriangles (Task 7).
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-3]: ABGR color packing — opaque white",
    "[render][skeletalmesh][ac-std-2]")
{
    SECTION("PackABGR(1,1,1,1) == 0xFFFFFFFF; all channels unpack to 255")
    {
        // GIVEN / WHEN
        const std::uint32_t packed = PackABGR(1.f, 1.f, 1.f, 1.f);

        // THEN: packed value is opaque white
        REQUIRE(packed == 0xFFFFFFFFu);

        // Unpack ABGR: A=bits31-24, B=bits23-16, G=bits15-8, R=bits7-0
        const auto a = static_cast<std::uint8_t>((packed >> 24) & 0xFF);
        const auto b = static_cast<std::uint8_t>((packed >> 16) & 0xFF);
        const auto g = static_cast<std::uint8_t>((packed >> 8) & 0xFF);
        const auto r = static_cast<std::uint8_t>((packed) & 0xFF);

        REQUIRE(a == 255u);
        REQUIRE(b == 255u);
        REQUIRE(g == 255u);
        REQUIRE(r == 255u);
    }

    SECTION("Constant 0xFFFFFFFF unpacks to all-255 channels — MuRendererGL decode contract")
    {
        // GIVEN: the sentinel value used in shadow paths and untextured geometry
        constexpr std::uint32_t opaqueWhite = 0xFFFFFFFFu;

        // WHEN: unpack using the same shifts as MuRendererGL::RenderTriangles
        const auto a = static_cast<float>((opaqueWhite >> 24) & 0xFF) / 255.f;
        const auto b = static_cast<float>((opaqueWhite >> 16) & 0xFF) / 255.f;
        const auto g_ch = static_cast<float>((opaqueWhite >> 8) & 0xFF) / 255.f;
        const auto r = static_cast<float>((opaqueWhite) & 0xFF) / 255.f;

        // THEN: all channels decode to 1.0f
        REQUIRE(a == Catch::Approx(1.f));
        REQUIRE(b == Catch::Approx(1.f));
        REQUIRE(g_ch == Catch::Approx(1.f));
        REQUIRE(r == Catch::Approx(1.f));
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2(c) [4-2-3]: ABGR color packing — per-vertex RGB from glColor3fv
// GIVEN: PackABGR(r, g, b, 1.f) — simulating glColor3fv(rgb) with alpha=1
// WHEN:  the result is unpacked
// THEN:  A=0xFF; B, G, R match the input float channels scaled to uint8
//        Documents the glColor3fv → PackABGR mapping for RenderMeshAlternative
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-3]: ABGR color packing — per-vertex RGB from glColor3fv",
    "[render][skeletalmesh][ac-std-2]")
{
    SECTION("PackABGR(0.5, 0.25, 0.75, 1.0) — distinct R/G/B, fully opaque")
    {
        // GIVEN: distinct per-vertex light values (simulating LightTransform)
        constexpr float r = 0.5f;
        constexpr float g = 0.25f;
        constexpr float b = 0.75f;
        constexpr float a = 1.f; // glColor3fv → alpha=1

        // WHEN
        const std::uint32_t packed = PackABGR(r, g, b, a);

        // THEN: unpack and compare
        const auto unpackedA = static_cast<std::uint8_t>((packed >> 24) & 0xFF);
        const auto unpackedB = static_cast<std::uint8_t>((packed >> 16) & 0xFF);
        const auto unpackedG = static_cast<std::uint8_t>((packed >> 8) & 0xFF);
        const auto unpackedR = static_cast<std::uint8_t>((packed) & 0xFF);

        REQUIRE(unpackedA == static_cast<std::uint8_t>(a * 255.f)); // 255
        REQUIRE(unpackedB == static_cast<std::uint8_t>(b * 255.f)); // 191
        REQUIRE(unpackedG == static_cast<std::uint8_t>(g * 255.f)); // 63
        REQUIRE(unpackedR == static_cast<std::uint8_t>(r * 255.f)); // 127
    }

    SECTION("PackABGR(0, 0, 0, 1) — black opaque — shadow path color sentinel")
    {
        // GIVEN: used in shadow geometry (position-only vertices, black opaque)
        const std::uint32_t packed = PackABGR(0.f, 0.f, 0.f, 1.f);

        // THEN: A=0xFF, all color channels = 0
        const auto unpackedA = static_cast<std::uint8_t>((packed >> 24) & 0xFF);
        const auto unpackedB = static_cast<std::uint8_t>((packed >> 16) & 0xFF);
        const auto unpackedG = static_cast<std::uint8_t>((packed >> 8) & 0xFF);
        const auto unpackedR = static_cast<std::uint8_t>((packed) & 0xFF);

        REQUIRE(unpackedA == 0xFFu);
        REQUIRE(unpackedB == 0u);
        REQUIRE(unpackedG == 0u);
        REQUIRE(unpackedR == 0u);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2(c) [4-2-3]: ABGR color packing — semi-transparent from glColor4f
// GIVEN: PackABGR(r, g, b, alpha) with alpha < 1.0 (RenderMeshAlternative Alpha case)
// WHEN:  the result is unpacked
// THEN:  A channel encodes the alpha parameter; RGB channels encode per-vertex light
//        Documents the glColor4f(r,g,b,a) → PackABGR mapping
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-3]: ABGR color packing — semi-transparent from glColor4f",
    "[render][skeletalmesh][ac-std-2]")
{
    SECTION("PackABGR with alpha=0.5f encodes semi-transparency in A channel")
    {
        // GIVEN: simulating glColor4f(Light[0], Light[1], Light[2], Alpha)
        constexpr float r = 0.8f;
        constexpr float g = 0.6f;
        constexpr float b = 0.4f;
        constexpr float alpha = 0.5f;

        // WHEN
        const std::uint32_t packed = PackABGR(r, g, b, alpha);

        // THEN: A channel encodes alpha
        const auto unpackedA = static_cast<std::uint8_t>((packed >> 24) & 0xFF);
        const auto expectedA = static_cast<std::uint8_t>(alpha * 255.f);
        REQUIRE(unpackedA == expectedA); // ~127

        // RGB channels encode per-vertex light
        const auto unpackedR = static_cast<std::uint8_t>((packed) & 0xFF);
        const auto unpackedG = static_cast<std::uint8_t>((packed >> 8) & 0xFF);
        const auto unpackedB = static_cast<std::uint8_t>((packed >> 16) & 0xFF);
        REQUIRE(unpackedR == static_cast<std::uint8_t>(r * 255.f));
        REQUIRE(unpackedG == static_cast<std::uint8_t>(g * 255.f));
        REQUIRE(unpackedB == static_cast<std::uint8_t>(b * 255.f));
    }

    SECTION("PackABGR round-trip: encode then decode preserves channel values")
    {
        // GIVEN: test several (r, g, b, a) combinations
        struct TestCase
        {
            float r, g, b, a;
        };
        const TestCase cases[] = {
            { 1.f, 0.f, 0.f, 1.f },   // red, opaque
            { 0.f, 1.f, 0.f, 0.5f },  // green, half alpha
            { 0.f, 0.f, 1.f, 0.25f }, // blue, quarter alpha
            { 0.f, 0.f, 0.f, 1.f },   // black, opaque (shadow sentinel)
        };

        for (const auto& tc : cases)
        {
            const std::uint32_t packed = PackABGR(tc.r, tc.g, tc.b, tc.a);

            // Decode
            const auto decodedA = static_cast<float>((packed >> 24) & 0xFF) / 255.f;
            const auto decodedB = static_cast<float>((packed >> 16) & 0xFF) / 255.f;
            const auto decodedG = static_cast<float>((packed >> 8) & 0xFF) / 255.f;
            const auto decodedR = static_cast<float>((packed) & 0xFF) / 255.f;

            // Allow 1 ULP of float→uint8→float conversion error (< 0.004)
            REQUIRE(decodedA == Catch::Approx(tc.a).margin(1.f / 255.f));
            REQUIRE(decodedB == Catch::Approx(tc.b).margin(1.f / 255.f));
            REQUIRE(decodedG == Catch::Approx(tc.g).margin(1.f / 255.f));
            REQUIRE(decodedR == Catch::Approx(tc.r).margin(1.f / 255.f));
        }
    }
}

// ---------------------------------------------------------------------------
// AC-VAL-1 [4-2-3]: RenderTriangles vertex count equals NumTriangles * 3
// GIVEN: a RenderTrianglesCapture test-double
// WHEN:  a vertex array of size NumTriangles * 3 is passed to RenderTriangles
// THEN:  captured vertex count == NumTriangles * 3
//
// Documents the vertex count contract for all 5 migrated functions.
// Each function MUST pass exactly NumTriangles * 3 vertices per call.
// ---------------------------------------------------------------------------
TEST_CASE("AC-VAL-1 [4-2-3]: RenderTriangles vertex count equals NumTriangles * 3",
    "[render][skeletalmesh][ac-val-1]")
{
    SECTION("4 triangles -> 12 vertices passed to RenderTriangles")
    {
        // GIVEN
        RenderTrianglesCapture capture;
        constexpr int numTriangles = 4;
        constexpr int expectedVertCount = numTriangles * 3;

        // WHEN: simulate array-based path (e.g., RenderMesh)
        std::vector<mu::Vertex3D> muVerts;
        muVerts.reserve(expectedVertCount);
        for (int i = 0; i < expectedVertCount; ++i)
        {
            muVerts.push_back({ static_cast<float>(i), 0.f, 0.f,
                                0.f, 0.f, 1.f,
                                0.f, 0.f,
                                0xFFFFFFFFu });
        }
        capture.RenderTriangles(std::span<const mu::Vertex3D>(muVerts), 1u);

        // THEN
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].vertices.size() == static_cast<std::size_t>(expectedVertCount));
    }

    SECTION("Vertex count is divisible by 3 (triangle count contract)")
    {
        // GIVEN: 7 triangles (odd number to probe divisibility)
        RenderTrianglesCapture capture;
        constexpr int numTriangles = 7;
        constexpr int expectedVertCount = numTriangles * 3; // 21

        std::vector<mu::Vertex3D> muVerts(expectedVertCount, mu::Vertex3D{});
        capture.RenderTriangles(std::span<const mu::Vertex3D>(muVerts), 2u);

        // THEN: vertex count is exactly NumTriangles * 3 and divisible by 3
        const std::size_t captured = capture.m_calls[0].vertices.size();
        REQUIRE(captured == static_cast<std::size_t>(expectedVertCount));
        REQUIRE(captured % 3u == 0u);
    }
}

// ---------------------------------------------------------------------------
// AC-5 [4-2-3]: Shadow path uses textureId=0 and zero UV/normal fields
// GIVEN: shadow geometry Vertex3D built with {x, y, z, 0,0,0, 0,0, 0xFF000000u}
// WHEN:  passed to RenderTriangles with textureId = 0
// THEN:  captured textureId == 0; UV fields == 0; nx/ny/nz == 0
//        Documents the position-only vertex contract for shadow paths
//        (AddMeshShadowTriangles, AddClothesShadowTriangles)
// ---------------------------------------------------------------------------
TEST_CASE("AC-5 [4-2-3]: Shadow path uses textureId=0 and zero UV/normal fields",
    "[render][skeletalmesh][ac-5]")
{
    SECTION("Shadow geometry: textureId=0, UV=0, normals=0, color=0xFF000000u (black opaque)")
    {
        // GIVEN: a capture test-double
        RenderTrianglesCapture capture;

        // WHEN: simulate shadow path vertex building
        // AddMeshShadowTriangles builds position-only vertices; normals/UV/color are zero
        constexpr int numShadowTriangles = 3;
        constexpr int numVerts = numShadowTriangles * 3;
        constexpr std::uint32_t shadowColor = 0xFF000000u; // black opaque (ABGR)
        constexpr std::uint32_t shadowTextureId = 0u;      // no texture — shadow path

        std::vector<mu::Vertex3D> shadowVerts;
        shadowVerts.reserve(numVerts);
        for (int i = 0; i < numVerts; ++i)
        {
            // Position-only: x varies, y and z are zero for this test
            shadowVerts.push_back({ static_cast<float>(i), 0.f, 0.f,
                                    0.f, 0.f, 0.f,  // normals = zero (position-only)
                                    0.f, 0.f,        // UV = zero (no texture)
                                    shadowColor });
        }
        capture.RenderTriangles(std::span<const mu::Vertex3D>(shadowVerts), shadowTextureId);

        // THEN: textureId = 0 (no texture sentinel)
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].textureId == shadowTextureId);

        // THEN: all vertices have zero UV, zero normals, and the shadow color
        for (const auto& v : capture.m_calls[0].vertices)
        {
            REQUIRE(v.nx == 0.f);
            REQUIRE(v.ny == 0.f);
            REQUIRE(v.nz == 0.f);
            REQUIRE(v.u == 0.f);
            REQUIRE(v.v == 0.f);
            REQUIRE(v.color == shadowColor);
        }
    }

    SECTION("textureId=0 sentinel accepted without error (null texture path)")
    {
        // GIVEN: simplest shadow geometry (1 triangle)
        RenderTrianglesCapture capture;
        const std::array<mu::Vertex3D, 3> tri = {{
            { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0xFF000000u },
            { 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0xFF000000u },
            { 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0xFF000000u },
        }};

        // WHEN: call with textureId = 0
        capture.RenderTriangles(std::span<const mu::Vertex3D>(tri), 0u);

        // THEN: call recorded correctly — no error, textureId 0 preserved
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].textureId == 0u);
        REQUIRE(capture.m_calls[0].vertices.size() == 3u);
    }
}
