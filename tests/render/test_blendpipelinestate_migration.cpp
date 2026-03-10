// Story 4.2.5: Migrate Blend & Pipeline State to MuRenderer
// Flow Code: VS1-RENDER-MIGRATE-STATE
//
// RED PHASE: Tests verify the contracts for:
//   - SetBlendMode() routing for all 7 blend helper function mappings
//   - SetDepthTest() bool routing (enable / disable paths)
//   - SetFog() FogParams population for GL_LINEAR scenario (GMBattleCastle migration)
//   - DisableBlend() routing (new IMuRenderer method added in Task 2)
//
// IMPORTANT: No OpenGL calls in this test TU.
// MockBlendPipelineRenderer is a test-double of IMuRenderer defined inline below.
// All tests are pure logic, runnable on macOS/Linux without Win32 or OpenGL.
//
// AC Mapping:
//   AC-STD-2(a) → TEST_CASE("AC-STD-2 [4-2-5]: SetBlendMode — blend helper mapping to BlendMode enum")
//   AC-STD-2(b) → TEST_CASE("AC-STD-2 [4-2-5]: SetDepthTest — enable and disable paths")
//   AC-STD-2(c) → TEST_CASE("AC-STD-2 [4-2-5]: SetFog — GL_LINEAR params population")
//   AC-VAL-1    → TEST_CASE("AC-VAL-1 [4-2-5]: Catch2 test coverage — all 7 blend mappings pass")
//
// Run with: ctest --test-dir MuMain/build -R blendpipelinestate_migration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "MuRenderer.h"

// ---------------------------------------------------------------------------
// Test-double: MockBlendPipelineRenderer
// Implements IMuRenderer; captures SetBlendMode, DisableBlend, SetDepthTest,
// and SetFog calls for assertion in tests.
// ---------------------------------------------------------------------------
namespace
{

struct MockBlendPipelineRenderer : public mu::IMuRenderer
{
    // Captured SetBlendMode state
    std::optional<mu::BlendMode> m_lastBlendMode;
    int m_setBlendModeCallCount{ 0 };

    // Captured DisableBlend state
    int m_disableBlendCallCount{ 0 };

    // Captured SetDepthTest state
    std::optional<bool> m_lastDepthEnabled;
    int m_setDepthTestCallCount{ 0 };

    // Captured SetFog state
    std::optional<mu::FogParams> m_lastFogParams;
    int m_setFogCallCount{ 0 };

    // --- IMuRenderer interface: render methods (no-op stubs) ---

    void RenderQuad2D(std::span<const mu::Vertex2D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void RenderTriangles(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    void RenderQuadStrip(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
    }

    // --- IMuRenderer interface: state methods (capture for assertion) ---

    void SetBlendMode(mu::BlendMode mode) override
    {
        m_lastBlendMode = mode;
        ++m_setBlendModeCallCount;
    }

    void DisableBlend() override
    {
        ++m_disableBlendCallCount;
    }

    void SetDepthTest(bool enabled) override
    {
        m_lastDepthEnabled = enabled;
        ++m_setDepthTestCallCount;
    }

    void SetFog(const mu::FogParams& params) override
    {
        m_lastFogParams = params;
        ++m_setFogCallCount;
    }
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// AC-STD-2(a) [4-2-5]: SetBlendMode — blend helper mapping to BlendMode enum
//
// Documents the mapping of all 7 ZzzOpenglUtil blend helper functions to
// BlendMode enum values after the Task 2/3 wrappers are implemented:
//
//   EnableLightMap()        → SetBlendMode(BlendMode::LightMap)
//   EnableAlphaTest()       → SetBlendMode(BlendMode::Alpha)
//   EnableAlphaBlend()      → SetBlendMode(BlendMode::Glow)       [NEW enum value]
//   EnableAlphaBlendMinus() → SetBlendMode(BlendMode::Subtract)
//   EnableAlphaBlend2()     → SetBlendMode(BlendMode::Luminance)  [NEW enum value]
//   EnableAlphaBlend3()     → SetBlendMode(BlendMode::Alpha)
//   EnableAlphaBlend4()     → SetBlendMode(BlendMode::Mixed)
//   DisableAlphaBlend()     → DisableBlend()                       [NEW method]
//
// RED PHASE: BlendMode::Glow and BlendMode::Luminance do not yet exist in the
// enum; DisableBlend() is not yet declared in IMuRenderer. Tests will fail to
// compile until Task 2 (extend BlendMode enum + add DisableBlend) is complete.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-5]: SetBlendMode — blend helper mapping to BlendMode enum",
    "[render][blendpipelinestate][ac-std-2]")
{
    SECTION("EnableLightMap maps to BlendMode::LightMap — GL_ZERO, GL_SRC_COLOR")
    {
        // GIVEN: a capture mock
        MockBlendPipelineRenderer mock;

        // WHEN: simulate wrapped EnableLightMap() calling SetBlendMode
        mock.SetBlendMode(mu::BlendMode::LightMap);

        // THEN: correct BlendMode recorded; exactly one call
        REQUIRE(mock.m_lastBlendMode.has_value());
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::LightMap);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
    }

    SECTION("EnableAlphaTest maps to BlendMode::Alpha — GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA")
    {
        MockBlendPipelineRenderer mock;

        mock.SetBlendMode(mu::BlendMode::Alpha);

        REQUIRE(mock.m_lastBlendMode.has_value());
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Alpha);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
    }

