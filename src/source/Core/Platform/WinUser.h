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

// Window messaging. The engine only ever sends WM_DESTROY/WM_CLOSE (a request to
// quit) and one WM_IME_CONTROL (IME, which SDL owns on Linux). The portable
// versions turn the quit messages into an SDL quit and ignore the rest, so the
// main loop's existing SDL_EVENT_QUIT handling stops the game. Implemented in
// WinUser.cpp.
LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL    PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void    PostQuitMessage(int nExitCode);

// Window / cursor queries, backed by SDL. The engine reads the cursor as
// GetCursorPos followed by ScreenToClient(window); SDL_GetMouseState already
// returns window-relative coordinates, so GetCursorPos yields those directly and
// ScreenToClient is the identity. GetActiveWindow reports the focused window (so
// `== NULL` means the game is unfocused). Implemented in WinUser.cpp.
BOOL GetCursorPos(LPPOINT lpPoint);
BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint);
HWND GetActiveWindow();
UINT GetDoubleClickTime();

// The keyboard-focus window, or null when the game is unfocused. The engine
// compares this against the main window (g_hWnd) to ask "is the game focused?",
// so it returns g_hWnd while the SDL window holds focus.
HWND GetFocus();

#endif // _WIN32
