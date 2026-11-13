// Portable window and cursor shims.
//
// On Windows these come from <windows.h>; elsewhere the few window, cursor and
// display calls the engine still makes are backed by SDL or stubbed.
#pragma once

#ifdef _WIN32

// The real API is in <windows.h> (pulled in via the PCH on Windows).

#else  // ---- non-Windows ----------------------------------------------------

#include "Core/Platform/WinCompat.h" // HWND, UINT, LPCWSTR

// Window messaging. The engine sends one WM_IME_CONTROL (IME, which SDL owns off
// Windows), so this does nothing. Quits go through Core/Platform/Quit.h.
inline LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM)
{
    return 0;
}

// Window / cursor queries, backed by SDL. The engine reads the cursor as
// GetCursorPos followed by ScreenToClient(window); GetCursorPos already returns
// the position relative to the game window (in editor builds also while the
// mouse is in one of the editor's own windows), so ScreenToClient is the
// identity. SetCursorPos warps in the window the mouse is in. GetActiveWindow
// reports the focused window (so `== NULL` means the game is unfocused).
// Implemented in WinUser.cpp.
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
BOOL            SetCursorPos(int x, int y);
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
// ShowCursor keeps Win32 display-counter semantics (visible iff counter >= 0)
// and drives SDL's cursor to match, so the game's "hide the OS cursor, draw my
// own" calls actually take effect. Defined in WinUser.cpp (needs SDL).
int  ShowCursor(BOOL bShow);
// Re-apply the current cursor visibility to SDL. The engine hides the cursor
// before the SDL video subsystem exists; call this once the window is up so the
// pending state takes effect.
void MuApplyCursorVisibility();
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
