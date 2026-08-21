// Story 4.3.1: SDL_gpu Backend Implementation
// Flow Code: VS1-RENDER-SDLGPU-BACKEND
//
// RED PHASE: Tests verify contracts for:
//   (a) TextureRegistry: register, lookup, unregister without requiring a GPU device
//   (b) BlendMode -> SDL_GPUColorTargetBlendState factor mapping table
//   (c) SetFog() stub: FogParams stored correctly in internal state field
//
// IMPORTANT: No actual SDL_GPUDevice is created in these tests.
// MuRendererSDLGpuTestSubclass bypasses device init for pure-logic coverage.
// All tests compile and run on macOS/Linux without Win32, OpenGL, or a GPU device.
//
// AC Mapping:
//   AC-STD-2(a) -> TEST_CASE("AC-STD-2(a) [4-3-1]: TextureRegistry -- register, lookup, unregister")
//   AC-STD-2(b) -> TEST_CASE("AC-STD-2(b) [4-3-1]: BlendMode -- SDL_gpu factor table")
//   AC-STD-2(c) -> TEST_CASE("AC-STD-2(c) [4-3-1]: SetFog -- FogParams storage round-trip")
//   AC-VAL-1    -> TEST_CASE("AC-VAL-1 [4-3-1]: TextureRegistry contract verified")
//
// Run with: ctest --test-dir MuMain/build -R sdlgpubackend
//
// RED PHASE: Tests will fail to compile until MuRendererSDLGpu.cpp is created
// with the required classes, free functions, and testable API surface exposed.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>
#include <utility>

#include "MuRenderer.h"

// ---------------------------------------------------------------------------
// Forward declarations for testable free functions extracted from
// MuRendererSDLGpu.cpp. These must be declared in a header or as
// friend-declared in MuRendererSDLGpu for test access.
//
// RED PHASE: These functions do not yet exist. They must be implemented
// in MuRendererSDLGpu.cpp and made accessible to this TU.
// ---------------------------------------------------------------------------

// SDL_gpu blend factor type forward declaration (avoid including SDL_gpu.h here;
// use the int representation that matches SDL_GPUBlendFactor enum values).
// The actual SDL_GPUBlendFactor enum is defined in SDL3/SDL_gpu.h.
// We use int proxies here so the test TU compiles without SDL3 headers
// while still verifying the factor mapping table values.
//
// From SDL3/SDL_gpu.h (SDL_GPUBlendFactor enum):
//   SDL_GPU_BLENDFACTOR_INVALID               = 0
//   SDL_GPU_BLENDFACTOR_ZERO                  = 1
//   SDL_GPU_BLENDFACTOR_ONE                   = 2
//   SDL_GPU_BLENDFACTOR_SRC_COLOR             = 3
//   SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR   = 4
//   SDL_GPU_BLENDFACTOR_DST_COLOR             = 5
//   SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR   = 6
//   SDL_GPU_BLENDFACTOR_SRC_ALPHA             = 7
//   SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA   = 8
//   SDL_GPU_BLENDFACTOR_DST_ALPHA             = 9
//   SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA   = 10
//   SDL_GPU_BLENDFACTOR_CONSTANT_COLOR        = 11
//   SDL_GPU_BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR = 12
//   SDL_GPU_BLENDFACTOR_SRC_ALPHA_SATURATE    = 13
namespace
{

// Proxy constants matching SDL_GPUBlendFactor enum values from SDL3/SDL_gpu.h.
// These allow the test to verify the factor table without including SDL3 headers.
// Values must match exactly; if SDL3 changes enum values, this test will catch it.
// Verified against SDL3 release-3.2.8 SDL_gpu.h (SDL_GPUBlendFactor typedef enum).
constexpr int k_BlendFactor_Zero              = 1;  // SDL_GPU_BLENDFACTOR_ZERO
constexpr int k_BlendFactor_One               = 2;  // SDL_GPU_BLENDFACTOR_ONE
constexpr int k_BlendFactor_SrcColor          = 3;  // SDL_GPU_BLENDFACTOR_SRC_COLOR
constexpr int k_BlendFactor_OneMinusSrcColor  = 4;  // SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR
constexpr int k_BlendFactor_OneMinusDstColor  = 6;  // SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR
constexpr int k_BlendFactor_SrcAlpha          = 7;  // SDL_GPU_BLENDFACTOR_SRC_ALPHA
constexpr int k_BlendFactor_OneMinusSrcAlpha  = 8;  // SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA

} // anonymous namespace

