// Story 7.9.8: Adopt SDL_ttf for Cross-Platform Font Rendering
// Flow Code: VS1-RENDER-FONT-SDLTTF
//
// RED PHASE: Tests document the interface contracts for:
//   (a) AC-3: CUIRenderTextSDLTtf color storage — PackColorDWORD helper (pure logic, no GPU)
//   (b) AC-2: GPU text engine lifecycle — SKIP (requires SDL3 GPU device)
//   (c) AC-4: Factory type selection — SKIP (requires Win32 HDC type at runtime)
//   (d) AC-5: Text rendering parity — SKIP (requires SDL3 GPU device)
//   (e) AC-6: Deferred rendering compatibility — SKIP (requires SDL3 GPU device)
//   (f) AC-STD-NFR-1: Performance targets — SKIP (requires GPU device + runtime measurement)
//
// AC-1 (SDL_ttf FetchContent integration) is verified separately via CMake script:
//   tests/render/test_ac1_sdl_ttf_cmake_7_9_8.cmake
//
// IMPORTANT: No SDL3 GPU device, no Win32, no OpenGL in this test TU.
// All tests compile and run on macOS/Linux without Win32 APIs.
//
// AC Mapping:
//   AC-1         -> test_ac1_sdl_ttf_cmake_7_9_8.cmake (cmake script, file-presence check)
//   AC-2         -> TEST_CASE("AC-2 [7-9-8]: ...") [SKIP — requires SDL3 GPU device]
//   AC-3         -> TEST_CASE("AC-3 [7-9-8]: SetTextColor packs RGBA into ABGR DWORD ...")
//   AC-3         -> TEST_CASE("AC-3 [7-9-8]: SetBgColor packs RGBA into ABGR DWORD ...")
//   AC-3         -> TEST_CASE("AC-3 [7-9-8]: transparent color packs to zero alpha DWORD ...")
//   AC-4         -> TEST_CASE("AC-4 [7-9-8]: ...") [SKIP — factory requires HDC/Win32 at runtime]
//   AC-5         -> TEST_CASE("AC-5 [7-9-8]: ...") [SKIP — requires SDL3 GPU device]
//   AC-6         -> TEST_CASE("AC-6 [7-9-8]: ...") [SKIP — requires SDL3 GPU device]
//   AC-STD-NFR-1 -> TEST_CASE("AC-STD-NFR-1 [7-9-8]: ...") [SKIP — requires GPU + timing]
//
// Run with: ctest --test-dir MuMain/build -R sdl_ttf_7_9_8
// Note: SKIP'd tests compile cleanly and report as skipped (not failed) in CTest.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

// ---------------------------------------------------------------------------
// Forward declaration: mu::sdlttf::PackColorDWORD
//
// Packs individual RGBA channels into the ABGR DWORD format used internally
// by CUIRenderTextSDLTtf for SetTextColor/SetBgColor storage.
//
// Byte layout (lowest address first):
//   bits [7:0]   = Red   (r)
//   bits [15:8]  = Green (g)
//   bits [23:16] = Blue  (b)
//   bits [31:24] = Alpha (a)
//
// This follows the same ABGR convention used by the renderer for vertex colors,
// so the color DWORD can be passed directly to SDL_GPU without byte-swapping.
//
// RED PHASE: Implemented in UIControls.cpp once CUIRenderTextSDLTtf is written.
//            Tests fail to link until PackColorDWORD is defined there.
// ---------------------------------------------------------------------------
namespace mu
{
namespace sdlttf
{

[[nodiscard]] std::uint32_t PackColorDWORD(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);

} // namespace sdlttf
} // namespace mu

// ---------------------------------------------------------------------------
// AC-3: Color packing — SetTextColor / SetBgColor storage contract
//
// CUIRenderTextSDLTtf stores text and background colors as packed ABGR DWORDs.
// These tests verify the packing math that GetTextColor() / GetBgColor() expose.
// Tests are pure arithmetic — no GPU device or Win32 API required.
// ---------------------------------------------------------------------------

