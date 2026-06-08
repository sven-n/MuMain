// Portable MessageBox shim (issue #462, Phase 3).
//
// On Windows MessageBox comes from <windows.h>; elsewhere we route it through
// SDL's message box so the engine's error/info dialogs work on Linux. Only the
// MessageBox surface the engine actually uses is provided.
#pragma once

#ifdef _WIN32

// Real MessageBox is in <windows.h> (pulled in via the PCH on Windows).

#else  // ---- non-Windows ----------------------------------------------------

#include "Core/Platform/WinCompat.h"  // HWND, UINT, LPCWSTR, MB_* flags

// MessageBox return values (winuser.h).
#ifndef IDOK
#define IDOK     1
#define IDCANCEL 2
#define IDABORT  3
#define IDRETRY  4
#define IDIGNORE 5
#define IDYES    6
#define IDNO     7
#endif

// Shows a modal dialog and returns one of the ID* values above. Implemented in
// WinUser.cpp on top of SDL_ShowMessageBox.
int MessageBoxW(HWND owner, LPCWSTR text, LPCWSTR caption, UINT type);

// The engine builds UNICODE, so unqualified MessageBox maps to the wide form
// (exactly as <winuser.h> does on Windows).
#ifndef MessageBox
#define MessageBox MessageBoxW
#endif

#endif // _WIN32
