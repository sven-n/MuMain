// Portable <strsafe.h> shim (issue #462, Phase 3).
//
// The in-game shop code copies and formats wide strings through the strsafe
// Cch family. On Windows the real header is used; elsewhere this maps the used
// subset onto the bounded CRT shims. Counts are in characters, like strsafe.
#pragma once

#ifdef _WIN32

#include <strsafe.h>

#else  // ---- non-Windows ----------------------------------------------------

#include <cstdarg>
#include <cstring>   // strnlen
#include <cwchar>    // wcslen, wmemcpy, wcsnlen
#include "Core/Platform/WinCompat.h"   // HRESULT, S_OK
#include "Core/Platform/SecureCrt.h"   // _vsnwprintf

#ifndef STRSAFE_E_INSUFFICIENT_BUFFER
#define STRSAFE_E_INSUFFICIENT_BUFFER (static_cast<HRESULT>(0x8007007AL))
#endif
#ifndef STRSAFE_E_INVALID_PARAMETER
#define STRSAFE_E_INVALID_PARAMETER   (static_cast<HRESULT>(0x80070057L))
#endif

inline HRESULT StringCchCopyW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszSrc)
{
    if (!pszDest || !pszSrc || cchDest == 0) return STRSAFE_E_INVALID_PARAMETER;
    const size_t len = wcslen(pszSrc);
    if (len >= cchDest)
    {
        wmemcpy(pszDest, pszSrc, cchDest - 1);
        pszDest[cchDest - 1] = L'\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    wmemcpy(pszDest, pszSrc, len + 1);
    return S_OK;
}

inline HRESULT StringCchVPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, va_list args)
{
    if (!pszDest || !pszFormat || cchDest == 0) return STRSAFE_E_INVALID_PARAMETER;
    const int r = _vsnwprintf(pszDest, cchDest, pszFormat, args);
    if (r < 0 || static_cast<size_t>(r) >= cchDest)
    {
        // Always leave a null-terminated buffer, like the Win32 contract: on
        // truncation/encoding error vswprintf may not terminate within count.
        pszDest[cchDest - 1] = L'\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    return S_OK;
}

inline HRESULT StringCchPrintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, ...)
{
    va_list args;
    va_start(args, pszFormat);
    const HRESULT hr = StringCchVPrintfW(pszDest, cchDest, pszFormat, args);
    va_end(args);
    return hr;
}

inline HRESULT StringCchLengthW(const wchar_t* psz, size_t cchMax, size_t* pcchLength)
{
    if (!psz || cchMax == 0) return STRSAFE_E_INVALID_PARAMETER;
    const size_t len = wcsnlen(psz, cchMax);
    if (len >= cchMax) return STRSAFE_E_INVALID_PARAMETER;
    if (pcchLength) *pcchLength = len;
    return S_OK;
}

inline HRESULT StringCchLengthA(const char* psz, size_t cchMax, size_t* pcchLength)
{
    if (!psz || cchMax == 0) return STRSAFE_E_INVALID_PARAMETER;
    const size_t len = strnlen(psz, cchMax);
    if (len >= cchMax) return STRSAFE_E_INVALID_PARAMETER;
    if (pcchLength) *pcchLength = len;
    return S_OK;
}

// The project builds UNICODE, so the generic names map to the wide forms.
#ifndef StringCchCopy
#define StringCchCopy    StringCchCopyW
#define StringCchPrintf  StringCchPrintfW
#define StringCchVPrintf StringCchVPrintfW
#define StringCchLength  StringCchLengthW
#endif

#endif // _WIN32
