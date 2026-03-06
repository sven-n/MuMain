#ifdef MU_ENABLE_SDL3

#include "SDLEventLoop.h"
#include "../MuPlatform.h"
#include "../PlatformCompat.h"
#include <SDL3/SDL.h>
#include <algorithm>

// External game state — set to true to trigger clean shutdown
extern bool Destroy;

// Window and display state (Winmain.cpp)
extern bool g_bWndActive;
extern BOOL g_bUseWindowMode;

// FPS throttle state (Winmain.cpp — shared with game loop)
extern double g_TargetFpsBeforeInactive;
extern bool g_HasInactiveFpsOverride;

// FPS control (SceneManager.cpp)
double GetTargetFps();
void SetTargetFps(double fps);

// Throttled FPS when inactive in fullscreen.
// Must match REFERENCE_FPS in ZzzAI.h — duplicated to avoid Platform->Gameplay coupling.
constexpr double INACTIVE_REFERENCE_FPS = 25.0;

// Mouse state (ZzzOpenglUtil.cpp) — cleared on focus-loss in windowed mode
extern bool MouseLButton;
extern bool MouseLButtonPop;
extern bool MouseRButton;
extern bool MouseRButtonPop;
extern bool MouseRButtonPush;
extern bool MouseLButtonDBClick;
extern bool MouseMButton;
extern bool MouseMButtonPop;
extern bool MouseMButtonPush;
extern int MouseWheel;

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
    // (Winmain.cpp:491-494). In windowed mode g_bWndActive stays true so that
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
    // Equivalent to Win32 clearing async key state when the window loses focus.
    // [VS1-SDL-INPUT-KEYBOARD]
    std::fill(std::begin(g_sdl3KeyboardState), std::end(g_sdl3KeyboardState), false);

    g_ConsoleDebug->Write(MCD_NORMAL, L"[VS1-SDL-WINDOW-FOCUS] Focus lost\r\n");
}

} // anonymous namespace

namespace mu
{

bool SDLEventLoop::PollEvents()
{
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
            if (event.key.scancode < 512)
            {
                g_sdl3KeyboardState[event.key.scancode] = true;
            }
            break;

        case SDL_EVENT_KEY_UP:
            if (event.key.scancode < 512)
            {
                g_sdl3KeyboardState[event.key.scancode] = false;
            }
            break;

        default:
            break;
        }
    }
    return true;
}

} // namespace mu

#endif // MU_ENABLE_SDL3
