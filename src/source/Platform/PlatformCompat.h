#pragma once

#ifdef _WIN32
// On Windows, all functions come from Win32 APIs — this header is a no-op.
#else

#include "PlatformTypes.h"

#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

// ---- Timing shims ----

inline uint32_t timeGetTime()
{
    using namespace std::chrono;
    return static_cast<uint32_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

inline uint32_t GetTickCount()
{
    return timeGetTime();
}

// ---- MessageBoxW shim ----
// MB_ flag constants
#define MB_OK 0x00
#define MB_YESNO 0x04
#define MB_OKCANCEL 0x01
#define MB_ICONERROR 0x10
#define MB_ICONWARNING 0x30
#define MB_ICONSTOP 0x10
#define MB_ICONINFORMATION 0x40

// Return values
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

// ---- Mouse cursor and position stubs (non-Windows, no SDL3 dependency) ----

inline DWORD GetDoubleClickTime()
{
    // 500ms is the standard double-click interval on Windows.
    // SDL3 does not expose a platform double-click time API.
    return 500u;
}

inline void GetCursorPos(POINT* ppt)
{
    // On SDL3 path, mouse position is maintained in MouseX/MouseY globals
    // via SDL_EVENT_MOUSE_MOTION. CInput::Update() calls GetCursorPos()
    // but the CInput cursor position system is superseded by the global
    // mouse state populated by SDLEventLoop.
    if (ppt != nullptr)
    {
        ppt->x = 0;
        ppt->y = 0;
    }
}

inline void ScreenToClient(HWND /*hwnd*/, POINT* /*ppt*/)
{
    // No-op on SDL3 path: mouse coordinates from SDL_EVENT_MOUSE_MOTION
    // are already window-relative. CInput::Update() calls ScreenToClient()
    // but on the SDL3 path cursor position is fed directly via SDL events
    // into MouseX/MouseY — the CInput position is not used for gameplay.
}

inline HWND GetActiveWindow()
{
    // On SDL3 path, focus state is managed by SDLEventLoop via g_bWndActive.
    // Return non-null so CInput::Update() does not zero out cursor state.
    // Use address of static sentinel to avoid integer-to-pointer cast warnings.
    static int s_activeWindowSentinel = 0;
    return static_cast<void*>(&s_activeWindowSentinel);
}

#ifdef MU_ENABLE_SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <cstdint>

// ---- Mouse cursor visibility shims (SDL3 path) ----
// SDL3 split SDL2's SDL_ShowCursor(int) into two separate functions.
// Win32 ShowCursor has a reference-count mechanism; the shim does not replicate
// the reference count since the codebase uses balanced show/hide calls.
// [VS1-SDL-INPUT-MOUSE]
inline void ShowCursor(bool show)
{
    if (show)
    {
        SDL_ShowCursor();
    }
    else
    {
        SDL_HideCursor();
    }
}

// MuPlatformLogMouseWarpFailed — implemented in SDLKeyboardState.cpp (compiled
// with the project PCH that provides CErrorReport / g_ErrorReport). Declared here
// so the inline SetCursorPos shim can call it without pulling ErrorReport.h into
// every TU that includes PlatformCompat.h. [VS1-SDL-INPUT-MOUSE]
void MuPlatformLogMouseWarpFailed(const char* sdlError);

// ---- Cursor warp shim (SDL3 path) ----
// SDL_WarpMouseInWindow with nullptr targets the currently focused window.
// Call sites pass window-relative coordinates scaled to WindowWidth/Height.
// [VS1-SDL-INPUT-MOUSE]
inline void SetCursorPos(int x, int y)
{
    if (!SDL_WarpMouseInWindow(nullptr, static_cast<float>(x), static_cast<float>(y)))
    {
        MuPlatformLogMouseWarpFailed(SDL_GetError());
    }
}

// ---- HIBYTE macro (non-Windows) ----
// Extracts the high byte of a 16-bit value.
// Used by all HIBYTE(GetAsyncKeyState(vk)) call sites (104 sites in codebase).
#ifndef HIBYTE
#define HIBYTE(w) (static_cast<uint8_t>((static_cast<uint16_t>(w) >> 8) & 0xFF))
#endif

// ---- Keyboard input shim (GetAsyncKeyState) ----
// g_sdl3KeyboardState[512] is defined in SDLKeyboardState.cpp and populated
// by SDLEventLoop::PollEvents(). Indexed by SDL_Scancode (0-511).
extern bool g_sdl3KeyboardState[512];

// Map Win32 Virtual Key code to SDL3 SDL_Scancode.
// Returns SDL_SCANCODE_UNKNOWN (0) for any VK code with no mapping.
[[nodiscard]] inline SDL_Scancode MuVkToSdlScancode(int vk)
{
    // ASCII letter range: 'A'-'Z' (0x41-0x5A) -> SDL_SCANCODE_A + offset
    static_assert(SDL_SCANCODE_A == 4, "SDL_SCANCODE_A must be 4 — verify SDL3 release-3.2.8");
    if (vk >= 'A' && vk <= 'Z')
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (vk - 'A'));
    }

    // ASCII digit range: '1'-'9' (0x31-0x39) -> SDL_SCANCODE_1 + offset
    // '0' (0x30) -> SDL_SCANCODE_0 (not contiguous with 1-9 in SDL3)
    static_assert(SDL_SCANCODE_1 == 30, "SDL_SCANCODE_1 must be 30 — verify SDL3 release-3.2.8");
    if (vk >= '1' && vk <= '9')
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (vk - '1'));
    }
    if (vk == '0')
    {
        return SDL_SCANCODE_0;
    }

    switch (vk)
    {
    // Control / navigation
    case 0x08:
        return SDL_SCANCODE_BACKSPACE; // VK_BACK
    case 0x09:
        return SDL_SCANCODE_TAB; // VK_TAB
    case 0x0D:
        return SDL_SCANCODE_RETURN; // VK_RETURN
    case 0x10:
        return SDL_SCANCODE_LSHIFT; // VK_SHIFT
    case 0x11:
        return SDL_SCANCODE_LCTRL; // VK_CONTROL
    case 0x12:
        return SDL_SCANCODE_LALT; // VK_MENU
    case 0x13:
        return SDL_SCANCODE_PAUSE; // VK_PAUSE
    case 0x14:
        return SDL_SCANCODE_CAPSLOCK; // VK_CAPITAL
    case 0x1B:
        return SDL_SCANCODE_ESCAPE; // VK_ESCAPE
    case 0x20:
        return SDL_SCANCODE_SPACE; // VK_SPACE
    case 0x21:
        return SDL_SCANCODE_PAGEUP; // VK_PRIOR
    case 0x22:
        return SDL_SCANCODE_PAGEDOWN; // VK_NEXT
    case 0x23:
        return SDL_SCANCODE_END; // VK_END
    case 0x24:
        return SDL_SCANCODE_HOME; // VK_HOME
    case 0x25:
        return SDL_SCANCODE_LEFT; // VK_LEFT
    case 0x26:
        return SDL_SCANCODE_UP; // VK_UP
    case 0x27:
        return SDL_SCANCODE_RIGHT; // VK_RIGHT
    case 0x28:
        return SDL_SCANCODE_DOWN; // VK_DOWN
    case 0x2C:
        return SDL_SCANCODE_PRINTSCREEN; // VK_SNAPSHOT
    case 0x2D:
        return SDL_SCANCODE_INSERT; // VK_INSERT
    case 0x2E:
        return SDL_SCANCODE_DELETE; // VK_DELETE

    // Numpad digits
    case 0x60:
        return SDL_SCANCODE_KP_0; // VK_NUMPAD0
    case 0x61:
        return SDL_SCANCODE_KP_1; // VK_NUMPAD1
    case 0x62:
        return SDL_SCANCODE_KP_2; // VK_NUMPAD2
    case 0x63:
        return SDL_SCANCODE_KP_3; // VK_NUMPAD3
    case 0x64:
        return SDL_SCANCODE_KP_4; // VK_NUMPAD4
    case 0x65:
        return SDL_SCANCODE_KP_5; // VK_NUMPAD5
    case 0x66:
        return SDL_SCANCODE_KP_6; // VK_NUMPAD6
    case 0x67:
        return SDL_SCANCODE_KP_7; // VK_NUMPAD7
    case 0x68:
        return SDL_SCANCODE_KP_8; // VK_NUMPAD8
    case 0x69:
        return SDL_SCANCODE_KP_9; // VK_NUMPAD9

    // Numpad operators
    case 0x6A:
        return SDL_SCANCODE_KP_MULTIPLY; // VK_MULTIPLY
    case 0x6B:
        return SDL_SCANCODE_KP_PLUS; // VK_ADD
    // VK_SEPARATOR (0x6C) intentionally omitted: no SDL3 equivalent.
    // Falls through to default → logs MU_ERR_INPUT_UNMAPPED_VK if called.
    case 0x6D:
        return SDL_SCANCODE_KP_MINUS; // VK_SUBTRACT
    case 0x6E:
        return SDL_SCANCODE_KP_DECIMAL; // VK_DECIMAL
    case 0x6F:
        return SDL_SCANCODE_KP_DIVIDE; // VK_DIVIDE

    // Function keys
    case 0x70:
        return SDL_SCANCODE_F1;
    case 0x71:
        return SDL_SCANCODE_F2;
    case 0x72:
        return SDL_SCANCODE_F3;
    case 0x73:
        return SDL_SCANCODE_F4;
    case 0x74:
        return SDL_SCANCODE_F5;
    case 0x75:
        return SDL_SCANCODE_F6;
    case 0x76:
        return SDL_SCANCODE_F7;
    case 0x77:
        return SDL_SCANCODE_F8;
    case 0x78:
        return SDL_SCANCODE_F9;
    case 0x79:
        return SDL_SCANCODE_F10;
    case 0x7A:
        return SDL_SCANCODE_F11;
    case 0x7B:
        return SDL_SCANCODE_F12;

    // Lock keys
    case 0x90:
        return SDL_SCANCODE_NUMLOCKCLEAR; // VK_NUMLOCK
    case 0x91:
        return SDL_SCANCODE_SCROLLLOCK; // VK_SCROLL

    // Left/Right modifier variants
    case 0xA0:
        return SDL_SCANCODE_LSHIFT; // VK_LSHIFT
    case 0xA1:
        return SDL_SCANCODE_RSHIFT; // VK_RSHIFT
    case 0xA2:
        return SDL_SCANCODE_LCTRL; // VK_LCONTROL
    case 0xA3:
        return SDL_SCANCODE_RCTRL; // VK_RCONTROL
    case 0xA4:
        return SDL_SCANCODE_LALT; // VK_LMENU
    case 0xA5:
        return SDL_SCANCODE_RALT; // VK_RMENU

    // OEM keys
    case 0xBA:
        return SDL_SCANCODE_SEMICOLON; // VK_OEM_1
    case 0xBB:
        return SDL_SCANCODE_EQUALS; // VK_OEM_PLUS
    case 0xBC:
        return SDL_SCANCODE_COMMA; // VK_OEM_COMMA
    case 0xBD:
        return SDL_SCANCODE_MINUS; // VK_OEM_MINUS
    case 0xBE:
        return SDL_SCANCODE_PERIOD; // VK_OEM_PERIOD

    default:
        return SDL_SCANCODE_UNKNOWN;
    }
}

