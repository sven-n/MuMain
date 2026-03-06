// Story 2.2.1: SDL3 Keyboard Input Migration [VS1-SDL-INPUT-KEYBOARD]
// RED PHASE: Tests written BEFORE implementation — all will FAIL until
// PlatformCompat.h shim and MuVkToSdlScancode() are implemented.
//
// Framework: Catch2 v3.7.1
// Location:  MuMain/tests/platform/test_platform_input.cpp
// Guard:     #ifdef MU_ENABLE_SDL3 — tests only compile when SDL3 is enabled

#ifdef MU_ENABLE_SDL3

#include "PlatformCompat.h"
#include "PlatformKeys.h"
#include <SDL3/SDL_scancode.h>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

// ---------------------------------------------------------------------------
// AC-2: VK -> SDL_Scancode mapping — navigation / control keys
// Flow: VS1-SDL-INPUT-KEYBOARD
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-KEYBOARD]: MuVkToSdlScancode navigation keys", "[platform][input][ac2]")
{
    SECTION("VK_LEFT maps to SDL_SCANCODE_LEFT")
    {
        REQUIRE(MuVkToSdlScancode(VK_LEFT) == SDL_SCANCODE_LEFT);
    }
    SECTION("VK_RIGHT maps to SDL_SCANCODE_RIGHT")
    {
        REQUIRE(MuVkToSdlScancode(VK_RIGHT) == SDL_SCANCODE_RIGHT);
    }
    SECTION("VK_UP maps to SDL_SCANCODE_UP")
    {
        REQUIRE(MuVkToSdlScancode(VK_UP) == SDL_SCANCODE_UP);
    }
    SECTION("VK_DOWN maps to SDL_SCANCODE_DOWN")
    {
        REQUIRE(MuVkToSdlScancode(VK_DOWN) == SDL_SCANCODE_DOWN);
    }
    SECTION("VK_INSERT maps to SDL_SCANCODE_INSERT")
    {
        REQUIRE(MuVkToSdlScancode(VK_INSERT) == SDL_SCANCODE_INSERT);
    }
    SECTION("VK_DELETE maps to SDL_SCANCODE_DELETE")
    {
        REQUIRE(MuVkToSdlScancode(VK_DELETE) == SDL_SCANCODE_DELETE);
    }
    SECTION("VK_HOME maps to SDL_SCANCODE_HOME")
    {
        REQUIRE(MuVkToSdlScancode(VK_HOME) == SDL_SCANCODE_HOME);
    }
    SECTION("VK_END maps to SDL_SCANCODE_END")
    {
        REQUIRE(MuVkToSdlScancode(VK_END) == SDL_SCANCODE_END);
    }
    SECTION("VK_PRIOR maps to SDL_SCANCODE_PAGEUP")
    {
        REQUIRE(MuVkToSdlScancode(VK_PRIOR) == SDL_SCANCODE_PAGEUP);
    }
    SECTION("VK_NEXT maps to SDL_SCANCODE_PAGEDOWN")
    {
        REQUIRE(MuVkToSdlScancode(VK_NEXT) == SDL_SCANCODE_PAGEDOWN);
    }
    SECTION("VK_RETURN maps to SDL_SCANCODE_RETURN")
    {
        REQUIRE(MuVkToSdlScancode(VK_RETURN) == SDL_SCANCODE_RETURN);
    }
    SECTION("VK_ESCAPE maps to SDL_SCANCODE_ESCAPE")
    {
        REQUIRE(MuVkToSdlScancode(VK_ESCAPE) == SDL_SCANCODE_ESCAPE);
    }
    SECTION("VK_SPACE maps to SDL_SCANCODE_SPACE")
    {
        REQUIRE(MuVkToSdlScancode(VK_SPACE) == SDL_SCANCODE_SPACE);
    }
    SECTION("VK_TAB maps to SDL_SCANCODE_TAB")
    {
        REQUIRE(MuVkToSdlScancode(VK_TAB) == SDL_SCANCODE_TAB);
    }
    SECTION("VK_BACK maps to SDL_SCANCODE_BACKSPACE")
    {
        REQUIRE(MuVkToSdlScancode(VK_BACK) == SDL_SCANCODE_BACKSPACE);
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-KEYBOARD]: MuVkToSdlScancode modifier keys", "[platform][input][ac2]")
{
    SECTION("VK_SHIFT maps to SDL_SCANCODE_LSHIFT")
    {
        REQUIRE(MuVkToSdlScancode(VK_SHIFT) == SDL_SCANCODE_LSHIFT);
    }
    SECTION("VK_CONTROL maps to SDL_SCANCODE_LCTRL")
    {
        REQUIRE(MuVkToSdlScancode(VK_CONTROL) == SDL_SCANCODE_LCTRL);
    }
    SECTION("VK_MENU maps to SDL_SCANCODE_LALT")
    {
        REQUIRE(MuVkToSdlScancode(VK_MENU) == SDL_SCANCODE_LALT);
    }
    SECTION("VK_LCONTROL maps to SDL_SCANCODE_LCTRL (AC-2 missing constant fix)")
    {
        REQUIRE(MuVkToSdlScancode(VK_LCONTROL) == SDL_SCANCODE_LCTRL);
    }
    SECTION("VK_SNAPSHOT maps to SDL_SCANCODE_PRINTSCREEN (AC-2 missing constant fix)")
    {
        REQUIRE(MuVkToSdlScancode(VK_SNAPSHOT) == SDL_SCANCODE_PRINTSCREEN);
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-KEYBOARD]: MuVkToSdlScancode F1 through F12", "[platform][input][ac2]")
{
    SECTION("VK_F1 maps to SDL_SCANCODE_F1")
    {
        REQUIRE(MuVkToSdlScancode(VK_F1) == SDL_SCANCODE_F1);
    }
    SECTION("VK_F2 maps to SDL_SCANCODE_F2")
    {
        REQUIRE(MuVkToSdlScancode(VK_F2) == SDL_SCANCODE_F2);
    }
    SECTION("VK_F3 maps to SDL_SCANCODE_F3")
    {
        REQUIRE(MuVkToSdlScancode(VK_F3) == SDL_SCANCODE_F3);
    }
    SECTION("VK_F4 maps to SDL_SCANCODE_F4")
    {
        REQUIRE(MuVkToSdlScancode(VK_F4) == SDL_SCANCODE_F4);
    }
    SECTION("VK_F5 maps to SDL_SCANCODE_F5")
    {
        REQUIRE(MuVkToSdlScancode(VK_F5) == SDL_SCANCODE_F5);
    }
    SECTION("VK_F6 maps to SDL_SCANCODE_F6")
    {
        REQUIRE(MuVkToSdlScancode(VK_F6) == SDL_SCANCODE_F6);
    }
    SECTION("VK_F7 maps to SDL_SCANCODE_F7")
    {
        REQUIRE(MuVkToSdlScancode(VK_F7) == SDL_SCANCODE_F7);
    }
    SECTION("VK_F8 maps to SDL_SCANCODE_F8")
    {
        REQUIRE(MuVkToSdlScancode(VK_F8) == SDL_SCANCODE_F8);
    }
    SECTION("VK_F9 maps to SDL_SCANCODE_F9")
    {
        REQUIRE(MuVkToSdlScancode(VK_F9) == SDL_SCANCODE_F9);
    }
    SECTION("VK_F10 maps to SDL_SCANCODE_F10")
    {
        REQUIRE(MuVkToSdlScancode(VK_F10) == SDL_SCANCODE_F10);
    }
    SECTION("VK_F11 maps to SDL_SCANCODE_F11")
    {
        REQUIRE(MuVkToSdlScancode(VK_F11) == SDL_SCANCODE_F11);
    }
    SECTION("VK_F12 maps to SDL_SCANCODE_F12")
    {
        REQUIRE(MuVkToSdlScancode(VK_F12) == SDL_SCANCODE_F12);
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-KEYBOARD]: MuVkToSdlScancode numpad keys", "[platform][input][ac2]")
{
    SECTION("VK_NUMPAD0 maps to SDL_SCANCODE_KP_0")
    {
        REQUIRE(MuVkToSdlScancode(VK_NUMPAD0) == SDL_SCANCODE_KP_0);
    }
    SECTION("VK_NUMPAD1 maps to SDL_SCANCODE_KP_1")
    {
        REQUIRE(MuVkToSdlScancode(VK_NUMPAD1) == SDL_SCANCODE_KP_1);
    }
    SECTION("VK_NUMPAD5 maps to SDL_SCANCODE_KP_5")
    {
        REQUIRE(MuVkToSdlScancode(VK_NUMPAD5) == SDL_SCANCODE_KP_5);
    }
    SECTION("VK_NUMPAD9 maps to SDL_SCANCODE_KP_9")
    {
        REQUIRE(MuVkToSdlScancode(VK_NUMPAD9) == SDL_SCANCODE_KP_9);
    }
}

// ---------------------------------------------------------------------------
// AC-2: ASCII letter and digit range mapping (Task 5)
// VK codes 'A'-'Z' (0x41-0x5A) and '0'-'9' (0x30-0x39)
// Flow: VS1-SDL-INPUT-KEYBOARD
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-KEYBOARD]: MuVkToSdlScancode ASCII A through Z", "[platform][input][ac2]")
{
    // SDL3 guarantees SDL_SCANCODE_A through SDL_SCANCODE_Z are contiguous
    static_assert(SDL_SCANCODE_A == 4, "SDL_SCANCODE_A must be 4 — verify SDL3 release-3.2.8 headers");

    SECTION("'A' (0x41) maps to SDL_SCANCODE_A")
    {
        REQUIRE(MuVkToSdlScancode('A') == SDL_SCANCODE_A);
    }
    SECTION("'Z' (0x5A) maps to SDL_SCANCODE_Z")
    {
        REQUIRE(MuVkToSdlScancode('Z') == SDL_SCANCODE_Z);
    }
    SECTION("'Q' maps to SDL_SCANCODE_Q (WASD/QERF camera control)")
    {
        REQUIRE(MuVkToSdlScancode('Q') == SDL_SCANCODE_Q);
    }
    SECTION("'W' maps to SDL_SCANCODE_W (WASD camera control)")
    {
        REQUIRE(MuVkToSdlScancode('W') == SDL_SCANCODE_W);
    }
    SECTION("'S' maps to SDL_SCANCODE_S (WASD camera control)")
    {
        REQUIRE(MuVkToSdlScancode('S') == SDL_SCANCODE_S);
    }
    SECTION("'D' maps to SDL_SCANCODE_D (WASD camera control)")
    {
        REQUIRE(MuVkToSdlScancode('D') == SDL_SCANCODE_D);
    }
    SECTION("All A-Z map contiguously (range check)")
    {
        for (int vk = 'A'; vk <= 'Z'; ++vk)
        {
            SDL_Scancode sc = MuVkToSdlScancode(vk);
            SDL_Scancode expected = static_cast<SDL_Scancode>(SDL_SCANCODE_A + (vk - 'A'));
            REQUIRE(sc == expected);
        }
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-KEYBOARD]: MuVkToSdlScancode ASCII 0 through 9", "[platform][input][ac2]")
{
    // SDL3 guarantees SDL_SCANCODE_1 through SDL_SCANCODE_9 are contiguous; SDL_SCANCODE_0 follows
    static_assert(SDL_SCANCODE_1 == 30, "SDL_SCANCODE_1 must be 30 — verify SDL3 release-3.2.8 headers");

    SECTION("'1' (0x31) maps to SDL_SCANCODE_1 (Alt+1 skill shortcut)")
    {
        REQUIRE(MuVkToSdlScancode('1') == SDL_SCANCODE_1);
    }
    SECTION("'9' (0x39) maps to SDL_SCANCODE_9 (Alt+9 skill shortcut)")
    {
        REQUIRE(MuVkToSdlScancode('9') == SDL_SCANCODE_9);
    }
    SECTION("'0' (0x30) maps to SDL_SCANCODE_0 (Alt+0 skill shortcut)")
    {
        REQUIRE(MuVkToSdlScancode('0') == SDL_SCANCODE_0);
    }
    SECTION("Digits 1-9 map contiguously (range check)")
    {
        for (int vk = '1'; vk <= '9'; ++vk)
        {
            SDL_Scancode sc = MuVkToSdlScancode(vk);
            SDL_Scancode expected = static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (vk - '1'));
            REQUIRE(sc == expected);
        }
    }
}

// ---------------------------------------------------------------------------
// AC-2: Unmapped VK code returns SDL_SCANCODE_UNKNOWN
// Flow: VS1-SDL-INPUT-KEYBOARD
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-KEYBOARD]: MuVkToSdlScancode unmapped VK returns SDL_SCANCODE_UNKNOWN",
          "[platform][input][ac2]")
{
    SECTION("VK code 0xFF (reserved/undefined) returns SDL_SCANCODE_UNKNOWN")
    {
        REQUIRE(MuVkToSdlScancode(0xFF) == SDL_SCANCODE_UNKNOWN);
    }
    SECTION("VK code 0x00 returns SDL_SCANCODE_UNKNOWN")
    {
        REQUIRE(MuVkToSdlScancode(0x00) == SDL_SCANCODE_UNKNOWN);
    }
    SECTION("VK code 0xE0 (OEM-specific, reserved) returns SDL_SCANCODE_UNKNOWN")
    {
        REQUIRE(MuVkToSdlScancode(0xE0) == SDL_SCANCODE_UNKNOWN);
    }
}

