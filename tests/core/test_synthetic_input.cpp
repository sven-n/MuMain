// doctest unit tests for the scripted key/click injector: the key-name table,
// the virtual-key to scancode mapping the injected events are built from, and
// the frame sequence a `hotkey` or `click-ui` walks through.
//
// The events themselves need a window and an initialised SDL, which a unit
// test has neither of; the injector pushes nothing in that case and still
// walks its frames, which is what is asserted here. That the events reach the
// UI is a live property, verified by driving a client through the socket.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "synthetic"

#include "doctest.h"

#include <SDL3/SDL.h>

#include "Core/Input/KeyState.h"
#include "Core/Input/SyntheticInput.h"
#include "Core/Platform/WinCompat.h"

#include <string>

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

TEST_CASE("A click walks press, hold and release across frames [core][synthetic-input]")
{
    ResetInjector guard;

    CHECK(Click(1000.0f, 725.0f, MouseButton::Left));
    // Scheduled, not yet applied: the frame has not begun.
    CHECK_FALSE(IsKeyHeld(VK_LBUTTON));

    BeginFrame();
    CHECK(IsKeyHeld(VK_LBUTTON));
    CHECK(Core::Input::IsKeyDown(VK_LBUTTON));
    CHECK_FALSE(IsKeyHeld(VK_RBUTTON));

    // Held for a second frame, so the press and its release are never delivered
    // in one pump of the event loop.
    BeginFrame();
    CHECK(IsKeyHeld(VK_LBUTTON));

    BeginFrame();
    CHECK_FALSE(IsKeyHeld(VK_LBUTTON));
    CHECK_FALSE(IsIdle());

    BeginFrame();
    CHECK(IsIdle());
}

TEST_CASE("Typed text is refused when it is empty, oversized or not text [core][synthetic-input]")
{
    ResetInjector guard;

    CHECK_FALSE(TypeText("", false));
    CHECK_FALSE(TypeText(std::string(MaxTypedTextBytes + 1, 'a'), false));
    // A newline or a tab is a key, not field content.
    CHECK_FALSE(TypeText("line\nbreak", false));
    CHECK_FALSE(TypeText("tab\there", false));
    CHECK(IsIdle());

    CHECK(TypeText(std::string(MaxTypedTextBytes, 'a'), false));
    CHECK_FALSE(IsIdle());
    // Non-ASCII is text like any other; the event carries UTF-8 unchanged.
    CHECK_FALSE(TypeText("Kalima", false));
    Reset();
    CHECK(TypeText("Gens\xc3\xb3", false));
}

TEST_CASE("Typed text takes one frame, plus two when it submits [core][synthetic-input]")
{
    ResetInjector guard;

    CHECK(TypeText("GuildName", false));
    BeginFrame();  // characters
    CHECK_FALSE(IsIdle());
    BeginFrame();
    CHECK(IsIdle());

    CHECK(TypeText("GuildName", true));
    BeginFrame();  // characters
    CHECK_FALSE(IsIdle());
    BeginFrame();  // Return down -- the field has the text by now
    CHECK(IsKeyHeld(VK_RETURN));
    BeginFrame();  // Return up
    CHECK_FALSE(IsKeyHeld(VK_RETURN));
    CHECK_FALSE(IsIdle());
    BeginFrame();
    CHECK(IsIdle());
}

TEST_CASE("Injected keys map to the scancodes the event carries [core][synthetic-input]")
{
    // The injector builds its SDL key events from this mapping; a key it does
    // not translate produces no event at all, so the two have to agree.
    CHECK(Core::Input::ScancodeForVirtualKey(VK_ESCAPE) == SDL_SCANCODE_ESCAPE);
    CHECK(Core::Input::ScancodeForVirtualKey(VK_RETURN) == SDL_SCANCODE_RETURN);
    CHECK(Core::Input::ScancodeForVirtualKey(VK_F12) == SDL_SCANCODE_F12);
    CHECK(Core::Input::ScancodeForVirtualKey('I') == SDL_SCANCODE_I);
    CHECK(Core::Input::ScancodeForVirtualKey('7') == SDL_SCANCODE_7);
    CHECK(Core::Input::ScancodeForVirtualKey('0') == SDL_SCANCODE_0);

    for (const char* name : {"esc", "enter", "tab", "space", "backspace", "home", "end", "insert", "delete",
                             "pageup", "pagedown", "up", "down", "left", "right", "printscreen", "f1", "f12",
                             "a", "z", "0", "9"})
    {
        CAPTURE(name);
        const std::optional<int> virtualKey = VirtualKeyFromName(name);
        REQUIRE(virtualKey.has_value());
        CHECK(Core::Input::ScancodeForVirtualKey(*virtualKey) != SDL_SCANCODE_UNKNOWN);
    }
}
