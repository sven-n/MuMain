#include "stdafx.h"
#include "Core/Input/SyntheticInput.h"

#include "Core/Input/KeyState.h"
#include "UI/Scaling/UITransform.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

// Mouse state the event loop fills from real SDL events (ZzzOpenglUtil.cpp,
// Winmain.cpp); a click writes the same globals so the UI cannot tell the
// difference.
extern int MouseX;
extern int MouseY;
extern float g_fWindowMouseX;
extern float g_fWindowMouseY;
extern bool MouseLButton;
extern bool MouseLButtonPush;
extern bool MouseLButtonPop;
extern bool MouseRButton;
extern bool MouseRButtonPush;
extern bool MouseRButtonPop;
extern int g_iMousePopPosition_x;
extern int g_iMousePopPosition_y;
extern int g_iNoMouseTime;
extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

namespace
{
constexpr int ReferenceWidth = 640;
constexpr int ReferenceHeight = 480;

enum class Kind : std::uint8_t
{
    None,
    Key,
    Click,
};

// Frames of the sequence. A key is down for the first frame only, which is
// what the key-state scan turns into a press edge; a click holds one frame
// longer so a button that acts on release sees down, held, up.
enum class Stage : std::uint8_t
{
    Idle,
    Pressed,
    Held,
    Released,
};

struct Injection
{
    Kind kind = Kind::None;
    Stage stage = Stage::Idle;
    int virtualKey = 0;
    float windowX = 0.0f;
    float windowY = 0.0f;
    Core::Input::Synthetic::MouseButton button = Core::Input::Synthetic::MouseButton::Left;
};

Injection g_injection;

struct NamedKey
{
    std::string_view name;
    int virtualKey;
};

// Every key the SDL key-state shim translates (`VkToScancode`), by the name
// a caller types.
constexpr NamedKey NamedKeys[] = {
    {"esc", VK_ESCAPE},     {"escape", VK_ESCAPE}, {"enter", VK_RETURN},
    {"return", VK_RETURN},  {"tab", VK_TAB},       {"space", VK_SPACE},
    {"backspace", VK_BACK}, {"home", VK_HOME},     {"end", VK_END},
    {"insert", VK_INSERT},  {"delete", VK_DELETE}, {"pageup", VK_PRIOR},
    {"pagedown", VK_NEXT},  {"up", VK_UP},         {"down", VK_DOWN},
    {"left", VK_LEFT},      {"right", VK_RIGHT},   {"printscreen", VK_SNAPSHOT},
    {"f1", VK_F1},          {"f2", VK_F2},         {"f3", VK_F3},
    {"f4", VK_F4},          {"f5", VK_F5},         {"f6", VK_F6},
    {"f7", VK_F7},          {"f8", VK_F8},         {"f9", VK_F9},
    {"f10", VK_F10},        {"f11", VK_F11},       {"f12", VK_F12},
};

constexpr std::string_view KeyNameList =
    "a-z, 0-9, esc, enter, tab, space, backspace, home, end, insert, delete, pageup, pagedown, "
    "up, down, left, right, printscreen, f1-f12";

std::string Lowercase(std::string_view text)
{
    std::string lowered(text);
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lowered;
}

int VirtualKeyForButton(Core::Input::Synthetic::MouseButton button)
{
    return button == Core::Input::Synthetic::MouseButton::Left ? VK_LBUTTON : VK_RBUTTON;
}

void ApplyPointerPosition()
{
    g_fWindowMouseX = g_injection.windowX;
    g_fWindowMouseY = g_injection.windowY;
    const auto transform =
        UI::Scaling::ScreenOverlayTransform(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    MouseX = std::clamp(static_cast<int>(UI::Scaling::LogicalX(transform, g_injection.windowX)), 0, ReferenceWidth);
    MouseY = std::clamp(static_cast<int>(UI::Scaling::LogicalY(transform, g_injection.windowY)), 0, ReferenceHeight);
    g_iNoMouseTime = 0;
}

void ApplyButtonDown()
{
    if (g_injection.button == Core::Input::Synthetic::MouseButton::Left)
    {
        MouseLButtonPop = false;
        MouseLButtonPush = !MouseLButton;
        MouseLButton = true;
        Core::Input::RecordLeftMouseButtonPressEdge();
        return;
    }
    MouseRButtonPop = false;
    MouseRButtonPush = !MouseRButton;
    MouseRButton = true;
}

void ApplyButtonUp()
{
    if (g_injection.button == Core::Input::Synthetic::MouseButton::Left)
    {
        MouseLButtonPop = MouseLButton;
        MouseLButton = false;
        g_iMousePopPosition_x = MouseX;
        g_iMousePopPosition_y = MouseY;
        return;
    }
    MouseRButtonPop = MouseRButton;
    MouseRButton = false;
}

// Takes an injected press back, for an injection that is dropped before its
// sequence ended. Unlike ApplyButtonUp this raises no release edge: the
// caller has already been told its command did not finish, so the click must
// not complete behind its back.
void RetractButton()
{
    if (g_injection.button == Core::Input::Synthetic::MouseButton::Left)
    {
        MouseLButton = false;
        MouseLButtonPush = false;
        MouseLButtonPop = false;
        Core::Input::ClearLeftMouseButtonPressEdge();
        return;
    }
    MouseRButton = false;
    MouseRButtonPush = false;
    MouseRButtonPop = false;
}

void AdvanceKey()
{
    // Pressed -> Released: down for exactly one scan.
    g_injection.stage = g_injection.stage == Stage::Pressed ? Stage::Released : Stage::Idle;
    if (g_injection.stage == Stage::Idle)
    {
        g_injection.kind = Kind::None;
    }
}

void AdvanceClick()
{
    switch (g_injection.stage)
    {
    case Stage::Pressed:
        g_injection.stage = Stage::Held;
        return;
    case Stage::Held:
        ApplyPointerPosition();
        ApplyButtonUp();
        g_injection.stage = Stage::Released;
        return;
    case Stage::Released:
    case Stage::Idle:
        g_injection = {};
        return;
    }
}
} // namespace

namespace Core::Input::Synthetic
{
std::optional<int> VirtualKeyFromName(std::string_view name)
{
    const std::string lowered = Lowercase(name);
    if (lowered.size() == 1)
    {
        const char c = lowered[0];
        if (c >= 'a' && c <= 'z')
        {
            return static_cast<int>(std::toupper(static_cast<unsigned char>(c)));
        }
        if (c >= '0' && c <= '9')
        {
            return static_cast<int>(c);
        }
        return std::nullopt;
    }

    for (const NamedKey& key : NamedKeys)
    {
        if (key.name == lowered)
        {
            return key.virtualKey;
        }
    }
    return std::nullopt;
}

std::string_view KeyNames()
{
    return KeyNameList;
}

std::optional<MouseButton> MouseButtonFromName(std::string_view name)
{
    const std::string lowered = Lowercase(name);
    if (lowered == "left")
    {
        return MouseButton::Left;
    }
    if (lowered == "right")
    {
        return MouseButton::Right;
    }
    return std::nullopt;
}

bool PressKey(int virtualKey)
{
    if (!IsIdle())
    {
        return false;
    }
    g_injection = {};
    g_injection.kind = Kind::Key;
    g_injection.virtualKey = virtualKey;
    return true;
}

bool Click(float windowX, float windowY, MouseButton button)
{
    if (!IsIdle())
    {
        return false;
    }
    g_injection = {};
    g_injection.kind = Kind::Click;
    g_injection.windowX = windowX;
    g_injection.windowY = windowY;
    g_injection.button = button;
    return true;
}

bool IsIdle()
{
    return g_injection.kind == Kind::None;
}

bool IsKeyHeld(int virtualKey)
{
    const bool down = g_injection.stage == Stage::Pressed || g_injection.stage == Stage::Held;
    if (!down)
    {
        return false;
    }
    if (g_injection.kind == Kind::Key)
    {
        return virtualKey == g_injection.virtualKey;
    }
    return g_injection.kind == Kind::Click && virtualKey == VirtualKeyForButton(g_injection.button);
}

void BeginFrame()
{
    switch (g_injection.kind)
    {
    case Kind::None:
        return;
    case Kind::Key:
        if (g_injection.stage == Stage::Idle)
        {
            g_injection.stage = Stage::Pressed;
            return;
        }
        AdvanceKey();
        return;
    case Kind::Click:
        if (g_injection.stage == Stage::Idle)
        {
            ApplyPointerPosition();
            ApplyButtonDown();
            g_injection.stage = Stage::Pressed;
            return;
        }
        AdvanceClick();
        return;
    }
}

void Reset()
{
    // A click that is dropped mid-sequence has already written the button
    // down; take that back, or the game keeps seeing a button held by a frame
    // that will never come. A key needs nothing: `IsKeyHeld` reads the stage,
    // which goes away with the injection.
    const bool holdingButton =
        g_injection.kind == Kind::Click && (g_injection.stage == Stage::Pressed || g_injection.stage == Stage::Held);
    if (holdingButton)
    {
        RetractButton();
    }
    g_injection = {};
}
} // namespace Core::Input::Synthetic
