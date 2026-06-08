// Portable code-page conversion shim (issue #462, Phase 3).
//
// The engine converts between UTF-8 (narrow) and UTF-16/UTF-32 (wide) with the
// Win32 MultiByteToWideChar / WideCharToMultiByte. On Windows these come from
// <windows.h>; elsewhere this provides portable implementations (WinNls.cpp).
//
// Only the CP_UTF8 path the engine uses is meaningful; CP_ACP is treated as
// UTF-8 too (the standard Linux locale). dwFlags and the default-char arguments
// are accepted for signature compatibility and otherwise ignored.
#pragma once

#ifdef _WIN32

// MultiByteToWideChar / WideCharToMultiByte come from <windows.h>.

#else  // ---- non-Windows ----------------------------------------------------

#include "Core/Platform/WinCompat.h"  // UINT, DWORD, LPCSTR, LPWSTR, LPBOOL

#ifndef CP_ACP
#define CP_ACP  0
#endif
#ifndef CP_UTF8
#define CP_UTF8 65001
#endif

// Same contract as Win32: cbMultiByte/cchWideChar == -1 means the source is
// null-terminated (the terminator is converted and counted); a zero output size
// returns the required length without writing. Invalid input maps to U+FFFD.
int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr,
                        int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);

int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr,
                        int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
                        LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);

#endif // _WIN32
