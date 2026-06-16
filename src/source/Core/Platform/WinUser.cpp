// Non-Windows implementation of the MessageBox shim declared in WinUser.h.
// Routes the engine's error/info dialogs through SDL's message box.
#ifndef _WIN32

#include "Core/Platform/WinUser.h"

#include <SDL3/SDL.h>
#include <string>

namespace
{
    // wchar_t is UTF-32 on Linux; SDL wants UTF-8. Encode directly so non-ASCII
    // (e.g. Korean) text survives instead of relying on the C locale.
    std::string ToUtf8(LPCWSTR s)
    {
        std::string out;
        if (s == nullptr) return out;
        for (const wchar_t* p = s; *p; ++p)
        {
            unsigned int c = static_cast<unsigned int>(*p);
            // Map surrogates and out-of-range values to U+FFFD so we never emit
            // invalid UTF-8 to SDL / the font renderer.
            if ((c >= 0xD800 && c <= 0xDFFF) || c > 0x10FFFF) c = 0xFFFD;

            if (c < 0x80)
            {
                out.push_back(static_cast<char>(c));
            }
            else if (c < 0x800)
            {
                out.push_back(static_cast<char>(0xC0 | (c >> 6)));
                out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
            }
            else if (c < 0x10000)
            {
                out.push_back(static_cast<char>(0xE0 | (c >> 12)));
                out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
            }
            else
            {
                out.push_back(static_cast<char>(0xF0 | (c >> 18)));
                out.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
            }
        }
        return out;
    }

    Uint32 IconFlags(UINT type)
    {
        if (type & (MB_ICONERROR | MB_ICONSTOP)) return SDL_MESSAGEBOX_ERROR;
        if (type & MB_ICONEXCLAMATION)           return SDL_MESSAGEBOX_WARNING;
        return SDL_MESSAGEBOX_INFORMATION;
    }
}

int MessageBoxW(HWND /*owner*/, LPCWSTR text, LPCWSTR caption, UINT type)
{
    const std::string message = ToUtf8(text);
    const std::string title   = caption ? ToUtf8(caption) : std::string("MU");

    // Parent the dialog to the focused window so it stays in front and modal to
    // the game; a null parent can spawn behind the window on some Linux WMs.
    SDL_Window* parent = SDL_GetKeyboardFocus();

    if ((type & MB_YESNO) == MB_YESNO)
    {
        const SDL_MessageBoxButtonData buttons[2] = {
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, IDYES, "Yes" },
            { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, IDNO,  "No"  },
        };
        const SDL_MessageBoxData data = {
            IconFlags(type), parent, title.c_str(), message.c_str(),
            2, buttons, nullptr
        };
        int buttonId = IDNO;
        if (!SDL_ShowMessageBox(&data, &buttonId)) return IDNO;  // dialog failed/dismissed
        return (buttonId == IDYES) ? IDYES : IDNO;
    }

    SDL_ShowSimpleMessageBox(IconFlags(type), title.c_str(), message.c_str(), parent);
    return IDOK;
}

namespace
{
    bool IsQuitMessage(UINT msg) { return msg == WM_DESTROY || msg == WM_CLOSE || msg == WM_QUIT; }

    bool RequestQuit()
    {
        SDL_Event quit;
        SDL_zero(quit);
        quit.type = SDL_EVENT_QUIT;
        if (!SDL_PushEvent(&quit))
        {
            SDL_Log("Failed to push quit event: %s", SDL_GetError());
            return false;
        }
        return true;
    }
}

LRESULT SendMessage(HWND /*hWnd*/, UINT Msg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (IsQuitMessage(Msg)) RequestQuit();
    return 0;
}

BOOL PostMessage(HWND /*hWnd*/, UINT Msg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (IsQuitMessage(Msg)) return RequestQuit() ? TRUE : FALSE;
    return TRUE;
}

void PostQuitMessage(int /*nExitCode*/)
{
    RequestQuit();
}

BOOL GetCursorPos(LPPOINT lpPoint)
{
    if (!lpPoint) return FALSE;
    float x = 0.0f, y = 0.0f;
    SDL_GetMouseState(&x, &y);  // already window-relative
    lpPoint->x = static_cast<LONG>(x);
    lpPoint->y = static_cast<LONG>(y);
    return TRUE;
}

// GetCursorPos already returns client-space coordinates, so this is the identity.
BOOL ScreenToClient(HWND /*hWnd*/, LPPOINT lpPoint)
{
    return lpPoint ? TRUE : FALSE;
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
