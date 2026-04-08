#ifdef MU_ENABLE_SDL3

#include "SDLEventLoop.h"
#include "../MuPlatform.h"
#include "../PlatformCompat.h"
#include <SDL3/SDL.h>

// SDL text input buffer (SDLKeyboardState.cpp) — populated here, read by UIControls.
// [VS1-SDL-INPUT-TEXT]
extern char g_szSDLTextInput[32];
extern bool g_bSDLTextInputReady;

// Mouse button press-edge flag — set on BUTTON_DOWN, cleared by ScanAsyncKeyState.
// Survives same-frame BUTTON_UP so fast clicks aren't missed. [Story 7-9-9, AC-5]
bool g_bMouseLButtonPressEdge = false;

// External game state — set to true to trigger clean shutdown
extern bool Destroy;

// Window and display state (MuMain.cpp)
extern bool g_bWndActive;
extern BOOL g_bUseWindowMode;

// FPS throttle state (MuMain.cpp — shared with game loop)
extern double g_TargetFpsBeforeInactive;
extern bool g_HasInactiveFpsOverride;

// FPS control (SceneManager.cpp)
double GetTargetFps();
void SetTargetFps(double fps);

// Throttled FPS when inactive in fullscreen.
// Must match REFERENCE_FPS in ZzzAI.h — duplicated to avoid Platform->Gameplay coupling.
constexpr double INACTIVE_REFERENCE_FPS = 25.0;

// Mouse state (ZzzOpenglUtil.cpp) — cleared on focus-loss in windowed mode,
// and populated from SDL mouse events in PollEvents(). [VS1-SDL-INPUT-MOUSE]
extern bool MouseLButton;
extern bool MouseLButtonPop;
extern bool MouseLButtonPush;
extern bool MouseRButton;
extern bool MouseRButtonPop;
extern bool MouseRButtonPush;
extern bool MouseLButtonDBClick;
extern bool MouseMButton;
extern bool MouseMButtonPop;
extern bool MouseMButtonPush;
extern int MouseWheel;

// Mouse position and screen-rate scaling (ZzzOpenglUtil.cpp / MuMain.cpp / stdafx.h)
// [VS1-SDL-INPUT-MOUSE]
extern int MouseX;
extern int MouseY;
extern int g_iNoMouseTime;
extern int g_iMousePopPosition_x;
extern int g_iMousePopPosition_y;
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

namespace
{

void HandleFocusGain()
{
    g_bWndActive = true;

    if (g_HasInactiveFpsOverride)
    {
        SetTargetFps(g_TargetFpsBeforeInactive);
        g_HasInactiveFpsOverride = false;
    }

    if (g_bUseWindowMode == FALSE)
    {
        mu::MuPlatform::SetMouseGrab(true);
    }

    g_ConsoleDebug->Write(MCD_NORMAL, L"[VS1-SDL-WINDOW-FOCUS] Focus gained\r\n");
}

void HandleFocusLoss()
{
    // Only set inactive in fullscreen mode — matches Win32 ACTIVE_FOCUS_OUT guard
    // (MuMain.cpp:491-494). In windowed mode g_bWndActive stays true so that
    // game systems like the slide-help timer continue to work on Alt-Tab.
    if (g_bUseWindowMode == FALSE)
    {
        g_bWndActive = false;
    }

    if (g_bUseWindowMode == FALSE && !g_HasInactiveFpsOverride)
    {
        g_TargetFpsBeforeInactive = GetTargetFps();
        SetTargetFps(INACTIVE_REFERENCE_FPS);
        g_HasInactiveFpsOverride = true;
    }

    if (g_bUseWindowMode == TRUE)
    {
        MouseLButton = false;
        MouseLButtonPop = false;
        MouseLButtonPush = false;
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;
        MouseLButtonDBClick = false;
        MouseMButton = false;
        MouseMButtonPop = false;
        MouseMButtonPush = false;
        MouseWheel = 0;
    }

    mu::MuPlatform::SetMouseGrab(false);

    // Clear all keyboard state on focus loss — prevents stuck keys on Alt-Tab.
    // Unconditional (both windowed and fullscreen), unlike mouse state above which is
    // windowed-only: stuck keys are unsafe regardless of window mode because game logic
    // that polls GetAsyncKeyState() continues running even when g_bWndActive is false.
    // Win32 equivalent: the OS clears async key state implicitly on WM_KILLFOCUS.
    // [VS1-SDL-INPUT-KEYBOARD]
    std::fill(std::begin(g_sdl3KeyboardState), std::end(g_sdl3KeyboardState), false);

    g_ConsoleDebug->Write(MCD_NORMAL, L"[VS1-SDL-WINDOW-FOCUS] Focus lost\r\n");
}

} // anonymous namespace

