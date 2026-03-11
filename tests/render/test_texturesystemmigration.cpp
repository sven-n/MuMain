// Story 4.4.1: Texture System Migration (CGlobalBitmap -> SDL_gpu)
// RED PHASE: Tests compile but FAIL until:
//   Task 2: MapGLFilterToSDL, MapGLWrapToSDL, PadRGBToRGBA helpers implemented
//            in anonymous namespace of GlobalBitmap.cpp and forward-declared here.
//   Task 7.3: mu::RegisterTexture / LookupTexture / UnregisterTexture / ClearTextureRegistry
//             already implemented (story 4.3.1) — these tests become GREEN immediately.
//   Task 7.4/7.5: MapGLFilterToSDL, MapGLWrapToSDL, PadRGBToRGBA must be implemented.
//
// Tests compile and run on macOS/Linux — no GPU device, no Win32, no OpenGL required.
// Pure logic tests: registry map operations, enum mapping, pixel buffer manipulation.
//
// Flow Code: VS1-RENDER-TEXTURE-MIGRATE
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstddef>
#include <vector>

// ---------------------------------------------------------------------------
// Forward declarations of testable symbols from MuRendererSDLGpu.cpp.
// TextureRegistry free functions are in mu:: namespace (story 4.3.1 impl).
// ---------------------------------------------------------------------------
namespace mu
{

[[nodiscard]] void* LookupTexture(std::uint32_t id);
void RegisterTexture(std::uint32_t id, void* pTex);
void UnregisterTexture(std::uint32_t id);
void ClearTextureRegistry();

} // namespace mu

// ---------------------------------------------------------------------------
// Forward declarations of testable helpers from GlobalBitmap.cpp.
// These are defined at global scope in GlobalBitmap.cpp (not in an anonymous namespace)
// so that the test TU can forward-declare and link them directly.
// They will be defined once Tasks 2.1 and 2.2 in story 4.4.1 are implemented.
//
// GL filter constants (matching OpenGL header values, used as plain ints here
// to avoid pulling in OpenGL headers in the test TU):
//   GL_NEAREST        = 0x2600
//   GL_LINEAR         = 0x2601
//   GL_CLAMP_TO_EDGE  = 0x812F
//   GL_REPEAT         = 0x2901
//
// SDL_GPUFilter enum values (from SDL3/SDL_gpu.h release-3.2.8):
//   SDL_GPU_FILTER_NEAREST = 0
//   SDL_GPU_FILTER_LINEAR  = 1
//
// SDL_GPUSamplerAddressMode enum values:
//   SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT          = 0
//   SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE   = 2
// ---------------------------------------------------------------------------

// SDL_GPUFilter and SDL_GPUSamplerAddressMode are represented as int to
// avoid SDL3 headers in the test TU. The forward-declared functions return
// int so tests can compare against expected integer enum values.
//
// In GlobalBitmap.cpp the actual return types are SDL_GPUFilter and
// SDL_GPUSamplerAddressMode — the test functions are declared with int
// return types matching the underlying enum's integral type.
//
// Implementation note: These functions are forward-declared with C linkage
// to avoid name-mangling issues when linking against the anonymous-namespace
// implementations. Alternatively, they may be exposed via a thin helper
// header. The forward declarations below assume the implementations are
// accessible with these exact signatures after Task 2.1 in story 4.4.1.

// MapGLFilterToSDL: maps GL_NEAREST -> SDL_GPU_FILTER_NEAREST (0),
//                          GL_LINEAR -> SDL_GPU_FILTER_LINEAR (1)
// Declared with int return to avoid SDL3 type dependency in the test TU.
[[nodiscard]] int MapGLFilterToSDL(unsigned int uiFilter);

// MapGLWrapToSDL: maps GL_CLAMP_TO_EDGE -> SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE (2),
//                        GL_REPEAT       -> SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT (0)
// Declared with int return to avoid SDL3 type dependency in the test TU.
[[nodiscard]] int MapGLWrapToSDL(unsigned int uiWrapMode);

// PadRGBToRGBA: pads an RGB pixel buffer (3 bytes/pixel) to RGBA8 (4 bytes/pixel)
// by appending alpha=255 for every pixel. Returns a new vector of size width*height*4.
[[nodiscard]] std::vector<std::uint8_t> PadRGBToRGBA(const std::uint8_t* rgbData, int width, int height);

