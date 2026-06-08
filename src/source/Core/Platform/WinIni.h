// Portable .ini profile shim (issue #462, Phase 3).
//
// The config layer persists settings with the Win32 private-profile API. On
// Windows these come from <windows.h>; elsewhere this provides portable
// implementations (WinIni.cpp) that read/write a UTF-8 .ini file with the same
// contract (case-insensitive sections/keys; a null value deletes a key; a null
// key deletes a section; a zero buffer size returns the required length).
#pragma once

#ifdef _WIN32

// GetPrivateProfile*/WritePrivateProfile* come from <windows.h>.

#else  // ---- non-Windows ----------------------------------------------------

#include "Core/Platform/WinCompat.h"  // UINT, DWORD, INT, LPCWSTR, LPWSTR, BOOL

UINT  GetPrivateProfileIntW(LPCWSTR lpAppName, LPCWSTR lpKeyName, INT nDefault, LPCWSTR lpFileName);
DWORD GetPrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                               LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName);
BOOL  WritePrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString,
                                 LPCWSTR lpFileName);

#endif // _WIN32
