// Portable shims for the MSVC "secure CRT" (_s) functions the engine uses
// (issue #462, Phase 1). On Windows the real CRT provides these, so this header
// is empty there; elsewhere it maps them onto the standard C library.
//
// Each function is provided in both the explicit-count form
// (`f(buf, count, ...)`) and the MSVC template form that deduces the size from
// a stack array (`f(buf, ...)`), so existing call sites compile unchanged.
#pragma once

#ifndef _WIN32

#include <cstdio>
#include <cstdarg>
#include <cwchar>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstddef>

typedef int errno_t;

#ifndef _TRUNCATE
#define _TRUNCATE (static_cast<size_t>(-1))
#endif

// EINVAL / ERANGE without pulling Windows error codes.
inline errno_t mu__einval() { return EINVAL; }
inline errno_t mu__erange() { return ERANGE; }

// ---- wcscpy_s ----------------------------------------------------------------
inline errno_t wcscpy_s(wchar_t* dst, size_t count, const wchar_t* src)
{
    if (dst == nullptr || src == nullptr || count == 0) return mu__einval();
    const size_t len = wcslen(src);
    if (len + 1 > count) { dst[0] = L'\0'; return mu__erange(); }
    wmemcpy(dst, src, len + 1);
    return 0;
}
template <size_t N> inline errno_t wcscpy_s(wchar_t (&dst)[N], const wchar_t* src) { return wcscpy_s(dst, N, src); }

// ---- wcsncpy_s (supports _TRUNCATE) -----------------------------------------
inline errno_t wcsncpy_s(wchar_t* dst, size_t count, const wchar_t* src, size_t n)
{
    if (dst == nullptr || count == 0) return mu__einval();
    if (src == nullptr) { dst[0] = L'\0'; return (n == 0) ? 0 : mu__einval(); }
    const size_t srclen = wcslen(src);
    size_t tocopy = (n == _TRUNCATE) ? srclen : (n < srclen ? n : srclen);
    if (tocopy + 1 > count)
    {
        if (n == _TRUNCATE) { tocopy = count - 1; }   // truncation explicitly allowed
        else { dst[0] = L'\0'; return mu__erange(); }
    }
    wmemcpy(dst, src, tocopy);
    dst[tocopy] = L'\0';
    return 0;
}
template <size_t N> inline errno_t wcsncpy_s(wchar_t (&dst)[N], const wchar_t* src, size_t n) { return wcsncpy_s(dst, N, src, n); }

// ---- wcscat_s ----------------------------------------------------------------
inline errno_t wcscat_s(wchar_t* dst, size_t count, const wchar_t* src)
{
    if (dst == nullptr || src == nullptr || count == 0) return mu__einval();
    const size_t dlen = wcsnlen(dst, count);
    if (dlen == count) return mu__einval();   // not null-terminated
    const size_t slen = wcslen(src);
    if (dlen + slen + 1 > count) { dst[0] = L'\0'; return mu__erange(); }
    wmemcpy(dst + dlen, src, slen + 1);
    return 0;
}
template <size_t N> inline errno_t wcscat_s(wchar_t (&dst)[N], const wchar_t* src) { return wcscat_s(dst, N, src); }

// ---- swprintf_s --------------------------------------------------------------
inline int swprintf_s(wchar_t* buf, size_t count, const wchar_t* fmt, ...)
{
    va_list a; va_start(a, fmt);
    const int r = vswprintf(buf, count, fmt, a);
    va_end(a);
    return r;
}
template <size_t N> inline int swprintf_s(wchar_t (&buf)[N], const wchar_t* fmt, ...)
{
    va_list a; va_start(a, fmt);
    const int r = vswprintf(buf, N, fmt, a);
    va_end(a);
    return r;
}

// ---- _snwprintf_s(buf, count, maxcount, fmt, ...) ---------------------------
inline int _snwprintf_s(wchar_t* buf, size_t count, size_t maxcount, const wchar_t* fmt, ...)
{
    va_list a; va_start(a, fmt);
    const size_t lim = (maxcount == _TRUNCATE || maxcount + 1 > count) ? count : maxcount + 1;
    const int r = vswprintf(buf, lim, fmt, a);
    va_end(a);
    return r;
}
template <size_t N> inline int _snwprintf_s(wchar_t (&buf)[N], size_t maxcount, const wchar_t* fmt, ...)
{
    va_list a; va_start(a, fmt);
    const size_t lim = (maxcount == _TRUNCATE || maxcount + 1 > N) ? N : maxcount + 1;
    const int r = vswprintf(buf, lim, fmt, a);
    va_end(a);
    return r;
}

// ---- sprintf_s (narrow) ------------------------------------------------------
inline int sprintf_s(char* buf, size_t count, const char* fmt, ...)
{
    va_list a; va_start(a, fmt);
    const int r = vsnprintf(buf, count, fmt, a);
    va_end(a);
    return r;
}
template <size_t N> inline int sprintf_s(char (&buf)[N], const char* fmt, ...)
{
    va_list a; va_start(a, fmt);
    const int r = vsnprintf(buf, N, fmt, a);
    va_end(a);
    return r;
}

// ---- wcstok_s -> C11/glibc reentrant wcstok ---------------------------------
inline wchar_t* wcstok_s(wchar_t* str, const wchar_t* delim, wchar_t** context)
{
    return wcstok(str, delim, context);
}

// ---- _wfopen_s ---------------------------------------------------------------
inline errno_t _wfopen_s(FILE** pFile, const wchar_t* path, const wchar_t* mode)
{
    if (pFile == nullptr) return mu__einval();
    char narrowPath[1024] = { 0 };
    char narrowMode[16] = { 0 };
    wcstombs(narrowPath, path, sizeof(narrowPath) - 1);
    wcstombs(narrowMode, mode, sizeof(narrowMode) - 1);
    *pFile = fopen(narrowPath, narrowMode);
    return (*pFile != nullptr) ? 0 : errno;
}

#endif // !_WIN32
