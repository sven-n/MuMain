// Portable shims for the Win32 timer / wide-string / path helpers the engine
// calls (issue #462, Phase 3). On Windows these come from the SDK/CRT, so this
// header is empty there; elsewhere it maps them onto the standard library.
#pragma once

#ifndef _WIN32

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <cwctype>
#include <strings.h>  // strcasecmp
#include "Core/Platform/WinCompat.h"  // DWORD, FILE handle types

namespace mu_detail
{
    // Shared monotonic epoch so GetTickCount() and timeGetTime() agree.
    inline std::chrono::steady_clock::time_point tickEpoch()
    {
        static const std::chrono::steady_clock::time_point epoch = std::chrono::steady_clock::now();
        return epoch;
    }
}

// Milliseconds since first use (relative timing; matches how the engine uses it).
inline DWORD GetTickCount()
{
    using namespace std::chrono;
    return static_cast<DWORD>(duration_cast<milliseconds>(steady_clock::now() - mu_detail::tickEpoch()).count());
}
inline DWORD timeGetTime() { return GetTickCount(); }

// Case-insensitive compares.
inline int _wcsicmp(const wchar_t* a, const wchar_t* b) { return wcscasecmp(a, b); }
inline int wcsicmp(const wchar_t* a, const wchar_t* b) { return wcscasecmp(a, b); }
inline int _stricmp(const char* a, const char* b) { return strcasecmp(a, b); }

// In-place wide upper-case.
inline wchar_t* _wcsupr(wchar_t* s)
{
    for (wchar_t* p = s; *p; ++p) *p = static_cast<wchar_t>(towupper(*p));
    return s;
}

// Wide fopen: convert the path/mode to the locale encoding and open.
inline FILE* _wfopen(const wchar_t* path, const wchar_t* mode)
{
    char narrowPath[4096] = { 0 };
    char narrowMode[16] = { 0 };
    if (wcstombs(narrowPath, path, sizeof(narrowPath) - 1) == static_cast<size_t>(-1)) return nullptr;
    if (wcstombs(narrowMode, mode, sizeof(narrowMode) - 1) == static_cast<size_t>(-1)) return nullptr;
    return fopen(narrowPath, narrowMode);
}

// Split a wide path into components (POSIX has no drive). Matches MSVC's
// _wsplitpath: any output may be null; `ext` keeps its leading dot.
inline void _wsplitpath(const wchar_t* path, wchar_t* drive, wchar_t* dir, wchar_t* fname, wchar_t* ext)
{
    if (drive) drive[0] = L'\0';

    const wchar_t* lastSep = nullptr;
    for (const wchar_t* p = path; *p; ++p)
        if (*p == L'/' || *p == L'\\') lastSep = p;
    const wchar_t* nameStart = lastSep ? lastSep + 1 : path;

    if (dir)
    {
        const size_t dlen = static_cast<size_t>(nameStart - path);
        wmemcpy(dir, path, dlen);
        dir[dlen] = L'\0';
    }

    const wchar_t* dot = nullptr;
    for (const wchar_t* p = nameStart; *p; ++p)
        if (*p == L'.') dot = p;

    if (fname)
    {
        const size_t flen = dot ? static_cast<size_t>(dot - nameStart) : wcslen(nameStart);
        wmemcpy(fname, nameStart, flen);
        fname[flen] = L'\0';
    }
    if (ext)
    {
        if (dot) wcscpy(ext, dot);
        else ext[0] = L'\0';
    }
}

#endif // !_WIN32
