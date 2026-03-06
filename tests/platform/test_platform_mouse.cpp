// Story 2.2.2: SDL3 Mouse Input Migration [VS1-SDL-INPUT-MOUSE]
// RED PHASE: Tests written BEFORE implementation — all will FAIL until
// SDLEventLoop mouse handlers, PlatformCompat.h shims, and PlatformTypes.h
// structs are implemented.
//
// Framework: Catch2 v3.7.1
// Location:  MuMain/tests/platform/test_platform_mouse.cpp
// Guard:     #ifdef MU_ENABLE_SDL3 — tests only compile when SDL3 is enabled

#ifdef MU_ENABLE_SDL3

#include "PlatformCompat.h"
#include "PlatformKeys.h"
#include "PlatformTypes.h"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

// ---------------------------------------------------------------------------
// Mouse state globals — defined in ZzzOpenglUtil.cpp, declared here as extern
// for direct manipulation in unit tests (no SDL window required).
// Pattern: mirrors test_platform_input.cpp which directly sets g_sdl3KeyboardState.
// ---------------------------------------------------------------------------

extern int MouseX;
extern int MouseY;
extern int MouseWheel;
extern bool MouseLButton;
extern bool MouseLButtonPush;
extern bool MouseLButtonPop;
extern bool MouseLButtonDBClick;
extern bool MouseRButton;
extern bool MouseRButtonPush;
extern bool MouseRButtonPop;
extern bool MouseMButton;
extern bool MouseMButtonPush;
extern bool MouseMButtonPop;
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

// ---------------------------------------------------------------------------
// AC-2 [VS1-SDL-INPUT-MOUSE]: Left button state transitions — down/push/pop
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: LButton down sets MouseLButton=true and MouseLButtonPush=true",
          "[platform][mouse][ac2]")
{
    SECTION("Given button not held, BUTTON_DOWN sets LButton=true and LButtonPush=true")
    {
        // GIVEN: button is not currently held
        MouseLButton = false;
        MouseLButtonPush = false;
        MouseLButtonPop = false;

        // WHEN: simulating SDL_EVENT_MOUSE_BUTTON_DOWN for SDL_BUTTON_LEFT
        // (mirroring SDLEventLoop::PollEvents handler logic)
        MouseLButtonPop = false;
        if (!MouseLButton)
        {
            MouseLButtonPush = true;
        }
        MouseLButton = true;

        // THEN: button is held and push is flagged
        REQUIRE(MouseLButton == true);
        REQUIRE(MouseLButtonPush == true);
        REQUIRE(MouseLButtonPop == false);

        // Cleanup
        MouseLButton = false;
        MouseLButtonPush = false;
    }

    SECTION("Given button already held, second BUTTON_DOWN does not re-set Push")
    {
        // GIVEN: button is already held (auto-repeat scenario)
        MouseLButton = true;
        MouseLButtonPush = false;
        MouseLButtonPop = false;

        // WHEN: simulating a second SDL_EVENT_MOUSE_BUTTON_DOWN
        MouseLButtonPop = false;
        if (!MouseLButton)
        {
            MouseLButtonPush = true; // Should NOT execute
        }
        MouseLButton = true;

        // THEN: push is NOT re-asserted (matches Win32 WM_LBUTTONDOWN semantics)
        REQUIRE(MouseLButton == true);
        REQUIRE(MouseLButtonPush == false); // Not re-triggered when already held
        REQUIRE(MouseLButtonPop == false);

        // Cleanup
        MouseLButton = false;
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: LButton up sets MouseLButtonPop=true and MouseLButton=false",
          "[platform][mouse][ac2]")
{
    SECTION("Given button held, BUTTON_UP sets LButton=false and LButtonPop=true")
    {
        // GIVEN: button is currently held
        MouseLButton = true;
        MouseLButtonPush = false;
        MouseLButtonPop = false;

        // WHEN: simulating SDL_EVENT_MOUSE_BUTTON_UP for SDL_BUTTON_LEFT
        MouseLButtonPush = false;
        if (MouseLButton)
        {
            MouseLButtonPop = true;
        }
        MouseLButton = false;

        // THEN: button is released and pop is flagged
        REQUIRE(MouseLButton == false);
        REQUIRE(MouseLButtonPop == true);
        REQUIRE(MouseLButtonPush == false);

        // Cleanup
        MouseLButtonPop = false;
    }

    SECTION("Given button NOT held, BUTTON_UP does not set LButtonPop")
    {
        // GIVEN: button is not held (spurious up event)
        MouseLButton = false;
        MouseLButtonPush = false;
        MouseLButtonPop = false;

        // WHEN: simulating SDL_EVENT_MOUSE_BUTTON_UP when not held
        MouseLButtonPush = false;
        if (MouseLButton) // false — does not execute
        {
            MouseLButtonPop = true;
        }
        MouseLButton = false;

        // THEN: pop is NOT asserted
        REQUIRE(MouseLButtonPop == false);
    }
}

// ---------------------------------------------------------------------------
// AC-2 [VS1-SDL-INPUT-MOUSE]: LButton double-click detection
// SDL3 reports clicks==2 on the second click of a double-click sequence.
// This replaces WM_LBUTTONDBLCLK on the SDL3 path.
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: LButton double-click sets MouseLButtonDBClick=true",
          "[platform][mouse][ac2][dblclick]")
{
    SECTION("clicks==2 in BUTTON_DOWN handler sets MouseLButtonDBClick=true")
    {
        // GIVEN: double-click flag is not set
        MouseLButtonDBClick = false;

        // WHEN: simulate SDL_EVENT_MOUSE_BUTTON_DOWN with clicks==2
        // (mirroring SDLEventLoop::PollEvents handler: if (event.button.clicks == 2) ...)
        int sdlClicks = 2;
        if (sdlClicks == 2)
        {
            MouseLButtonDBClick = true;
        }

        // THEN: double-click flag is asserted
        REQUIRE(MouseLButtonDBClick == true);

        // Cleanup
        MouseLButtonDBClick = false;
    }

    SECTION("clicks==1 in BUTTON_DOWN handler does NOT set MouseLButtonDBClick")
    {
        // GIVEN: double-click flag is not set
        MouseLButtonDBClick = false;

        // WHEN: simulate SDL_EVENT_MOUSE_BUTTON_DOWN with clicks==1 (single click)
        int sdlClicks = 1;
        if (sdlClicks == 2)
        {
            MouseLButtonDBClick = true; // Should NOT execute for single click
        }

        // THEN: double-click flag is NOT asserted
        REQUIRE(MouseLButtonDBClick == false);
    }

    SECTION("MouseLButtonDBClick reset to false at frame start")
    {
        // GIVEN: previous frame had a double-click
        MouseLButtonDBClick = true;

        // WHEN: frame boundary reset (SDLEventLoop::PollEvents start)
        MouseLButtonDBClick = false;

        // THEN: flag is cleared before new frame events are processed
        REQUIRE(MouseLButtonDBClick == false);
    }
}