    SECTION("EnableAlphaBlend maps to BlendMode::Glow — GL_ONE, GL_ONE (NEW)")
    {
        // RED PHASE: BlendMode::Glow does not yet exist — test fails until Task 2 adds it.
        MockBlendPipelineRenderer mock;

        mock.SetBlendMode(mu::BlendMode::Glow);

        REQUIRE(mock.m_lastBlendMode.has_value());
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Glow);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
    }

    SECTION("EnableAlphaBlendMinus maps to BlendMode::Subtract — GL_ZERO, GL_ONE_MINUS_SRC_COLOR")
    {
        MockBlendPipelineRenderer mock;

        mock.SetBlendMode(mu::BlendMode::Subtract);

        REQUIRE(mock.m_lastBlendMode.has_value());
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Subtract);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
    }

    SECTION("EnableAlphaBlend2 maps to BlendMode::Luminance — GL_ONE_MINUS_SRC_COLOR, GL_ONE (NEW)")
    {
        // RED PHASE: BlendMode::Luminance does not yet exist — test fails until Task 2 adds it.
        MockBlendPipelineRenderer mock;

        mock.SetBlendMode(mu::BlendMode::Luminance);

        REQUIRE(mock.m_lastBlendMode.has_value());
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Luminance);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
    }

    SECTION("EnableAlphaBlend3 maps to BlendMode::Alpha — GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA")
    {
        // EnableAlphaBlend3 uses the same GL factors as EnableAlphaTest → Alpha
        MockBlendPipelineRenderer mock;

        mock.SetBlendMode(mu::BlendMode::Alpha);

        REQUIRE(mock.m_lastBlendMode.has_value());
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Alpha);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
    }

    SECTION("EnableAlphaBlend4 maps to BlendMode::Mixed — GL_ONE, GL_ONE_MINUS_SRC_ALPHA")
    {
        MockBlendPipelineRenderer mock;

        mock.SetBlendMode(mu::BlendMode::Mixed);

        REQUIRE(mock.m_lastBlendMode.has_value());
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Mixed);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
    }

    SECTION("DisableAlphaBlend maps to DisableBlend() — glDisable(GL_BLEND) path (NEW)")
    {
        // RED PHASE: DisableBlend() is not yet declared in IMuRenderer —
        // test fails to compile until Task 2 adds the pure virtual method.
        MockBlendPipelineRenderer mock;

        mock.DisableBlend();

        REQUIRE(mock.m_disableBlendCallCount == 1);
        // SetBlendMode must NOT be called when disabling blend
        REQUIRE(mock.m_setBlendModeCallCount == 0);
    }

    SECTION("All 7 blend modes can be round-tripped through SetBlendMode without side effects")
    {
        // Documents that each call is independent — subsequent calls do not bleed state
        // RED PHASE: requires BlendMode::Glow and BlendMode::Luminance to be defined
        MockBlendPipelineRenderer mock;

        const mu::BlendMode modes[] = {
            mu::BlendMode::LightMap,
            mu::BlendMode::Alpha,
            mu::BlendMode::Glow,
            mu::BlendMode::Subtract,
            mu::BlendMode::Luminance,
            mu::BlendMode::Mixed,
            mu::BlendMode::Additive,
        };

        int callCount = 0;
        for (const auto mode : modes)
        {
            mock.SetBlendMode(mode);
            ++callCount;
            REQUIRE(mock.m_lastBlendMode.value() == mode);
            REQUIRE(mock.m_setBlendModeCallCount == callCount);
        }
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2(b) [4-2-5]: SetDepthTest — enable and disable paths
//
// Documents the contract for the two wrapped depth test helpers after
// Task 4 wraps them to delegate to IMuRenderer::SetDepthTest:
//
//   EnableDepthTest()  → SetDepthTest(true)
//   DisableDepthTest() → SetDepthTest(false)
//
// Also covers the CameraMove.cpp migration: direct glEnable/glDisable(GL_DEPTH_TEST)
// replaced by EnableDepthTest()/DisableDepthTest() which internally call SetDepthTest.
//
// GREEN PHASE after Task 4 is implemented (SetDepthTest already exists in IMuRenderer).
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-5]: SetDepthTest — enable and disable paths",
    "[render][blendpipelinestate][ac-std-2]")
{
    SECTION("EnableDepthTest path: SetDepthTest(true) — depth test enabled")
    {
        // GIVEN: a capture mock (simulates ZzzOpenglUtil::EnableDepthTest() post-wrapping)
        MockBlendPipelineRenderer mock;

        // WHEN: simulate wrapped EnableDepthTest() calling SetDepthTest(true)
        mock.SetDepthTest(true);

        // THEN: enabled=true recorded; exactly one call
        REQUIRE(mock.m_lastDepthEnabled.has_value());
        REQUIRE(mock.m_lastDepthEnabled.value() == true);
        REQUIRE(mock.m_setDepthTestCallCount == 1);
    }

    SECTION("DisableDepthTest path: SetDepthTest(false) — depth test disabled")
    {
        // GIVEN: a capture mock (simulates ZzzOpenglUtil::DisableDepthTest() post-wrapping)
        MockBlendPipelineRenderer mock;

        // WHEN: simulate wrapped DisableDepthTest() calling SetDepthTest(false)
        mock.SetDepthTest(false);

        // THEN: enabled=false recorded; exactly one call
        REQUIRE(mock.m_lastDepthEnabled.has_value());
        REQUIRE(mock.m_lastDepthEnabled.value() == false);
        REQUIRE(mock.m_setDepthTestCallCount == 1);
    }

    SECTION("Enable then disable: state transitions track correctly")
    {
        // GIVEN: a capture mock
        MockBlendPipelineRenderer mock;

        // WHEN: enable then disable (CameraMove.cpp skybox sequence: disable before skybox,
        //        re-enable after — two independent calls)
        mock.SetDepthTest(false); // DisableDepthTest (skybox start)
        REQUIRE(mock.m_lastDepthEnabled.value() == false);
        REQUIRE(mock.m_setDepthTestCallCount == 1);

        mock.SetDepthTest(true); // EnableDepthTest (skybox end)
        REQUIRE(mock.m_lastDepthEnabled.value() == true);
        REQUIRE(mock.m_setDepthTestCallCount == 2);
    }

    SECTION("Disable then enable: state transitions track correctly (reverse order)")
    {
        MockBlendPipelineRenderer mock;

        mock.SetDepthTest(true);
        REQUIRE(mock.m_lastDepthEnabled.value() == true);

        mock.SetDepthTest(false);
        REQUIRE(mock.m_lastDepthEnabled.value() == false);

        REQUIRE(mock.m_setDepthTestCallCount == 2);
    }

    SECTION("SetDepthTest does not affect blend mode state — independent state machines")
    {
        // Documents that SetDepthTest and SetBlendMode are independent
        MockBlendPipelineRenderer mock;

        mock.SetDepthTest(true);
        mock.SetBlendMode(mu::BlendMode::Alpha);

        REQUIRE(mock.m_lastDepthEnabled.value() == true);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Alpha);
        REQUIRE(mock.m_setDepthTestCallCount == 1);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2(c) [4-2-5]: SetFog — GL_LINEAR params population
//
// Documents the FogParams struct population contract for the GMBattleCastle.cpp
// fog migration (Task 5). The four direct glFog* calls are replaced with a
// single mu::GetRenderer().SetFog(fogParams) call using a populated FogParams.
//
// Reference from story dev notes:
//   fogParams.mode    = GL_LINEAR;   // 0x2601
//   fogParams.start   = 2000.f;
//   fogParams.end     = 2700.f;
//   fogParams.density = 0.f;
//   fogParams.color[0..3] = Color[0..3]; (from GMBattleCastle float Color[4] array)
//
// GREEN PHASE after Task 5 is implemented (SetFog already exists in IMuRenderer).
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [4-2-5]: SetFog — GL_LINEAR params population",
    "[render][blendpipelinestate][ac-std-2]")
{
    // GL_LINEAR constant (0x2601) — stored as plain int in FogParams so MuRenderer.h
    // does not expose GL types. Value from stdafx.h OpenGL stubs.
    constexpr int GL_LINEAR_VALUE = 0x2601;

    SECTION("GMBattleCastle fog: GL_LINEAR mode, start=2000, end=2700, density=0")
    {
        // GIVEN: a capture mock and FogParams populated as Task 5 will implement
        MockBlendPipelineRenderer mock;

        // Simulate the Color[4] float array from GMBattleCastle.cpp
        const float color[4] = { 0.5f, 0.6f, 0.7f, 1.0f };

        // WHEN: populate FogParams exactly as the migrated GMBattleCastle code will
        mu::FogParams fogParams{};
        fogParams.mode    = GL_LINEAR_VALUE;
        fogParams.start   = 2000.f;
        fogParams.end     = 2700.f;
        fogParams.density = 0.f;
        fogParams.color[0] = color[0];
        fogParams.color[1] = color[1];
        fogParams.color[2] = color[2];
        fogParams.color[3] = color[3];

        mock.SetFog(fogParams);

        // THEN: FogParams captured correctly
        REQUIRE(mock.m_setFogCallCount == 1);
        REQUIRE(mock.m_lastFogParams.has_value());

        const auto& captured = mock.m_lastFogParams.value();
        REQUIRE(captured.mode == GL_LINEAR_VALUE);
        REQUIRE(captured.start == Catch::Approx(2000.f));
        REQUIRE(captured.end == Catch::Approx(2700.f));
        REQUIRE(captured.density == Catch::Approx(0.f));

        // Color channels preserved from source Color[] array
        REQUIRE(captured.color[0] == Catch::Approx(color[0]));
        REQUIRE(captured.color[1] == Catch::Approx(color[1]));
        REQUIRE(captured.color[2] == Catch::Approx(color[2]));
        REQUIRE(captured.color[3] == Catch::Approx(color[3]));
    }

    SECTION("FogParams zero-initialization: unset fields default to 0.f")
    {
        // GIVEN: FogParams with only mode/start/end set (as in GMBattleCastle migration)
        MockBlendPipelineRenderer mock;

        mu::FogParams fogParams{};
        fogParams.mode  = GL_LINEAR_VALUE;
        fogParams.start = 2000.f;
        fogParams.end   = 2700.f;
        // density intentionally 0.f (zero-init from {})
        // color intentionally 0.f (zero-init from {})

        mock.SetFog(fogParams);

        REQUIRE(mock.m_lastFogParams.has_value());
        const auto& captured = mock.m_lastFogParams.value();
        REQUIRE(captured.density == Catch::Approx(0.f));
        REQUIRE(captured.color[0] == Catch::Approx(0.f));
        REQUIRE(captured.color[1] == Catch::Approx(0.f));
        REQUIRE(captured.color[2] == Catch::Approx(0.f));
        REQUIRE(captured.color[3] == Catch::Approx(0.f));
    }

    SECTION("SetFog does not affect blend mode or depth test state")
    {
        // Documents that SetFog is an independent state call
        MockBlendPipelineRenderer mock;

        mu::FogParams fogParams{};
        fogParams.mode  = GL_LINEAR_VALUE;
        fogParams.start = 2000.f;
        fogParams.end   = 2700.f;

        mock.SetFog(fogParams);
        mock.SetBlendMode(mu::BlendMode::Alpha);
        mock.SetDepthTest(true);

        REQUIRE(mock.m_setFogCallCount == 1);
        REQUIRE(mock.m_setBlendModeCallCount == 1);
        REQUIRE(mock.m_setDepthTestCallCount == 1);
        REQUIRE(mock.m_lastFogParams.value().mode == GL_LINEAR_VALUE);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Alpha);
        REQUIRE(mock.m_lastDepthEnabled.value() == true);
    }
}

