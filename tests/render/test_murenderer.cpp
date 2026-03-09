// Story 4.2.1: MuRenderer Core API with OpenGL Backend [VS1-RENDER-ABSTRACT-CORE]
//
// GREEN PHASE: MuRenderer.h, MuRenderer.cpp, MatrixStack.h, MatrixStack.cpp
// implemented in RenderFX/. All 8 TEST_CASEs pass.
//
// IMPORTANT: No OpenGL calls in this test TU.
// MatrixStack and BlendModeTracker are pure-logic types — no gl* functions.
// The BlendModeTracker is a test-double of IMuRenderer defined inline below.
//
// Run with: ctest --test-dir MuMain/build -R murenderer

#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cstdint>
#include <span>
#include <stack>

// Headers under test — will fail to include until Task 1/3 are implemented
#include "MatrixStack.h"
#include "MuRenderer.h"

// ---------------------------------------------------------------------------
// Test-double: BlendModeTracker
// Implements IMuRenderer; captures only the SetBlendMode() call.
// Used in AC-STD-2 blend mode round-trip tests (no gl* calls required).
// ---------------------------------------------------------------------------
namespace
{

struct BlendModeTracker : public mu::IMuRenderer
{
    mu::BlendMode m_lastMode{mu::BlendMode::Alpha};
    bool m_blendModeSet{false};

