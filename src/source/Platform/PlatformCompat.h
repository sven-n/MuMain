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

#ifdef MU_ENABLE_SDL3
#include <SDL3/SDL.h>

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
