// Story 2.2.1: SDL3 Keyboard Input Migration [VS1-SDL-INPUT-KEYBOARD]
// Story 2.2.3: SDL3 Text Input Migration [VS1-SDL-INPUT-TEXT]
// Defines the g_sdl3KeyboardState array (indexed by SDL_Scancode, size 512),
// the g_szSDLTextInput / g_bSDLTextInputReady
// globals, and MuStartTextInput() / MuStopTextInput() lifecycle functions.
//
// Separated from PlatformCompat.h to avoid pulling CErrorReport/g_ErrorReport
// into every translation unit that includes PlatformCompat.h.
// Compiled with the project PCH (stdafx.h) via MUPlatform REUSE_FROM MUCore.

#ifdef MU_ENABLE_SDL3

#include "PlatformCompat.h"
#include "../MuPlatform.h"
#include "../IPlatformWindow.h"

// Keyboard state array: true when the key at that SDL_Scancode index is held.
// Populated by SDLEventLoop::PollEvents() on KEY_DOWN / KEY_UP events.
// Cleared in HandleFocusLoss() to prevent stuck keys on Alt-Tab.
// Size matches SDL_NUM_SCANCODES (512 in SDL3 release-3.2.8).
bool g_sdl3KeyboardState[512] = {};

// Logs a cursor warp failure via the post-mortem error log.
// Called by the SetCursorPos() shim in PlatformCompat.h.
// [VS1-SDL-INPUT-MOUSE]
void MuPlatformLogMouseWarpFailed(const char* sdlError)
{
    g_ErrorReport.Write(L"MU_ERR_MOUSE_WARP_FAILED [VS1-SDL-INPUT-MOUSE]: cursor warp failed: %hs\r\n", sdlError);
}

// SDL text input buffer — populated by SDL_EVENT_TEXT_INPUT in SDLEventLoop::PollEvents().
// Up to SDL_TEXTINPUTEVENT_TEXT_SIZE (32) UTF-8 bytes per event.
// Declared extern in PlatformCompat.h for access by CUITextInputBox::DoActionSub().
// [VS1-SDL-INPUT-TEXT]
char g_szSDLTextInput[32] = {};
bool g_bSDLTextInputReady = false;

// MuStartTextInput — activate SDL3 text input for the current window.
// Called by CUITextInputBox::GiveFocus() when a text field gains focus.
// SDL3: SDL_StartTextInput(SDL_Window*) requires the window pointer (changed from SDL2).
// [VS1-SDL-INPUT-TEXT]
void MuStartTextInput()
{
    SDL_Window* pWnd = static_cast<SDL_Window*>(
        mu::MuPlatform::GetWindow() ? mu::MuPlatform::GetWindow()->GetNativeHandle() : nullptr);
    if (pWnd != nullptr)
    {
        SDL_StartTextInput(pWnd);
        g_ErrorReport.Write(L"[VS1-SDL-INPUT-TEXT] SDL_StartTextInput activated\r\n");
    }
    else
    {
        g_ErrorReport.Write(L"MU_ERR_TEXT_START_FAILED [VS1-SDL-INPUT-TEXT]: no SDL window available\r\n");
    }
}

// MuStopTextInput — deactivate SDL3 text input for the current window.
// Called by CUITextInputBox::SetState(UISTATE_HIDE) when a text field loses focus/hides.
// [VS1-SDL-INPUT-TEXT]
void MuStopTextInput()
{
    SDL_Window* pWnd = static_cast<SDL_Window*>(
        mu::MuPlatform::GetWindow() ? mu::MuPlatform::GetWindow()->GetNativeHandle() : nullptr);
    if (pWnd != nullptr)
    {
        SDL_StopTextInput(pWnd);
    }
}

#endif // MU_ENABLE_SDL3