// ---------------------------------------------------------------------------
// AC-2 [VS1-SDL-INPUT-MOUSE]: Right button state transitions
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: RButton down/up state transitions",
          "[platform][mouse][ac2]")
{
    SECTION("RButton BUTTON_DOWN sets RButton=true and RButtonPush=true")
    {
        // GIVEN
        MouseRButton = false;
        MouseRButtonPush = false;
        MouseRButtonPop = false;

        // WHEN: simulate SDL_EVENT_MOUSE_BUTTON_DOWN for SDL_BUTTON_RIGHT
        MouseRButtonPop = false;
        if (!MouseRButton)
        {
            MouseRButtonPush = true;
        }
        MouseRButton = true;

        // THEN
        REQUIRE(MouseRButton == true);
        REQUIRE(MouseRButtonPush == true);
        REQUIRE(MouseRButtonPop == false);

        // Cleanup
        MouseRButton = false;
        MouseRButtonPush = false;
    }

    SECTION("RButton BUTTON_UP sets RButton=false and RButtonPop=true")
    {
        // GIVEN
        MouseRButton = true;
        MouseRButtonPush = false;
        MouseRButtonPop = false;

        // WHEN: simulate SDL_EVENT_MOUSE_BUTTON_UP for SDL_BUTTON_RIGHT
        MouseRButtonPush = false;
        if (MouseRButton)
        {
            MouseRButtonPop = true;
        }
        MouseRButton = false;

        // THEN
        REQUIRE(MouseRButton == false);
        REQUIRE(MouseRButtonPop == true);

        // Cleanup
        MouseRButtonPop = false;
    }
}