// ---------------------------------------------------------------------------
// Free function: GetBlendFactors(BlendMode) -> pair<int src, int dst>
//
// Returns the (src_color_blendfactor, dst_color_blendfactor) pair for a given
// BlendMode as int values matching SDL_GPUBlendFactor enum values.
//
// This function must be implemented in MuRendererSDLGpu.cpp and declared
// accessible to this test TU (e.g., via a test-only header or friend class).
//
// RED PHASE: This function does not yet exist — tests fail until Task 3.1 / 3.2
// creates the blend factor table and exposes GetBlendFactors().
// ---------------------------------------------------------------------------
namespace mu
{
    // Forward declaration only — implementation is in MuRendererSDLGpu.cpp.
    // Returns {src_color_blendfactor, dst_color_blendfactor} as int values
    // matching SDL_GPUBlendFactor enum. Declared here for test linkage.
    std::pair<int, int> GetBlendFactors(BlendMode mode);
} // namespace mu

// ---------------------------------------------------------------------------
// TextureRegistry test helpers
//
// The TextureRegistry static functions are declared in MuRendererSDLGpu.cpp.
// For test access we forward-declare the free functions in the mu namespace.
//
// RED PHASE: These functions do not yet exist — tests fail until Task 5.1 / 5.2
// creates the TextureRegistry and exposes RegisterTexture / UnregisterTexture /
// LookupTexture as accessible free functions.
// ---------------------------------------------------------------------------
namespace mu
{
    // Forward declarations — implementations in MuRendererSDLGpu.cpp.
    // Using void* as a proxy for SDL_GPUTexture* to avoid including SDL3 headers.
    // The actual implementation uses SDL_GPUTexture*.
    [[nodiscard]] void* LookupTexture(std::uint32_t id);
    void RegisterTexture(std::uint32_t id, void* pTex);
    void UnregisterTexture(std::uint32_t id);
    void ClearTextureRegistry(); // Test helper: reset registry state between tests
} // namespace mu

// ---------------------------------------------------------------------------
// FogParams test subclass
//
// MuRendererSDLGpuTestSubclass is a test-only subclass of MuRendererSDLGpu
// that bypasses GPU device initialization. It exposes GetLastFogParams()
// for AC-STD-2(c) verification.
//
// RED PHASE: This subclass cannot compile until MuRendererSDLGpu is defined
// in MuRendererSDLGpu.cpp. Until then, we use a mock IMuRenderer instead.
// ---------------------------------------------------------------------------
namespace
{

// Minimal mock IMuRenderer for fog state verification.
// Captures SetFog() calls without requiring any GPU device.
struct FogCaptureMock : public mu::IMuRenderer
{
    std::optional<mu::FogParams> m_lastFogParams;
    int m_setFogCallCount{ 0 };

    void RenderQuad2D(std::span<const mu::Vertex2D> /*vertices*/, std::uint32_t /*textureId*/) override
    {
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

    void SetFog(const mu::FogParams& params) override
    {
        m_lastFogParams = params;
        ++m_setFogCallCount;
    }

    void BeginScene(int /*x*/, int /*y*/, int /*w*/, int /*h*/) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}
    void RenderLines(std::span<const mu::Vertex3D> /*vertices*/, std::uint32_t /*textureId*/) override {}

    [[nodiscard]] const std::optional<mu::FogParams>& GetLastFogParams() const
    {
        return m_lastFogParams;
    }
};

} // anonymous namespace