TEST_CASE("AC-3 [7-9-8]: SetTextColor packs opaque red into ABGR DWORD", "[render][sdlttf][color][7-9-8]")
{
    // GIVEN: opaque red — R=255, G=0, B=0, A=255
    // WHEN:  color is packed via PackColorDWORD
    // THEN:  result is 0xFF0000FF (A=0xFF, B=0x00, G=0x00, R=0xFF in ABGR layout)
    const std::uint32_t packed = mu::sdlttf::PackColorDWORD(255, 0, 0, 255);
    REQUIRE(packed == 0xFF0000FFu);
}

TEST_CASE("AC-3 [7-9-8]: SetTextColor packs opaque white into ABGR DWORD", "[render][sdlttf][color][7-9-8]")
{
    // GIVEN: opaque white — R=255, G=255, B=255, A=255
    // THEN: all four bytes are 0xFF → result is 0xFFFFFFFF
    const std::uint32_t packed = mu::sdlttf::PackColorDWORD(255, 255, 255, 255);
    REQUIRE(packed == 0xFFFFFFFFu);
}

TEST_CASE("AC-3 [7-9-8]: SetBgColor packs transparent black into ABGR DWORD", "[render][sdlttf][color][7-9-8]")
{
    // GIVEN: transparent black — R=0, G=0, B=0, A=0
    // THEN: result is 0x00000000 (fully transparent)
    const std::uint32_t packed = mu::sdlttf::PackColorDWORD(0, 0, 0, 0);
    REQUIRE(packed == 0x00000000u);
}

TEST_CASE("AC-3 [7-9-8]: SetTextColor channel isolation — blue channel only", "[render][sdlttf][color][7-9-8]")
{
    // GIVEN: opaque blue — R=0, G=0, B=255, A=255
    // THEN: ABGR layout puts B=0xFF in bits [23:16] → 0xFF_FF_00_00
    const std::uint32_t packed = mu::sdlttf::PackColorDWORD(0, 0, 255, 255);
    REQUIRE(packed == 0xFFFF0000u);
}

TEST_CASE("AC-3 [7-9-8]: SetTextColor channel isolation — green channel only", "[render][sdlttf][color][7-9-8]")
{
    // GIVEN: opaque green — R=0, G=255, B=0, A=255
    // THEN: ABGR layout puts G=0xFF in bits [15:8] → 0xFF_00_FF_00
    const std::uint32_t packed = mu::sdlttf::PackColorDWORD(0, 255, 0, 255);
    REQUIRE(packed == 0xFF00FF00u);
}

TEST_CASE("AC-3 [7-9-8]: SetTextColor alpha controls transparency", "[render][sdlttf][color][7-9-8]")
{
    // GIVEN: semi-transparent white — R=255, G=255, B=255, A=128
    // THEN: high byte (alpha) is 0x80, all color channels are 0xFF → 0x80FFFFFF
    const std::uint32_t packed = mu::sdlttf::PackColorDWORD(255, 255, 255, 128);
    REQUIRE(packed == 0x80FFFFFFu);
}

// ---------------------------------------------------------------------------
// AC-2: GPU text engine lifecycle
//
// TTF_CreateGPUTextEngine requires a live SDL_GPUDevice — cannot run headless.
// SKIP so the test compiles clean on macOS/Linux CI without a GPU device.
// Once SDL3 GPU is available at test time, remove SKIP and add device setup.
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [7-9-8]: GPU text engine creates and destroys without crash",
          "[render][sdlttf][gpu][7-9-8]")
{
    SKIP("AC-2 requires SDL3 GPU device — run manually with a real GPU");
    // GIVEN: a valid SDL_GPUDevice (created by SDL_CreateGPUDevice)
    // WHEN:  TTF_CreateGPUTextEngine(device) is called during renderer init
    // THEN:  engine pointer is non-null
    //        Engine destruction via TTF_DestroyGPUTextEngine does not crash
    //        Engine is stored as MuRendererSDLGpu private state, accessible via
    //        mu::GetTextEngine() (or equivalent accessor)
}

