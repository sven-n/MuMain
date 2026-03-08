#pragma once

#ifdef _WIN32
// On Windows, all functions come from Win32 APIs.
// Include <string> and <filesystem> explicitly for utilities below.
#include <filesystem>
#include <string>

// ---- Executable directory utility (Windows) ----
// mu_get_app_dir() — portable companion to the non-Windows shim in the #else branch.
// Returns the directory containing the running executable as a std::filesystem::path.
// GameConfig.cpp calls this instead of GetModuleFileNameW directly, so that GameConfig.cpp
// has no #ifdef _WIN32 in game logic. [VS1-NET-CONFIG-SERVER]
inline std::filesystem::path mu_get_app_dir()
{
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    // len == 0: API failure; len == MAX_PATH: path was truncated (no null terminator guaranteed).
    // In either case fall back to current working directory, matching the non-Windows shims.
    if (len == 0 || len == MAX_PATH)
    {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(buf).parent_path();
}

#else

#include "PlatformTypes.h"

#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

// ---- Timing shims ----

inline BOOL IsBadReadPtr(const void* /*lp*/, UINT_PTR /*ucb*/)
{
    return FALSE;
}

inline BOOL IsBadWritePtr(void* /*lp*/, UINT_PTR /*ucb*/)
{
    return FALSE;
}

#define STD_INPUT_HANDLE ((DWORD) - 10)
#define STD_OUTPUT_HANDLE ((DWORD) - 11)
#define STD_ERROR_HANDLE ((DWORD) - 12)

#define ENABLE_PROCESSED_INPUT 0x0001
#define ENABLE_LINE_INPUT 0x0002
#define ENABLE_ECHO_INPUT 0x0004
#define ENABLE_WINDOW_INPUT 0x0008
#define ENABLE_MOUSE_INPUT 0x0010
#define ENABLE_INSERT_MODE 0x0020
#define ENABLE_QUICK_EDIT_MODE 0x0040
#define ENABLE_EXTENDED_FLAGS 0x0080

#define ENABLE_PROCESSED_OUTPUT 0x0001
#define ENABLE_WRAP_AT_EOL_OUTPUT 0x0002
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004

struct COORD
{
    short X;
    short Y;
};

struct SMALL_RECT
{
    short Left;
    short Top;
    short Right;
    short Bottom;
};

struct CONSOLE_SCREEN_BUFFER_INFO
{
    COORD dwSize;
    COORD dwCursorPosition;
    WORD wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
};

inline HANDLE GetStdHandle(DWORD /*nStdHandle*/)
{
    return (HANDLE)-1;
}

inline BOOL SetConsoleMode(HANDLE /*hConsoleHandle*/, DWORD /*dwMode*/)
{
    return TRUE;
}

inline BOOL AllocConsole()
{
    return TRUE;
}

inline BOOL FreeConsole()
{
    return TRUE;
}

inline void Sleep(DWORD /*dwMilliseconds*/)
{
    // Implementation can use std::this_thread::sleep_for if needed
}

inline BOOL IsWindowVisible(HWND /*hWnd*/)
{
    return FALSE;
}

inline BOOL EnumChildWindows(HWND /*hWndParent*/, void* /*lpEnumFunc*/, LPARAM /*lParam*/)
{
    return TRUE;
}

inline void SetLastError(DWORD /*dwErrCode*/) {}
inline DWORD GetLastError()
{
    return 0;
}
inline BOOL UpdateWindow(HWND /*hWnd*/)
{
    return TRUE;
}
inline BOOL GetConsoleScreenBufferInfo(HANDLE /*hConsoleOutput*/,
                                       CONSOLE_SCREEN_BUFFER_INFO* /*lpConsoleScreenBufferInfo*/)
{
    return TRUE;
}
inline BOOL FillConsoleOutputCharacter(HANDLE /*hConsoleOutput*/, TCHAR /*cCharacter*/, DWORD /*nLength*/,
                                       COORD /*dwWriteCoord*/, LPDWORD /*lpNumberOfCharsWritten*/)
{
    return TRUE;
}
inline BOOL FillConsoleOutputAttribute(HANDLE /*hConsoleOutput*/, WORD /*wAttribute*/, DWORD /*nLength*/,
                                       COORD /*dwWriteCoord*/, LPDWORD /*lpNumberOfAttrsWritten*/)
{
    return TRUE;
}
inline BOOL SetConsoleCursorPosition(HANDLE /*hConsoleOutput*/, COORD /*dwCursorPosition*/)
{
    return TRUE;
}
inline BOOL SetConsoleTextAttribute(HANDLE /*hConsoleOutput*/, WORD /*wAttributes*/)
{
    return TRUE;
}
inline void* GetSystemMenu(HWND /*hWnd*/, BOOL /*bRevert*/)
{
    return nullptr;
}
inline BOOL DeleteMenu(void* /*hMenu*/, UINT /*uPosition*/, UINT /*uFlags*/)
{
    return TRUE;
}
inline BOOL DrawMenuBar(HWND /*hWnd*/)
{
    return TRUE;
}

#define SC_CLOSE 0xF060
#define MF_BYCOMMAND 0x00000000L

inline uint32_t timeGetTime()
{
    using namespace std::chrono;
    return static_cast<uint32_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

inline uint32_t GetTickCount()
{
    return timeGetTime();
}

// ---- Text measurement stub ----
// GetTextExtentPoint32 stub — used extensively in UIControls.cpp and NewUIChatInputBox.cpp
// for text measurement (CutStr, tooltip sizing). On non-Windows the font DC is nullptr;
// return a fixed estimate so layout code does not crash.
// Phase 4 (font system migration introduces IPlatformFont::MeasureText) will replace this.
inline BOOL GetTextExtentPoint32(HDC /*hDC*/, const wchar_t* pszText, int cch, SIZE* lpSize)
{
    // Estimate: 8px per character width, 16px height — acceptable fallback until Phase 4.
    if (lpSize != nullptr)
    {
        lpSize->cx = (cch > 0 ? cch : 0) * 8;
        lpSize->cy = 16;
    }
    return TRUE;
}

// lstrlen shim — alias to wcslen — used in UIControls.cpp GetTextExtentPoint32 call sites
#ifndef lstrlen
inline int lstrlen(const wchar_t* s)
{
    return s ? static_cast<int>(wcslen(s)) : 0;
}
#endif

// ---- Win32 IME stubs (non-Windows only) ----
// Used in UIControls.cpp: SaveIMEStatus, RestoreIMEStatus, CheckTextInputBoxIME
// On SDL3 path these are all no-ops — IME handled by SDL3 internally.
using HIMC = void*;
#define IME_CMODE_ALPHANUMERIC 0x0000
#define IME_SMODE_NONE 0x0000
#define IME_CONVERSIONMODE 1
#define IME_SENTENCEMODE 2
#define IMN_SETOPENSTATUS 0x0005
#define IMN_SETCONVERSIONMODE 0x0006
#define IMN_SETSENTENCEMODE 0x0007
inline HIMC ImmGetContext(HWND /*hwnd*/)
{
    return nullptr;
}
inline BOOL ImmGetConversionStatus(HIMC /*himc*/, DWORD* pdwConv, DWORD* pdwSent)
{
    if (pdwConv)
    {
        *pdwConv = IME_CMODE_ALPHANUMERIC;
    }
    if (pdwSent)
    {
        *pdwSent = IME_SMODE_NONE;
    }
    return TRUE;
}
inline BOOL ImmSetConversionStatus(HIMC /*himc*/, DWORD /*dwConv*/, DWORD /*dwSent*/)
{
    return TRUE;
}
inline BOOL ImmReleaseContext(HWND /*hwnd*/, HIMC /*himc*/)
{
    return TRUE;
}

// ImmGetCompositionWindow / ImmSetCompositionWindow — used in CUITextInputBox::SetIMEPosition()
// Forward-declare COMPOSITIONFORM (defined below with window message stubs)
struct COMPOSITIONFORM;
inline BOOL ImmGetCompositionWindow(HIMC /*himc*/, COMPOSITIONFORM* /*lpCompForm*/)
{
    return FALSE;
}
inline BOOL ImmSetCompositionWindow(HIMC /*himc*/, COMPOSITIONFORM* /*lpCompForm*/)
{
    return FALSE;
}

// ---- Win32 window message stubs (non-Windows only) ----
// Used in UIControls.cpp: CUITextInputBox::SetIMEPosition, GiveFocus, SetState
using UINT = unsigned int;
#define WM_IME_CONTROL 0x0283
#define IMC_SETCOMPOSITIONWINDOW 0x000C
inline LRESULT SendMessage(HWND /*hwnd*/, UINT /*msg*/, WPARAM /*wp*/, LPARAM /*lp*/)
{
    return 0;
}
inline LRESULT SendMessageW(HWND /*hwnd*/, UINT /*msg*/, WPARAM /*wp*/, LPARAM /*lp*/)
{
    return 0;
}
// NOLINT — PostMessage is intentional no-op for non-Windows builds
inline LRESULT PostMessage(HWND /*hwnd*/, UINT /*msg*/, WPARAM /*wp*/, LPARAM /*lp*/)
{
    return 0;
} // NOLINT
inline LRESULT PostMessageW(HWND /*hwnd*/, UINT /*msg*/, WPARAM /*wp*/, LPARAM /*lp*/)
{
    return 0;
}
// EM_SETSEL: edit control message — no-op on SDL3 path
#define EM_SETSEL 0x00B1
#define EM_LIMITTEXT 0x00C5
#define EM_SETLIMITTEXT 0x00C5
#define EM_GETLINECOUNT 0x00BA
#define EM_SCROLL 0x00B5
#define EM_LINESCROLL 0x00B6
#define SB_LINEUP 0
#define SB_LINEDOWN 1
#define SB_PAGEUP 2
#define SB_PAGEDOWN 3
#define WM_SETFONT 0x0030

// SetFocus / GetFocus stubs — UIControls.cpp uses these in CUITextInputBox::GiveFocus
// On SDL3 path focus is managed by SDLEventLoop; returning a sentinel avoids null dereferences.
inline HWND SetFocus(HWND /*hwnd*/)
{
    return reinterpret_cast<HWND>(static_cast<uintptr_t>(1));
}
inline HWND GetFocus()
{
    return reinterpret_cast<HWND>(static_cast<uintptr_t>(1));
}

// ShowWindow stub — used in CUITextInputBox::SetState (Win32 path shows/hides the edit HWND)
#define SW_HIDE 0
#define SW_SHOW 5
inline BOOL ShowWindow(HWND /*hwnd*/, int /*nCmdShow*/)
{
    return TRUE;
}

// DestroyWindow stub — used in CUITextInputBox destructor and SetSize failure path
inline BOOL DestroyWindow(HWND /*hwnd*/)
{
    return TRUE;
}

// GetDC / ReleaseDC stubs — used in CUITextInputBox::SetSize for GDI DIBSection creation
inline HDC GetDC(HWND /*hwnd*/)
{
    return nullptr;
}
inline int ReleaseDC(HWND /*hwnd*/, HDC /*hdc*/)
{
    return 1;
}

// SetWindowPos stub — used in CUITextInputBox::SetSize to reposition the Win32 edit control
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
inline BOOL SetWindowPos(HWND /*hwnd*/, HWND /*hwndInsertAfter*/, int /*x*/, int /*y*/, int /*cx*/, int /*cy*/,
                         UINT /*flags*/)
{
    return TRUE;
}

// GetCaretPos stub — used in CUITextInputBox::WriteText and SetIMEPosition
inline BOOL GetCaretPos(POINT* lpPoint)
{
    if (lpPoint != nullptr)
    {
        lpPoint->x = 0;
        lpPoint->y = 0;
    }
    return TRUE;
}

// GetWindowText / GetWindowTextW stub — used in CUITextInputBox::GetText (Win32 path reads
// text from the edit HWND). On SDL3 path GetText is guarded by #ifdef _WIN32.
inline int GetWindowText(HWND /*hwnd*/, wchar_t* lpString, int nMaxCount)
{
    if (lpString != nullptr && nMaxCount > 0)
    {
        lpString[0] = L'\0';
    }
    return 0;
}
#define GetWindowTextW GetWindowText

// SetWindowTextW stub — used in CUITextInputBox Render path (SetWindowTextW(m_hEditWnd, ...))
inline BOOL SetWindowTextW(HWND /*hwnd*/, const wchar_t* /*lpString*/)
{
    return TRUE;
}

// GetWindowRect stub — used in UIControls render/scroll paths
inline BOOL GetWindowRect(HWND /*hwnd*/, RECT* lpRect)
{
    if (lpRect != nullptr)
    {
        lpRect->left = 0;
        lpRect->top = 0;
        lpRect->right = 0;
        lpRect->bottom = 0;
    }
    return TRUE;
}

// WNDPROC type alias and SetWindowLongPtrW / GetWindowLongPtrW / CallWindowProcW stubs
// Used in UIControls.cpp:CUITextInputBox::Init (subclasses the Win32 edit control)
using WNDPROC = LRESULT (*)(HWND, UINT, WPARAM, LPARAM);
#define GWLP_WNDPROC (-4)
#define GWLP_USERDATA (-21)
using LONG_PTR = intptr_t;
inline LONG_PTR SetWindowLongPtrW(HWND /*hwnd*/, int /*nIndex*/, LONG_PTR /*dwNewLong*/)
{
    return 0;
}
inline LONG_PTR GetWindowLongPtrW(HWND /*hwnd*/, int /*nIndex*/)
{
    return 0;
}
inline LRESULT CallWindowProcW(WNDPROC /*proc*/, HWND /*hwnd*/, UINT /*msg*/, WPARAM /*wp*/, LPARAM /*lp*/)
{
    return 0;
}

// CreateWindowW stub — on SDL3 path CUITextInputBox::Init calls CreateWindowW(L"edit", ...)
// to create a Win32 edit control. The stub returns nullptr so all if(m_hEditWnd) guards
// prevent Win32 edit control operations. SDL3 text input path bypasses the edit HWND entirely.
#define WS_CHILD 0x40000000L
#define WS_VISIBLE 0x10000000L
#define WS_VSCROLL 0x00200000L
#define ES_AUTOHSCROLL 0x0080L
#define ES_AUTOVSCROLL 0x0040L
#define ES_MULTILINE 0x0004L
#define ES_PASSWORD 0x0020L
using HMENU = void*;
inline HWND CreateWindowW(const wchar_t* /*cls*/, const wchar_t* /*wndName*/, DWORD /*style*/, int /*x*/, int /*y*/,
                          int /*w*/, int /*h*/, HWND /*parent*/, HMENU /*menu*/, HINSTANCE /*inst*/, void* /*param*/)
{
    return nullptr; // SDL3 path: no Win32 edit control — text handled via SDL_EVENT_TEXT_INPUT
}

// GDI stubs — used in CUIRenderTextOriginal::Create/Release (GDI font rendering).
// These are compilation stubs for Phase 4 (font system migration). On non-Windows they
// are safe no-ops that prevent crashes until rendering is migrated.
using HBITMAP = void*;
using HGDIOBJ = void*;
struct BITMAPINFOHEADER
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
struct PALETTEENTRY
{
    uint8_t peRed, peGreen, peBlue, peFlags;
};
struct RGBQUAD
{
    uint8_t rgbBlue, rgbGreen, rgbRed, rgbReserved;
};
struct BITMAPINFO
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
};
#define BI_RGB 0
#define DIB_RGB_COLORS 0
inline BOOL DeleteDC(HDC /*hdc*/)
{
    return TRUE;
}
inline BOOL DeleteObject(HGDIOBJ /*obj*/)
{
    return TRUE;
}
inline HDC CreateCompatibleDC(HDC /*hdc*/)
{
    return nullptr;
}
inline HBITMAP CreateDIBSection(HDC /*hdc*/, const BITMAPINFO* /*bmi*/, UINT /*usage*/, void** ppvBits,
                                HANDLE /*hSection*/, DWORD /*offset*/)
{
    if (ppvBits != nullptr)
    {
        *ppvBits = nullptr;
    }
    return nullptr;
}
inline HGDIOBJ SelectObject(HDC /*hdc*/, HGDIOBJ /*obj*/)
{
    return nullptr;
}

// HFONT stub — used in UIControls.h CUITextInputBox::SetFont (HFONT hFont parameter)
// HFONT is already defined in PlatformTypes.h as void* — no re-definition needed.

// Clipboard stubs — used in UIControls.cpp:ClipboardCheck() for numeric paste validation.
// On SDL3 path, clipboard comes from SDL_GetClipboardText() (see MuClipboardIsNumericOnly).
// Win32 clipboard functions are stubbed to safe no-ops so ClipboardCheck compiles.
using HGLOBAL = void*;
#define CF_TEXT 1
inline BOOL OpenClipboard(HWND /*hwnd*/)
{
    return FALSE;
} // SDL3 path uses SDL_GetClipboardText
inline HGLOBAL GetClipboardData(UINT /*uFormat*/)
{
    return nullptr;
}
inline void* GlobalLock(HGLOBAL /*hMem*/)
{
    return nullptr;
}
inline BOOL GlobalUnlock(HGLOBAL /*hMem*/)
{
    return TRUE;
}
inline BOOL CloseClipboard()
{
    return TRUE;
}

// Win32 message constants needed by EditWndProc switch statement in UIControls.cpp.
// EditWndProc is never called on the SDL3 path (no Win32 edit HWND), but must compile.
#define WM_CHAR 0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_IME_COMPOSITION 0x010F
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION 0x010E
#define WM_IME_NOTIFY 0x0282

// COMPOSITIONFORM struct — used by ZzzInterface.cpp:263 SendMessage WM_IME_CONTROL call
// and CUITextInputBox::SetIMEPosition. With SendMessage shimmed to a no-op, the code
// compiles and does nothing on non-Windows.
#define CFS_POINT 0x0002
#define CFS_FORCE_POSITION 0x0020
struct COMPOSITIONFORM
{
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT rcArea;
};

// VK_ESCAPE and VK_RETURN — used in EditWndProc WM_CHAR handler
#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif
#ifndef VK_RETURN
#define VK_RETURN 0x0D
#endif
#ifndef VK_BACK
#define VK_BACK 0x08
#endif
#ifndef VK_TAB
#define VK_TAB 0x09
#endif
#ifndef VK_UP
#define VK_UP 0x26
#endif
#ifndef VK_DOWN
#define VK_DOWN 0x28
#endif
#ifndef VK_LEFT
#define VK_LEFT 0x25
#endif
#ifndef VK_RIGHT
#define VK_RIGHT 0x27
#endif
#ifndef VK_F5
#define VK_F5 0x74
#endif

// LPSTR type alias — used in ClipboardCheck (GlobalLock returns LPSTR)
using LPSTR = char*;

// ---- MessageBoxW shim ----
// MessageBox result constants are in PlatformTypes.h
#define MB_ICONINFORMATION 0x40

// Return values
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

// ---- Mouse cursor and position stubs (non-Windows, no SDL3 dependency) ----

inline DWORD GetDoubleClickTime()
{
    // 500ms is the standard double-click interval on Windows.
    // SDL3 does not expose a platform double-click time API.
    return 500u;
}

inline void GetCursorPos(POINT* ppt)
{
    // On SDL3 path, mouse position is maintained in MouseX/MouseY globals
    // via SDL_EVENT_MOUSE_MOTION. CInput::Update() calls GetCursorPos()
    // but the CInput cursor position system is superseded by the global
    // mouse state populated by SDLEventLoop.
    if (ppt != nullptr)
    {
        ppt->x = 0;
        ppt->y = 0;
    }
}

inline void ScreenToClient(HWND /*hwnd*/, POINT* /*ppt*/)
{
    // No-op on SDL3 path: mouse coordinates from SDL_EVENT_MOUSE_MOTION
    // are already window-relative. CInput::Update() calls ScreenToClient()
    // but on the SDL3 path cursor position is fed directly via SDL events
    // into MouseX/MouseY — the CInput position is not used for gameplay.
}

inline HWND GetActiveWindow()
{
    // On SDL3 path, focus state is managed by SDLEventLoop via g_bWndActive.
    // Return non-null so CInput::Update() does not zero out cursor state.
    // Use address of static sentinel to avoid integer-to-pointer cast warnings.
    static int s_activeWindowSentinel = 0;
    return static_cast<void*>(&s_activeWindowSentinel);
}

#ifdef MU_ENABLE_SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <cstdint>

// ---- SDL text input buffer externals (SDL3 path) ----
// g_szSDLTextInput: UTF-8 encoded character(s) from keyboard/IME, populated each frame
//   when SDL_EVENT_TEXT_INPUT fires. Defined in SDLKeyboardState.cpp.
// g_bSDLTextInputReady: true for one frame when new text is available.
// [VS1-SDL-INPUT-TEXT]
extern char g_szSDLTextInput[32];
extern bool g_bSDLTextInputReady;

// MuStartTextInput / MuStopTextInput — SDL3 text input lifecycle.
// Implementations in SDLKeyboardState.cpp (compiled with project PCH for g_ErrorReport).
// Declared here so CUITextInputBox::GiveFocus / SetState can call them without
// including SDL3 headers directly. [VS1-SDL-INPUT-TEXT]
void MuStartTextInput();
void MuStopTextInput();

// ---- UTF-8 to wchar_t decoder (SDL3 path) ----
// Decode one UTF-8 codepoint from src, advance src past it.
// Returns the wchar_t (BMP codepoint U+0000-U+FFFF) or L'\0' on error/end-of-string.
// MU Online text is BMP-only (U+0000-U+FFFF) — wchar_t is safe on all target platforms.
// [VS1-SDL-INPUT-TEXT]
inline wchar_t MuSdlUtf8NextChar(const char*& src)
{
    auto byte = static_cast<unsigned char>(*src);
    if (byte == 0)
    {
        return L'\0';
    }
    uint32_t codepoint = 0;
    int extraBytes = 0;
    if (byte < 0x80)
    {
        codepoint = byte;
        extraBytes = 0;
    }
    else if (byte < 0xC0)
    {
        ++src;
        return L'\0'; // continuation byte at start — malformed
    }
    else if (byte < 0xE0)
    {
        codepoint = byte & 0x1F;
        extraBytes = 1;
    }
    else if (byte < 0xF0)
    {
        codepoint = byte & 0x0F;
        extraBytes = 2;
    }
    else
    {
        codepoint = byte & 0x07;
        extraBytes = 3;
    }
    ++src;
    for (int i = 0; i < extraBytes; ++i)
    {
        byte = static_cast<unsigned char>(*src);
        if ((byte & 0xC0) != 0x80)
        {
            return L'\0'; // malformed — not a continuation byte
        }
        codepoint = (codepoint << 6) | (byte & 0x3F);
        ++src;
    }
    // Clamp to BMP (U+FFFF max) — surrogate range excluded
    if (codepoint > 0xFFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
    {
        return L'?';
    }
    return static_cast<wchar_t>(codepoint);
}

// ---- SDL3 clipboard replacement (MU_ENABLE_SDL3 path) ----
// MuClipboardIsNumericOnly — SDL3-based replacement for ClipboardCheck() in UIControls.cpp.
// Returns true if clipboard text contains only digit characters ('0'-'9').
// Used by EditWndProc WM_CHAR handler for UIOPTION_NUMBERONLY fields (Ctrl+V paste check).
// [VS1-SDL-INPUT-TEXT]
inline bool MuClipboardIsNumericOnly()
{
    char* text = SDL_GetClipboardText();
    if (text == nullptr)
    {
        return false;
    }
    bool allDigits = true;
    for (const char* p = text; *p != '\0'; ++p)
    {
        if (*p < '0' || *p > '9')
        {
            allDigits = false;
            break;
        }
    }
    SDL_free(text);
    return allDigits;
}

// ---- Mouse cursor visibility shims (SDL3 path) ----
// SDL3 split SDL2's SDL_ShowCursor(int) into two separate functions.
// Win32 ShowCursor has a reference-count mechanism; the shim does not replicate
// the reference count since the codebase uses balanced show/hide calls.
// [VS1-SDL-INPUT-MOUSE]
inline void ShowCursor(bool show)
{
    if (show)
    {
        SDL_ShowCursor();
    }
    else
    {
        SDL_HideCursor();
    }
}

// MuPlatformLogMouseWarpFailed — implemented in SDLKeyboardState.cpp (compiled
// with the project PCH that provides CErrorReport / g_ErrorReport). Declared here
// so the inline SetCursorPos shim can call it without pulling ErrorReport.h into
// every TU that includes PlatformCompat.h. [VS1-SDL-INPUT-MOUSE]
void MuPlatformLogMouseWarpFailed(const char* sdlError);

// ---- Cursor warp shim (SDL3 path) ----
// SDL_WarpMouseInWindow with nullptr targets the currently focused window.
// Call sites pass window-relative coordinates scaled to WindowWidth/Height.
// [VS1-SDL-INPUT-MOUSE]
inline void SetCursorPos(int x, int y)
{
    if (!SDL_WarpMouseInWindow(nullptr, static_cast<float>(x), static_cast<float>(y)))
    {
        MuPlatformLogMouseWarpFailed(SDL_GetError());
    }
}

// ---- HIBYTE macro (non-Windows) ----
// Extracts the high byte of a 16-bit value.
// Used by all HIBYTE(GetAsyncKeyState(vk)) call sites (104 sites in codebase).
#ifndef HIBYTE
#define HIBYTE(w) (static_cast<uint8_t>((static_cast<uint16_t>(w) >> 8) & 0xFF))
#endif

// ---- Keyboard input shim (GetAsyncKeyState) ----
// g_sdl3KeyboardState[512] is defined in SDLKeyboardState.cpp and populated
// by SDLEventLoop::PollEvents(). Indexed by SDL_Scancode (0-511).
extern bool g_sdl3KeyboardState[512];

// Map Win32 Virtual Key code to SDL3 SDL_Scancode.
// Returns SDL_SCANCODE_UNKNOWN (0) for any VK code with no mapping.
[[nodiscard]] inline SDL_Scancode MuVkToSdlScancode(int vk)
{
    // ASCII letter range: 'A'-'Z' (0x41-0x5A) -> SDL_SCANCODE_A + offset
    static_assert(SDL_SCANCODE_A == 4, "SDL_SCANCODE_A must be 4 — verify SDL3 release-3.2.8");
    if (vk >= 'A' && vk <= 'Z')
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (vk - 'A'));
    }

    // ASCII digit range: '1'-'9' (0x31-0x39) -> SDL_SCANCODE_1 + offset
    // '0' (0x30) -> SDL_SCANCODE_0 (not contiguous with 1-9 in SDL3)
    static_assert(SDL_SCANCODE_1 == 30, "SDL_SCANCODE_1 must be 30 — verify SDL3 release-3.2.8");
    if (vk >= '1' && vk <= '9')
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (vk - '1'));
    }
    if (vk == '0')
    {
        return SDL_SCANCODE_0;
    }

    switch (vk)
    {
    // Control / navigation
    case 0x08:
        return SDL_SCANCODE_BACKSPACE; // VK_BACK
    case 0x09:
        return SDL_SCANCODE_TAB; // VK_TAB
    case 0x0D:
        return SDL_SCANCODE_RETURN; // VK_RETURN
    case 0x10:
        return SDL_SCANCODE_LSHIFT; // VK_SHIFT
    case 0x11:
        return SDL_SCANCODE_LCTRL; // VK_CONTROL
    case 0x12:
        return SDL_SCANCODE_LALT; // VK_MENU
    case 0x13:
        return SDL_SCANCODE_PAUSE; // VK_PAUSE
    case 0x14:
        return SDL_SCANCODE_CAPSLOCK; // VK_CAPITAL
    case 0x1B:
        return SDL_SCANCODE_ESCAPE; // VK_ESCAPE
    case 0x20:
        return SDL_SCANCODE_SPACE; // VK_SPACE
    case 0x21:
        return SDL_SCANCODE_PAGEUP; // VK_PRIOR
    case 0x22:
        return SDL_SCANCODE_PAGEDOWN; // VK_NEXT
    case 0x23:
        return SDL_SCANCODE_END; // VK_END
    case 0x24:
        return SDL_SCANCODE_HOME; // VK_HOME
    case 0x25:
        return SDL_SCANCODE_LEFT; // VK_LEFT
    case 0x26:
        return SDL_SCANCODE_UP; // VK_UP
    case 0x27:
        return SDL_SCANCODE_RIGHT; // VK_RIGHT
    case 0x28:
        return SDL_SCANCODE_DOWN; // VK_DOWN
    case 0x2C:
        return SDL_SCANCODE_PRINTSCREEN; // VK_SNAPSHOT
    case 0x2D:
        return SDL_SCANCODE_INSERT; // VK_INSERT
    case 0x2E:
        return SDL_SCANCODE_DELETE; // VK_DELETE

    // Numpad digits
    case 0x60:
        return SDL_SCANCODE_KP_0; // VK_NUMPAD0
    case 0x61:
        return SDL_SCANCODE_KP_1; // VK_NUMPAD1
    case 0x62:
        return SDL_SCANCODE_KP_2; // VK_NUMPAD2
    case 0x63:
        return SDL_SCANCODE_KP_3; // VK_NUMPAD3
    case 0x64:
        return SDL_SCANCODE_KP_4; // VK_NUMPAD4
    case 0x65:
        return SDL_SCANCODE_KP_5; // VK_NUMPAD5
    case 0x66:
        return SDL_SCANCODE_KP_6; // VK_NUMPAD6
    case 0x67:
        return SDL_SCANCODE_KP_7; // VK_NUMPAD7
    case 0x68:
        return SDL_SCANCODE_KP_8; // VK_NUMPAD8
    case 0x69:
        return SDL_SCANCODE_KP_9; // VK_NUMPAD9

    // Numpad operators
    case 0x6A:
        return SDL_SCANCODE_KP_MULTIPLY; // VK_MULTIPLY
    case 0x6B:
        return SDL_SCANCODE_KP_PLUS; // VK_ADD
    // VK_SEPARATOR (0x6C) intentionally omitted: no SDL3 equivalent.
    // Falls through to default → logs MU_ERR_INPUT_UNMAPPED_VK if called.
    case 0x6D:
        return SDL_SCANCODE_KP_MINUS; // VK_SUBTRACT
    case 0x6E:
        return SDL_SCANCODE_KP_DECIMAL; // VK_DECIMAL
    case 0x6F:
        return SDL_SCANCODE_KP_DIVIDE; // VK_DIVIDE

    // Function keys
    case 0x70:
        return SDL_SCANCODE_F1;
    case 0x71:
        return SDL_SCANCODE_F2;
    case 0x72:
        return SDL_SCANCODE_F3;
    case 0x73:
        return SDL_SCANCODE_F4;
    case 0x74:
        return SDL_SCANCODE_F5;
    case 0x75:
        return SDL_SCANCODE_F6;
    case 0x76:
        return SDL_SCANCODE_F7;
    case 0x77:
        return SDL_SCANCODE_F8;
    case 0x78:
        return SDL_SCANCODE_F9;
    case 0x79:
        return SDL_SCANCODE_F10;
    case 0x7A:
        return SDL_SCANCODE_F11;
    case 0x7B:
        return SDL_SCANCODE_F12;

    // Lock keys
    case 0x90:
        return SDL_SCANCODE_NUMLOCKCLEAR; // VK_NUMLOCK
    case 0x91:
        return SDL_SCANCODE_SCROLLLOCK; // VK_SCROLL

    // Left/Right modifier variants
    case 0xA0:
        return SDL_SCANCODE_LSHIFT; // VK_LSHIFT
    case 0xA1:
        return SDL_SCANCODE_RSHIFT; // VK_RSHIFT
    case 0xA2:
        return SDL_SCANCODE_LCTRL; // VK_LCONTROL
    case 0xA3:
        return SDL_SCANCODE_RCTRL; // VK_RCONTROL
    case 0xA4:
        return SDL_SCANCODE_LALT; // VK_LMENU
    case 0xA5:
        return SDL_SCANCODE_RALT; // VK_RMENU

    // OEM keys
    case 0xBA:
        return SDL_SCANCODE_SEMICOLON; // VK_OEM_1
    case 0xBB:
        return SDL_SCANCODE_EQUALS; // VK_OEM_PLUS
    case 0xBC:
        return SDL_SCANCODE_COMMA; // VK_OEM_COMMA
    case 0xBD:
        return SDL_SCANCODE_MINUS; // VK_OEM_MINUS
    case 0xBE:
        return SDL_SCANCODE_PERIOD; // VK_OEM_PERIOD

    default:
        return SDL_SCANCODE_UNKNOWN;
    }
}

