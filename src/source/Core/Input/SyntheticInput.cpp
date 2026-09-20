#include "stdafx.h"
#include "Core/Input/SyntheticInput.h"

#include "Core/Input/KeyState.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>

namespace
{
enum class Kind : std::uint8_t
{
    None,
    Key,
    Click,
    Text,
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
    // Owned storage for a Kind::Text injection. SDL_TextInputEvent carries a
    // `const char*` that SDL copies only for events it generates itself, so a
    // pushed one points at this string and it has to outlive the frame that
    // consumes the event -- it does, since the injection is only cleared a
    // frame after its last push.
    std::string text;
    bool pressEnter = false;
};

Injection g_injection;

// Numbers the injections, so a command can recognise its own. Never reused:
// an act compares the value it was given when its injection was accepted.
std::uint64_t g_generation = 0;

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

Uint8 SdlButton(Core::Input::Synthetic::MouseButton button)
{
    return button == Core::Input::Synthetic::MouseButton::Left ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
}

// The game's only window; null before it exists (and in unit tests, where SDL is
// not initialised) -- an injection then advances through its frames without
// producing any event.
SDL_Window* GameWindow()
{
    int count = 0;
    SDL_Window** windows = SDL_GetWindows(&count);
    SDL_Window* window = count > 0 ? windows[0] : nullptr;
    SDL_free(static_cast<void*>(windows));
    return window;
}

SDL_WindowID GameWindowId()
{
    SDL_Window* window = GameWindow();
    return window != nullptr ? SDL_GetWindowID(window) : 0;
}

// Pointer position for the click about to be pushed. A press is dispatched to
// whatever the pointer last moved over, so the move has to precede it.
void PushPointerMotion()
{
    const SDL_WindowID windowId = GameWindowId();
    if (windowId == 0)
        return;

    SDL_Event event{};
    event.type = SDL_EVENT_MOUSE_MOTION;
    event.motion.timestamp = SDL_GetTicksNS();
    event.motion.windowID = windowId;
    event.motion.x = g_injection.windowX;
    event.motion.y = g_injection.windowY;
    SDL_PushEvent(&event);
}

void PushMouseButton(bool down)
{
    const SDL_WindowID windowId = GameWindowId();
    if (windowId == 0)
        return;

    SDL_Event event{};
    event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    event.button.timestamp = SDL_GetTicksNS();
    event.button.windowID = windowId;
    event.button.button = SdlButton(g_injection.button);
    event.button.down = down;
    event.button.clicks = 1;
    event.button.x = g_injection.windowX;
    event.button.y = g_injection.windowY;
    SDL_PushEvent(&event);
}

void PushTextInput()
{
    const SDL_WindowID windowId = GameWindowId();
    if (windowId == 0)
        return;

    SDL_Event event{};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.timestamp = SDL_GetTicksNS();
    event.text.windowID = windowId;
    event.text.text = g_injection.text.c_str();
    SDL_PushEvent(&event);
}

void PushKey(bool down)
{
    const SDL_WindowID windowId = GameWindowId();
    if (windowId == 0)
        return;

    const auto scancode = static_cast<SDL_Scancode>(Core::Input::ScancodeForVirtualKey(g_injection.virtualKey));
    if (scancode == SDL_SCANCODE_UNKNOWN)
        return;

    SDL_Event event{};
    event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.timestamp = SDL_GetTicksNS();
    event.key.windowID = windowId;
    event.key.scancode = scancode;
    event.key.key = SDL_GetKeyFromScancode(scancode, SDL_KMOD_NONE, false);
    event.key.mod = SDL_KMOD_NONE;
    event.key.down = down;
    event.key.repeat = false;
    SDL_PushEvent(&event);
}

void AdvanceKey()
{
    // Pressed -> Released: down for exactly one scan, then the matching key-up.
    if (g_injection.stage == Stage::Pressed)
    {
        PushKey(false);
        g_injection.stage = Stage::Released;
        return;
    }
    g_injection = {};
}

// Frame 1 delivers the characters, frame 2 the Return press when one was asked
// for, frame 3 its release. A field that submits on Return therefore sees the
// text before the submit, in separate pumps of the event loop.
void AdvanceText()
{
    switch (g_injection.stage)
    {
    case Stage::Pressed:
        if (!g_injection.pressEnter)
        {
            g_injection = {};
            return;
        }
        g_injection.virtualKey = VK_RETURN;
        PushKey(true);
        g_injection.stage = Stage::Held;
        return;
    case Stage::Held:
        PushKey(false);
        g_injection.stage = Stage::Released;
        return;
    case Stage::Released:
    case Stage::Idle:
        g_injection = {};
        return;
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
        PushMouseButton(false);
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
    ++g_generation;
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
    ++g_generation;
    g_injection.kind = Kind::Click;
    g_injection.windowX = windowX;
    g_injection.windowY = windowY;
    g_injection.button = button;
    return true;
}

bool TypeText(std::string_view text, bool pressEnter)
{
    if (!IsIdle())
    {
        return false;
    }
    if (text.empty() || text.size() > MaxTypedTextBytes)
    {
        return false;
    }
    // Control characters are not text: a newline or a tab belongs to `PressKey`
    // (and `pressEnter` below), not into a field's contents.
    const bool hasControl =
        std::any_of(text.begin(), text.end(), [](char c) { return static_cast<unsigned char>(c) < 0x20; });
    if (hasControl)
    {
        return false;
    }

    g_injection = {};
    ++g_generation;
    g_injection.kind = Kind::Text;
    g_injection.text.assign(text);
    g_injection.pressEnter = pressEnter;
    return true;
}

bool IsIdle()
{
    return g_injection.kind == Kind::None;
}

std::uint64_t CurrentGeneration()
{
    return g_generation;
}

bool IsKeyHeld(int virtualKey)
{
    const bool down = g_injection.stage == Stage::Pressed || g_injection.stage == Stage::Held;
    if (!down)
    {
        return false;
    }
    if (g_injection.kind == Kind::Key || g_injection.kind == Kind::Text)
    {
        // Kind::Text only sets a key while it delivers the Return that submits
        // the field; until then its virtualKey is 0, which matches nothing.
        return virtualKey != 0 && virtualKey == g_injection.virtualKey;
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
            PushKey(true);
            g_injection.stage = Stage::Pressed;
            return;
        }
        AdvanceKey();
        return;
    case Kind::Click:
        if (g_injection.stage == Stage::Idle)
        {
            PushPointerMotion();
            PushMouseButton(true);
            g_injection.stage = Stage::Pressed;
            return;
        }
        AdvanceClick();
        return;
    case Kind::Text:
        if (g_injection.stage == Stage::Idle)
        {
            PushTextInput();
            g_injection.stage = Stage::Pressed;
            return;
        }
        AdvanceText();
        return;
    }
}

void Reset()
{
    // An injection dropped before its sequence ended has already delivered its
    // press as an event; the matching release has to follow, or the UI keeps
    // seeing a key or a button held down by a frame that will never come. It
    // is sent at once rather than on the release frame: the command it
    // belonged to has already been answered.
    const bool pressed = g_injection.stage == Stage::Pressed || g_injection.stage == Stage::Held;
    if (pressed && g_injection.kind == Kind::Key)
    {
        PushKey(false);
    }
    else if (pressed && g_injection.kind == Kind::Click)
    {
        PushMouseButton(false);
    }
    g_injection = {};
}
} // namespace Core::Input::Synthetic