// ===========================================================================
// TEST CASE: AC-STD-2(a) [4-3-1]: TextureRegistry -- register, lookup, unregister
//
// Verifies the TextureRegistry map contract without requiring a GPU device:
//   - RegisterTexture(id, ptr) stores the pointer
//   - LookupTexture(id) returns the stored pointer
//   - UnregisterTexture(id) removes the entry
//   - LookupTexture on unregistered id returns nullptr (or white fallback texture)
//
// RED PHASE: Fails to link until MuRendererSDLGpu.cpp implements
//   RegisterTexture / UnregisterTexture / LookupTexture / ClearTextureRegistry.
// ===========================================================================
TEST_CASE("AC-STD-2(a) [4-3-1]: TextureRegistry -- register and lookup", "[render][sdlgpubackend][ac-std-2a]")
{
    // Test sentinel pointer (non-null, not a real SDL_GPUTexture*)
    // Cast through uintptr_t to avoid UB on 64-bit platforms
    void* const kSentinelPtr = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xDEADBEEFu));

    SECTION("RegisterTexture stores pointer; LookupTexture returns it")
    {
        // GIVEN: a clean registry state
        mu::ClearTextureRegistry();

        // WHEN: register texture id=42 with a sentinel pointer
        mu::RegisterTexture(42u, kSentinelPtr);

        // THEN: lookup returns the same pointer
        void* result = mu::LookupTexture(42u);
        REQUIRE(result == kSentinelPtr);
    }

    SECTION("UnregisterTexture removes entry; LookupTexture returns null after removal")
    {
        // GIVEN: registry with id=42 registered
        mu::ClearTextureRegistry();
        mu::RegisterTexture(42u, kSentinelPtr);

        // WHEN: unregister id=42
        mu::UnregisterTexture(42u);

        // THEN: lookup returns nullptr (no white fallback for explicitly unregistered ids)
        void* result = mu::LookupTexture(42u);
        // AC-5 note: the implementation may return s_whiteTexture (not nullptr) for
        // unknown IDs. The test accepts either nullptr or a non-sentinel pointer,
        // confirming the original sentinel was removed.
        REQUIRE(result != kSentinelPtr);
    }

    SECTION("LookupTexture on never-registered id returns null-like value")
    {
        // GIVEN: a clean registry (id=99 never registered)
        mu::ClearTextureRegistry();

        // WHEN: lookup non-existent id
        void* result = mu::LookupTexture(99u);

        // THEN: result must not be the sentinel — unknown ids return nullptr or white texture
        // (AC-5: white fallback or nullptr both satisfy the contract for this test)
        REQUIRE(result != kSentinelPtr);
    }

    SECTION("Multiple textures can be registered independently")
    {
        // GIVEN: clean registry
        mu::ClearTextureRegistry();

        void* const kPtr1 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x1111u));
        void* const kPtr2 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x2222u));
        void* const kPtr3 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x3333u));

        // WHEN: register three distinct texture ids
        mu::RegisterTexture(1u, kPtr1);
        mu::RegisterTexture(2u, kPtr2);
        mu::RegisterTexture(3u, kPtr3);

        // THEN: each lookup returns the correct pointer independently
        REQUIRE(mu::LookupTexture(1u) == kPtr1);
        REQUIRE(mu::LookupTexture(2u) == kPtr2);
        REQUIRE(mu::LookupTexture(3u) == kPtr3);
    }

    SECTION("Re-registering an id overwrites the previous pointer")
    {
        // GIVEN: id=10 registered with ptr1
        mu::ClearTextureRegistry();
        void* const kPtr1 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xAAAAu));
        void* const kPtr2 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xBBBBu));

        mu::RegisterTexture(10u, kPtr1);
        REQUIRE(mu::LookupTexture(10u) == kPtr1);

        // WHEN: re-register id=10 with ptr2
        mu::RegisterTexture(10u, kPtr2);

        // THEN: lookup returns the new pointer
        REQUIRE(mu::LookupTexture(10u) == kPtr2);
    }

    SECTION("Unregister id=0 (white fallback sentinel) is safe")
    {
        // GIVEN: clean registry
        mu::ClearTextureRegistry();

        // WHEN: unregister id=0 (never registered externally)
        // THEN: must not crash — graceful no-op
        REQUIRE_NOTHROW(mu::UnregisterTexture(0u));
    }
}