// MuPlatformLogUnmappedVk — implemented in SDLKeyboardState.cpp (compiled with
// the project PCH that provides CErrorReport / g_ErrorReport). Declared here so
// the inline shim can call it without pulling ErrorReport.h into every TU that
// includes PlatformCompat.h. [VS1-SDL-INPUT-KEYBOARD]
void MuPlatformLogUnmappedVk(int vk);

// Mouse button state — populated by SDLEventLoop, used by GetAsyncKeyState shim.
// [VS1-SDL-INPUT-MOUSE]
extern bool MouseLButton;
extern bool MouseRButton;
extern bool MouseMButton;

// GetAsyncKeyState shim for non-Windows platforms.
// Returns 0x8000 (high bit set) when the key is currently held, 0 otherwise.
// Callers using HIBYTE(GetAsyncKeyState(vk)) & 0x80 or == 128 behave correctly:
//   HIBYTE(0x8000) == 0x80 == 128, satisfying both check patterns.
// Mouse button VK codes (0x01/0x02/0x04) are backed by global mouse state.
// Unmapped VK codes return 0 and log MU_ERR_INPUT_UNMAPPED_VK via g_ErrorReport.
// [VS1-SDL-INPUT-KEYBOARD] [VS1-SDL-INPUT-MOUSE]
[[nodiscard]] inline uint16_t GetAsyncKeyState(int vk)
{
    // Mouse button VK codes — backed by global mouse state populated by SDLEventLoop
    switch (vk)
    {
    case 0x01:
        return MouseLButton ? static_cast<uint16_t>(0x8000) : 0; // VK_LBUTTON
    case 0x02:
        return MouseRButton ? static_cast<uint16_t>(0x8000) : 0; // VK_RBUTTON
    case 0x04:
        return MouseMButton ? static_cast<uint16_t>(0x8000) : 0; // VK_MBUTTON
    default:
        break;
    }

    SDL_Scancode sc = MuVkToSdlScancode(vk);
    if (sc == SDL_SCANCODE_UNKNOWN)
    {
        MuPlatformLogUnmappedVk(vk);
        return 0;
    }
    return g_sdl3KeyboardState[sc] ? static_cast<uint16_t>(0x8000) : 0;
}