// ---------------------------------------------------------------------------
// AC-4: Factory type selection
//
// CUIRenderText::Create(renderType, hDC) selects CUIRenderTextSDLTtf on SDL3
// builds via the MU_ENABLE_SDL3 path. Testing this in isolation requires a
// valid HDC (Win32 type) which is not available in headless test environments.
// SKIP so the test compiles clean without Win32 dependency.
// ---------------------------------------------------------------------------

TEST_CASE("AC-4 [7-9-8]: factory selects CUIRenderTextSDLTtf on SDL3 builds",
          "[render][sdlttf][factory][7-9-8]")
{
    SKIP("AC-4 factory test requires Win32 HDC — verified by integration in renderer init");
    // GIVEN: MU_ENABLE_SDL3 is defined (SDL3 build)
    // WHEN:  CUIRenderText::Create(RENDER_TEXT_SDL_TTF, NULL) is called
    // THEN:  m_pRenderText is a CUIRenderTextSDLTtf instance
    //        GetRenderTextType() returns RENDER_TEXT_SDL_TTF
    //        Falls back to CUIRenderTextOriginal when MU_ENABLE_SDL3 is not defined
}

// ---------------------------------------------------------------------------
// AC-5: Text rendering parity
//
// Visual parity (button labels, login text, chat text visible and positioned)
// requires a running SDL3 GPU renderer. Not testable in headless CTest runs.
// ---------------------------------------------------------------------------

TEST_CASE("AC-5 [7-9-8]: button labels render visible and correctly positioned",
          "[render][sdlttf][parity][7-9-8]")
{
    SKIP("AC-5 requires running SDL3 GPU renderer — verify manually at 640x480 and 1024x768");
    // GIVEN: renderer initialized with CUIRenderTextSDLTtf active
    // WHEN:  g_pRenderText->RenderText(x, y, L"Button Label", ...) is called
    // THEN:  text quad is submitted to the deferred draw buffer
    //        text is visible in the rendered frame (not blank/white pixels)
    //        position matches expected screen coordinates at both test resolutions
}

// ---------------------------------------------------------------------------
// AC-6: Deferred rendering compatibility
//
// TTF_GetGPUTextDrawData must be called inside the render pass (after copy pass).
// Testing this requires a full frame cycle with a GPU device.
// ---------------------------------------------------------------------------

TEST_CASE("AC-6 [7-9-8]: text atlas updates execute in copy pass before render pass",
          "[render][sdlttf][deferred][7-9-8]")
{
    SKIP("AC-6 requires SDL3 GPU render loop — verify via manual frame debugging");
    // GIVEN: deferred draw buffer with queued RenderText commands
    // WHEN:  EndFrame() flushes the deferred buffer
    // THEN:  TTF atlas texture uploads occur in the copy pass
    //        TTF_GetGPUTextDrawData atlas textures are bound during the render pass
    //        No rendering artifacts (text not missing, no ordering issues with 2D quads)
}

// ---------------------------------------------------------------------------
// AC-STD-NFR-1: Performance
//
// < 0.5ms per frame for ~50 text elements requires GPU timing infrastructure.
// SKIP in headless CI; measure manually or via GPU profiler.
// ---------------------------------------------------------------------------

TEST_CASE("AC-STD-NFR-1 [7-9-8]: font atlas caching keeps per-frame cost under 0.5ms",
          "[render][sdlttf][perf][7-9-8]")
{
    SKIP("AC-STD-NFR-1 requires GPU timing — measure with SDL_GPUFence or RenderDoc");
    // GIVEN: renderer initialized, font atlas warmed up for common glyphs
    // WHEN:  50 RenderText calls are made in one frame (typical UI load)
    // THEN:  total GPU time for text submission < 0.5ms
    //        glyph atlas is reused across frames (no per-character texture uploads)
    //        TTF_GetGPUTextDrawData is called once per TTF_Text, not per character
}