// ===========================================================================
// AC-STD-2(a): TextureRegistry roundtrip
//
// GIVEN: A clean texture registry (ClearTextureRegistry called)
// WHEN:  RegisterTexture(42, mockPtr) is called
// THEN:  LookupTexture(42) == mockPtr
// WHEN:  UnregisterTexture(42) is called
// THEN:  LookupTexture(42) != mockPtr (returns white fallback or nullptr)
//
// Tests the mu:: namespace registry functions already implemented in story 4.3.1
// (MuRendererSDLGpu.cpp lines ~280-307). This test should be GREEN immediately.
// ===========================================================================
TEST_CASE("AC-STD-2(a): TextureRegistry -- RegisterTexture/LookupTexture/UnregisterTexture roundtrip",
          "[render][texture-migration][ac-std-2a]")
{
    // GIVEN: clean registry
    mu::ClearTextureRegistry();

    void* const kMockPtr = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xDEADBEEFu));
    constexpr std::uint32_t kTestId = 42u;

    SECTION("RegisterTexture stores pointer; LookupTexture returns it")
    {
        // WHEN: register
        mu::RegisterTexture(kTestId, kMockPtr);

        // THEN: lookup returns the mock pointer
        void* result = mu::LookupTexture(kTestId);
        REQUIRE(result == kMockPtr);

        mu::ClearTextureRegistry();
    }

    SECTION("UnregisterTexture removes entry; LookupTexture returns non-sentinel after removal")
    {
        // GIVEN: pointer registered
        mu::RegisterTexture(kTestId, kMockPtr);
        REQUIRE(mu::LookupTexture(kTestId) == kMockPtr);

        // WHEN: unregister
        mu::UnregisterTexture(kTestId);

        // THEN: lookup returns non-sentinel (white fallback or nullptr — not the original pointer)
        void* result = mu::LookupTexture(kTestId);
        CHECK(result != kMockPtr);

        mu::ClearTextureRegistry();
    }

    SECTION("ClearTextureRegistry removes all entries")
    {
        void* const kPtr1 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x1111u));
        void* const kPtr2 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x2222u));
        void* const kPtr3 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x3333u));

        // GIVEN: multiple entries registered
        mu::RegisterTexture(1u, kPtr1);
        mu::RegisterTexture(2u, kPtr2);
        mu::RegisterTexture(3u, kPtr3);

        REQUIRE(mu::LookupTexture(1u) == kPtr1);
        REQUIRE(mu::LookupTexture(2u) == kPtr2);
        REQUIRE(mu::LookupTexture(3u) == kPtr3);

        // WHEN: clear all
        mu::ClearTextureRegistry();

        // THEN: none of the original pointers are returned
        CHECK(mu::LookupTexture(1u) != kPtr1);
        CHECK(mu::LookupTexture(2u) != kPtr2);
        CHECK(mu::LookupTexture(3u) != kPtr3);
    }

    SECTION("Overwriting an existing entry replaces the stored pointer")
    {
        void* const kPtr1 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xAAAAu));
        void* const kPtr2 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xBBBBu));

        // GIVEN: slot 10 holds kPtr1
        mu::RegisterTexture(10u, kPtr1);
        REQUIRE(mu::LookupTexture(10u) == kPtr1);

        // WHEN: overwrite with kPtr2
        mu::RegisterTexture(10u, kPtr2);

        // THEN: lookup returns the new pointer
        REQUIRE(mu::LookupTexture(10u) == kPtr2);

        mu::ClearTextureRegistry();
    }

    SECTION("UnregisterTexture on id 0 does not crash (guard for white-texture slot)")
    {
        // Unregistering the white texture fallback slot (id=0) must not crash or
        // corrupt the registry (implementation may no-op or allow re-registration).
        REQUIRE_NOTHROW(mu::UnregisterTexture(0u));
    }
}