inline std::string mu_wchar_to_utf8(const wchar_t* src)
{
    std::string result;
    if (src == nullptr)
    {
        return result;
    }
    for (const wchar_t* p = src; *p; ++p)
    {
        auto ch = static_cast<uint32_t>(*p);
        if (ch < 0x80)
        {
            result += static_cast<char>(ch);
        }
        else if (ch < 0x800)
        {
            result += static_cast<char>(0xC0 | (ch >> 6));
            result += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else if (ch < 0x10000)
        {
            if (ch >= 0xD800 && ch <= 0xDFFF)
            {
                continue;
            }
            result += static_cast<char>(0xE0 | (ch >> 12));
            result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else if (ch <= 0x10FFFF)
        {
            result += static_cast<char>(0xF0 | (ch >> 18));
            result += static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (ch & 0x3F));
        }
    }
    return result;
}

inline int MessageBoxW(void* /*hwnd*/, const wchar_t* text, const wchar_t* caption, unsigned int type)
{
    std::string u8text = mu_wchar_to_utf8(text);
    std::string u8caption = mu_wchar_to_utf8(caption);

    // Map MB_ type to SDL message box flags
    SDL_MessageBoxFlags sdlFlags = SDL_MESSAGEBOX_INFORMATION;
    if ((type & MB_ICONERROR) || (type & MB_ICONSTOP))
        sdlFlags = SDL_MESSAGEBOX_ERROR;
    else if (type & MB_ICONWARNING)
        sdlFlags = SDL_MESSAGEBOX_WARNING;

    if ((type & 0x0F) == MB_YESNO)
    {
        // MB_YESNO: Two-button dialog via SDL_ShowMessageBox
        const SDL_MessageBoxButtonData buttons[] = {
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, IDYES, "Yes"},
            {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, IDNO, "No"},
        };

        const SDL_MessageBoxData messageboxdata = {
            sdlFlags, nullptr, u8caption.c_str(), u8text.c_str(), 2, buttons, nullptr,
        };

        int buttonid = IDNO;
        SDL_ShowMessageBox(&messageboxdata, &buttonid);
        return buttonid;
    }

    // MB_OK and other types: simple message box
    SDL_ShowSimpleMessageBox(sdlFlags, u8caption.c_str(), u8text.c_str(), nullptr);
    return IDOK;
}