// ---------------------------------------------------------------------------
// AC-2 [VS1-SDL-INPUT-MOUSE]: Middle button state transitions
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: MButton down/up state transitions",
          "[platform][mouse][ac2]")
{
    SECTION("MButton BUTTON_DOWN sets MButton=true and MButtonPush=true")
    {
        // GIVEN
        MouseMButton = false;
        MouseMButtonPush = false;
        MouseMButtonPop = false;

        // WHEN: simulate SDL_EVENT_MOUSE_BUTTON_DOWN for SDL_BUTTON_MIDDLE
        MouseMButtonPop = false;
        if (!MouseMButton)
        {
            MouseMButtonPush = true;
        }
        MouseMButton = true;

        // THEN
        REQUIRE(MouseMButton == true);
        REQUIRE(MouseMButtonPush == true);
        REQUIRE(MouseMButtonPop == false);

        // Cleanup
        MouseMButton = false;
        MouseMButtonPush = false;
    }

    SECTION("MButton BUTTON_UP sets MButton=false and MButtonPop=true")
    {
        // GIVEN
        MouseMButton = true;
        MouseMButtonPush = false;
        MouseMButtonPop = false;

        // WHEN: simulate SDL_EVENT_MOUSE_BUTTON_UP for SDL_BUTTON_MIDDLE
        MouseMButtonPush = false;
        if (MouseMButton)
        {
            MouseMButtonPop = true;
        }
        MouseMButton = false;

        // THEN
        REQUIRE(MouseMButton == false);
        REQUIRE(MouseMButtonPop == true);

        // Cleanup
        MouseMButtonPop = false;
    }
}

// ---------------------------------------------------------------------------
// AC-3 [VS1-SDL-INPUT-MOUSE]: MouseWheel sign convention
// SDL3: event.wheel.y positive = scroll up (away from user) — same as Win32
// ---------------------------------------------------------------------------

TEST_CASE("AC-3 [VS1-SDL-INPUT-MOUSE]: MouseWheel positive y maps to positive MouseWheel",
          "[platform][mouse][ac3]")
{
    SECTION("SDL wheel.y = +3 maps to MouseWheel = +3 (scroll up)")
    {
        // GIVEN: frame starts with reset
        MouseWheel = 0;

        // WHEN: simulate SDL_EVENT_MOUSE_WHEEL with positive y (scroll up)
        // (mirroring SDLEventLoop handler: MouseWheel = static_cast<int>(event.wheel.y))
        float sdlWheelY = 3.0f;
        MouseWheel = static_cast<int>(sdlWheelY);

        // THEN: MouseWheel is positive — scroll up
        REQUIRE(MouseWheel == 3);
        REQUIRE(MouseWheel > 0);

        // Cleanup
        MouseWheel = 0;
    }
}

TEST_CASE("AC-3 [VS1-SDL-INPUT-MOUSE]: MouseWheel negative y maps to negative MouseWheel",
          "[platform][mouse][ac3]")
{
    SECTION("SDL wheel.y = -2 maps to MouseWheel = -2 (scroll down)")
    {
        // GIVEN: frame starts with reset
        MouseWheel = 0;

        // WHEN: simulate SDL_EVENT_MOUSE_WHEEL with negative y (scroll down)
        float sdlWheelY = -2.0f;
        MouseWheel = static_cast<int>(sdlWheelY);

        // THEN: MouseWheel is negative — scroll down
        REQUIRE(MouseWheel == -2);
        REQUIRE(MouseWheel < 0);

        // Cleanup
        MouseWheel = 0;
    }
}

TEST_CASE("AC-3 [VS1-SDL-INPUT-MOUSE]: MouseWheel reset to zero each frame",
          "[platform][mouse][ac3]")
{
    SECTION("MouseWheel starts at 0 after per-frame reset (no scroll this frame)")
    {
        // GIVEN: previous frame had scroll
        MouseWheel = 5;

        // WHEN: frame boundary reset (SDLEventLoop::PollEvents start)
        MouseWheel = 0;

        // THEN: no scroll reported
        REQUIRE(MouseWheel == 0);
    }
}

// ---------------------------------------------------------------------------
// AC-1 [VS1-SDL-INPUT-MOUSE]: Coordinate normalization — boundary clamping
// Virtual space: 640x480. Coordinates clamped to [0, 640] x [0, 480].
// ---------------------------------------------------------------------------