// MuPlatformLogUnmappedVk — implemented in SDLKeyboardState.cpp (compiled with
// the project PCH that provides CErrorReport / g_ErrorReport). Declared here so
// the inline shim can call it without pulling ErrorReport.h into every TU that
// includes PlatformCompat.h. [VS1-SDL-INPUT-KEYBOARD]
void MuPlatformLogUnmappedVk(int vk);

// Mouse button state — populated by SDLEventLoop, used by GetAsyncKeyState shim.
// [VS1-SDL-INPUT-MOUSE]
extern bool MouseLButton;
extern bool MouseRButton;
extern bool MouseMButton;

// GetAsyncKeyState shim for non-Windows platforms.
// Returns 0x8000 (high bit set) when the key is currently held, 0 otherwise.
// Callers using HIBYTE(GetAsyncKeyState(vk)) & 0x80 or == 128 behave correctly:
//   HIBYTE(0x8000) == 0x80 == 128, satisfying both check patterns.
// Mouse button VK codes (0x01/0x02/0x04) are backed by global mouse state.
// Unmapped VK codes return 0 and log MU_ERR_INPUT_UNMAPPED_VK via g_ErrorReport.
// [VS1-SDL-INPUT-KEYBOARD] [VS1-SDL-INPUT-MOUSE]
[[nodiscard]] inline uint16_t GetAsyncKeyState(int vk)
{
    // Mouse button VK codes — backed by global mouse state populated by SDLEventLoop
    switch (vk)
    {
    case 0x01:
        return MouseLButton ? static_cast<uint16_t>(0x8000) : 0; // VK_LBUTTON
    case 0x02:
        return MouseRButton ? static_cast<uint16_t>(0x8000) : 0; // VK_RBUTTON
    case 0x04:
        return MouseMButton ? static_cast<uint16_t>(0x8000) : 0; // VK_MBUTTON
    default:
        break;
    }

    SDL_Scancode sc = MuVkToSdlScancode(vk);
    if (sc == SDL_SCANCODE_UNKNOWN)
    {
        MuPlatformLogUnmappedVk(vk);
        return 0;
    }
    return g_sdl3KeyboardState[sc] ? static_cast<uint16_t>(0x8000) : 0;
}