// ---------------------------------------------------------------------------
// AC-5: macOS Cmd key (SDL_SCANCODE_LGUI/RGUI) NOT mapped to VK_CONTROL
// Flow: VS1-SDL-INPUT-KEYBOARD
// ---------------------------------------------------------------------------

TEST_CASE("AC-5 [VS1-SDL-INPUT-KEYBOARD]: macOS Cmd key NOT mapped to game controls",
          "[platform][input][ac5]")
{
    SECTION("SDL_SCANCODE_LGUI is NOT mapped from VK_CONTROL (0x11)")
    {
        // VK_CONTROL must map to SDL_SCANCODE_LCTRL, never SDL_SCANCODE_LGUI
        SDL_Scancode sc = MuVkToSdlScancode(VK_CONTROL);
        REQUIRE(sc != SDL_SCANCODE_LGUI);
        REQUIRE(sc == SDL_SCANCODE_LCTRL);
    }
    SECTION("SDL_SCANCODE_RGUI is NOT mapped from any VK code in the game control range")
    {
        // None of the game hotkey VK codes should produce SDL_SCANCODE_RGUI
        // Check all modifier VK codes
        REQUIRE(MuVkToSdlScancode(VK_SHIFT) != SDL_SCANCODE_RGUI);
        REQUIRE(MuVkToSdlScancode(VK_CONTROL) != SDL_SCANCODE_RGUI);
        REQUIRE(MuVkToSdlScancode(VK_MENU) != SDL_SCANCODE_RGUI);
        REQUIRE(MuVkToSdlScancode(VK_LCONTROL) != SDL_SCANCODE_RGUI);
    }
}