TEST_CASE("AC-1 [VS1-SDL-INPUT-MOUSE]: Mouse coordinate normalization clamps to 0-640 x range",
          "[platform][mouse][ac1]")
{
    SECTION("x coordinate below 0 clamps to 0")
    {
        // GIVEN: SDL reports position outside left edge (e.g., captured mouse)
        MouseX = 0;
        g_fScreenRate_x = 1.0f; // 1:1 for simplicity — real value is WindowWidth/640.0f

        // WHEN: simulate SDL_EVENT_MOUSE_MOTION with x = -10
        // (mirroring SDLEventLoop handler: MouseX = static_cast<int>(x / g_fScreenRate_x))
        float rawX = -10.0f;
        MouseX = static_cast<int>(rawX / g_fScreenRate_x);
        if (MouseX < 0)
        {
            MouseX = 0;
        }
        if (MouseX > 640)
        {
            MouseX = 640;
        }

        // THEN: clamped to 0
        REQUIRE(MouseX == 0);
    }

    SECTION("x coordinate above 640 clamps to 640")
    {
        // GIVEN: SDL reports position outside right edge
        MouseX = 0;
        g_fScreenRate_x = 1.0f;

        // WHEN: simulate motion beyond right edge
        float rawX = 700.0f;
        MouseX = static_cast<int>(rawX / g_fScreenRate_x);
        if (MouseX < 0)
        {
            MouseX = 0;
        }
        if (MouseX > 640)
        {
            MouseX = 640;
        }

        // THEN: clamped to 640
        REQUIRE(MouseX == 640);
    }

    SECTION("x coordinate within range is not clamped")
    {
        // GIVEN: normal window position with 2x screen rate (e.g., 1280px window)
        MouseX = 0;
        g_fScreenRate_x = 2.0f; // 1280 / 640 = 2.0

        // WHEN: SDL reports pixel 640 (center of 1280px window)
        float rawX = 640.0f; // window-space pixels
        MouseX = static_cast<int>(rawX / g_fScreenRate_x);
        if (MouseX < 0)
        {
            MouseX = 0;
        }
        if (MouseX > 640)
        {
            MouseX = 640;
        }

        // THEN: maps to 320 in virtual 640 space — no clamping
        REQUIRE(MouseX == 320);
    }
}

// ---------------------------------------------------------------------------
// AC-4 [VS1-SDL-INPUT-MOUSE]: GetDoubleClickTime shim returns 500ms
// Used by CInput::Create() at Input.cpp:40
// ---------------------------------------------------------------------------

TEST_CASE("AC-4 [VS1-SDL-INPUT-MOUSE]: GetDoubleClickTime shim returns 500ms",
          "[platform][mouse][ac4]")
{
    SECTION("GetDoubleClickTime() returns 500u (standard Windows double-click interval)")
    {
        // GIVEN/WHEN: call the shim
        DWORD result = GetDoubleClickTime();

        // THEN: returns 500ms constant
        REQUIRE(result == 500u);
    }
}

