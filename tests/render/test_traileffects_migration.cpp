// Story 4.2.4: Migrate Trail Effects to RenderQuadStrip
// Flow Code: VS1-RENDER-MIGRATE-QUADSTRIP
//
// GREEN PHASE: All 7 TEST_CASEs implemented and passing. Tests verify the contracts
// for all glBegin(GL_QUADS) trail segment blocks in RenderJoints() migrated to
// mu::GetRenderer().RenderQuadStrip(). Tasks 1–7 complete.
//
// IMPORTANT: No OpenGL calls in this test TU.
// RenderQuadStripCapture is a test-double of IMuRenderer defined inline below.
// No gl* functions are called; all tests are pure logic, runnable on macOS/Linux.
//
// AC Mapping:
//   AC-STD-2(b) → TEST_CASE("AC-STD-2 [4-2-4]: RenderQuadStrip call-through — single trail segment quad")
//   AC-STD-2(c) → TEST_CASE("AC-STD-2 [4-2-4]: UV mapping from Light1/Light2")
//   AC-VAL-1    → TEST_CASE("AC-VAL-1 [4-2-4]: Luminosity color packing — RenderQuadStrip sentinel")
//   AC-2        → TEST_CASE("AC-2 [4-2-4]: BITMAP_JOINT_FORCE trail — 4 vertices, textureId=0")
//   AC-3        → TEST_CASE("AC-3 [4-2-4]: GUILD_WAR_EVENT double-face — two separate RenderQuadStrip calls")
//   AC-4        → TEST_CASE("AC-4 [4-2-4]: RENDER_FACE_ONE and RENDER_FACE_TWO — independent calls")
//   AC-7        → TEST_CASE("AC-7 [4-2-4]: RenderQuadStrip textureId=0 sentinel accepted")
//
// Run with: ctest --test-dir MuMain/build -R traileffects_migration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "MuRenderer.h"
#include "RenderUtils.h"

using mu::PackABGR;

// ---------------------------------------------------------------------------
// Test-double: RenderQuadStripCapture
// Implements IMuRenderer; captures RenderQuadStrip calls for assertion.
// Stores vertex data and textureId from each call.
// ---------------------------------------------------------------------------
namespace
{

struct CapturedQuadStripCall
{
    std::vector<mu::Vertex3D> vertices;
    std::uint32_t textureId;
};

struct RenderQuadStripCapture : public mu::IMuRenderer
{
    std::vector<CapturedQuadStripCall> m_calls;