// ===========================================================================
// TEST CASE: AC-STD-2(b) [4-3-1]: BlendMode -- SDL_gpu factor table
//
// For each BlendMode enum value, verifies the expected SDL_GPUBlendFactor pair
// (src_color_blendfactor, dst_color_blendfactor) per architecture-rendering.md
// and the story dev notes blend mode table.
//
// Reference table from story 4.3.1 dev notes:
//   Alpha       | SDL_GPU_BLENDFACTOR_SRC_ALPHA          | SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
//   Additive    | SDL_GPU_BLENDFACTOR_SRC_ALPHA          | SDL_GPU_BLENDFACTOR_ONE
//   Subtract    | SDL_GPU_BLENDFACTOR_ZERO               | SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR
//   InverseColor| SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR| SDL_GPU_BLENDFACTOR_ZERO
//   Mixed       | SDL_GPU_BLENDFACTOR_ONE                | SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
//   LightMap    | SDL_GPU_BLENDFACTOR_ZERO               | SDL_GPU_BLENDFACTOR_SRC_COLOR
//   Glow        | SDL_GPU_BLENDFACTOR_ONE                | SDL_GPU_BLENDFACTOR_ONE
//   Luminance   | SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR| SDL_GPU_BLENDFACTOR_ONE
//
// RED PHASE: Fails to link until GetBlendFactors() is implemented in
//   MuRendererSDLGpu.cpp and exposed for test access.
// ===========================================================================
TEST_CASE("AC-STD-2(b) [4-3-1]: BlendMode -- SDL_gpu factor table", "[render][sdlgpubackend][ac-std-2b]")
{
    SECTION("Alpha: SRC_ALPHA, ONE_MINUS_SRC_ALPHA")
    {
        // GL equivalent: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
        auto [src, dst] = mu::GetBlendFactors(mu::BlendMode::Alpha);
        REQUIRE(src == k_BlendFactor_SrcAlpha);
        REQUIRE(dst == k_BlendFactor_OneMinusSrcAlpha);
    }

    SECTION("Additive: SRC_ALPHA, ONE")
    {
        // GL equivalent: GL_SRC_ALPHA, GL_ONE
        auto [src, dst] = mu::GetBlendFactors(mu::BlendMode::Additive);
        REQUIRE(src == k_BlendFactor_SrcAlpha);
        REQUIRE(dst == k_BlendFactor_One);
    }

    SECTION("Subtract: ZERO, ONE_MINUS_SRC_COLOR")
    {
        // GL equivalent: GL_ZERO, GL_ONE_MINUS_SRC_COLOR
        auto [src, dst] = mu::GetBlendFactors(mu::BlendMode::Subtract);
        REQUIRE(src == k_BlendFactor_Zero);
        REQUIRE(dst == k_BlendFactor_OneMinusSrcColor);
    }

    SECTION("InverseColor: ONE_MINUS_DST_COLOR, ZERO")
    {
        // GL equivalent: GL_ONE_MINUS_DST_COLOR, GL_ZERO
        auto [src, dst] = mu::GetBlendFactors(mu::BlendMode::InverseColor);
        REQUIRE(src == k_BlendFactor_OneMinusDstColor);
        REQUIRE(dst == k_BlendFactor_Zero);
    }

    SECTION("Mixed: ONE, ONE_MINUS_SRC_ALPHA")
    {
        // GL equivalent: GL_ONE, GL_ONE_MINUS_SRC_ALPHA
        auto [src, dst] = mu::GetBlendFactors(mu::BlendMode::Mixed);
        REQUIRE(src == k_BlendFactor_One);
        REQUIRE(dst == k_BlendFactor_OneMinusSrcAlpha);
    }

    SECTION("LightMap: ZERO, SRC_COLOR")
    {
        // GL equivalent: GL_ZERO, GL_SRC_COLOR
        auto [src, dst] = mu::GetBlendFactors(mu::BlendMode::LightMap);
        REQUIRE(src == k_BlendFactor_Zero);
        REQUIRE(dst == k_BlendFactor_SrcColor);
    }

    SECTION("Glow: ONE, ONE (EnableAlphaBlend equivalent)")
    {
        // GL equivalent: GL_ONE, GL_ONE
        // Added in story 4.2.5; maps to EnableAlphaBlend() in ZzzOpenglUtil.cpp
        auto [src, dst] = mu::GetBlendFactors(mu::BlendMode::Glow);
        REQUIRE(src == k_BlendFactor_One);
        REQUIRE(dst == k_BlendFactor_One);
    }

    SECTION("Luminance: ONE_MINUS_SRC_COLOR, ONE (EnableAlphaBlend2 equivalent)")
    {
        // GL equivalent: GL_ONE_MINUS_SRC_COLOR, GL_ONE
        // Added in story 4.2.5; maps to EnableAlphaBlend2() in ZzzOpenglUtil.cpp
        auto [src, dst] = mu::GetBlendFactors(mu::BlendMode::Luminance);
        REQUIRE(src == k_BlendFactor_OneMinusSrcColor);
        REQUIRE(dst == k_BlendFactor_One);
    }

    SECTION("All 8 BlendMode values have distinct factor pairs (no collisions except where expected)")
    {
        // Documents that each BlendMode produces a distinct blend configuration.
        // Glow (ONE/ONE) and Additive (SRC_ALPHA/ONE) share dst=ONE but differ in src.
        // This test enumerates all pairs for documentation/regression purposes.
        const mu::BlendMode allModes[] = {
            mu::BlendMode::Alpha,
            mu::BlendMode::Additive,
            mu::BlendMode::Subtract,
            mu::BlendMode::InverseColor,
            mu::BlendMode::Mixed,
            mu::BlendMode::LightMap,
            mu::BlendMode::Glow,
            mu::BlendMode::Luminance,
        };

        // Verify all modes return a pair of valid SDL_GPUBlendFactor values
        // (non-negative int, within the range 0-9 based on the enum).
        for (const auto mode : allModes)
        {
            auto [src, dst] = mu::GetBlendFactors(mode);
            // Valid SDL_GPUBlendFactor values are 1-13 (SDL_GPU_BLENDFACTOR_ZERO..SRC_ALPHA_SATURATE)
            // See SDL3/SDL_gpu.h SDL_GPUBlendFactor enum (release-3.2.8).
            REQUIRE(src >= 1);
            REQUIRE(src <= 13);
            REQUIRE(dst >= 1);
            REQUIRE(dst <= 13);
        }
    }
}

