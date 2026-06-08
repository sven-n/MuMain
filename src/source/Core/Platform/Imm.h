// Portable IMM (Windows IME) shim (issue #462, Phase 3).
//
// On Windows the IME is driven through <imm.h>. On Linux SDL owns IME (the
// portable text field, #447), so the legacy Win32 IMM calls become no-ops: there
// is no IMM context to fetch, and conversion-mode control belongs to the platform
// IME. These stubs just let the legacy IME code paths compile.
#pragma once

#ifdef _WIN32

#include <imm.h>

#else  // ---- non-Windows ----------------------------------------------------

#include "Core/Platform/WinCompat.h"  // HWND, HIMC, DWORD, LPDWORD, POINT, RECT, BOOL

// Composition-window form + conversion-mode flags (imm.h).
#ifndef CFS_POINT
#define CFS_POINT 0x0002
#endif
#ifndef IMC_SETCOMPOSITIONWINDOW
#define IMC_SETCOMPOSITIONWINDOW 0x000B
#endif
#ifndef IME_CMODE_ALPHANUMERIC
#define IME_CMODE_ALPHANUMERIC 0x0000
#endif
#ifndef IME_CMODE_NATIVE
#define IME_CMODE_NATIVE 0x0001
#endif
#ifndef IME_SMODE_NONE
#define IME_SMODE_NONE 0x0000
#endif
#ifndef IME_SMODE_AUTOMATIC
#define IME_SMODE_AUTOMATIC 0x0004
#endif

typedef struct tagCOMPOSITIONFORM {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} COMPOSITIONFORM, * LPCOMPOSITIONFORM;

// No IMM context exists off Windows; SDL handles composition and conversion.
inline HIMC ImmGetContext(HWND)              { return nullptr; }
inline BOOL ImmReleaseContext(HWND, HIMC)    { return TRUE; }
inline HWND ImmGetDefaultIMEWnd(HWND)        { return nullptr; }

inline BOOL ImmGetConversionStatus(HIMC, LPDWORD lpfdwConversion, LPDWORD lpfdwSentence)
{
    if (lpfdwConversion) *lpfdwConversion = 0;
    if (lpfdwSentence)   *lpfdwSentence = 0;
    return TRUE;
}
inline BOOL ImmSetConversionStatus(HIMC, DWORD, DWORD) { return TRUE; }

#endif // _WIN32