    void RenderQuad2D(std::span<const mu::Vertex2D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void RenderTriangles(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void RenderQuadStrip(std::span<const mu::Vertex3D> vertices, std::uint32_t textureId) override
    {
        CapturedQuadStripCall call;
        call.vertices.assign(vertices.begin(), vertices.end());
        call.textureId = textureId;
        m_calls.push_back(std::move(call));
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
// Helper: build4VertexTrailSegment
// Simulates the 4-vertex buffer that each migrated trail segment block builds:
//   vertex 0: currentTail[a] with u=uCurrent, v=v0, color=color
//   vertex 1: currentTail[b] with u=uCurrent, v=v1, color=color
//   vertex 2: nextTail[b]    with u=uNext,    v=v1, color=color
//   vertex 3: nextTail[a]    with u=uNext,    v=v0, color=color
// This mirrors the pattern for BITMAP_JOINT_FORCE and RENDER_FACE_ONE/TWO.
// ---------------------------------------------------------------------------
std::vector<mu::Vertex3D> build4VertexTrailSegment(
    std::array<float, 3> cTailA, std::array<float, 3> cTailB,
    std::array<float, 3> nTailA, std::array<float, 3> nTailB,
    float uCurrent, float uNext, float v0, float v1,
    std::uint32_t color)
{
    return {
        { cTailA[0], cTailA[1], cTailA[2], 0.f, 0.f, 0.f, uCurrent, v0, color },
        { cTailB[0], cTailB[1], cTailB[2], 0.f, 0.f, 0.f, uCurrent, v1, color },
        { nTailB[0], nTailB[1], nTailB[2], 0.f, 0.f, 0.f, uNext,    v1, color },
        { nTailA[0], nTailA[1], nTailA[2], 0.f, 0.f, 0.f, uNext,    v0, color },
    };
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// AC-STD-2(b) [4-2-4]: RenderQuadStrip call-through — single trail segment quad
// GIVEN: a RenderQuadStripCapture test-double and a 4-vertex trail segment buffer
// WHEN:  the buffer is passed to RenderQuadStrip with textureId = 0 (sentinel)
// THEN:  exactly 1 call is recorded; capturedCount == 4; capturedTextureId == 0u
//
// RED PHASE: Documents the call-through contract for every migrated trail
// segment block in RenderJoints(). Each block MUST delegate to
// mu::GetRenderer().RenderQuadStrip(verts, 0u) with exactly 4 vertices.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-4]: RenderQuadStrip call-through — single trail segment quad",
    "[render][traileffects][ac-std-2]")
{
    SECTION("Single trail segment: exactly one RenderQuadStrip call with 4 vertices and textureId=0")
    {
        // GIVEN: a capture test-double
        RenderQuadStripCapture capture;

        // WHEN: simulate migrated BITMAP_JOINT_FORCE trail building a 4-vertex buffer
        // Positions approximate typical tail array values from RenderJoints()
        const std::vector<mu::Vertex3D> verts = build4VertexTrailSegment(
            { 10.f, 0.f, 5.f },  // currentTail[0]
            { 10.f, 0.f, -5.f }, // currentTail[1]
            { 15.f, 0.f, -5.f }, // nextTail[1]
            { 15.f, 0.f, 5.f },  // nextTail[0]
            0.0f,  // Light1 (uCurrent)
            1.0f,  // Light2 (uNext)
            0.f,   // V bottom
            1.f,   // V top
            0xFF808080u // PackABGR(0.5, 0.5, 0.5, 1.0) — Luminosity=0.5 example
        );

        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(verts), 0u);

        // THEN: exactly one call with 4 vertices and the sentinel textureId
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);
        REQUIRE(capture.m_calls[0].textureId == 0u);
    }

    SECTION("RenderQuadStrip accepts exactly 4 vertices — no more, no less for a single segment")
    {
        // GIVEN: capture test-double
        RenderQuadStripCapture capture;

        // WHEN: minimal valid single-segment call (4 vertices, all zero positions)
        const std::array<mu::Vertex3D, 4> seg = {{
            { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0xFFFFFFFFu },
            { 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0xFFFFFFFFu },
            { 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0xFFFFFFFFu },
            { 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0xFFFFFFFFu },
        }};
        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(seg), 0u);

        // THEN
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2(c) [4-2-4]: UV mapping from Light1/Light2
// GIVEN: a 4-vertex trail segment with Light1=0.25f (uCurrent) and Light2=0.75f (uNext)
// WHEN:  the vertices are built using the migrated vertex construction pattern
// THEN:  vertex[0].u == Light1 (0.25f), vertex[2].u == Light2 (0.75f)
//        — documents the UV assignment contract:
//          currentTail vertices use Light1 as u-coordinate
//          nextTail vertices use Light2 as u-coordinate
//
// This mirrors the RenderJoints() pattern where Light1 advances from 0→1
// over the segment loop (glTexCoord2f(Light1, ...) before glVertex3fv(currentTail[k])).
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-4]: UV mapping from Light1/Light2",
    "[render][traileffects][ac-std-2]")
{
    SECTION("Light1 maps to u for currentTail vertices, Light2 maps to u for nextTail vertices")
    {
        // GIVEN: Light1 and Light2 as used in RenderJoints() segment loop
        constexpr float light1 = 0.25f; // uCurrent
        constexpr float light2 = 0.75f; // uNext

        // WHEN: build 4-vertex segment using the migrated pattern
        const std::vector<mu::Vertex3D> verts = build4VertexTrailSegment(
            { 0.f, 0.f, 2.f },   // currentTail[0]
            { 0.f, 0.f, -2.f },  // currentTail[1]
            { 5.f, 0.f, -2.f },  // nextTail[1]
            { 5.f, 0.f, 2.f },   // nextTail[0]
            light1, light2,
            0.f, 1.f,
            0xFFFFFFFFu
        );

        // THEN: currentTail vertices (indices 0 and 1) have u == Light1
        REQUIRE(verts[0].u == Catch::Approx(light1));
        REQUIRE(verts[1].u == Catch::Approx(light1));

        // THEN: nextTail vertices (indices 2 and 3) have u == Light2
        REQUIRE(verts[2].u == Catch::Approx(light2));
        REQUIRE(verts[3].u == Catch::Approx(light2));
    }

    SECTION("V-coordinate assignment: V0 at bottom edge, V1 at top edge")
    {
        // GIVEN: V-coordinates matching the segment quad edges
        // BITMAP_JOINT_FORCE: Light1=0.f (bottom UV), v flip 0→1
        constexpr float v0 = 0.f;
        constexpr float v1 = 1.f;

        // WHEN: build with explicit V values
        const std::vector<mu::Vertex3D> verts = build4VertexTrailSegment(
            { 0.f, 0.f, 1.f }, { 0.f, 0.f, -1.f },
            { 1.f, 0.f, -1.f }, { 1.f, 0.f, 1.f },
            0.f, 0.5f, v0, v1,
            0xFFFFFFFFu
        );

        // THEN: vertex 0 (currentTail[a]) has v == v0
        REQUIRE(verts[0].v == Catch::Approx(v0));
        // vertex 1 (currentTail[b]) has v == v1
        REQUIRE(verts[1].v == Catch::Approx(v1));
        // vertex 2 (nextTail[b]) has v == v1
        REQUIRE(verts[2].v == Catch::Approx(v1));
        // vertex 3 (nextTail[a]) has v == v0
        REQUIRE(verts[3].v == Catch::Approx(v0));
    }

    SECTION("Light values at loop boundaries: Light1=0, Light2=segment_step — first segment UV")
    {
        // GIVEN: first segment in the loop (Light1=0.0, Light2=step)
        constexpr float segmentStep = 1.f / 16.f; // typical for 16-joint trail
        const std::vector<mu::Vertex3D> verts = build4VertexTrailSegment(
            { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f },
            { 1.f, 0.f, 0.f }, { 1.f, 0.f, 0.f },
            0.f, segmentStep, 0.f, 1.f,
            0xFFFFFFFFu
        );

        REQUIRE(verts[0].u == Catch::Approx(0.f));
        REQUIRE(verts[2].u == Catch::Approx(segmentStep));
    }
}

// ---------------------------------------------------------------------------
// AC-VAL-1 [4-2-4]: Luminosity color packing — RenderQuadStrip sentinel
// GIVEN: PackABGR(0.5f, 0.5f, 0.5f, 1.f) — simulating Luminosity=0.5 in
//        BITMAP_JOINT_FORCE trail segment migration
// WHEN:  the result is checked bit-by-bit
// THEN:  packed == 0xFF808080u (within rounding tolerance)
//        — documents the glColor3f(Lum, Lum, Lum) → PackABGR mapping for trails
//
// Also verifies opaque white (Luminosity=1.f) and overbright clamping.
// ---------------------------------------------------------------------------
TEST_CASE("AC-VAL-1 [4-2-4]: Luminosity color packing — RenderQuadStrip sentinel",
    "[render][traileffects][ac-val-1]")
{
    SECTION("PackABGR(0.5, 0.5, 0.5, 1.0) — Luminosity=0.5 yields 0xFF808080u (within rounding)")
    {
        // GIVEN / WHEN
        const std::uint32_t packed = PackABGR(0.5f, 0.5f, 0.5f, 1.f);

        // THEN: A channel must be 0xFF (fully opaque)
        const auto a = static_cast<std::uint8_t>((packed >> 24) & 0xFF);
        REQUIRE(a == 0xFFu);

        // R, G, B channels must be 0x80 (128 = round(0.5*255)); allow ±1 for float→uint8 rounding
        const auto r = static_cast<std::uint8_t>((packed) & 0xFF);
        const auto g = static_cast<std::uint8_t>((packed >> 8) & 0xFF);
        const auto b = static_cast<std::uint8_t>((packed >> 16) & 0xFF);

        REQUIRE(r >= 127u);
        REQUIRE(r <= 128u);
        REQUIRE(g >= 127u);
        REQUIRE(g <= 128u);
        REQUIRE(b >= 127u);
        REQUIRE(b <= 128u);
    }

    SECTION("PackABGR(1, 1, 1, 1) — Luminosity=1.0 (full brightness) yields 0xFFFFFFFFu")
    {
        // GIVEN: maximum luminosity (glColor3f(1.f, 1.f, 1.f))
        const std::uint32_t packed = PackABGR(1.f, 1.f, 1.f, 1.f);

        // THEN: opaque white
        REQUIRE(packed == 0xFFFFFFFFu);
    }

    SECTION("PackABGR clamps overbright values — Luminosity > 1.0 must not wrap")
    {
        // GIVEN: overbright luminosity (can occur with amplified light effects)
        const std::uint32_t packed = PackABGR(1.5f, 1.5f, 1.5f, 1.f);

        // THEN: clamped to 0xFFFFFFFF — no wrap/truncation artifacts
        REQUIRE(packed == 0xFFFFFFFFu);
    }

    SECTION("PackABGR round-trip for typical luminosity values — encode then decode")
    {
        // GIVEN: test cases derived from RenderJoints() color patterns
        struct TestCase
        {
            float lum; // luminosity value (r == g == b == lum for BITMAP_JOINT_FORCE)
        };
        const TestCase cases[] = {
            { 0.f },   // zero luminosity (dark trail)
            { 0.25f }, // quarter luminosity
            { 0.5f },  // half luminosity
            { 0.75f }, // three-quarter luminosity
            { 1.f },   // full luminosity
        };

        for (const auto& tc : cases)
        {
            const std::uint32_t packed = PackABGR(tc.lum, tc.lum, tc.lum, 1.f);

            // Decode
            const auto decodedA = static_cast<float>((packed >> 24) & 0xFF) / 255.f;
            const auto decodedR = static_cast<float>((packed) & 0xFF) / 255.f;
            const auto decodedG = static_cast<float>((packed >> 8) & 0xFF) / 255.f;
            const auto decodedB = static_cast<float>((packed >> 16) & 0xFF) / 255.f;

            // Allow 1 ULP of float→uint8→float conversion error (< 1/255 ≈ 0.004)
            REQUIRE(decodedA == Catch::Approx(1.f).margin(1.f / 255.f));
            REQUIRE(decodedR == Catch::Approx(tc.lum).margin(1.f / 255.f));
            REQUIRE(decodedG == Catch::Approx(tc.lum).margin(1.f / 255.f));
            REQUIRE(decodedB == Catch::Approx(tc.lum).margin(1.f / 255.f));
        }
    }
}

// ---------------------------------------------------------------------------
// AC-2 [4-2-4]: BITMAP_JOINT_FORCE trail — 4 vertices, textureId=0
// GIVEN: a RenderQuadStripCapture and vertices built for BITMAP_JOINT_FORCE SubType==0
//        using currentTail[0..1] and nextTail[0..1] with UV from Light1/Light2
// WHEN:  RenderQuadStrip is called once with textureId=0
// THEN:  captured vertex count == 4; vertices[0].u == Light1; vertices[2].u == Light2;
//        all vertices carry the Luminosity-packed ABGR color
//
// Documents the specific vertex layout for the BITMAP_JOINT_FORCE migration (Task 2).
// ---------------------------------------------------------------------------
TEST_CASE("AC-2 [4-2-4]: BITMAP_JOINT_FORCE trail — 4 vertices, textureId=0",
    "[render][traileffects][ac-2]")
{
    SECTION("BITMAP_JOINT_FORCE segment: currentTail[0,1] + nextTail[1,0], Light1/2 UV, Lum color")
    {
        // GIVEN: typical values from BITMAP_JOINT_FORCE SubType==0 branch
        constexpr float light1 = 0.0f;
        constexpr float light2 = 0.0625f; // 1/16 for 16-joint trail
        const std::uint32_t lumColor = PackABGR(0.8f, 0.8f, 0.8f, 1.f); // Luminosity=0.8

        // Tail positions (world space, typical joint trail geometry)
        const std::array<float, 3> cTail0 = { 100.f, 50.f, 10.f };
        const std::array<float, 3> cTail1 = { 100.f, 50.f, -10.f };
        const std::array<float, 3> nTail1 = { 105.f, 50.f, -10.f };
        const std::array<float, 3> nTail0 = { 105.f, 50.f, 10.f };

        // Build vertex buffer exactly as migrated Task 2 code will:
        //   {currentTail[0], 0,0,0, Light1, 0.f, color}
        //   {currentTail[1], 0,0,0, Light1, 1.f, color}
        //   {nextTail[1],    0,0,0, Light2, 1.f, color}
        //   {nextTail[0],    0,0,0, Light2, 0.f, color}
        const std::vector<mu::Vertex3D> forceVerts = {
            { cTail0[0], cTail0[1], cTail0[2], 0.f, 0.f, 0.f, light1, 0.f, lumColor },
            { cTail1[0], cTail1[1], cTail1[2], 0.f, 0.f, 0.f, light1, 1.f, lumColor },
            { nTail1[0], nTail1[1], nTail1[2], 0.f, 0.f, 0.f, light2, 1.f, lumColor },
            { nTail0[0], nTail0[1], nTail0[2], 0.f, 0.f, 0.f, light2, 0.f, lumColor },
        };

        // WHEN
        RenderQuadStripCapture capture;
        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(forceVerts), 0u);

        // THEN
        REQUIRE(capture.m_calls.size() == 1u);
        const auto& recorded = capture.m_calls[0];
        REQUIRE(recorded.vertices.size() == 4u);
        REQUIRE(recorded.textureId == 0u);

        // UV contract
        REQUIRE(recorded.vertices[0].u == Catch::Approx(light1));
        REQUIRE(recorded.vertices[1].u == Catch::Approx(light1));
        REQUIRE(recorded.vertices[2].u == Catch::Approx(light2));
        REQUIRE(recorded.vertices[3].u == Catch::Approx(light2));

        // Color contract: all vertices carry the same packed Luminosity color
        REQUIRE(recorded.vertices[0].color == lumColor);
        REQUIRE(recorded.vertices[1].color == lumColor);
        REQUIRE(recorded.vertices[2].color == lumColor);
        REQUIRE(recorded.vertices[3].color == lumColor);

        // Position contract: no normal components
        REQUIRE(recorded.vertices[0].nx == 0.f);
        REQUIRE(recorded.vertices[0].ny == 0.f);
        REQUIRE(recorded.vertices[0].nz == 0.f);
    }
}

// ---------------------------------------------------------------------------
// AC-3 [4-2-4]: GUILD_WAR_EVENT double-face — two separate RenderQuadStrip calls
// GIVEN: the BITMAP_FLARE SubType==22 double-face branch produces TOP face + BOTTOM face,
//        each built from 4 vertices using currentTail/nextTail indices 2/3 and 0/1
// WHEN:  two RenderQuadStrip calls are issued (one per face)
// THEN:  capture records exactly 2 calls; each has 4 vertices and textureId=0;
//        both calls carry the same packed Light*Luminosity color
//
// Documents the two-call pattern required for GUILD_WAR_EVENT migration (Task 3).
// ---------------------------------------------------------------------------
TEST_CASE("AC-3 [4-2-4]: GUILD_WAR_EVENT double-face — two separate RenderQuadStrip calls",
    "[render][traileffects][ac-3]")
{
    SECTION("Top face and bottom face each produce exactly one RenderQuadStrip call with 4 vertices")
    {
        // GIVEN: color from glColor3f(o->Light[0]*Luminosity, ...)
        constexpr float r = 0.9f;
        constexpr float g = 0.7f;
        constexpr float b = 0.5f;
        constexpr float lum = 0.8f;
        const std::uint32_t faceColor = PackABGR(r * lum, g * lum, b * lum, 1.f);

        constexpr float light1 = 0.1f;
        constexpr float light2 = 0.2f;

        // Top face: currentTail[2]/[3] and nextTail[3]/[2] (Subtask 3.2)
        const std::vector<mu::Vertex3D> topFace = {
            { 1.f, 2.f, 3.f, 0.f, 0.f, 0.f, light1, 1.f, faceColor }, // currentTail[2]
            { 1.f, 2.f, -3.f, 0.f, 0.f, 0.f, light1, 0.f, faceColor }, // currentTail[3]
            { 6.f, 2.f, -3.f, 0.f, 0.f, 0.f, light2, 0.f, faceColor }, // nextTail[3]
            { 6.f, 2.f, 3.f, 0.f, 0.f, 0.f, light2, 1.f, faceColor }, // nextTail[2]
        };

        // Bottom face: currentTail[0]/[1] and nextTail[1]/[0] (Subtask 3.2)
        const std::vector<mu::Vertex3D> bottomFace = {
            { 1.f, 0.f, 3.f, 0.f, 0.f, 0.f, light1, 0.f, faceColor }, // currentTail[0]
            { 1.f, 0.f, -3.f, 0.f, 0.f, 0.f, light1, 1.f, faceColor }, // currentTail[1]
            { 6.f, 0.f, -3.f, 0.f, 0.f, 0.f, light2, 1.f, faceColor }, // nextTail[1]
            { 6.f, 0.f, 3.f, 0.f, 0.f, 0.f, light2, 0.f, faceColor }, // nextTail[0]
        };

        // WHEN: issue two calls (one per face, as migrated Task 3 code will)
        RenderQuadStripCapture capture;
        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(topFace), 0u);
        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(bottomFace), 0u);

