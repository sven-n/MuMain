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

    void RequestQuit()
    {
        SDL_Event quit;
        SDL_zero(quit);
        quit.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit);
    }
}

LRESULT SendMessage(HWND /*hWnd*/, UINT Msg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (IsQuitMessage(Msg)) RequestQuit();
    return 0;
}

BOOL PostMessage(HWND /*hWnd*/, UINT Msg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (IsQuitMessage(Msg)) RequestQuit();
    return TRUE;
}

#endif // !_WIN32