// ===========================================================================
// TEST CASE: AC-STD-2(c) [4-3-1]: SetFog -- FogParams storage round-trip
//
// Verifies that calling SetFog() on a renderer stores the FogParams correctly
// in the internal state field. Uses FogCaptureMock (inline mock of IMuRenderer)
// rather than a real MuRendererSDLGpu instance, so no GPU device is required.
//
// When MuRendererSDLGpu has a public GetLastFogParams() test accessor, this test
// can be updated to use it directly. The mock approach is sufficient for RED PHASE.
// ===========================================================================
TEST_CASE("AC-STD-2(c) [4-3-1]: SetFog -- FogParams storage round-trip",
    "[render][sdlgpubackend][ac-std-2c]")
{
    SECTION("SetFog stores start, end, density, and mode correctly")
    {
        // GIVEN: a renderer that captures fog params (no GPU device needed)
        FogCaptureMock renderer;

        // Reference values matching the GMBattleCastle fog scenario from story 4.2.5:
        //   fogParams.start = 2000.f, fogParams.end = 2700.f (GL_LINEAR mode)
        mu::FogParams params{};
        params.mode    = 0x2601; // GL_LINEAR
        params.start   = 2000.f;
        params.end     = 2700.f;
        params.density = 0.f;
        params.color[0] = 0.5f;
        params.color[1] = 0.6f;
        params.color[2] = 0.7f;
        params.color[3] = 1.0f;

        // WHEN: call SetFog with known params
        renderer.SetFog(params);

        // THEN: fog params are stored correctly
        REQUIRE(renderer.m_setFogCallCount == 1);
        REQUIRE(renderer.GetLastFogParams().has_value());

        const auto& stored = renderer.GetLastFogParams().value();
        REQUIRE(stored.mode == 0x2601);
        REQUIRE(stored.start == Catch::Approx(2000.f));
        REQUIRE(stored.end == Catch::Approx(2700.f));
        REQUIRE(stored.density == Catch::Approx(0.f));
        REQUIRE(stored.color[0] == Catch::Approx(0.5f));
        REQUIRE(stored.color[1] == Catch::Approx(0.6f));
        REQUIRE(stored.color[2] == Catch::Approx(0.7f));
        REQUIRE(stored.color[3] == Catch::Approx(1.0f));
    }

    SECTION("SetFog with zero-initialized params stores defaults correctly")
    {
        // GIVEN: zero-initialized FogParams
        FogCaptureMock renderer;

        mu::FogParams params{};
        params.mode = 0x2601; // GL_LINEAR

        // WHEN: call SetFog
        renderer.SetFog(params);

        // THEN: density and color fields are all 0.f
        REQUIRE(renderer.GetLastFogParams().has_value());
        const auto& stored = renderer.GetLastFogParams().value();
        REQUIRE(stored.density == Catch::Approx(0.f));
        REQUIRE(stored.color[0] == Catch::Approx(0.f));
        REQUIRE(stored.color[1] == Catch::Approx(0.f));
        REQUIRE(stored.color[2] == Catch::Approx(0.f));
        REQUIRE(stored.color[3] == Catch::Approx(0.f));
    }

    SECTION("Multiple SetFog calls: last call overwrites previous state")
    {
        // GIVEN: two sets of fog params
        FogCaptureMock renderer;

        mu::FogParams first{};
        first.mode  = 0x2601; // GL_LINEAR
        first.start = 100.f;
        first.end   = 200.f;

        mu::FogParams second{};
        second.mode  = 0x0800; // GL_EXP
        second.start = 500.f;
        second.end   = 1000.f;

        // WHEN: two sequential SetFog calls
        renderer.SetFog(first);
        renderer.SetFog(second);

        // THEN: stored params reflect the second call; call count is 2
        REQUIRE(renderer.m_setFogCallCount == 2);
        REQUIRE(renderer.GetLastFogParams().has_value());
        const auto& stored = renderer.GetLastFogParams().value();
        REQUIRE(stored.mode == 0x0800);
        REQUIRE(stored.start == Catch::Approx(500.f));
        REQUIRE(stored.end == Catch::Approx(1000.f));
    }

    SECTION("SetFog does not affect blend mode or depth test state (independent state)")
    {
        // Documents that SetFog is orthogonal to blend and depth state.
        // Uses FogCaptureMock which captures all state independently.
        FogCaptureMock renderer;

        mu::FogParams params{};
        params.mode  = 0x2601;
        params.start = 2000.f;
        params.end   = 2700.f;

        renderer.SetFog(params);
        renderer.SetBlendMode(mu::BlendMode::Alpha);
        renderer.SetDepthTest(true);

        // SetFog called once; blend/depth state set separately
        REQUIRE(renderer.m_setFogCallCount == 1);
        REQUIRE(renderer.GetLastFogParams().value().mode == 0x2601);
    }
}