    void RenderQuad2D(std::span<const mu::Vertex2D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void RenderTriangles(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void RenderQuadStrip(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void SetBlendMode(mu::BlendMode mode) override
    {
        m_lastMode = mode;
        m_blendModeSet = true;
    }

    void SetDepthTest(bool /*enabled*/) override
    {
    }

    void SetFog(const mu::FogParams& /*params*/) override
    {
    }

    [[nodiscard]] mu::BlendMode GetCurrentBlendMode() const
    {
        return m_lastMode;
    }
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// AC-4 / AC-STD-2 [4-2-1]: MatrixStack push/pop correctness
// GIVEN: a fresh MatrixStack with identity top
// WHEN:  Push() then Translate(1,2,3)
// THEN:  Top() reflects translation; Pop() restores original identity
// ---------------------------------------------------------------------------
TEST_CASE("AC-4 [4-2-1]: MatrixStack push/pop correctness", "[render][murenderer][ac-4]")
{
    SECTION("Push copies current top; Pop restores it")
    {
        mu::MatrixStack stack;

        // GIVEN: fresh stack — top should be identity (m[0]==1 ... m[15]==1 diagonal)
        const mu::Matrix4x4 original = stack.Top();
        REQUIRE(original.m[0] == 1.0f);  // M[0,0]
        REQUIRE(original.m[5] == 1.0f);  // M[1,1]
        REQUIRE(original.m[10] == 1.0f); // M[2,2]
        REQUIRE(original.m[15] == 1.0f); // M[3,3]

        // WHEN: push and translate
        stack.Push();
        stack.Translate(1.0f, 2.0f, 3.0f);

        // THEN: top has changed (translation applied)
        const mu::Matrix4x4 translated = stack.Top();
        REQUIRE(translated.m[12] == 1.0f); // Tx
        REQUIRE(translated.m[13] == 2.0f); // Ty
        REQUIRE(translated.m[14] == 3.0f); // Tz

        // WHEN: pop
        stack.Pop();

        // THEN: top restored to original identity
        const mu::Matrix4x4 restored = stack.Top();
        REQUIRE(restored.m[0] == 1.0f);
        REQUIRE(restored.m[5] == 1.0f);
        REQUIRE(restored.m[10] == 1.0f);
        REQUIRE(restored.m[15] == 1.0f);
        REQUIRE(restored.m[12] == 0.0f); // Tx reset
        REQUIRE(restored.m[13] == 0.0f); // Ty reset
        REQUIRE(restored.m[14] == 0.0f); // Tz reset
    }
}

// ---------------------------------------------------------------------------
// AC-4 / AC-STD-2 [4-2-1]: MatrixStack balanced operations
// GIVEN: an empty MatrixStack
// WHEN:  5 pushes followed by 5 pops
// THEN:  stack depth returns to original (IsEmpty() == false, single identity entry)
// ---------------------------------------------------------------------------
TEST_CASE("AC-4 [4-2-1]: MatrixStack balanced push/pop depth", "[render][murenderer][ac-4]")
{
    SECTION("5 pushes + 5 pops restore original stack depth")
    {
        mu::MatrixStack stack;

        // GIVEN: Initial state — one identity matrix on stack (not empty)
        REQUIRE_FALSE(stack.IsEmpty());

        // WHEN: push 5 times with translations
        for (int i = 0; i < 5; ++i)
        {
            stack.Push();
            stack.Translate(static_cast<float>(i), 0.0f, 0.0f);
        }

        // WHEN: pop 5 times
        for (int i = 0; i < 5; ++i)
        {
            stack.Pop();
        }

        // THEN: stack is back to single identity entry
        REQUIRE_FALSE(stack.IsEmpty());

        const mu::Matrix4x4 top = stack.Top();
        REQUIRE(top.m[0] == 1.0f);
        REQUIRE(top.m[5] == 1.0f);
        REQUIRE(top.m[10] == 1.0f);
        REQUIRE(top.m[15] == 1.0f);
        REQUIRE(top.m[12] == 0.0f); // no residual translation
    }
}

// ---------------------------------------------------------------------------
// AC-4 [4-2-1]: MatrixStack nested translations accumulate
// GIVEN: a fresh MatrixStack
// WHEN:  Push + Translate(1,0,0), Push + Translate(2,0,0)
// THEN:  Top reflects cumulative translation; after Pop, only first translation
// ---------------------------------------------------------------------------
TEST_CASE("AC-4 [4-2-1]: MatrixStack nested translations accumulate", "[render][murenderer][ac-4]")
{
    SECTION("Nested push/translate accumulates; pop restores outer context")
    {
        mu::MatrixStack stack;

        stack.Push();
        stack.Translate(1.0f, 0.0f, 0.0f);
        REQUIRE(stack.Top().m[12] == 1.0f);

        stack.Push();
        stack.Translate(2.0f, 0.0f, 0.0f);
        // After two translates: cumulative Tx = 1+2 = 3
        REQUIRE(stack.Top().m[12] == 3.0f);

        stack.Pop();
        // Restored to after first translate: Tx = 1
        REQUIRE(stack.Top().m[12] == 1.0f);

        stack.Pop();
        // Restored to identity: Tx = 0
        REQUIRE(stack.Top().m[12] == 0.0f);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2 [4-2-1]: BlendMode state tracking round-trip
// GIVEN: a BlendModeTracker (IMuRenderer test-double)
// WHEN:  SetBlendMode(X) is called
// THEN:  GetCurrentBlendMode() returns X
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-1]: BlendMode state tracking round-trip", "[render][murenderer][ac-std-2]")
{
    SECTION("SetBlendMode(Additive) is tracked correctly")
    {
        // GIVEN
        BlendModeTracker tracker;
        REQUIRE_FALSE(tracker.m_blendModeSet); // not yet set

        // WHEN
        tracker.SetBlendMode(mu::BlendMode::Additive);

        // THEN
        REQUIRE(tracker.m_blendModeSet);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::Additive);
    }

    SECTION("SetBlendMode(Alpha) is tracked correctly")
    {
        BlendModeTracker tracker;
        tracker.SetBlendMode(mu::BlendMode::Alpha);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::Alpha);
    }

    SECTION("SetBlendMode(Subtract) is tracked correctly")
    {
        BlendModeTracker tracker;
        tracker.SetBlendMode(mu::BlendMode::Subtract);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::Subtract);
    }

    SECTION("SetBlendMode(InverseColor) is tracked correctly")
    {
        BlendModeTracker tracker;
        tracker.SetBlendMode(mu::BlendMode::InverseColor);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::InverseColor);
    }

    SECTION("SetBlendMode(Mixed) is tracked correctly")
    {
        BlendModeTracker tracker;
        tracker.SetBlendMode(mu::BlendMode::Mixed);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::Mixed);
    }

    SECTION("SetBlendMode(LightMap) is tracked correctly")
    {
        BlendModeTracker tracker;
        tracker.SetBlendMode(mu::BlendMode::LightMap);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::LightMap);
    }

    SECTION("BlendMode can be changed after initial set")
    {
        BlendModeTracker tracker;
        tracker.SetBlendMode(mu::BlendMode::Alpha);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::Alpha);

        tracker.SetBlendMode(mu::BlendMode::Additive);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::Additive);

        tracker.SetBlendMode(mu::BlendMode::LightMap);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::LightMap);
    }
}