// ===========================================================================
// AC-STD-2(b): GL/SDL format mapping — filter and wrap mode conversion
//
// GIVEN: GL filter/wrap enum integer values (GL_NEAREST, GL_LINEAR, etc.)
// WHEN:  MapGLFilterToSDL / MapGLWrapToSDL are called
// THEN:  Correct SDL_GPUFilter / SDL_GPUSamplerAddressMode integer values returned
//
// RED PHASE: FAILS until MapGLFilterToSDL and MapGLWrapToSDL are implemented
//            in GlobalBitmap.cpp anonymous namespace (Task 2.1 in story 4.4.1).
//
// GL enum reference (from gl.h):
//   GL_NEAREST        = 0x2600 = 9728
//   GL_LINEAR         = 0x2601 = 9729
//   GL_CLAMP_TO_EDGE  = 0x812F = 33071
//   GL_REPEAT         = 0x2901 = 10497
//
// SDL_GPUFilter reference (from SDL3/SDL_gpu.h release-3.2.8):
//   SDL_GPU_FILTER_NEAREST = 0
//   SDL_GPU_FILTER_LINEAR  = 1
//
// SDL_GPUSamplerAddressMode reference:
//   SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT          = 0
//   SDL_GPU_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1
//   SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE   = 2
// ===========================================================================
TEST_CASE("AC-STD-2(b): UploadTexture -- GL/SDL format mapping", "[render][texture-migration][ac-std-2b]")
{
    // GL enum constants (defined locally to avoid OpenGL headers in the test TU)
    constexpr unsigned int kGLNearest = 0x2600u;     // GL_NEAREST
    constexpr unsigned int kGLLinear = 0x2601u;      // GL_LINEAR
    constexpr unsigned int kGLClampToEdge = 0x812Fu; // GL_CLAMP_TO_EDGE
    constexpr unsigned int kGLRepeat = 0x2901u;      // GL_REPEAT

    // Expected SDL_GPUFilter enum values
    constexpr int kSDLFilterNearest = 0; // SDL_GPU_FILTER_NEAREST
    constexpr int kSDLFilterLinear = 1;  // SDL_GPU_FILTER_LINEAR

    // Expected SDL_GPUSamplerAddressMode enum values
    constexpr int kSDLAddressModeRepeat = 0;      // SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT
    constexpr int kSDLAddressModeClampToEdge = 2; // SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE

    SECTION("GL_NEAREST maps to SDL_GPU_FILTER_NEAREST")
    {
        // GIVEN: GL_NEAREST (0x2600)
        // WHEN: MapGLFilterToSDL called
        int result = MapGLFilterToSDL(kGLNearest);

        // THEN: returns SDL_GPU_FILTER_NEAREST (0)
        REQUIRE(result == kSDLFilterNearest);
    }

    SECTION("GL_LINEAR maps to SDL_GPU_FILTER_LINEAR")
    {
        // GIVEN: GL_LINEAR (0x2601)
        // WHEN: MapGLFilterToSDL called
        int result = MapGLFilterToSDL(kGLLinear);

        // THEN: returns SDL_GPU_FILTER_LINEAR (1)
        REQUIRE(result == kSDLFilterLinear);
    }

    SECTION("Unknown filter value defaults to SDL_GPU_FILTER_NEAREST")
    {
        // GIVEN: an unrecognized GL filter value
        // WHEN: MapGLFilterToSDL called with unknown value
        int result = MapGLFilterToSDL(0xFFFFu);

        // THEN: returns SDL_GPU_FILTER_NEAREST as safe default (not LINEAR)
        REQUIRE(result == kSDLFilterNearest);
    }

    SECTION("GL_CLAMP_TO_EDGE maps to SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE")
    {
        // GIVEN: GL_CLAMP_TO_EDGE (0x812F)
        // WHEN: MapGLWrapToSDL called
        int result = MapGLWrapToSDL(kGLClampToEdge);

        // THEN: returns SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE (2)
        REQUIRE(result == kSDLAddressModeClampToEdge);
    }

    SECTION("GL_REPEAT maps to SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT")
    {
        // GIVEN: GL_REPEAT (0x2901)
        // WHEN: MapGLWrapToSDL called
        int result = MapGLWrapToSDL(kGLRepeat);

        // THEN: returns SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT (0)
        REQUIRE(result == kSDLAddressModeRepeat);
    }

    SECTION("Unknown wrap mode defaults to SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE")
    {
        // GIVEN: an unrecognized GL wrap mode value
        // WHEN: MapGLWrapToSDL called with unknown value
        int result = MapGLWrapToSDL(0xFFFFu);

        // THEN: returns SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE as safe default
        REQUIRE(result == kSDLAddressModeClampToEdge);
    }
}

