// doctest unit tests for the scripted key/click injector: the key-name
// table and the frame sequence a `hotkey` or `click-ui` walks through.
// The mouse globals it writes are plain variables, so no window is needed.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "synthetic"

#include "doctest.h"

#include "Core/Input/KeyState.h"
#include "Core/Input/SyntheticInput.h"
#include "Core/Platform/WinCompat.h"

extern int MouseX;
extern int MouseY;
extern float g_fWindowMouseX;
extern float g_fWindowMouseY;
extern bool MouseLButton;
extern bool MouseLButtonPush;
extern bool MouseLButtonPop;
extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

using namespace Core::Input::Synthetic;

namespace
{
struct ResetInjector
{
    ResetInjector()
    {
        Reset();
    }
    ~ResetInjector()
    {
        Reset();
    }
};
} // namespace

TEST_CASE("Synthetic input names the keys the shim translates [core][synthetic-input]")
{
    CHECK(VirtualKeyFromName("esc") == VK_ESCAPE);
    CHECK(VirtualKeyFromName("Escape") == VK_ESCAPE);
    CHECK(VirtualKeyFromName("HOME") == VK_HOME);
    CHECK(VirtualKeyFromName("enter") == VK_RETURN);
    CHECK(VirtualKeyFromName("f1") == VK_F1);
    CHECK(VirtualKeyFromName("F12") == VK_F12);
    CHECK(VirtualKeyFromName("printscreen") == VK_SNAPSHOT);
    CHECK(VirtualKeyFromName("i") == 'I');
    CHECK(VirtualKeyFromName("I") == 'I');
    CHECK(VirtualKeyFromName("7") == '7');

    CHECK_FALSE(VirtualKeyFromName("bogus").has_value());
    CHECK_FALSE(VirtualKeyFromName("").has_value());
    CHECK_FALSE(VirtualKeyFromName("?").has_value());
    CHECK_FALSE(VirtualKeyFromName("f13").has_value());

    CHECK(MouseButtonFromName("left") == MouseButton::Left);
    CHECK(MouseButtonFromName("Right") == MouseButton::Right);
    CHECK_FALSE(MouseButtonFromName("middle").has_value());
}

TEST_CASE("A hotkey is down for exactly one frame [core][synthetic-input]")
{
    ResetInjector guard;
    CHECK(IsIdle());
    CHECK(PressKey(VK_HOME));
    CHECK_FALSE(IsIdle());

    // Scheduled, not yet applied: the frame has not begun.
    CHECK_FALSE(IsKeyHeld(VK_HOME));
    CHECK_FALSE(Core::Input::IsKeyDown(VK_HOME));

    BeginFrame();
    CHECK(IsKeyHeld(VK_HOME));
    CHECK(Core::Input::IsKeyDown(VK_HOME));
    CHECK_FALSE(IsKeyHeld(VK_END));

    BeginFrame();
    CHECK_FALSE(IsKeyHeld(VK_HOME));
    CHECK_FALSE(IsIdle());

    BeginFrame();
    CHECK(IsIdle());
}

TEST_CASE("A second injection is refused while one is in flight [core][synthetic-input]")
{
    ResetInjector guard;
    CHECK(PressKey(VK_ESCAPE));
    CHECK_FALSE(PressKey('I'));
    CHECK_FALSE(Click(1.0f, 1.0f, MouseButton::Left));

    BeginFrame();
    BeginFrame();
    BeginFrame();
    CHECK(IsIdle());
    CHECK(Click(1.0f, 1.0f, MouseButton::Left));
}

TEST_CASE("A click walks press, hold, release through the mouse globals [core][synthetic-input]")
{
    ResetInjector guard;
    WindowWidth = 1280;
    WindowHeight = 960;
    MouseLButton = false;
    MouseLButtonPush = false;
    MouseLButtonPop = false;

    CHECK(Click(1000.0f, 725.0f, MouseButton::Left));

    BeginFrame();
    CHECK(g_fWindowMouseX == doctest::Approx(1000.0f));
    CHECK(g_fWindowMouseY == doctest::Approx(725.0f));
    // The overlay space is 640x480 stretched over the window.
    CHECK(MouseX == 500);
    CHECK(MouseY == 362);
    CHECK(MouseLButton);
    CHECK(MouseLButtonPush);
    CHECK_FALSE(MouseLButtonPop);
    CHECK(IsKeyHeld(VK_LBUTTON));
    CHECK(Core::Input::IsKeyDown(VK_LBUTTON));

    // The scene clears the one-shot push at the end of the frame.
    MouseLButtonPush = false;
    BeginFrame();
    CHECK(MouseLButton);
    CHECK(IsKeyHeld(VK_LBUTTON));

    BeginFrame();
    CHECK_FALSE(MouseLButton);
    CHECK(MouseLButtonPop);
    CHECK_FALSE(IsKeyHeld(VK_LBUTTON));
    CHECK_FALSE(IsIdle());

    BeginFrame();
    CHECK(IsIdle());
}