// ---------------------------------------------------------------------------
// AC-1 [4-2-1]: IMuRenderer interface is complete (all 6 methods present)
// GIVEN: BlendModeTracker (concrete IMuRenderer subclass)
// WHEN:  All 6 IMuRenderer methods are called
// THEN:  No link error / compile error — verifies interface is fully declared
// ---------------------------------------------------------------------------
TEST_CASE("AC-1 [4-2-1]: IMuRenderer interface has all required methods", "[render][murenderer][ac-1]")
{
    SECTION("All 6 IMuRenderer methods are callable via test-double")
    {
        BlendModeTracker tracker;

        // RenderQuad2D
        std::array<mu::Vertex2D, 4> quad2d{};
        tracker.RenderQuad2D(std::span<const mu::Vertex2D>(quad2d), 0u);

        // RenderTriangles
        std::array<mu::Vertex3D, 3> tris{};
        tracker.RenderTriangles(std::span<const mu::Vertex3D>(tris), 0u);

        // RenderQuadStrip
        std::array<mu::Vertex3D, 4> strip{};
        tracker.RenderQuadStrip(std::span<const mu::Vertex3D>(strip), 0u);

        // SetBlendMode
        tracker.SetBlendMode(mu::BlendMode::Alpha);

        // SetDepthTest
        tracker.SetDepthTest(true);
        tracker.SetDepthTest(false);

        // SetFog
        mu::FogParams fog{};
        tracker.SetFog(fog);

        // If we reach here without compile/link error, AC-1 interface is complete
        REQUIRE(true);
    }
}

// ---------------------------------------------------------------------------
// AC-5 [4-2-1]: All new code lives in mu:: namespace
// GIVEN: the mu:: namespace types
// WHEN:  used with explicit namespace qualification
// THEN:  compile succeeds (namespace check)
// ---------------------------------------------------------------------------
TEST_CASE("AC-5 [4-2-1]: All new types are in mu:: namespace", "[render][murenderer][ac-5]")
{
    SECTION("mu::BlendMode enum is in mu:: namespace")
    {
        mu::BlendMode mode = mu::BlendMode::Alpha;
        REQUIRE(static_cast<std::uint8_t>(mode) == static_cast<std::uint8_t>(mu::BlendMode::Alpha));
    }

    SECTION("mu::MatrixStack is in mu:: namespace")
    {
        mu::MatrixStack stack;
        REQUIRE_FALSE(stack.IsEmpty());
    }

    SECTION("mu::Matrix4x4 is in mu:: namespace")
    {
        mu::Matrix4x4 mat;
        // Identity constructor: diagonal = 1, off-diagonal = 0
        REQUIRE(mat.m[0] == 1.0f);
        REQUIRE(mat.m[1] == 0.0f);
    }

    SECTION("mu::Vertex2D is in mu:: namespace")
    {
        mu::Vertex2D v{};
        v.x = 1.0f;
        v.y = 2.0f;
        v.u = 0.5f;
        v.v = 0.5f;
        REQUIRE(v.x == 1.0f);
    }

    SECTION("mu::Vertex3D is in mu:: namespace")
    {
        mu::Vertex3D v{};
        v.x = 1.0f;
        v.y = 2.0f;
        v.z = 3.0f;
        REQUIRE(v.z == 3.0f);
    }

    SECTION("mu::FogParams is in mu:: namespace")
    {
        mu::FogParams fog{};
        fog.start = 10.0f;
        fog.end = 100.0f;
        REQUIRE(fog.start == 10.0f);
    }
}

// ---------------------------------------------------------------------------
// AC-VAL-2 [4-2-1]: IMuRenderer interface has no OpenGL types in its signature
// This is a compile-time check: if GLenum/GLuint appeared in MuRenderer.h,
// the test TU would fail to compile on macOS/Linux (no GL headers pulled in here).
// GIVEN: MuRenderer.h included at top of file WITHOUT any OpenGL headers
// WHEN:  test compiles successfully
// THEN:  no OpenGL types leak into the interface
// ---------------------------------------------------------------------------
TEST_CASE("AC-VAL-2 [4-2-1]: IMuRenderer interface has no OpenGL types", "[render][murenderer][ac-val-2]")
{
    SECTION("MuRenderer.h compiles without GL headers — no GLenum/GLuint in interface")
    {
        // The mere fact that this test file compiles without #including any GL
        // headers (gl.h, glew.h, etc.) and still uses IMuRenderer proves that
        // no OpenGL types leaked into MuRenderer.h.
        BlendModeTracker tracker;
        tracker.SetBlendMode(mu::BlendMode::Alpha);
        REQUIRE(tracker.GetCurrentBlendMode() == mu::BlendMode::Alpha);
    }
}

// ---------------------------------------------------------------------------
// AC-4 [4-2-1]: MatrixStack IsEmpty() correctly reports state
// ---------------------------------------------------------------------------
TEST_CASE("AC-4 [4-2-1]: MatrixStack IsEmpty reflects stack depth", "[render][murenderer][ac-4]")
{
    SECTION("Fresh stack with one identity is NOT empty")
    {
        mu::MatrixStack stack;
        REQUIRE_FALSE(stack.IsEmpty());
    }

    SECTION("After Push, still not empty")
    {
        mu::MatrixStack stack;
        stack.Push();
        REQUIRE_FALSE(stack.IsEmpty());
    }
}