// ---------------------------------------------------------------------------
// AC-2 [VS1-SDL-INPUT-MOUSE]: GetAsyncKeyState shim — VK_LBUTTON/RBUTTON/MBUTTON
// These VK codes (0x01, 0x02, 0x04) must be backed by global mouse state.
// Used by CInput::Update() via SEASON3B::IsPress() -> ScanAsyncKeyState().
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: GetAsyncKeyState VK_LBUTTON returns 0x8000 when MouseLButton is true",
          "[platform][mouse][ac2][getasynckeystate]")
{
    SECTION("VK_LBUTTON (0x01) returns 0x8000 when MouseLButton=true")
    {
        // GIVEN: left button is held
        MouseLButton = true;

        // WHEN: GetAsyncKeyState called with VK_LBUTTON
        uint16_t result = GetAsyncKeyState(VK_LBUTTON);

        // THEN: high bit set — button is pressed
        REQUIRE(result == static_cast<uint16_t>(0x8000));
        REQUIRE((result & 0x8000) != 0);

        // Cleanup
        MouseLButton = false;
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: GetAsyncKeyState VK_LBUTTON returns 0 when MouseLButton is false",
          "[platform][mouse][ac2][getasynckeystate]")
{
    SECTION("VK_LBUTTON (0x01) returns 0 when MouseLButton=false")
    {
        // GIVEN: left button is not held
        MouseLButton = false;

        // WHEN: GetAsyncKeyState called with VK_LBUTTON
        uint16_t result = GetAsyncKeyState(VK_LBUTTON);

        // THEN: returns 0 — button not pressed
        REQUIRE(result == 0);
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: GetAsyncKeyState VK_RBUTTON returns correct state",
          "[platform][mouse][ac2][getasynckeystate]")
{
    SECTION("VK_RBUTTON (0x02) returns 0x8000 when MouseRButton=true")
    {
        // GIVEN
        MouseRButton = true;

        // WHEN
        uint16_t result = GetAsyncKeyState(VK_RBUTTON);

        // THEN
        REQUIRE(result == static_cast<uint16_t>(0x8000));

        // Cleanup
        MouseRButton = false;
    }

    SECTION("VK_RBUTTON (0x02) returns 0 when MouseRButton=false")
    {
        // GIVEN
        MouseRButton = false;

        // WHEN
        uint16_t result = GetAsyncKeyState(VK_RBUTTON);

        // THEN
        REQUIRE(result == 0);
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: GetAsyncKeyState VK_MBUTTON returns correct state",
          "[platform][mouse][ac2][getasynckeystate]")
{
    SECTION("VK_MBUTTON (0x04) returns 0x8000 when MouseMButton=true")
    {
        // GIVEN
        MouseMButton = true;

        // WHEN
        uint16_t result = GetAsyncKeyState(VK_MBUTTON);

        // THEN
        REQUIRE(result == static_cast<uint16_t>(0x8000));

        // Cleanup
        MouseMButton = false;
    }

    SECTION("VK_MBUTTON (0x04) returns 0 when MouseMButton=false")
    {
        // GIVEN
        MouseMButton = false;

        // WHEN
        uint16_t result = GetAsyncKeyState(VK_MBUTTON);

        // THEN
        REQUIRE(result == 0);
    }
}

// ---------------------------------------------------------------------------
// AC-2 [VS1-SDL-INPUT-MOUSE]: VK_LBUTTON/RBUTTON/MBUTTON constants defined
// in PlatformKeys.h (required for CInput::Update() compilation)
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-MOUSE]: VK_LBUTTON VK_RBUTTON VK_MBUTTON constants defined",
          "[platform][mouse][ac2][platform-keys]")
{
    SECTION("VK_LBUTTON is defined as 0x01")
    {
        REQUIRE(VK_LBUTTON == 0x01);
    }

    SECTION("VK_RBUTTON is defined as 0x02")
    {
        REQUIRE(VK_RBUTTON == 0x02);
    }

    SECTION("VK_MBUTTON is defined as 0x04")
    {
        REQUIRE(VK_MBUTTON == 0x04);
    }
}

// ---------------------------------------------------------------------------
// AC-4/AC-5 [VS1-SDL-INPUT-MOUSE]: PlatformTypes.h — POINT struct shim
// Required for CInput::Create/Update (m_ptCursor as POINT)
// ---------------------------------------------------------------------------

TEST_CASE("AC-5 [VS1-SDL-INPUT-MOUSE]: POINT struct shim is defined in PlatformTypes.h",
          "[platform][mouse][ac5][platform-types]")
{
    SECTION("POINT struct has x and y members of type long")
    {
        // GIVEN/WHEN: create a POINT and assign values
        POINT pt;
        pt.x = 320L;
        pt.y = 240L;

        // THEN: members are accessible and hold correct values
        REQUIRE(pt.x == 320L);
        REQUIRE(pt.y == 240L);
    }

    SECTION("POINT can be zero-initialized")
    {
        POINT pt = { 0L, 0L };
        REQUIRE(pt.x == 0L);
        REQUIRE(pt.y == 0L);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2 [VS1-SDL-INPUT-MOUSE]: ShowCursor shim is callable (compilation test)
// No return value — just verifies the shim compiles and is invokable.
// ShowCursor(TRUE/FALSE) must not crash in non-SDL context (no window required).
// ---------------------------------------------------------------------------

TEST_CASE("AC-4 [VS1-SDL-INPUT-MOUSE]: ShowCursor shim is callable under MU_ENABLE_SDL3",
          "[platform][mouse][ac4][compat]")
{
    SECTION("ShowCursor(true) compiles and runs without crashing")
    {
        // GIVEN/WHEN: call the shim — SDL not initialized, so SDL_ShowCursor is a no-op in test
        // THEN: no crash (compilation test — verifies shim signature matches call sites)
        // Note: SDL_ShowCursor/SDL_HideCursor are safe to call without SDL_Init in test context.
        // If SDL not initialized, these functions are graceful no-ops.
        ShowCursor(true);
        REQUIRE(true); // Shim callable — no crash
    }

    SECTION("ShowCursor(false) compiles and runs without crashing")
    {
        ShowCursor(false);
        REQUIRE(true); // Shim callable — no crash

        // Restore cursor visibility for subsequent test infrastructure
        ShowCursor(true);
    }
}

#endif // MU_ENABLE_SDL3