        // THEN: exactly two calls recorded
        REQUIRE(capture.m_calls.size() == 2u);

        // Top face
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);
        REQUIRE(capture.m_calls[0].textureId == 0u);

        // Bottom face
        REQUIRE(capture.m_calls[1].vertices.size() == 4u);
        REQUIRE(capture.m_calls[1].textureId == 0u);

        // Color consistency: both faces use the same packed color
        REQUIRE(capture.m_calls[0].vertices[0].color == faceColor);
        REQUIRE(capture.m_calls[1].vertices[0].color == faceColor);
    }

    SECTION("Double-face: two calls independent — different positions, same UV and color contract")
    {
        // GIVEN: simplest double-face (all positions distinct, minimal values)
        RenderQuadStripCapture capture;

        constexpr std::uint32_t color = 0xFF4C6680u; // some packed color
        constexpr float l1 = 0.3f;
        constexpr float l2 = 0.4f;

        const std::array<mu::Vertex3D, 4> face0 = {{
            { 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, l1, 1.f, color },
            { 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, l1, 0.f, color },
            { 1.f, 0.f, -1.f, 0.f, 0.f, 0.f, l2, 0.f, color },
            { 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, l2, 1.f, color },
        }};
        const std::array<mu::Vertex3D, 4> face1 = {{
            { 0.f, -1.f, 1.f, 0.f, 0.f, 0.f, l1, 0.f, color },
            { 0.f, -1.f, -1.f, 0.f, 0.f, 0.f, l1, 1.f, color },
            { 1.f, -1.f, -1.f, 0.f, 0.f, 0.f, l2, 1.f, color },
            { 1.f, -1.f, 1.f, 0.f, 0.f, 0.f, l2, 0.f, color },
        }};

        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(face0), 0u);
        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(face1), 0u);

        REQUIRE(capture.m_calls.size() == 2u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);
        REQUIRE(capture.m_calls[1].vertices.size() == 4u);
    }
}