inline std::string mu_wchar_to_utf8(const wchar_t* src)
{
    std::string result;
    if (src == nullptr)
    {
        return result;
    }
    for (const wchar_t* p = src; *p; ++p)
    {
        auto ch = static_cast<uint32_t>(*p);
        if (ch < 0x80)
        {
            result += static_cast<char>(ch);
        }
        else if (ch < 0x800)
        {
            result += static_cast<char>(0xC0 | (ch >> 6));
            result += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else if (ch < 0x10000)
        {
            if (ch >= 0xD800 && ch <= 0xDFFF)
            {
                continue;
            }
            result += static_cast<char>(0xE0 | (ch >> 12));
            result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else if (ch <= 0x10FFFF)
        {
            result += static_cast<char>(0xF0 | (ch >> 18));
            result += static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (ch & 0x3F));
        }
    }
    return result;
}

inline int MessageBoxW(void* /*hwnd*/, const wchar_t* text, const wchar_t* caption, unsigned int type)
{
    std::string u8text = mu_wchar_to_utf8(text);
    std::string u8caption = mu_wchar_to_utf8(caption);

    // Map MB_ type to SDL message box flags
    SDL_MessageBoxFlags sdlFlags = SDL_MESSAGEBOX_INFORMATION;
    if ((type & MB_ICONERROR) || (type & MB_ICONSTOP))
        sdlFlags = SDL_MESSAGEBOX_ERROR;
    else if (type & MB_ICONWARNING)
        sdlFlags = SDL_MESSAGEBOX_WARNING;

    if ((type & 0x0F) == MB_YESNO)
    {
        // MB_YESNO: Two-button dialog via SDL_ShowMessageBox
        const SDL_MessageBoxButtonData buttons[] = {
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, IDYES, "Yes"},
            {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, IDNO, "No"},
        };

        const SDL_MessageBoxData messageboxdata = {
            sdlFlags, nullptr, u8caption.c_str(), u8text.c_str(), 2, buttons, nullptr,
        };

        int buttonid = IDNO;
        SDL_ShowMessageBox(&messageboxdata, &buttonid);
        return buttonid;
    }

    // MB_OK and other types: simple message box
    SDL_ShowSimpleMessageBox(sdlFlags, u8caption.c_str(), u8text.c_str(), nullptr);
    return IDOK;
}