// ===========================================================================
// AC-STD-2(c): RGB pixel padding to RGBA8
//
// GIVEN: A synthetic 2x2 RGB buffer (3 bytes/pixel, 12 bytes total)
// WHEN:  PadRGBToRGBA(buf, 2, 2) is called
// THEN:  Result vector has size 16 (4 bytes/pixel, 4 pixels)
//        Alpha channel is 255 for every pixel
//        RGB channels of each pixel are preserved in correct order
//
// RED PHASE: FAILS until PadRGBToRGBA is implemented in GlobalBitmap.cpp
//            anonymous namespace (Task 2.1 in story 4.4.1).
//
// This is pure CPU logic — no GPU device required.
// ===========================================================================
TEST_CASE("AC-STD-2(c): UploadTexture -- RGB pixel padding to RGBA8", "[render][texture-migration][ac-std-2c]")
{
    // Synthetic 2x2 RGB pixel buffer:
    //   Pixel 0: R=255, G=0,   B=0   (red)
    //   Pixel 1: R=0,   G=255, B=0   (green)
    //   Pixel 2: R=0,   G=0,   B=255 (blue)
    //   Pixel 3: R=128, G=128, B=128 (grey)
    //
    // Layout: [R0,G0,B0, R1,G1,B1, R2,G2,B2, R3,G3,B3] = 12 bytes
    const std::uint8_t kRGBBuf[] = {
        255, 0,   0,   // pixel 0: red
        0,   255, 0,   // pixel 1: green
        0,   0,   255, // pixel 2: blue
        128, 128, 128  // pixel 3: grey
    };
    constexpr int kWidth = 2;
    constexpr int kHeight = 2;

    SECTION("Output vector has correct size (width * height * 4 bytes)")
    {
        // GIVEN: 2x2 RGB buffer
        // WHEN: PadRGBToRGBA called
        std::vector<std::uint8_t> result = PadRGBToRGBA(kRGBBuf, kWidth, kHeight);

        // THEN: output size is 4 pixels * 4 bytes/pixel = 16 bytes
        REQUIRE(result.size() == 16u);
    }

    SECTION("Pixel 0 (red): R,G,B channels preserved; A=255")
    {
        std::vector<std::uint8_t> result = PadRGBToRGBA(kRGBBuf, kWidth, kHeight);
        REQUIRE(result.size() == 16u);

        // Pixel 0 at byte offset 0: R=255, G=0, B=0, A=255
        CHECK(result[0] == 255); // R
        CHECK(result[1] == 0);   // G
        CHECK(result[2] == 0);   // B
        CHECK(result[3] == 255); // A (padded)
    }

    SECTION("Pixel 1 (green): R,G,B channels preserved; A=255")
    {
        std::vector<std::uint8_t> result = PadRGBToRGBA(kRGBBuf, kWidth, kHeight);
        REQUIRE(result.size() == 16u);

        // Pixel 1 at byte offset 4: R=0, G=255, B=0, A=255
        CHECK(result[4] == 0);   // R
        CHECK(result[5] == 255); // G
        CHECK(result[6] == 0);   // B
        CHECK(result[7] == 255); // A (padded)
    }

    SECTION("Pixel 2 (blue): R,G,B channels preserved; A=255")
    {
        std::vector<std::uint8_t> result = PadRGBToRGBA(kRGBBuf, kWidth, kHeight);
        REQUIRE(result.size() == 16u);

        // Pixel 2 at byte offset 8: R=0, G=0, B=255, A=255
        CHECK(result[8] == 0);    // R
        CHECK(result[9] == 0);    // G
        CHECK(result[10] == 255); // B
        CHECK(result[11] == 255); // A (padded)
    }

    SECTION("Pixel 3 (grey): R,G,B channels preserved; A=255")
    {
        std::vector<std::uint8_t> result = PadRGBToRGBA(kRGBBuf, kWidth, kHeight);
        REQUIRE(result.size() == 16u);

        // Pixel 3 at byte offset 12: R=128, G=128, B=128, A=255
        CHECK(result[12] == 128); // R
        CHECK(result[13] == 128); // G
        CHECK(result[14] == 128); // B
        CHECK(result[15] == 255); // A (padded)
    }

    SECTION("All alpha channels are 255 (no partial transparency)")
    {
        std::vector<std::uint8_t> result = PadRGBToRGBA(kRGBBuf, kWidth, kHeight);
        REQUIRE(result.size() == 16u);

        // Check alpha byte (index 3, 7, 11, 15) for all 4 pixels
        for (int i = 0; i < kWidth * kHeight; ++i)
        {
            CHECK(result[static_cast<std::size_t>(i) * 4u + 3u] == 255);
        }
    }

    SECTION("1x1 single pixel RGB padded to RGBA correctly")
    {
        const std::uint8_t kSinglePixel[] = {64u, 128u, 192u};

        std::vector<std::uint8_t> result = PadRGBToRGBA(kSinglePixel, 1, 1);

        REQUIRE(result.size() == 4u);
        CHECK(result[0] == 64u);
        CHECK(result[1] == 128u);
        CHECK(result[2] == 192u);
        CHECK(result[3] == 255u); // A padded
    }
}
