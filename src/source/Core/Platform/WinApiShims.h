// Portable shims for the Win32 timer / wide-string / path helpers the engine
// calls (issue #462, Phase 3). On Windows these come from the SDK/CRT, so this
// header is empty there; elsewhere it maps them onto the standard library.
#pragma once

#ifndef _WIN32

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>    // strlen
#include <ctime>      // tzset
#include <cwchar>
#include <cwctype>
#include <strings.h>  // strcasecmp
#include <unistd.h>   // readlink
#include "Core/Platform/WinCompat.h"  // DWORD, FILE handle types
#include "Core/Platform/WinNls.h"     // MultiByteToWideChar / CP_UTF8

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

// 64-bit millisecond tick (no 49-day wrap).
inline unsigned long long GetTickCount64()
{
    using namespace std::chrono;
    return static_cast<unsigned long long>(
        duration_cast<milliseconds>(steady_clock::now() - mu_detail::tickEpoch()).count());
}

// Debug output -> stderr.
inline void OutputDebugStringA(const char* s)    { if (s) std::fputs(s, stderr); }
inline void OutputDebugStringW(const wchar_t* s) { if (s) std::fputws(s, stderr); }
#ifndef OutputDebugString
#define OutputDebugString OutputDebugStringW
#endif

// Non-secure _snwprintf(buf, count, fmt, ...) -> the bounded _vsnwprintf core.
inline int _snwprintf(wchar_t* buf, size_t count, const wchar_t* fmt, ...)
{
    va_list a; va_start(a, fmt);
    const int r = _vsnwprintf(buf, count, fmt, a);
    va_end(a);
    return r;
}

inline void _tzset() { ::tzset(); }

// Narrow int -> string (MSVC itoa/_itoa); a leading '-' only for base 10.
inline char* itoa(int value, char* buffer, int radix)
{
    if (!buffer) return buffer;
    if (radix < 2 || radix > 36) { buffer[0] = '\0'; return buffer; }
    const bool negative = (value < 0 && radix == 10);
    unsigned int v = negative ? -static_cast<unsigned int>(value) : static_cast<unsigned int>(value);
    char digits[33];
    int n = 0;
    do { const unsigned int d = v % static_cast<unsigned int>(radix);
         digits[n++] = static_cast<char>(d < 10 ? '0' + d : 'a' + (d - 10));
         v /= static_cast<unsigned int>(radix); } while (v != 0);
    int j = 0;
    if (negative) buffer[j++] = '-';
    while (n > 0) buffer[j++] = digits[--n];
    buffer[j] = '\0';
    return buffer;
}
inline char* _itoa(int value, char* buffer, int radix) { return itoa(value, buffer, radix); }

// Path of the running executable (Win32 GetModuleFileNameW). The engine only
// queries its own module (hModule == null), which maps to /proc/self/exe.
inline DWORD GetModuleFileNameW(HMODULE /*hModule*/, LPWSTR lpFilename, DWORD nSize)
{
    if (!lpFilename || nSize == 0) return 0;
    char path[4096];
    const ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (n <= 0) { lpFilename[0] = L'\0'; return 0; }
    // UTF-8 -> wide via the project's own converter (locale-independent, unlike
    // mbstowcs which fails on non-ASCII paths in the default "C" locale).
    const int converted = MultiByteToWideChar(CP_UTF8, 0, path, static_cast<int>(n),
                                              lpFilename, static_cast<int>(nSize - 1));
    if (converted <= 0) { lpFilename[0] = L'\0'; return 0; }
    lpFilename[converted] = L'\0';
    return static_cast<DWORD>(converted);
}

// Terminate the process (Win32 ExitProcess). Never returns, like the real one.
[[noreturn]] inline void ExitProcess(UINT uExitCode) { exit(static_cast<int>(uExitCode)); }

// String length (Win32 lstrlen; the engine builds UNICODE, hence the wide form).
inline int lstrlen(const wchar_t* s) { return s ? static_cast<int>(wcslen(s)) : 0; }
inline int lstrlen(const char* s)    { return s ? static_cast<int>(strlen(s)) : 0; }

// Wide string -> int.
inline int _wtoi(const wchar_t* s) { return s ? static_cast<int>(wcstol(s, nullptr, 10)) : 0; }

// Bytes in the multibyte character at c. Narrow strings are UTF-8 on Linux, so
// this is the UTF-8 sequence length taken from the lead byte.
inline size_t _mbclen(const unsigned char* c)
{
    if (!c || *c < 0x80)      return 1;
    if ((*c & 0xE0) == 0xC0)  return 2;
    if ((*c & 0xF0) == 0xE0)  return 3;
    if ((*c & 0xF8) == 0xF0)  return 4;
    return 1;
}

// Int -> wide string in the given radix (MSVC _itow). A leading '-' is emitted
// only for base 10, matching MSVC; the caller's buffer must be large enough.
inline wchar_t* _itow(int value, wchar_t* buffer, int radix)
{
    if (!buffer) return buffer;
    if (radix < 2 || radix > 36) { buffer[0] = L'\0'; return buffer; }

    const bool negative = (value < 0 && radix == 10);
    // Unsigned negation is well-defined and yields the correct magnitude even
    // for INT_MIN, unlike negating through a (possibly 32-bit) signed long.
    unsigned int v = negative ? -static_cast<unsigned int>(value)
                              : static_cast<unsigned int>(value);

    wchar_t digits[33];
    int n = 0;
    do {
        const unsigned int d = v % static_cast<unsigned int>(radix);
        digits[n++] = static_cast<wchar_t>(d < 10 ? L'0' + d : L'a' + (d - 10));
        v /= static_cast<unsigned int>(radix);
    } while (v != 0);

    int j = 0;
    if (negative) buffer[j++] = L'-';
    while (n > 0) buffer[j++] = digits[--n];
    buffer[j] = L'\0';
    return buffer;
}

// Case-insensitive, length-limited wide compare (MSVC _wcsnicmp/wcsnicmp).
inline int _wcsnicmp(const wchar_t* a, const wchar_t* b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        const wint_t ca = towlower(static_cast<wint_t>(a[i]));
        const wint_t cb = towlower(static_cast<wint_t>(b[i]));
        if (ca != cb) return (ca < cb) ? -1 : 1;
        if (a[i] == L'\0') break;
    }
    return 0;
}
inline int wcsnicmp(const wchar_t* a, const wchar_t* b, size_t n) { return _wcsnicmp(a, b, n); }

// Case-insensitive compares.
inline int _wcsicmp(const wchar_t* a, const wchar_t* b) { return wcscasecmp(a, b); }
inline int wcsicmp(const wchar_t* a, const wchar_t* b) { return wcscasecmp(a, b); }
inline int _stricmp(const char* a, const char* b) { return strcasecmp(a, b); }

// In-place wide upper-case.
inline wchar_t* _wcsupr(wchar_t* s)
{
    if (!s) return nullptr;
    for (wchar_t* p = s; *p; ++p) *p = static_cast<wchar_t>(towupper(*p));
    return s;
}

// Wide fopen: convert the path/mode to the locale encoding and open.
inline FILE* _wfopen(const wchar_t* path, const wchar_t* mode)
{
    if (!path || !mode) return nullptr;
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
    if (!path) return;
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