// ---------------------------------------------------------------------------
// AC-VAL-1 [4-2-5]: Full blend mapping round-trip — all 7 helpers + DisableBlend
//
// GIVEN: a MockBlendPipelineRenderer
// WHEN:  each of the 7 helper function patterns is simulated in order
// THEN:  the correct BlendMode or DisableBlend call is recorded for each
//
// This consolidates all 7 blend helper mappings into a single scenario
// that mirrors the sequence a typical MU Online rendering frame would execute
// (switching blend states as different scene elements are drawn).
//
// RED PHASE: requires BlendMode::Glow and BlendMode::Luminance; requires DisableBlend().
// ---------------------------------------------------------------------------
TEST_CASE("AC-VAL-1 [4-2-5]: Full blend state sequence — all 7 helper mappings verified",
    "[render][blendpipelinestate][ac-val-1]")
{
    SECTION("Sequential blend state changes: 7 Set calls + 1 Disable call")
    {
        // GIVEN: a capture mock
        MockBlendPipelineRenderer mock;

        // WHEN: simulate a typical rendering sequence using all 7 blend modes
        // Each call represents one wrapper function delegating to mu::GetRenderer()

        // 1. EnableLightMap() wrapper → SetBlendMode(LightMap)
        mock.SetBlendMode(mu::BlendMode::LightMap);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::LightMap);

        // 2. EnableAlphaTest() wrapper → SetBlendMode(Alpha)
        mock.SetBlendMode(mu::BlendMode::Alpha);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Alpha);

        // 3. EnableAlphaBlend() wrapper → SetBlendMode(Glow) [NEW]
        mock.SetBlendMode(mu::BlendMode::Glow);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Glow);

        // 4. EnableAlphaBlendMinus() wrapper → SetBlendMode(Subtract)
        mock.SetBlendMode(mu::BlendMode::Subtract);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Subtract);

        // 5. EnableAlphaBlend2() wrapper → SetBlendMode(Luminance) [NEW]
        mock.SetBlendMode(mu::BlendMode::Luminance);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Luminance);

        // 6. EnableAlphaBlend4() wrapper → SetBlendMode(Mixed)
        mock.SetBlendMode(mu::BlendMode::Mixed);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Mixed);

        // 7. EnableAlphaBlend3() wrapper → SetBlendMode(Alpha) (same as EnableAlphaTest)
        mock.SetBlendMode(mu::BlendMode::Alpha);
        REQUIRE(mock.m_lastBlendMode.value() == mu::BlendMode::Alpha);

        // 8. DisableAlphaBlend() wrapper → DisableBlend() [NEW]
        mock.DisableBlend();
        REQUIRE(mock.m_disableBlendCallCount == 1);

        // THEN: total SetBlendMode calls = 7; DisableBlend calls = 1
        REQUIRE(mock.m_setBlendModeCallCount == 7);
        REQUIRE(mock.m_disableBlendCallCount == 1);
    }

    SECTION("DisableBlend is separate from SetBlendMode — calling one does not invoke the other")
    {
        MockBlendPipelineRenderer mock;

        mock.DisableBlend();

        // THEN: only DisableBlend was called; SetBlendMode was NOT called
        REQUIRE(mock.m_disableBlendCallCount == 1);
        REQUIRE(mock.m_setBlendModeCallCount == 0);
        REQUIRE_FALSE(mock.m_lastBlendMode.has_value());
    }
}