#else // !MU_ENABLE_SDL3

// Fallback stub when SDL3 is not available
inline int MessageBoxW(void* /*hwnd*/, const wchar_t* /*text*/, const wchar_t* /*caption*/, unsigned int /*type*/)
{
    return IDOK;
}

#endif // MU_ENABLE_SDL3

#define MessageBox MessageBoxW

// ---- Executable directory shim ----
// mu_get_app_dir() — cross-platform replacement for GetModuleFileNameW + dirname.
// Returns the directory containing the running executable as a std::filesystem::path.
// Used by GameConfig.cpp to locate config.ini next to the executable.
// Phase 5/5.4: GetModuleFileName → portable executable path.
// [VS1-NET-CONFIG-SERVER]
#include <climits>
#ifdef __linux__
#include <unistd.h>
inline std::filesystem::path mu_get_app_dir()
{
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0)
    {
        return std::filesystem::current_path();
    }
    buf[len] = '\0';
    return std::filesystem::path(buf).parent_path();
}
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
inline std::filesystem::path mu_get_app_dir()
{
    char buf[PATH_MAX];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0)
    {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(buf).parent_path();
}
#else
// Fallback for other non-Windows platforms
inline std::filesystem::path mu_get_app_dir()
{
    return std::filesystem::current_path();
}
#endif

