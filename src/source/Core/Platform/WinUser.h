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

// ---- Legacy Win32 window / display management --------------------------------
// SDL owns the window, GL context and resolution on Linux, so the engine's
// legacy Win32 window/resolution code is stubbed: these no-ops let it compile
// (and report success) while the real work happens through SDL elsewhere.

// Display settings (winuser.h / wingdi.h). Only the fields the resolution code
// touches are provided; layout is irrelevant since ChangeDisplaySettings is a
// no-op here.
typedef struct _devicemodeW {
    WCHAR dmDeviceName[32];
    WORD  dmSpecVersion, dmDriverVersion, dmSize, dmDriverExtra;
    DWORD dmFields;
    LONG  dmPositionX, dmPositionY;
    DWORD dmDisplayOrientation, dmDisplayFixedOutput;
    WORD  dmColor, dmDuplex, dmYResolution, dmTTOption, dmCollate;
    WCHAR dmFormName[32];
    WORD  dmLogPixels;
    DWORD dmBitsPerPel, dmPelsWidth, dmPelsHeight, dmDisplayFlags, dmDisplayFrequency;
} DEVMODEW, DEVMODE, * LPDEVMODE, * PDEVMODE;

typedef struct tagMSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD  time;
    POINT  pt;
} MSG, * LPMSG, * PMSG;

inline HWND     SetFocus(HWND hWnd)                                  { return hWnd; }
inline BOOL     SetForegroundWindow(HWND)                            { return TRUE; }
inline BOOL     SetWindowPos(HWND, HWND, int, int, int, int, UINT)   { return TRUE; }
inline LONG_PTR SetWindowLongPtr(HWND, int, LONG_PTR)                { return 0; }
inline BOOL     SetCursorPos(int, int)                              { return TRUE; }
inline BOOL     AdjustWindowRect(LPRECT, DWORD, BOOL)               { return TRUE; }
inline BOOL     ShowWindow(HWND, int)                              { return TRUE; }
inline LONG     ChangeDisplaySettings(DEVMODE*, DWORD)             { return DISP_CHANGE_SUCCESSFUL; }

// The legacy code pumps the Win32 queue after a resolution change; SDL drives
// the real loop, so there is never anything to peek here.
inline BOOL    PeekMessage(LPMSG, HWND, UINT, UINT, UINT)          { return FALSE; }
inline BOOL    TranslateMessage(const MSG*)                        { return FALSE; }
inline LRESULT DispatchMessage(const MSG*)                         { return 0; }

inline int     FillRect(HDC, const RECT*, HBRUSH)                  { return 1; }

// Cursor / capture / client-rect helpers -- SDL owns the cursor and input.
inline int   ShowCursor(BOOL)                     { return 0; }
inline HWND  SetCapture(HWND)                      { return nullptr; }
inline BOOL  ReleaseCapture()                      { return TRUE; }
inline BOOL  ClipCursor(const RECT*)               { return TRUE; }
inline BOOL  GetClientRect(HWND, LPRECT lpRect)    { if (lpRect) { lpRect->left = lpRect->top = lpRect->right = lpRect->bottom = 0; } return TRUE; }
inline BOOL  ClientToScreen(HWND, LPPOINT)         { return TRUE; }
inline HWND  FindWindowW(LPCWSTR, LPCWSTR)         { return nullptr; }
#ifndef FindWindow
#define FindWindow FindWindowW
#endif
inline HDC   GetDC(HWND)                           { return nullptr; }
inline int   ReleaseDC(HWND, HDC)                  { return 1; }

// Display enumeration (winuser.h). No legacy enumeration off Windows; SDL owns
// the modes. ENUM_CURRENT_SETTINGS-style queries report failure.
#ifndef ENUM_CURRENT_SETTINGS
#define ENUM_CURRENT_SETTINGS (static_cast<DWORD>(-1))
#endif
inline BOOL  EnumDisplaySettingsW(LPCWSTR, DWORD, DEVMODE*) { return FALSE; }
#ifndef EnumDisplaySettings
#define EnumDisplaySettings EnumDisplaySettingsW
#endif
inline LPWSTR  GetCommandLineW()                                   { static wchar_t empty[1] = { 0 }; return empty; }
#ifndef GetCommandLine
#define GetCommandLine GetCommandLineW
#endif

// Open a document/URL. Returns a "succeeded" sentinel (> 32, as Win32 does).
inline HINSTANCE ShellExecuteW(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, int)
{
    return reinterpret_cast<HINSTANCE>(33);
}
#ifndef ShellExecute
#define ShellExecute ShellExecuteW
#endif

#endif // _WIN32