#else // !MU_ENABLE_SDL3

// Fallback stub when SDL3 is not available
inline int MessageBoxW(void* /*hwnd*/, const wchar_t* /*text*/, const wchar_t* /*caption*/, unsigned int /*type*/)
{
    return IDOK;
}

#endif // MU_ENABLE_SDL3

#define MessageBox MessageBoxW

// ---- File I/O shims ----
// Convert wchar_t path (UTF-32 on Linux/macOS) to UTF-8, normalize backslashes

// Guard against platforms that already define errno_t
#ifndef MU_ERRNO_T_DEFINED
using errno_t = int;
#define MU_ERRNO_T_DEFINED
#endif

inline FILE* mu_wfopen(const wchar_t* path, const wchar_t* mode)
{
    // Manual UTF-8 conversion to avoid deprecated std::wstring_convert
    std::string u8path;
    for (const wchar_t* p = path; *p != L'\0'; ++p)
    {
        wchar_t ch = *p;
        if (ch == L'\\')
        {
            u8path += '/';
        }
        else if (ch < 0x80)
        {
            u8path += static_cast<char>(ch);
        }
        else if (ch < 0x800)
        {
            u8path += static_cast<char>(0xC0 | (ch >> 6));
            u8path += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else if (ch < 0x10000)
        {
            // Skip UTF-16 surrogate codepoints (U+D800-U+DFFF) — invalid in UTF-8
            if (ch >= 0xD800 && ch <= 0xDFFF)
            {
                continue;
            }
            u8path += static_cast<char>(0xE0 | (ch >> 12));
            u8path += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            u8path += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else if (ch <= 0x10FFFF)
        {
            u8path += static_cast<char>(0xF0 | (ch >> 18));
            u8path += static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
            u8path += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            u8path += static_cast<char>(0x80 | (ch & 0x3F));
        }
        // Codepoints above U+10FFFF are invalid Unicode — skip them
    }

    std::string u8mode;
    for (const wchar_t* m = mode; *m != L'\0'; ++m)
    {
        // File mode strings are always ASCII (e.g., "rb", "w")
        assert(static_cast<unsigned>(*m) < 0x80 && "fopen mode must be ASCII");
        u8mode += static_cast<char>(*m);
    }

    return fopen(u8path.c_str(), u8mode.c_str());
}

inline errno_t mu_wfopen_s(FILE** pFile, const wchar_t* path, const wchar_t* mode)
{
    if (pFile == nullptr || path == nullptr || mode == nullptr)
    {
        return EINVAL;
    }
    *pFile = mu_wfopen(path, mode);
    return (*pFile) ? 0 : errno;
}

// NOLINTBEGIN(bugprone-reserved-identifier)
#define _wfopen_s mu_wfopen_s
#define _wfopen mu_wfopen
// NOLINTEND(bugprone-reserved-identifier)

// ---- RtlSecureZeroMemory shim ----

inline void* mu_SecureZeroMemory(void* ptr, size_t cnt)
{
    volatile char* vptr = static_cast<volatile char*>(ptr);
    while (cnt--)
    {
        *vptr++ = 0;
    }
    return ptr;
}

#define RtlSecureZeroMemory mu_SecureZeroMemory

#endif // _WIN32