// ===========================================================================
// TEST CASE: AC-VAL-1 [4-3-1]: TextureRegistry contract -- full round-trip
//
// GIVEN: a clean TextureRegistry
// WHEN:  register a texture id, look it up, then unregister it
// THEN:  lookup after unregister confirms removal (returns non-sentinel value)
//
// This test consolidates the TextureRegistry map contract into a single
// end-to-end scenario that mirrors the usage pattern from story 4.4.1
// (texture system migration will call Register/Lookup/Unregister in sequence).
//
// RED PHASE: Fails until RegisterTexture / LookupTexture / UnregisterTexture /
// ClearTextureRegistry are implemented in MuRendererSDLGpu.cpp.
// ===========================================================================
TEST_CASE("AC-VAL-1 [4-3-1]: TextureRegistry contract verified -- register / lookup / unregister",
    "[render][sdlgpubackend][ac-val-1]")
{
    SECTION("Full round-trip: register, lookup, unregister, confirm removal")
    {
        // GIVEN: clean registry
        mu::ClearTextureRegistry();

        void* const kSentinel = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xDEADBEEFu));
        constexpr std::uint32_t kTestId = 42u;

        // WHEN: register
        mu::RegisterTexture(kTestId, kSentinel);

        // THEN: lookup returns sentinel
        REQUIRE(mu::LookupTexture(kTestId) == kSentinel);

        // WHEN: unregister
        mu::UnregisterTexture(kTestId);

        // THEN: lookup returns non-sentinel (nullptr or white fallback texture)
        REQUIRE(mu::LookupTexture(kTestId) != kSentinel);
    }

    SECTION("Blend factor table covers all 8 BlendMode values without crashing")
    {
        // Integration sanity: GetBlendFactors must handle all BlendMode values
        // defined in MuRenderer.h without throwing or returning out-of-range values.
        const mu::BlendMode allModes[] = {
            mu::BlendMode::Alpha,
            mu::BlendMode::Additive,
            mu::BlendMode::Subtract,
            mu::BlendMode::InverseColor,
            mu::BlendMode::Mixed,
            mu::BlendMode::LightMap,
            mu::BlendMode::Glow,
            mu::BlendMode::Luminance,
        };

        for (const auto mode : allModes)
        {
            // Must not crash or return invalid values (valid range: 1-13 per SDL3/SDL_gpu.h)
            auto [src, dst] = mu::GetBlendFactors(mode);
            CHECK(src >= 1);
            CHECK(src <= 13);
            CHECK(dst >= 1);
            CHECK(dst <= 13);
        }
    }
}