// ---------------------------------------------------------------------------
// AC-1 & AC-4: GetAsyncKeyState shim — high-byte / async state model
// Tests operate on g_sdl3KeyboardState directly (no SDL init required)
// Flow: VS1-SDL-INPUT-KEYBOARD
// ---------------------------------------------------------------------------

TEST_CASE("AC-1 [VS1-SDL-INPUT-KEYBOARD]: GetAsyncKeyState shim returns 0x8000 when key is held",
          "[platform][input][ac1]")
{
    SECTION("Set SDL_SCANCODE_A state true, GetAsyncKeyState('A') returns 0x8000")
    {
        // GIVEN: keyboard state for SDL_SCANCODE_A is set to pressed
        SDL_Scancode sc = MuVkToSdlScancode('A');
        REQUIRE(sc != SDL_SCANCODE_UNKNOWN);
        g_sdl3KeyboardState[sc] = true;

        // WHEN: shim is called with VK code 'A'
        uint16_t result = GetAsyncKeyState('A');

        // THEN: high bit (0x8000) is set — key is held
        REQUIRE(result == static_cast<uint16_t>(0x8000));

        // Cleanup
        g_sdl3KeyboardState[sc] = false;
    }
}

TEST_CASE("AC-1 [VS1-SDL-INPUT-KEYBOARD]: GetAsyncKeyState shim returns 0 when key is not held",
          "[platform][input][ac1]")
{
    SECTION("SDL_SCANCODE_A state false, GetAsyncKeyState('A') returns 0")
    {
        // GIVEN: keyboard state for SDL_SCANCODE_A is clear
        SDL_Scancode sc = MuVkToSdlScancode('A');
        REQUIRE(sc != SDL_SCANCODE_UNKNOWN);
        g_sdl3KeyboardState[sc] = false;

        // WHEN: shim is called
        uint16_t result = GetAsyncKeyState('A');

        // THEN: returns 0 — key is not held
        REQUIRE(result == 0);
    }
}