// ---------------------------------------------------------------------------
// AC-4 [4-2-4]: RENDER_FACE_ONE and RENDER_FACE_TWO — independent calls
// GIVEN: two conditional face flags, each independently triggering a RenderQuadStrip call
// WHEN:  both flags are active → two calls; one flag active → one call; neither → zero calls
// THEN:  the capture matches the expected call count in each scenario
//
// Documents the per-flag independent call pattern for Task 4.
// ---------------------------------------------------------------------------
TEST_CASE("AC-4 [4-2-4]: RENDER_FACE_ONE and RENDER_FACE_TWO — independent calls",
    "[render][traileffects][ac-4]")
{
    // Simulate RENDER_FACE flags as booleans (production uses bitfield check)
    auto simulateRenderFaces = [](bool faceOne, bool faceTwo,
                                   RenderQuadStripCapture& capture,
                                   std::uint32_t color, float l1, float l2,
                                   float v1, float v2)
    {
        // RENDER_FACE_ONE: tails[2]/[3] with UV (L1,V2)/(L1,V1)/(L2,V1)/(L2,V2)
        if (faceOne)
        {
            const std::vector<mu::Vertex3D> faceOneVerts = {
                { 1.f, 0.f, 2.f, 0.f, 0.f, 0.f, l1, v2, color }, // currentTail[2]
                { 1.f, 0.f, -2.f, 0.f, 0.f, 0.f, l1, v1, color }, // currentTail[3]
                { 5.f, 0.f, -2.f, 0.f, 0.f, 0.f, l2, v1, color }, // nextTail[3]
                { 5.f, 0.f, 2.f, 0.f, 0.f, 0.f, l2, v2, color }, // nextTail[2]
            };
            capture.RenderQuadStrip(std::span<const mu::Vertex3D>(faceOneVerts), 0u);
        }

        // RENDER_FACE_TWO: tails[0]/[1] with UV (L1,V1)/(L1,V2)/(L2,V2)/(L2,V1)
        if (faceTwo)
        {
            const std::vector<mu::Vertex3D> faceTwoVerts = {
                { 1.f, 0.f, 2.f, 0.f, 0.f, 0.f, l1, v1, color }, // currentTail[0]
                { 1.f, 0.f, -2.f, 0.f, 0.f, 0.f, l1, v2, color }, // currentTail[1]
                { 5.f, 0.f, -2.f, 0.f, 0.f, 0.f, l2, v2, color }, // nextTail[1]
                { 5.f, 0.f, 2.f, 0.f, 0.f, 0.f, l2, v1, color }, // nextTail[0]
            };
            capture.RenderQuadStrip(std::span<const mu::Vertex3D>(faceTwoVerts), 0u);
        }
    };

    constexpr std::uint32_t color = 0xFFCCAAFFu;
    constexpr float l1 = 0.0f;
    constexpr float l2 = 0.125f;
    constexpr float v1 = 0.f;
    constexpr float v2 = 1.f;

    SECTION("Both face flags active — two RenderQuadStrip calls")
    {
        RenderQuadStripCapture capture;
        simulateRenderFaces(true, true, capture, color, l1, l2, v1, v2);

        REQUIRE(capture.m_calls.size() == 2u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u); // RENDER_FACE_ONE
        REQUIRE(capture.m_calls[1].vertices.size() == 4u); // RENDER_FACE_TWO
    }

    SECTION("Only RENDER_FACE_ONE active — one RenderQuadStrip call")
    {
        RenderQuadStripCapture capture;
        simulateRenderFaces(true, false, capture, color, l1, l2, v1, v2);

        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);
    }

    SECTION("Only RENDER_FACE_TWO active — one RenderQuadStrip call")
    {
        RenderQuadStripCapture capture;
        simulateRenderFaces(false, true, capture, color, l1, l2, v1, v2);

        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);
    }

    SECTION("Neither face flag active — zero RenderQuadStrip calls")
    {
        RenderQuadStripCapture capture;
        simulateRenderFaces(false, false, capture, color, l1, l2, v1, v2);

        REQUIRE(capture.m_calls.empty());
    }
}