namespace mu
{

bool SDLEventLoop::PollEvents()
{
    // Text input state: do NOT clear here — PollEvents may run multiple times
    // per render frame (frame rate throttling). Clearing here would discard
    // TEXT_INPUT events before DoActionSub() gets a chance to consume them.
    // Text is cleared after consumption in DoActionSub (UIControls.cpp).
    // [VS1-SDL-INPUT-TEXT]

    // Reset per-frame mouse state before processing new events. [VS1-SDL-INPUT-MOUSE]
    // MouseLButtonDBClick: mirrors MuMain.cpp:611 which clears this each frame.
    // MouseWheel: per-frame accumulated value — cleared each frame like WM_MOUSEWHEEL.
    MouseLButtonDBClick = false;
    MouseWheel = 0;

    // Position-drift clearing: mirrors MuMain.cpp:612-613.
    // If the cursor has moved since the last left-button-up, clear the stale pop flag.
    // On Win32 this pattern auto-clears MouseLButtonPop when the cursor drifts from
    // the release position, preventing stale click-detection in fast-click interactions.
    if (MouseLButtonPop && (g_iMousePopPosition_x != MouseX || g_iMousePopPosition_y != MouseY))
    {
        MouseLButtonPop = false;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            Destroy = true;
            return false;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            Destroy = true;
            return false;

        case SDL_EVENT_WINDOW_RESIZED:
            // Resize handling deferred to future story (EPIC-4 rendering migration)
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            HandleFocusGain();
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            HandleFocusLoss();
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            HandleFocusLoss();
            break;

        case SDL_EVENT_WINDOW_RESTORED:
            HandleFocusGain();
            break;

        // Story 2.2.1: Keyboard state tracking [VS1-SDL-INPUT-KEYBOARD]
        // Maintains g_sdl3KeyboardState[] array for GetAsyncKeyState() shim.
        // event.key.repeat is intentionally ignored — the shim models Win32
        // async state (held=true, released=false), not WM_KEYDOWN repeat events.
        case SDL_EVENT_KEY_DOWN:
            if (static_cast<unsigned>(event.key.scancode) < 512u)
            {
                g_sdl3KeyboardState[event.key.scancode] = true;
            }
            break;

        case SDL_EVENT_KEY_UP:
            if (static_cast<unsigned>(event.key.scancode) < 512u)
            {
                g_sdl3KeyboardState[event.key.scancode] = false;
            }
            break;

        // Story 2.2.3: Text input tracking [VS1-SDL-INPUT-TEXT]
        // SDL_EVENT_TEXT_INPUT fires once per composed character or character sequence
        // (after IME commit). event.text.text[] is a null-terminated UTF-8 string (up to 32 bytes).
        // Multiple TEXT_INPUT events per frame are concatenated (rare but possible with IME).
        // Per-frame reset of g_szSDLTextInput / g_bSDLTextInputReady is at start of PollEvents().
        case SDL_EVENT_TEXT_INPUT:
            // Copy into g_szSDLTextInput for CUITextInputBox::DoActionSub() to consume.
            // [VS1-SDL-INPUT-TEXT]
            {
                size_t existing = strlen(g_szSDLTextInput);
                size_t incoming = strlen(event.text.text);
                if (existing + incoming < sizeof(g_szSDLTextInput))
                {
                    memcpy(g_szSDLTextInput + existing, event.text.text, incoming + 1);
                }
                g_bSDLTextInputReady = true;
            }
            break;

        // Story 2.2.2: Mouse input tracking [VS1-SDL-INPUT-MOUSE]
        // Mirrors WM_MOUSEMOVE / WM_LBUTTONDOWN / WM_RBUTTONDOWN / WM_MOUSEWHEEL handlers
        // in WndProc (MuMain.cpp). Coordinates normalized to 640x480 virtual space.
        case SDL_EVENT_MOUSE_MOTION:
            // SDL3: event.motion.x/y are float (window-relative pixels).
            // Divide by screen rate to map to 640x480 virtual coordinate space.
            MouseX = static_cast<int>(event.motion.x / g_fScreenRate_x);
            MouseY = static_cast<int>(event.motion.y / g_fScreenRate_y);
            if (MouseX < 0)
            {
                MouseX = 0;
            }
            if (MouseX > 640)
            {
                MouseX = 640;
            }
            if (MouseY < 0)
            {
                MouseY = 0;
            }
            if (MouseY > 480)
            {
                MouseY = 480;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            g_iNoMouseTime = 0;
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
                MouseLButtonPop = false;
                if (!MouseLButton)
                {
                    MouseLButtonPush = true;
                }
                MouseLButton = true;
                g_bMouseLButtonPressEdge = true; // Edge flag for ScanAsyncKeyState [Story 7-9-9, AC-5]
                if (event.button.clicks == 2)
                {
                    MouseLButtonDBClick = true;
                }
                // SDL_CaptureMouse replaces SetCapture — ensures events received outside window
                SDL_CaptureMouse(true);
                break;
            case SDL_BUTTON_RIGHT:
                MouseRButtonPop = false;
                if (!MouseRButton)
                {
                    MouseRButtonPush = true;
                }
                MouseRButton = true;
                SDL_CaptureMouse(true);
                break;
            case SDL_BUTTON_MIDDLE:
                MouseMButtonPop = false;
                if (!MouseMButton)
                {
                    MouseMButtonPush = true;
                }
                MouseMButton = true;
                SDL_CaptureMouse(true);
                break;
            default:
                break;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            g_iNoMouseTime = 0;
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
                MouseLButtonPush = false;
                if (MouseLButton)
                {
                    MouseLButtonPop = true;
                }
                MouseLButton = false;
                g_iMousePopPosition_x = MouseX;
                g_iMousePopPosition_y = MouseY;
                SDL_CaptureMouse(false);
                break;
            case SDL_BUTTON_RIGHT:
                MouseRButtonPush = false;
                if (MouseRButton)
                {
                    MouseRButtonPop = true;
                }
                MouseRButton = false;
                SDL_CaptureMouse(false);
                break;
            case SDL_BUTTON_MIDDLE:
                MouseMButtonPush = false;
                if (MouseMButton)
                {
                    MouseMButtonPop = true;
                }
                MouseMButton = false;
                SDL_CaptureMouse(false);
                break;
            default:
                break;
            }
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            // SDL3: event.wheel.y positive = scroll up (away from user) — same sign as Win32 WHEEL_DELTA.
            // Truncate to int — sub-integer trackpad deltas are acceptable to truncate.
            MouseWheel = static_cast<int>(event.wheel.y);
            break;

        default:
            break;
        }
    }
    return true;
}

} // namespace mu

#endif // MU_ENABLE_SDL3