TEST_CASE("AC-1 [VS1-SDL-INPUT-KEYBOARD]: GetAsyncKeyState shim returns 0 for unmapped VK code",
          "[platform][input][ac1]")
{
    SECTION("Unmapped VK 0xFF returns 0 (not 0x8000)")
    {
        // GIVEN/WHEN: VK code with no SDL scancode mapping
        uint16_t result = GetAsyncKeyState(0xFF);

        // THEN: returns 0 — no key state available
        REQUIRE(result == 0);
    }
}

// ---------------------------------------------------------------------------
// AC-4: Key repeat behavior — HIBYTE correctness
// Win32 async state model: HIBYTE(GetAsyncKeyState(vk)) & 0x80 == true when held
// Flow: VS1-SDL-INPUT-KEYBOARD
// ---------------------------------------------------------------------------

TEST_CASE("AC-4 [VS1-SDL-INPUT-KEYBOARD]: HIBYTE of 0x8000 equals 128 (0x80)",
          "[platform][input][ac4]")
{
    SECTION("HIBYTE(static_cast<uint16_t>(0x8000)) == 128")
    {
        // This validates the core Win32 HIBYTE pattern used by 104 call sites
        uint8_t hibyte = HIBYTE(static_cast<uint16_t>(0x8000));
        REQUIRE(hibyte == 128);
        REQUIRE((hibyte & 0x80) != 0);
    }
    SECTION("HIBYTE pattern: held key satisfies both == 128 and & 0x80 checks")
    {
        // Both patterns used in codebase:
        //   HIBYTE(GetAsyncKeyState(vk)) == 128   (e.g., ZzzInterface.cpp)
        //   HIBYTE(GetAsyncKeyState(vk)) & 0x80   (e.g., NewUICommon.cpp loop)
        SDL_Scancode sc = MuVkToSdlScancode(VK_ESCAPE);
        REQUIRE(sc != SDL_SCANCODE_UNKNOWN);
        g_sdl3KeyboardState[sc] = true;

        uint16_t raw = GetAsyncKeyState(VK_ESCAPE);
        uint8_t hi = HIBYTE(raw);

        REQUIRE(hi == 128);
        REQUIRE((hi & 0x80) != 0);

        g_sdl3KeyboardState[sc] = false;
    }
    SECTION("HIBYTE of 0 equals 0 (key not held)")
    {
        REQUIRE(HIBYTE(static_cast<uint16_t>(0)) == 0);
        REQUIRE((HIBYTE(static_cast<uint16_t>(0)) & 0x80) == 0);
    }
}