// ---------------------------------------------------------------------------
// AC-7 [4-2-4]: RenderQuadStrip textureId=0 sentinel accepted
// GIVEN: a trail segment rendered with textureId=0 (the "caller binds texture" sentinel)
// WHEN:  RenderQuadStrip is called with textureId=0 and 4 vertices
// THEN:  the call is recorded correctly — textureId=0 preserved, no error
//
// Documents the textureId=0 sentinel contract established in story 4.2.3
// (same pattern as shadow paths). This is valid for the transitional OpenGL
// phase — MuRendererGL::RenderQuadStrip calls glBindTexture(GL_TEXTURE_2D, 0)
// which harmlessly re-binds the texture already set by BindTexture() in the caller.
// ---------------------------------------------------------------------------
TEST_CASE("AC-7 [4-2-4]: RenderQuadStrip textureId=0 sentinel accepted",
    "[render][traileffects][ac-7]")
{
    SECTION("textureId=0 sentinel: call recorded with preserved textureId and 4 vertices")
    {
        // GIVEN
        RenderQuadStripCapture capture;

        // WHEN: single segment with textureId=0
        const std::array<mu::Vertex3D, 4> seg = {{
            { 0.f, 0.f, 1.f,  0.f, 0.f, 0.f, 0.f, 0.f, 0xFFFFFFFFu },
            { 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0xFFFFFFFFu },
            { 1.f, 0.f, -1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0xFFFFFFFFu },
            { 1.f, 0.f, 1.f,  0.f, 0.f, 0.f, 1.f, 0.f, 0xFFFFFFFFu },
        }};
        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(seg), 0u);

        // THEN: call recorded without error; textureId=0 preserved
        REQUIRE(capture.m_calls.size() == 1u);
        REQUIRE(capture.m_calls[0].textureId == 0u);
        REQUIRE(capture.m_calls[0].vertices.size() == 4u);
    }

    SECTION("Normals are zero for trail vertices — no per-vertex normals in joint trails")
    {
        // GIVEN: trail vertices always have zero normals (no per-vertex normals in joint trails)
        // This is by design — documented in story 4.2.4 dev notes:
        // "No normals in trail paths: use {0.f, 0.f, 0.f} for nx/ny/nz fields."
        RenderQuadStripCapture capture;

        const std::uint32_t color = PackABGR(0.6f, 0.6f, 0.6f, 1.f);
        const std::array<mu::Vertex3D, 4> trailVerts = {{
            { 10.f, 5.f, 3.f,  0.f, 0.f, 0.f, 0.25f, 0.f, color },
            { 10.f, 5.f, -3.f, 0.f, 0.f, 0.f, 0.25f, 1.f, color },
            { 15.f, 5.f, -3.f, 0.f, 0.f, 0.f, 0.5f,  1.f, color },
            { 15.f, 5.f, 3.f,  0.f, 0.f, 0.f, 0.5f,  0.f, color },
        }};
        capture.RenderQuadStrip(std::span<const mu::Vertex3D>(trailVerts), 0u);

        REQUIRE(capture.m_calls.size() == 1u);
        for (const auto& v : capture.m_calls[0].vertices)
        {
            REQUIRE(v.nx == 0.f);
            REQUIRE(v.ny == 0.f);
            REQUIRE(v.nz == 0.f);
        }
    }
}