// ---- File I/O shims ----
// Convert wchar_t path (UTF-32 on Linux/macOS) to UTF-8, normalize backslashes

// Guard against platforms that already define errno_t
#ifndef MU_ERRNO_T_DEFINED
using errno_t = int;
#define MU_ERRNO_T_DEFINED
#endif

inline FILE* mu_wfopen(const wchar_t* path, const wchar_t* mode)
{
    // Manual UTF-8 conversion to avoid deprecated std::wstring_convert
    std::string u8path;
    for (const wchar_t* p = path; *p != L'\0'; ++p)
    {
        wchar_t ch = *p;
        if (ch == L'\\')
        {
            u8path += '/';
        }
        else if (ch < 0x80)
        {
            u8path += static_cast<char>(ch);
        }
        else if (ch < 0x800)
        {
            u8path += static_cast<char>(0xC0 | (ch >> 6));
            u8path += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else if (ch < 0x10000)
        {
            // Skip UTF-16 surrogate codepoints (U+D800-U+DFFF) — invalid in UTF-8
            if (ch >= 0xD800 && ch <= 0xDFFF)
            {
                continue;
            }
            u8path += static_cast<char>(0xE0 | (ch >> 12));
            u8path += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            u8path += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else if (ch <= 0x10FFFF)
        {
            u8path += static_cast<char>(0xF0 | (ch >> 18));
            u8path += static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
            u8path += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            u8path += static_cast<char>(0x80 | (ch & 0x3F));
        }
        // Codepoints above U+10FFFF are invalid Unicode — skip them
    }

    std::string u8mode;
    for (const wchar_t* m = mode; *m != L'\0'; ++m)
    {
        // File mode strings are always ASCII (e.g., "rb", "w")
        assert(static_cast<unsigned>(*m) < 0x80 && "fopen mode must be ASCII");
        u8mode += static_cast<char>(*m);
    }

    return fopen(u8path.c_str(), u8mode.c_str());
}

inline errno_t mu_wfopen_s(FILE** pFile, const wchar_t* path, const wchar_t* mode)
{
    if (pFile == nullptr || path == nullptr || mode == nullptr)
    {
        return EINVAL;
    }
    *pFile = mu_wfopen(path, mode);
    return (*pFile) ? 0 : errno;
}

// NOLINTBEGIN(bugprone-reserved-identifier)
#define _wfopen_s mu_wfopen_s
#define _wfopen mu_wfopen
// NOLINTEND(bugprone-reserved-identifier)

// ---- RtlSecureZeroMemory shim ----

inline void* mu_SecureZeroMemory(void* ptr, size_t cnt)
{
    volatile char* vptr = static_cast<volatile char*>(ptr);
    while (cnt--)
    {
        *vptr++ = 0;
    }
    return ptr;
}

#define RtlSecureZeroMemory mu_SecureZeroMemory

#endif // _WIN32

// ---- wchar_t <-> char16_t conversion utilities (all platforms) ----
// Flow Code: VS1-NET-CHAR16T-ENCODING
//
// On Windows/MinGW: sizeof(wchar_t)==2, same bit layout as char16_t → reinterpret_cast (zero overhead).
// On Linux/macOS:   sizeof(wchar_t)==4 (UTF-32) → transcode BMP codepoints; surrogate pairs for U+10000+.
//
// Used at the C++/.NET interop boundary where Connection() accepts const char16_t* host
// while callers (WSclient.cpp, UIWindows.cpp) supply wchar_t* host strings.
// Note: <string> is included explicitly in both #ifdef _WIN32 and #else sections above.

// MuPlatformLogChar16MarshalingMismatch — implemented in MuPlatform.cpp (compiled with PCH).
// Emits AC-STD-5 required error log pattern for encoding mismatch diagnostics.
// Called by mu_wchar_to_char16 as a defensive guard. [VS1-NET-CHAR16T-ENCODING]
void MuPlatformLogChar16MarshalingMismatch(const char* context);

inline std::u16string mu_wchar_to_char16(const wchar_t* src)
{
    if (src == nullptr)
    {
        return {};
    }
    if constexpr (sizeof(wchar_t) == sizeof(char16_t))
    {
        // Windows/MinGW: wchar_t is UTF-16LE identical to char16_t — safe reinterpret
        std::u16string result(reinterpret_cast<const char16_t*>(src));
        if (result.empty() && *src != L'\0')
        {
            MuPlatformLogChar16MarshalingMismatch("mu_wchar_to_char16");
        }
        return result;
    }
    else
    {
        // Linux/macOS: wchar_t is 4 bytes (UTF-32), transcode BMP codepoints
        std::u16string result;
        for (const wchar_t* p = src; *p != L'\0'; ++p)
        {
            const char32_t cp = static_cast<char32_t>(*p);
            if (cp < 0x10000U)
            {
                result.push_back(static_cast<char16_t>(cp));
            }
            else
            {
                // Surrogate pair for non-BMP (U+10000 and above)
                const char32_t u = cp - 0x10000U;
                result.push_back(static_cast<char16_t>(0xD800U | (u >> 10)));
                result.push_back(static_cast<char16_t>(0xDC00U | (u & 0x3FFU)));
            }
        }
        if (result.empty() && *src != L'\0')
        {
            MuPlatformLogChar16MarshalingMismatch("mu_wchar_to_char16");
        }
        return result;
    }
}

inline std::wstring mu_char16_to_wchar(const char16_t* src)
{
    if (src == nullptr)
    {
        return {};
    }
    if constexpr (sizeof(wchar_t) == sizeof(char16_t))
    {
        // Windows/MinGW: wchar_t is UTF-16LE identical to char16_t — safe reinterpret
        return std::wstring(reinterpret_cast<const wchar_t*>(src));
    }
    else
    {
        // Linux/macOS: transcode UTF-16 → UTF-32
        std::wstring result;
        for (const char16_t* p = src; *p != u'\0'; ++p)
        {
            const char16_t cu = *p;
            if (cu >= 0xD800U && cu <= 0xDBFFU && *(p + 1) >= 0xDC00U && *(p + 1) <= 0xDFFFU)
            {
                // Surrogate pair → single UTF-32 codepoint
                const char32_t high = cu - 0xD800U;
                const char32_t low = *(p + 1) - 0xDC00U;
                result.push_back(static_cast<wchar_t>(0x10000U + (high << 10) + low));
                ++p;
            }
            else
            {
                result.push_back(static_cast<wchar_t>(cu));
            }
        }
        return result;
    }
}