TEST_CASE("AC-4 [VS1-SDL-INPUT-KEYBOARD]: GetAsyncKeyState direct 0x8000 check (Winmain.cpp pattern)",
          "[platform][input][ac4]")
{
    SECTION("GetAsyncKeyState(VK_F12) & 0x8000 is true when key held (Winmain.cpp:912 pattern)")
    {
        // Winmain.cpp uses: GetAsyncKeyState(VK_F12) & 0x8000 (no HIBYTE)
        SDL_Scancode sc = MuVkToSdlScancode(VK_F12);
        REQUIRE(sc != SDL_SCANCODE_UNKNOWN);
        g_sdl3KeyboardState[sc] = true;

        REQUIRE((GetAsyncKeyState(VK_F12) & 0x8000) != 0);

        g_sdl3KeyboardState[sc] = false;
    }
    SECTION("GetAsyncKeyState(VK_F12) & 0x8000 is false when key not held")
    {
        SDL_Scancode sc = MuVkToSdlScancode(VK_F12);
        REQUIRE(sc != SDL_SCANCODE_UNKNOWN);
        g_sdl3KeyboardState[sc] = false;

        REQUIRE((GetAsyncKeyState(VK_F12) & 0x8000) == 0);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2: VK_LCONTROL and VK_SNAPSHOT — previously missing constants
// These VK codes are used in ZzzCharacter.cpp and SceneManager.cpp
// Flow: VS1-SDL-INPUT-KEYBOARD
// ---------------------------------------------------------------------------

TEST_CASE("AC-STD-2 [VS1-SDL-INPUT-KEYBOARD]: VK_LCONTROL and VK_SNAPSHOT defined in PlatformKeys.h",
          "[platform][input][ac-std2]")
{
    SECTION("VK_LCONTROL is defined as 0xA2")
    {
        REQUIRE(VK_LCONTROL == 0xA2);
    }
    SECTION("VK_SNAPSHOT is defined as 0x2C")
    {
        REQUIRE(VK_SNAPSHOT == 0x2C);
    }
    SECTION("VK_LCONTROL GetAsyncKeyState shim works (ZzzCharacter.cpp usage)")
    {
        SDL_Scancode sc = MuVkToSdlScancode(VK_LCONTROL);
        REQUIRE(sc == SDL_SCANCODE_LCTRL);
        g_sdl3KeyboardState[sc] = true;
        REQUIRE(GetAsyncKeyState(VK_LCONTROL) == static_cast<uint16_t>(0x8000));
        g_sdl3KeyboardState[sc] = false;
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2: g_sdl3KeyboardState array boundary — size 512 (SDL_NUM_SCANCODES)
// Flow: VS1-SDL-INPUT-KEYBOARD
// ---------------------------------------------------------------------------

TEST_CASE("AC-STD-2 [VS1-SDL-INPUT-KEYBOARD]: g_sdl3KeyboardState array size is 512",
          "[platform][input][ac-std2]")
{
    SECTION("Array can be indexed at 0 and 511 without bounds violation")
    {
        // GIVEN: array exists with 512 entries
        // Set and clear boundary entries
        g_sdl3KeyboardState[0] = true;
        REQUIRE(g_sdl3KeyboardState[0] == true);
        g_sdl3KeyboardState[0] = false;

        g_sdl3KeyboardState[511] = true;
        REQUIRE(g_sdl3KeyboardState[511] == true);
        g_sdl3KeyboardState[511] = false;
    }
}

#endif // MU_ENABLE_SDL3
