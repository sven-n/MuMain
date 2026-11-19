// Non-Windows implementation of the cursor and window shims declared in WinUser.h, backed by SDL.
#ifndef _WIN32

#include "Core/Platform/WinUser.h"
#include "Core/Platform/GameWindow.h"

#include <SDL3/SDL.h>

BOOL GetCursorPos(LPPOINT lpPoint)
{
    if (!lpPoint) return FALSE;
    float x = 0.0f, y = 0.0f;
    SDL_GetMouseState(&x, &y); // relative to the window the mouse is in
#ifdef _EDITOR
    // In another window of the game (the editor's own) the position is made
    // relative to the game window, as Win32's GetCursorPos plus
    // ScreenToClient(g_hWnd) gives it.
    SDL_Window* mouseWindow = SDL_GetMouseFocus();
    SDL_Window* gameWindow = Core::Platform::GameWindow();
    if (mouseWindow != nullptr && gameWindow != nullptr && mouseWindow != gameWindow)
    {
        int mouseWindowX = 0, mouseWindowY = 0, gameWindowX = 0, gameWindowY = 0;
        SDL_GetWindowPosition(mouseWindow, &mouseWindowX, &mouseWindowY);
        SDL_GetWindowPosition(gameWindow, &gameWindowX, &gameWindowY);
        x += static_cast<float>(mouseWindowX - gameWindowX);
        y += static_cast<float>(mouseWindowY - gameWindowY);
    }
#endif
    lpPoint->x = static_cast<LONG>(x);
    lpPoint->y = static_cast<LONG>(y);
    return TRUE;
}

// GetCursorPos already returns client-space coordinates, so this is the identity.
BOOL ScreenToClient(HWND /*hWnd*/, LPPOINT lpPoint)
{
    return lpPoint ? TRUE : FALSE;
}

BOOL SetCursorPos(int x, int y)
{
    SDL_WarpMouseInWindow(nullptr, static_cast<float>(x), static_cast<float>(y));
    return TRUE;
}

HWND GetActiveWindow()
{
    return reinterpret_cast<HWND>(SDL_GetKeyboardFocus());  // null when unfocused
}

UINT GetDoubleClickTime()
{
    return 500;  // the Win32 default
}

HWND GetFocus()
{
    // The UI asks "is the game focused?" with `GetFocus() == g_hWnd`, and
    // compares the result against widget handles that derive from g_hWnd. On the
    // SDL path g_hWnd is null (the Win32 HWND bridge is Windows-only), and the
    // portable text fields (#447) don't take Win32 focus, so the main window is
    // always treated as focused: returning null keeps every `== g_hWnd`
    // comparison consistent without the platform layer depending on g_hWnd.
    // (Note: this can't be GetActiveWindow()/SDL_GetKeyboardFocus(), which return
    // the SDL window rather than g_hWnd and would invert the comparison.)
    return nullptr;
}

namespace
{
    // Win32 keeps a process-wide cursor display counter; the cursor is shown
    // while it is >= 0. The game and the editor toggle it to hide the OS cursor
    // (they draw their own) and to reveal it over editor UI. Start at 0 to match
    // the Win32 default (cursor visible).
    int g_cursorDisplayCount = 0;

    // Some Wayland compositors (seen on KDE Plasma) do not reliably drop the
    // pointer for SDL_HideCursor() alone, leaving the OS cursor composited over
    // the game's own cursor sprite (issue #462). Committing an explicit blank
    // (1x1 transparent) cursor in addition to hiding is honoured everywhere, so
    // keep one around and set it whenever the cursor should be hidden.
    SDL_Cursor* BlankCursor()
    {
        static SDL_Cursor* blank = nullptr;
        if (blank == nullptr)
        {
            Uint32 transparent = 0;  // single fully-transparent ARGB pixel
            SDL_Surface* surface = SDL_CreateSurfaceFrom(
                1, 1, SDL_PIXELFORMAT_ARGB8888, &transparent, sizeof(transparent));
            if (surface != nullptr)
            {
                blank = SDL_CreateColorCursor(surface, 0, 0);
                SDL_DestroySurface(surface);  // cursor keeps its own copy
            }
        }
        return blank;
    }

    void ApplyCursorVisibility()
    {
        // No-op until the SDL video subsystem is up; MuApplyCursorVisibility()
        // re-applies the pending state once the window exists.
        if (SDL_WasInit(SDL_INIT_VIDEO) == 0) return;
        if (g_cursorDisplayCount >= 0)
        {
            // Restore a real cursor first: a prior hide may have left the blank
            // one active, so SDL_ShowCursor() alone would show nothing.
            SDL_SetCursor(SDL_GetDefaultCursor());
            SDL_ShowCursor();
        }
        else
        {
            SDL_HideCursor();
            if (SDL_Cursor* blank = BlankCursor()) SDL_SetCursor(blank);
        }
    }
}

int ShowCursor(BOOL bShow)
{
    g_cursorDisplayCount += bShow ? 1 : -1;
    ApplyCursorVisibility();
    return g_cursorDisplayCount;
}

void MuApplyCursorVisibility()
{
    ApplyCursorVisibility();
}

#endif // !_WIN32
