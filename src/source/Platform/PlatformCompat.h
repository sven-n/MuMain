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

// ---- Cross-platform UTF-8 conversion (Windows) ----
// mu_wchar_to_utf8() — delegates to WideCharToMultiByte on Windows.
// Provides the same interface as the non-Windows shim below, so call sites
// can use mu_wchar_to_utf8() unconditionally without #ifdef _WIN32.
inline std::string mu_wchar_to_utf8(const wchar_t* src)
{
    if (src == nullptr)
    {
        return std::string();
    }
    int len = WideCharToMultiByte(CP_UTF8, 0, src, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0)
    {
        return std::string();
    }
    std::string result(len - 1, 0); // -1 to exclude null terminator
    WideCharToMultiByte(CP_UTF8, 0, src, -1, result.data(), len, nullptr, nullptr);
    return result;
}

#else

#include "PlatformTypes.h"

#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <thread>
#include <cstring>
#include <filesystem>
#include <string>

// MSVC intrinsic stub — triggers a breakpoint on non-Windows/non-MSVC builds [VS0-QUAL-BUILDCOMPAT-MACOS]
#define __debugbreak() __builtin_trap()

// Story 7.3.0: String comparison and compat stubs [VS0-QUAL-BUILDCOMPAT-MACOS]
#define _wcsicmp wcscasecmp
#define wcsicmp wcscasecmp
#define wcsnicmp wcsncasecmp
#define _stricmp strcasecmp
#define _TRUNCATE ((size_t)-1)
#define _snwprintf swprintf
inline void OutputDebugString(const wchar_t* /*msg*/) {}

// Story 7.3.0: MSVC secure string stubs [VS0-QUAL-BUILDCOMPAT-MACOS]
inline errno_t wcsncpy_s(wchar_t* dest, size_t destSize, const wchar_t* src, size_t count)
{
    if (dest == nullptr || destSize == 0)
    {
        return EINVAL;
    }
    size_t toCopy = (count < destSize - 1) ? count : destSize - 1;
    wcsncpy(dest, src, toCopy);
    dest[toCopy] = L'\0';
    return 0;
}

// MSVC-compatible 3-arg template overload: deduces array size from dest [VS0-QUAL-BUILDCOMPAT-MACOS]
template <size_t N> inline errno_t wcsncpy_s(wchar_t (&dest)[N], const wchar_t* src, size_t count)
{
    return wcsncpy_s(dest, N, src, count);
}

inline wchar_t* wcstok_s(wchar_t* str, const wchar_t* delim, wchar_t** context)
{
    return wcstok(str, delim, context);
}

// Story 7.3.0: _wsplitpath stub — splits wide-char path into components [VS0-QUAL-BUILDCOMPAT-MACOS]
inline void _wsplitpath(const wchar_t* path, wchar_t* drive, wchar_t* dir, wchar_t* fname, wchar_t* ext)
{
    if (drive != nullptr)
    {
        drive[0] = L'\0';
    }
    if (path == nullptr)
    {
        if (dir)
            dir[0] = L'\0';
        if (fname)
            fname[0] = L'\0';
        if (ext)
            ext[0] = L'\0';
        return;
    }
    const wchar_t* lastSlash = wcsrchr(path, L'/');
    const wchar_t* lastBackslash = wcsrchr(path, L'\\');
    const wchar_t* lastSep = lastSlash > lastBackslash ? lastSlash : lastBackslash;
    const wchar_t* lastDot = wcsrchr(path, L'.');

    if (dir != nullptr)
    {
        if (lastSep != nullptr)
        {
            size_t dirLen = static_cast<size_t>(lastSep - path + 1);
            wcsncpy(dir, path, dirLen);
            dir[dirLen] = L'\0';
        }
        else
        {
            dir[0] = L'\0';
        }
    }
    const wchar_t* nameStart = lastSep ? lastSep + 1 : path;
    if (lastDot != nullptr && lastDot > nameStart)
    {
        if (fname != nullptr)
        {
            size_t nameLen = static_cast<size_t>(lastDot - nameStart);
            wcsncpy(fname, nameStart, nameLen);
            fname[nameLen] = L'\0';
        }
        if (ext != nullptr)
        {
            wcscpy(ext, lastDot);
        }
    }
    else
    {
        if (fname != nullptr)
        {
            wcscpy(fname, nameStart);
        }
        if (ext != nullptr)
        {
            ext[0] = L'\0';
        }
    }
}

// Story 7.3.0: MultiByteToWideChar / WideCharToMultiByte stubs [VS0-QUAL-BUILDCOMPAT-MACOS]
// Story 7.3.0: Forward declaration for WideCharToMultiByte stub below.
// Defined later in this file (~line 1137). [VS0-QUAL-BUILDCOMPAT-MACOS]
inline std::string mu_wchar_to_utf8(const wchar_t* src);

// Story 7.3.0: Win32-compatible signatures using proper UTF-8 encoding/decoding.
// MultiByteToWideChar: UTF-8 → wchar_t (inline decoder, handles multi-byte sequences).
// WideCharToMultiByte: wchar_t → UTF-8 (delegates to mu_wchar_to_utf8).
// Used throughout Data/ and Core/ files. [VS0-QUAL-BUILDCOMPAT-MACOS]
inline int MultiByteToWideChar(UINT /*CodePage*/, DWORD /*dwFlags*/, const char* lpMultiByteStr, int cbMultiByte,
                               wchar_t* lpWideCharStr, int cchWideChar)
{
    if (lpMultiByteStr == nullptr)
    {
        return 0;
    }
    const auto* src = reinterpret_cast<const unsigned char*>(lpMultiByteStr);
    const auto* end = (cbMultiByte == -1) ? src + strlen(lpMultiByteStr) + 1 : src + cbMultiByte;
    int count = 0;
    while (src < end)
    {
        uint32_t ch;
        int seqLen;
        if (*src < 0x80)
        {
            ch = *src;
            seqLen = 1;
        }
        else if ((*src & 0xE0) == 0xC0)
        {
            ch = *src & 0x1F;
            seqLen = 2;
        }
        else if ((*src & 0xF0) == 0xE0)
        {
            ch = *src & 0x0F;
            seqLen = 3;
        }
        else if ((*src & 0xF8) == 0xF0)
        {
            ch = *src & 0x07;
            seqLen = 4;
        }
        else
        {
            ++src;
            continue;
        }
        if (src + seqLen > end)
        {
            break;
        }
        // Validate continuation bytes have pattern 10xxxxxx (MEDIUM-1 fix)
        bool valid = true;
        for (int i = 1; i < seqLen; ++i)
        {
            if ((src[i] & 0xC0) != 0x80)
            {
                valid = false;
                break;
            }
            ch = (ch << 6) | (src[i] & 0x3F);
        }
        if (!valid)
        {
            ++src;
            continue;
        }

        // Reject overlong sequences (MEDIUM-2 fix)
        // Each sequence length must produce values in the correct range
        bool overlong = false;
        if (seqLen == 2 && ch < 0x80)
            overlong = true; // 2-byte: must be >= 0x80
        if (seqLen == 3 && ch < 0x800)
            overlong = true; // 3-byte: must be >= 0x800
        if (seqLen == 4 && ch < 0x10000)
            overlong = true; // 4-byte: must be >= 0x10000

        if (overlong)
        {
            ++src;
            continue;
        }

        src += seqLen;
        if (cchWideChar > 0 && count < cchWideChar)
        {
            lpWideCharStr[count] = static_cast<wchar_t>(ch);
        }
        ++count;
    }
    return count;
}

inline int WideCharToMultiByte(UINT /*CodePage*/, DWORD /*dwFlags*/, const wchar_t* lpWideCharStr, int cchWideChar,
                               char* lpMultiByteStr, int cbMultiByte, const char* /*lpDefaultChar*/,
                               int* /*lpUsedDefaultChar*/)
{
    if (lpWideCharStr == nullptr)
    {
        return 0;
    }
    // Build null-terminated source for mu_wchar_to_utf8
    std::wstring src;
    if (cchWideChar == -1)
    {
        src = lpWideCharStr;
    }
    else
    {
        src = std::wstring(lpWideCharStr, static_cast<size_t>(cchWideChar));
    }
    std::string utf8 = mu_wchar_to_utf8(src.c_str());
    // Include null terminator when input was null-terminated
    if (cchWideChar == -1)
    {
        utf8.push_back('\0');
    }
    int resultLen = static_cast<int>(utf8.size());
    if (cbMultiByte == 0)
    {
        return resultLen;
    }
    int toCopy = (resultLen < cbMultiByte) ? resultLen : cbMultiByte;
    memcpy(lpMultiByteStr, utf8.data(), static_cast<size_t>(toCopy));
    return toCopy;
}

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

inline void Sleep(DWORD dwMilliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(dwMilliseconds));
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
inline BOOL RemoveMenu(void* /*hMenu*/, UINT /*uPosition*/, UINT /*uFlags*/)
{
    return TRUE;
}

struct CHAR_INFO
{
    union
    {
        WCHAR UnicodeChar;
        CHAR AsciiChar;
    } Char;
    WORD Attributes;
};

inline BOOL ReadConsoleOutput(HANDLE /*hConsoleOutput*/, CHAR_INFO* /*lpBuffer*/, COORD /*dwBufferSize*/,
                              COORD /*dwBufferCoord*/, SMALL_RECT* /*lpReadRegion*/)
{
    return TRUE;
}
inline DWORD GetWindowThreadProcessId(HWND /*hWnd*/, LPDWORD /*lpdwProcessId*/)
{
    return 0;
}
inline DWORD GetCurrentThreadId()
{
    return 0;
}
inline DWORD GetCurrentProcessId()
{
    return 0;
}
inline int GetClassName(HWND /*hWnd*/, LPTSTR /*lpClassName*/, int /*nMaxCount*/)
{
    return 0;
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
// Story 7.6.1: ImmGetCompositionString — returns composition string length or data.
// No-op on non-Windows (IME is not supported). [VS0-QUAL-BUILDCOMP-MACOS]
inline LONG ImmGetCompositionString(HIMC /*himc*/, DWORD /*dwIndex*/, void* /*lpBuf*/, DWORD /*dwBufLen*/)
{
    return 0;
}

// ---- Win32 window message stubs (non-Windows only) ----
// Used in UIControls.cpp: CUITextInputBox::SetIMEPosition, GiveFocus, SetState
// UINT already defined in PlatformTypes.h
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
#define SW_NORMAL 1
inline BOOL ShowWindow(HWND /*hwnd*/, int /*nCmdShow*/)
{
    return TRUE;
}

// ShellExecute stub — used in iexplorer.h:OpenExplorer to launch URLs in a browser.
// On non-Windows, returns 0 (failure). A real implementation would use SDL_OpenURL or xdg-open.
inline HINSTANCE ShellExecute(HWND /*hwnd*/, const wchar_t* /*op*/, const wchar_t* /*file*/, const wchar_t* /*params*/,
                              const wchar_t* /*dir*/, int /*nShowCmd*/)
{
    return nullptr;
}

// DestroyWindow stub — used in CUITextInputBox destructor and SetSize failure path
inline BOOL DestroyWindow(HWND /*hwnd*/)
{
    return TRUE;
}

// ExitProcess stub — used in ZzzTexture.cpp fatal error path
inline void ExitProcess(UINT uExitCode)
{
    exit(static_cast<int>(uExitCode));
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
struct BITMAPFILEHEADER
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};
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
    // Story 7.3.0: SDL_WarpMouseInWindow returns void in SDL3 3.2.8 — no error check possible.
    // [VS0-QUAL-BUILDCOMPAT-MACOS]
    SDL_WarpMouseInWindow(nullptr, static_cast<float>(x), static_cast<float>(y));
}

// ---- HIBYTE macro (non-Windows) ----
// Extracts the high byte of a 16-bit value.
// Used by all HIBYTE(GetAsyncKeyState(vk)) call sites (104 sites in codebase).
#ifndef HIBYTE
#define HIBYTE(w) (static_cast<uint8_t>((static_cast<uint16_t>(w) >> 8) & 0xFF))
#endif

// Extracts the low byte of a 16-bit value.
#ifndef LOBYTE
#define LOBYTE(w) (static_cast<uint8_t>(static_cast<uint16_t>(w) & 0xFF))
#endif

// Packs two 16-bit values into a 32-bit LONG (non-Windows equivalent of Win32 MAKELONG).
#ifndef MAKELONG
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define MAKELONG(a, b)                                                                                                 \
    ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
// NOLINTEND(cppcoreguidelines-macro-usage)
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

// ---- WinINet stubs (non-Windows only) ----
// ShopListManager headers (DownloadInfo.h, IConnecter.h, FileDownloader.h, BannerInfo.h)
// use WinINet types unconditionally despite guarding the #include <wininet.h>.
// These are compilation stubs only — WinINet functionality requires Windows.
#define INTERNET_MAX_URL_LENGTH 2084
#define INTERNET_MAX_USER_NAME_LENGTH 256
#define INTERNET_MAX_PASSWORD_LENGTH 256
using INTERNET_PORT = unsigned short;
using HINTERNET = void*;

// __stdcall is a Win32 x86 calling convention attribute. Clang on arm64/x86_64
// recognises it but emits -Wignored-attributes (promoted to error via -Werror).
// Define as empty before ShopListManager headers are parsed.
#ifndef __stdcall
#define __stdcall
#endif

// ---- GDI / Win32 UI stubs (non-Windows only) ----
// UIControls.cpp is a ThirdParty Windows UI file that calls GDI/Win32 APIs.
// On macOS/Linux these are dead code paths (replaced by SDL3 GPU rendering).
// Stubs allow compilation; no functionality is expected from them.

// GDI color macro: COLORREF is DWORD (uint32_t), RGB packs R/G/B into low 24 bits.
#define RGB(r, g, b) (static_cast<DWORD>((r) | ((g) << 8) | ((b) << 16)))

// GDI DC operations — no-ops on non-Windows
inline DWORD SetBkColor(HDC /*hdc*/, DWORD /*color*/)
{
    return 0;
}
inline DWORD SetTextColor(HDC /*hdc*/, DWORD /*color*/)
{
    return 0;
}
inline BOOL TextOut(HDC /*hdc*/, int /*x*/, int /*y*/, const wchar_t* /*str*/, int /*len*/)
{
    return FALSE;
}

// Win32 window messages used as constants in UIControls.cpp
#define WM_PAINT 0x000F
#define WM_ERASEBKGND 0x0014

// Scroll bar identifier
#define SB_VERT 1

// IME composition string selector
#define GCS_COMPSTR 0x0008

// Scrollbar API — no-op on non-Windows [VS0-QUAL-BUILDCOMP-MACOS]
inline int GetScrollPos(HWND /*hwnd*/, int /*nBar*/)
{
    return 0;
}
inline int SetScrollPos(HWND /*hwnd*/, int /*nBar*/, int /*nPos*/, BOOL /*bRedraw*/)
{
    return 0;
}

// TIMERPROC callback type — used by SetTimer
using TIMERPROC = void(CALLBACK*)(HWND, UINT, UINT_PTR, DWORD);

// Timer API — no-op on non-Windows
inline UINT_PTR SetTimer(HWND /*hwnd*/, UINT_PTR /*id*/, UINT /*ms*/, TIMERPROC /*fn*/)
{
    return 0;
}
inline BOOL KillTimer(HWND /*hwnd*/, UINT /*id*/)
{
    return TRUE;
}

// GetCurrentDirectory — returns current working dir. Stub for non-Windows. [VS0-QUAL-BUILDCOMP-MACOS]
inline DWORD GetCurrentDirectory(DWORD nBufferLength, wchar_t* lpBuffer)
{
    (void)nBufferLength;
    (void)lpBuffer;
    return 0;
}

// _wtoi — MSVC-specific wide string to int. [VS0-QUAL-BUILDCOMP-MACOS]
#ifndef _wtoi
#define _wtoi(s) static_cast<int>(wcstol((s), nullptr, 10))
#endif

// ---- Story 7.6.1: Additional Win32 API stubs for macOS native build ----
// [VS0-QUAL-BUILDCOMP-MACOS]

// MAKEWORD — packs two bytes into a WORD (used in WinSock init, version checks)
#ifndef MAKEWORD
#define MAKEWORD(low, high) (static_cast<WORD>(((BYTE)(low)) | (static_cast<WORD>((BYTE)(high))) << 8))
#endif

// TEXT() macro — maps to wide string literal on UNICODE builds
#ifndef TEXT
#define TEXT(s) L##s
#endif

// GetTickCount64 — 64-bit tick counter (shim to chrono)
inline unsigned long long GetTickCount64()
{
    return static_cast<unsigned long long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count());
}

// IntersectRect — rectangle intersection test (GDI)
inline BOOL IntersectRect(RECT* dest, const RECT* src1, const RECT* src2)
{
    if (!dest || !src1 || !src2)
        return FALSE;
    dest->left = (src1->left > src2->left) ? src1->left : src2->left;
    dest->top = (src1->top > src2->top) ? src1->top : src2->top;
    dest->right = (src1->right < src2->right) ? src1->right : src2->right;
    dest->bottom = (src1->bottom < src2->bottom) ? src1->bottom : src2->bottom;
    if (dest->left >= dest->right || dest->top >= dest->bottom)
    {
        dest->left = dest->top = dest->right = dest->bottom = 0;
        return FALSE;
    }
    return TRUE;
}

// OutputDebugStringA — narrow-char variant (wide OutputDebugString already defined above)
inline void OutputDebugStringA(const char* /*msg*/) {}

// SwapBuffers — WGL buffer swap (dead code on macOS, SDL3 handles presentation)
inline BOOL SwapBuffers(HDC /*hdc*/)
{
    return FALSE;
}

// MSVC-specific safe string functions
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cwchar>

#define sprintf_s snprintf
#define swprintf_s swprintf

inline int wcscpy_s(wchar_t* dest, size_t destsz, const wchar_t* src)
{
    if (!dest || !src || destsz == 0)
        return -1;
    wcsncpy(dest, src, destsz - 1);
    dest[destsz - 1] = L'\0';
    return 0;
}
// MSVC 2-argument template overload: deduces buffer size from static array
template <size_t N> inline int wcscpy_s(wchar_t (&dest)[N], const wchar_t* src)
{
    return wcscpy_s(dest, N, src);
}

// wcscat_s — safe wide string concatenation (MSVC CRT)
inline int wcscat_s(wchar_t* dest, size_t destsz, const wchar_t* src)
{
    if (!dest || !src || destsz == 0)
        return -1;
    size_t dlen = wcslen(dest);
    if (dlen >= destsz)
        return -1;
    wcsncpy(dest + dlen, src, destsz - dlen - 1);
    dest[destsz - 1] = L'\0';
    return 0;
}
template <size_t N> inline int wcscat_s(wchar_t (&dest)[N], const wchar_t* src)
{
    return wcscat_s(dest, N, src);
}

// _itow — integer to wide string (MSVC CRT)
// Caller must provide a buffer of at least 34 wide chars (32 digits + sign + null).
inline wchar_t* mu_itow(int value, wchar_t* buffer, int radix)
{
    wchar_t tmp[34]; // max 32 digits for base-2 + sign + null
    if (radix == 10)
        swprintf(tmp, sizeof(tmp) / sizeof(tmp[0]), L"%d", value);
    else if (radix == 16)
        swprintf(tmp, sizeof(tmp) / sizeof(tmp[0]), L"%x", value);
    else
    {
        buffer[0] = L'\0';
        return buffer;
    }
    wcscpy(buffer, tmp);
    return buffer;
}
#define _itow mu_itow

// _mbclen — multibyte character length (MSVC MBCS)
inline size_t mu_mbclen(const unsigned char* c)
{
    if (!c || *c == 0)
        return 0;
    return (*c > 0x7f) ? 2 : 1;
}
#define _mbclen mu_mbclen

// GDI font creation constants (used in CreateFont calls — dead code on macOS)
#define CLIP_DEFAULT_PRECIS 0
#define DEFAULT_CHARSET 1
#define DEFAULT_PITCH 0
#define FF_DONTCARE 0
#define FW_BOLD 700
#define FW_NORMAL 400
#define NONANTIALIASED_QUALITY 3
#define CLEARTYPE_NATURAL_QUALITY 6
#define OUT_DEFAULT_PRECIS 0
#define ANSI_CHARSET 0
#define ANTIALIASED_QUALITY 4

// IME mode constants
#ifndef IME_CMODE_NATIVE
#define IME_CMODE_NATIVE 0x0001
#endif

// IME sentence mode constant — used in ZzzInterface.cpp
#define IME_SMODE_AUTOMATIC 0x0004

// ImmGetDefaultIMEWnd — returns default IME window handle (no-op on non-Windows)
inline HWND ImmGetDefaultIMEWnd(HWND /*hwnd*/)
{
    return nullptr;
}

// ---- Window message constants ----
#ifndef WM_USER
#define WM_USER 0x0400
#endif
#ifndef WM_CLOSE
#define WM_CLOSE 0x0010
#endif
#ifndef WM_KEYDOWN
#define WM_KEYDOWN 0x0100
#endif
#ifndef WM_KEYUP
#define WM_KEYUP 0x0101
#endif
#ifndef WM_LBUTTONDOWN
#define WM_LBUTTONDOWN 0x0201
#endif
#ifndef WM_RBUTTONDOWN
#define WM_RBUTTONDOWN 0x0204
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WM_ACTIVATE
#define WM_ACTIVATE 0x0006
#endif
#ifndef WA_INACTIVE
#define WA_INACTIVE 0
#endif
#ifndef WM_CTLCOLOREDIT
#define WM_CTLCOLOREDIT 0x0133
#endif
#ifndef WM_SETCURSOR
#define WM_SETCURSOR 0x0020
#endif
#ifndef WM_MOUSEMOVE
#define WM_MOUSEMOVE 0x0200
#endif
#ifndef WM_LBUTTONUP
#define WM_LBUTTONUP 0x0202
#endif
#ifndef WM_RBUTTONUP
#define WM_RBUTTONUP 0x0205
#endif
#ifndef WM_LBUTTONDBLCLK
#define WM_LBUTTONDBLCLK 0x0203
#endif
#ifndef WM_MBUTTONDOWN
#define WM_MBUTTONDOWN 0x0207
#endif
#ifndef WM_MBUTTONUP
#define WM_MBUTTONUP 0x0208
#endif
#ifndef WM_QUIT
#define WM_QUIT 0x0012
#endif
#ifndef PM_NOREMOVE
#define PM_NOREMOVE 0x0000
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

// ---- Window style constants (Winmain.cpp window creation) ----
#ifndef CS_HREDRAW
#define CS_HREDRAW 0x0002
#endif
#ifndef CS_VREDRAW
#define CS_VREDRAW 0x0001
#endif
#ifndef WS_OVERLAPPED
#define WS_OVERLAPPED 0x00000000L
#endif
#ifndef WS_CAPTION
#define WS_CAPTION 0x00C00000L
#endif
#ifndef WS_SYSMENU
#define WS_SYSMENU 0x00080000L
#endif
#ifndef WS_MINIMIZEBOX
#define WS_MINIMIZEBOX 0x00020000L
#endif
#ifndef WS_BORDER
#define WS_BORDER 0x00800000L
#endif
#ifndef WS_CLIPCHILDREN
#define WS_CLIPCHILDREN 0x02000000L
#endif
#ifndef WS_POPUP
#define WS_POPUP 0x80000000L
#endif
#ifndef WS_EX_TOPMOST
#define WS_EX_TOPMOST 0x00000008L
#endif
#ifndef WS_EX_APPWINDOW
#define WS_EX_APPWINDOW 0x00040000L
#endif

// ---- GDI brush / stock object constants ----
#ifndef BLACK_BRUSH
#define BLACK_BRUSH 4
#endif
#ifndef WHITE_BRUSH
#define WHITE_BRUSH 0
#endif

// ---- File sharing constants ----
#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ 0x00000001
#endif

// ---- System metrics constants ----
#ifndef SM_CXSCREEN
#define SM_CXSCREEN 0
#endif
#ifndef SM_CYSCREEN
#define SM_CYSCREEN 1
#endif

// ---- Font weight constants ----
#ifndef FW_SEMIBOLD
#define FW_SEMIBOLD 600
#endif

// ---- Timer resolution constants ----
#ifndef TIMERR_NOERROR
#define TIMERR_NOERROR 0
#endif

// ---- SystemParametersInfo constants ----
#ifndef SPI_SCREENSAVERRUNNING
#define SPI_SCREENSAVERRUNNING 97
#endif
#ifndef SPI_GETSCREENSAVETIMEOUT
#define SPI_GETSCREENSAVETIMEOUT 14
#endif
#ifndef SPI_SETSCREENSAVETIMEOUT
#define SPI_SETSCREENSAVETIMEOUT 15
#endif

// ---- Type aliases for Win32 handles ----
using HICON = void*;
using HCURSOR = void*;
using HBRUSH = void*;
using ATOM = WORD;
using PSTR = char*;
#ifndef APIENTRY
#define APIENTRY
#endif

// ---- PAINTSTRUCT — used in WM_PAINT handler ----
struct PAINTSTRUCT
{
    HDC hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
};
inline HDC BeginPaint(HWND /*hwnd*/, PAINTSTRUCT* ps)
{
    if (ps)
        memset(ps, 0, sizeof(*ps));
    return nullptr;
}
inline BOOL EndPaint(HWND /*hwnd*/, const PAINTSTRUCT* /*ps*/)
{
    return TRUE;
}

// ---- VS_FIXEDFILEINFO — version info struct ----
struct VS_FIXEDFILEINFO
{
    DWORD dwSignature;
    DWORD dwStrucVersion;
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
    DWORD dwProductVersionMS;
    DWORD dwProductVersionLS;
    DWORD dwFileFlagsMask;
    DWORD dwFileFlags;
    DWORD dwFileOS;
    DWORD dwFileType;
    DWORD dwFileSubtype;
    DWORD dwFileDateMS;
    DWORD dwFileDateLS;
};
inline DWORD GetFileVersionInfoSize(const wchar_t* /*lptstrFilename*/, DWORD* lpHandle)
{
    if (lpHandle)
        *lpHandle = 0;
    return 0;
}
inline BOOL GetFileVersionInfo(const wchar_t* /*lptstrFilename*/, DWORD /*dwHandle*/, DWORD /*dwLen*/, void* /*lpData*/)
{
    return FALSE;
}
inline BOOL VerQueryValue(const void* /*pBlock*/, const wchar_t* /*lpSubBlock*/, void** /*lplpBuffer*/, UINT* /*puLen*/)
{
    return FALSE;
}

// ---- DEVMODE — display settings struct (minimal for EnumDisplaySettings) ----
struct DEVMODE
{
    wchar_t dmDeviceName[32];
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    DWORD dmFields;
    WORD dmSize;
    WORD dmDriverExtra;
};
inline BOOL EnumDisplaySettings(const wchar_t* /*lpszDeviceName*/, DWORD /*iModeNum*/, DEVMODE* /*lpDevMode*/)
{
    return FALSE;
}
inline LONG ChangeDisplaySettings(DEVMODE* /*lpDevMode*/, DWORD /*dwFlags*/)
{
    return 0;
}

// ---- WNDCLASS — window class registration struct ----
struct WNDCLASS
{
    UINT style;
    LRESULT(CALLBACK* lpfnWndProc)(HWND, UINT, WPARAM, LPARAM);
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    const wchar_t* lpszMenuName;
    const wchar_t* lpszClassName;
};
inline ATOM RegisterClass(const WNDCLASS* /*lpWndClass*/)
{
    return 1;
}

// ---- _EXCEPTION_POINTERS — structured exception handling stub ----
struct _EXCEPTION_POINTERS
{
    void* ExceptionRecord;
    void* ContextRecord;
};

// ---- Window management function stubs (Winmain.cpp) ----
inline BOOL wglMakeCurrent(HDC /*hdc*/, HGLRC /*hglrc*/)
{
    return TRUE;
}
inline BOOL wglDeleteContext(HGLRC /*hglrc*/)
{
    return TRUE;
}
inline HGLRC wglCreateContext(HDC /*hdc*/)
{
    return nullptr;
}
inline HWND FindWindow(const wchar_t* /*lpClassName*/, const wchar_t* /*lpWindowName*/)
{
    return nullptr;
}
inline void PostQuitMessage(int /*nExitCode*/) {}
inline HWND SetCapture(HWND /*hwnd*/)
{
    return nullptr;
}
inline BOOL ReleaseCapture()
{
    return TRUE;
}
inline HGDIOBJ GetStockObject(int /*fnObject*/)
{
    return nullptr;
}
inline BOOL AdjustWindowRect(RECT* /*lpRect*/, DWORD /*dwStyle*/, BOOL /*bMenu*/)
{
    return TRUE;
}
inline HWND CreateWindowEx(DWORD /*dwExStyle*/, const wchar_t* /*lpClassName*/, const wchar_t* /*lpWindowName*/,
                           DWORD /*dwStyle*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/, HWND /*hWndParent*/,
                           HMENU /*hMenu*/, HINSTANCE /*hInstance*/, void* /*lpParam*/)
{
    return nullptr;
}
inline int GetSystemMetrics(int nIndex)
{
    if (nIndex == SM_CXSCREEN)
        return 1920;
    if (nIndex == SM_CYSCREEN)
        return 1080;
    return 0;
}
inline BOOL SetForegroundWindow(HWND /*hwnd*/)
{
    return TRUE;
}
inline BOOL SystemParametersInfo(UINT /*uiAction*/, UINT /*uiParam*/, void* /*pvParam*/, UINT /*fWinIni*/)
{
    return TRUE;
}
inline UINT timeBeginPeriod(UINT /*uPeriod*/)
{
    return TIMERR_NOERROR;
}
inline UINT timeEndPeriod(UINT /*uPeriod*/)
{
    return TIMERR_NOERROR;
}
inline HICON LoadIcon(HINSTANCE /*hInstance*/, const wchar_t* /*lpIconName*/)
{
    return nullptr;
}
inline HCURSOR LoadCursor(HINSTANCE /*hInstance*/, const wchar_t* /*lpCursorName*/)
{
    return nullptr;
}
#ifndef IDC_ARROW
#define IDC_ARROW ((const wchar_t*)32512)
#endif
#ifndef IDI_ICON1
#define IDI_ICON1 101
#endif
inline LRESULT DefWindowProc(HWND /*hwnd*/, UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return 0;
}
// Note: CreateWindow macro NOT defined here — conflicts with MuPlatform::CreateWindow().
// Winmain.cpp uses CreateWindowEx(0, ...) directly instead.

// ---- SetRect — GDI rectangle initializer ----
inline BOOL SetRect(RECT* lprc, int left, int top, int right, int bottom)
{
    if (!lprc)
        return FALSE;
    lprc->left = left;
    lprc->top = top;
    lprc->right = right;
    lprc->bottom = bottom;
    return TRUE;
}

// ---- CreateFont — GDI font creation (returns nullptr on non-Windows) ----
inline HFONT CreateFont(int /*nHeight*/, int /*nWidth*/, int /*nEscapement*/, int /*nOrientation*/, int /*fnWeight*/,
                        DWORD /*fdwItalic*/, DWORD /*fdwUnderline*/, DWORD /*fdwStrikeOut*/, DWORD /*fdwCharSet*/,
                        DWORD /*fdwOutputPrecision*/, DWORD /*fdwClipPrecision*/, DWORD /*fdwQuality*/,
                        DWORD /*fdwPitchAndFamily*/, const wchar_t* /*lpszFace*/)
{
    return nullptr;
}

// ---- _ASSERT — MSVC debug assertion macro ----
#ifndef _ASSERT
#define _ASSERT(expr) assert(expr)
#endif

// ---- Win32 error codes ----
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif

// ---- SYSTEMTIME — Win32 date/time structure ----
struct SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

inline void GetLocalTime(SYSTEMTIME* lpSystemTime)
{
    if (!lpSystemTime)
        return;
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    struct tm local_tm;
    localtime_r(&tt, &local_tm);
    lpSystemTime->wYear = static_cast<WORD>(local_tm.tm_year + 1900);
    lpSystemTime->wMonth = static_cast<WORD>(local_tm.tm_mon + 1);
    lpSystemTime->wDayOfWeek = static_cast<WORD>(local_tm.tm_wday);
    lpSystemTime->wDay = static_cast<WORD>(local_tm.tm_mday);
    lpSystemTime->wHour = static_cast<WORD>(local_tm.tm_hour);
    lpSystemTime->wMinute = static_cast<WORD>(local_tm.tm_min);
    lpSystemTime->wSecond = static_cast<WORD>(local_tm.tm_sec);
    lpSystemTime->wMilliseconds = 0;
}

// ---- GetCommandLine — Win32 command-line string (stub) ----
inline wchar_t* GetCommandLineW()
{
    static wchar_t empty[] = L"";
    return empty;
}
#define GetCommandLine GetCommandLineW

// ---- Registry API stubs (non-Windows) ----
// regkey.h (ThirdParty) uses these unconditionally. On non-Windows, registry
// operations are no-ops — configuration uses config.ini via GameConfig.
struct _HKEY
{
    int unused;
};
using HKEY = _HKEY*;
#define HKEY_CLASSES_ROOT (reinterpret_cast<HKEY>(static_cast<uintptr_t>(0x80000000)))
#define HKEY_CURRENT_USER (reinterpret_cast<HKEY>(static_cast<uintptr_t>(0x80000001)))
#define HKEY_LOCAL_MACHINE (reinterpret_cast<HKEY>(static_cast<uintptr_t>(0x80000002)))
#define HKEY_USERS (reinterpret_cast<HKEY>(static_cast<uintptr_t>(0x80000003)))
#define HKEY_CURRENT_CONFIG (reinterpret_cast<HKEY>(static_cast<uintptr_t>(0x80000005)))

#define REG_DWORD 4
#define REG_EXPAND_SZ 2
#define REG_SZ 1
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006

inline LONG RegOpenKeyEx(HKEY /*hKey*/, const wchar_t* /*lpSubKey*/, DWORD /*ulOptions*/, DWORD /*samDesired*/,
                         HKEY* /*phkResult*/)
{
    return 2L; /* ERROR_FILE_NOT_FOUND */
}

inline LONG RegQueryValueEx(HKEY /*hKey*/, const wchar_t* /*lpValueName*/, LPDWORD /*lpReserved*/, LPDWORD /*lpType*/,
                            BYTE* /*lpData*/, LPDWORD /*lpcbData*/)
{
    return 2L;
}

inline LONG RegSetValueEx(HKEY /*hKey*/, const wchar_t* /*lpValueName*/, DWORD /*Reserved*/, DWORD /*dwType*/,
                          const BYTE* /*lpData*/, DWORD /*cbData*/)
{
    return 2L;
}

inline LONG RegCreateKeyEx(HKEY /*hKey*/, const wchar_t* /*lpSubKey*/, DWORD /*Reserved*/, wchar_t* /*lpClass*/,
                           DWORD /*dwOptions*/, DWORD /*samDesired*/, void* /*lpSecurityAttributes*/,
                           HKEY* /*phkResult*/, LPDWORD /*lpdwDisposition*/)
{
    return 2L;
}

inline LONG RegCloseKey(HKEY /*hKey*/)
{
    return 0L;
}

#define RegOpenKeyExW RegOpenKeyEx
#define RegQueryValueExW RegQueryValueEx
#define RegSetValueExW RegSetValueEx
#define RegCreateKeyExW RegCreateKeyEx

// ---- MCI multimedia constants ----
#define MCI_SEQ_MAPPER 0xFFFF

// ---- GetModuleFileName stub ----
inline DWORD GetModuleFileName(HINSTANCE /*hModule*/, wchar_t* lpFilename, DWORD nSize)
{
    if (lpFilename && nSize > 0)
        lpFilename[0] = L'\0';
    return 0;
}
#define GetModuleFileNameW GetModuleFileName

// ---- PeekMessage / DispatchMessage stubs ----
struct MSG
{
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
};
#define PM_REMOVE 0x0001
inline BOOL PeekMessage(MSG* /*lpMsg*/, HWND /*hWnd*/, UINT /*wMsgFilterMin*/, UINT /*wMsgFilterMax*/,
                        UINT /*wRemoveMsg*/)
{
    return FALSE;
}
inline BOOL TranslateMessage(const MSG* /*lpMsg*/)
{
    return FALSE;
}
inline LRESULT DispatchMessage(const MSG* /*lpMsg*/)
{
    return 0;
}
#define PeekMessageW PeekMessage
#define DispatchMessageW DispatchMessage

// ---- _wtoi64 — wide string to 64-bit int ----
#ifndef _wtoi64
#define _wtoi64(s) static_cast<long long>(wcstoll((s), nullptr, 10))
#endif

// ---- u_int64 type alias (used in some legacy code as __int64) ----
using u_int64 = int64_t;

// ---- Win32 File API stubs (non-Windows) ----
// InGameShopSystem.cpp uses CreateFile/CloseHandle behind #ifdef FOR_WORK.
#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define OPEN_EXISTING 3
#define CREATE_ALWAYS 2
#define FILE_ATTRIBUTE_NORMAL 0x80
#define INVALID_HANDLE_VALUE (reinterpret_cast<HANDLE>(static_cast<intptr_t>(-1)))

inline HANDLE CreateFile(const wchar_t* /*lpFileName*/, DWORD /*dwDesiredAccess*/, DWORD /*dwShareMode*/,
                         void* /*lpSecurityAttributes*/, DWORD /*dwCreationDisposition*/,
                         DWORD /*dwFlagsAndAttributes*/, HANDLE /*hTemplateFile*/)
{
    return INVALID_HANDLE_VALUE;
}
#define CreateFileW CreateFile

inline BOOL ReadFile(HANDLE /*hFile*/, void* /*lpBuffer*/, DWORD /*nNumberOfBytesToRead*/,
                     LPDWORD /*lpNumberOfBytesRead*/, void* /*lpOverlapped*/)
{
    return FALSE;
}

inline DWORD GetFileSize(HANDLE /*hFile*/, LPDWORD /*lpFileSizeHigh*/)
{
    return 0;
}

inline BOOL CloseHandle(HANDLE /*hObject*/)
{
    return TRUE;
}

// ---- _countof macro (MSVC CRT) ----
#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

// ---- _wcsupr — in-place wide string uppercase (MSVC CRT) ----
inline wchar_t* mu_wcsupr(wchar_t* str)
{
    if (!str)
        return nullptr;
    for (wchar_t* p = str; *p; ++p)
        *p = towupper(*p);
    return str;
}
#define _wcsupr mu_wcsupr

// ---- Registry additional constants ----
#ifndef KEY_ALL_ACCESS
#define KEY_ALL_ACCESS 0xF003F
#endif
#ifndef REG_OPTION_NON_VOLATILE
#define REG_OPTION_NON_VOLATILE 0
#endif

inline LONG RegDeleteKey(HKEY /*hKey*/, const wchar_t* /*lpSubKey*/)
{
    return 2L;
}
inline LONG RegDeleteValue(HKEY /*hKey*/, const wchar_t* /*lpValueName*/)
{
    return 2L;
}
#define RegDeleteKeyW RegDeleteKey
#define RegDeleteValueW RegDeleteValue

// ---- vswprintf MSVC-compatible variant (no size param) ----
// muConsoleDebug.cpp calls vswprintf(buf, fmt, va) without buffer size.
// On POSIX, vswprintf requires a size parameter.
inline int mu_vswprintf(wchar_t* buffer, const wchar_t* format, va_list args)
{
    return vswprintf(buffer, 1024, format, args);
}

// ---- mu_swprintf / mu_swprintf_s: variadic wrappers matching stdafx.h signatures ----
// GCC/Clang: use std::swprintf with explicit buffer size.
// These mirror the templates in stdafx.h for use by test files that include
// PlatformCompat.h instead of the PCH. Guard prevents redefinition when
// stdafx.h is also included (e.g., via PCH).
#ifndef MU_SWPRINTF_DEFINED
#define MU_SWPRINTF_DEFINED
template <typename... Args> inline int mu_swprintf(wchar_t* buffer, const wchar_t* format, Args... args)
{
    return std::swprintf(buffer, 1024, format, args...);
}
template <typename... Args> inline int mu_swprintf_s(wchar_t* buffer, size_t size, const wchar_t* format, Args... args)
{
    return std::swprintf(buffer, size, format, args...);
}
template <size_t N, typename... Args>
inline int mu_swprintf_s(wchar_t (&buffer)[N], const wchar_t* format, Args... args)
{
    return std::swprintf(buffer, N, format, args...);
}
#endif // MU_SWPRINTF_DEFINED

// ---- GLU (OpenGL Utility) function stubs ----
// ZzzOpenglUtil.cpp calls gluPerspective/gluOrtho2D for camera setup.
// SDL3/SDL_opengl.h provides GL functions but NOT GLU. On macOS these
// are no-ops since the rendering pipeline uses SDL3 GPU.
// [VS0-QUAL-BUILDCOMP-MACOS]
inline void gluPerspective(double /*fovy*/, double /*aspect*/, double /*zNear*/, double /*zFar*/) {}
inline void gluOrtho2D(double /*left*/, double /*right*/, double /*bottom*/, double /*top*/) {}
inline void gluLookAt(double, double, double, double, double, double, double, double, double) {}

// ---- WGL (Windows GL Extensions) stubs ----
// ZzzOpenglUtil.cpp uses WGL function pointers for VSync and MSAA.
// On macOS, WGL doesn't exist; SDL3 handles presentation. These stubs
// allow the code to compile — VSync init will detect "not available"
// and early-return through the existing null-check paths.
// [VS0-QUAL-BUILDCOMP-MACOS]

// WGL function pointer types (from wglext.h)
typedef int (*PFNWGLSWAPINTERVALEXTPROC)(int);
typedef const char* (*PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef BOOL (*PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int*, const float*, UINT, int*, UINT*);

// WGL function stubs — all return failure/nullptr so extension detection
// correctly reports "not available"
inline void* wglGetProcAddress(const char* /*name*/)
{
    return nullptr;
}
inline void* wglGetProcAddress(const wchar_t* /*name*/)
{
    return nullptr;
}
inline HDC wglGetCurrentDC()
{
    return nullptr;
}

// WGL_ARB_multisample / WGL_ARB_pixel_format constants
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_ALPHA_BITS_ARB 0x201B
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042
#define GL_MULTISAMPLE_ARB 0x809D

// PIXELFORMATDESCRIPTOR — used by InitGLMultisample (Winmain.cpp, ZzzOpenglUtil.cpp)
struct PIXELFORMATDESCRIPTOR
{
    WORD nSize;
    WORD nVersion;
    DWORD dwFlags;
    BYTE iPixelType;
    BYTE cColorBits;
    BYTE cRedBits;
    BYTE cRedShift;
    BYTE cGreenBits;
    BYTE cGreenShift;
    BYTE cBlueBits;
    BYTE cBlueShift;
    BYTE cAlphaBits;
    BYTE cAlphaShift;
    BYTE cAccumBits;
    BYTE cAccumRedBits;
    BYTE cAccumGreenBits;
    BYTE cAccumBlueBits;
    BYTE cAccumAlphaBits;
    BYTE cDepthBits;
    BYTE cStencilBits;
    BYTE cAuxBuffers;
    BYTE iLayerType;
    BYTE bReserved;
    DWORD dwLayerMask;
    DWORD dwVisibleMask;
    DWORD dwDamageMask;
};
#define PFD_DOUBLEBUFFER 0x00000001
#define PFD_DRAW_TO_WINDOW 0x00000004
#define PFD_SUPPORT_OPENGL 0x00000020
#define PFD_TYPE_RGBA 0
#define PFD_MAIN_PLANE 0

inline int ChoosePixelFormat(HDC /*hdc*/, const PIXELFORMATDESCRIPTOR* /*ppfd*/)
{
    return 1;
}
inline int DescribePixelFormat(HDC /*hdc*/, int /*iPixelFormat*/, UINT /*nBytes*/, PIXELFORMATDESCRIPTOR* ppfd)
{
    if (ppfd)
        memset(ppfd, 0, sizeof(*ppfd));
    return 1;
}
inline BOOL SetPixelFormat(HDC /*hdc*/, int /*format*/, const PIXELFORMATDESCRIPTOR* /*ppfd*/)
{
    return TRUE;
}

// ---- GetDeviceCaps — display device capabilities stub ----
#define VREFRESH 116
#define HORZRES 8
#define VERTRES 10
inline int GetDeviceCaps(HDC /*hdc*/, int nIndex)
{
    if (nIndex == VREFRESH)
        return 60;
    if (nIndex == HORZRES)
        return 1920;
    if (nIndex == VERTRES)
        return 1080;
    return 0;
}

#endif // _WIN32

// ---- Cross-platform process CPU time measurement ----
// Story 7.6.4: mu_get_process_cpu_times replaces GetProcessTimes() in CpuUsage.cpp.
// Returns kernel and user CPU time in nanoseconds. Returns false on failure.
// [VS0-QUAL-WIN32CLEAN-CPUUSAGE]
#include <cstdint>
#ifdef _WIN32
inline bool mu_get_process_cpu_times(uint64_t* kernelNs, uint64_t* userNs)
{
    FILETIME creation, exitTime, kernel, user;
    if (!::GetProcessTimes(::GetCurrentProcess(), &creation, &exitTime, &kernel, &user))
    {
        return false;
    }
    // FILETIME is in 100-nanosecond intervals
    auto toNs = [](const FILETIME& ft) -> uint64_t
    { return ((static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime) * 100ULL; };
    *kernelNs = toNs(kernel);
    *userNs = toNs(user);
    return true;
}
#else
#include <sys/resource.h>
inline bool mu_get_process_cpu_times(uint64_t* kernelNs, uint64_t* userNs)
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0)
    {
        return false;
    }
    auto timevalToNs = [](const struct timeval& tv) -> uint64_t
    { return static_cast<uint64_t>(tv.tv_sec) * 1000000000ULL + static_cast<uint64_t>(tv.tv_usec) * 1000ULL; };
    *kernelNs = timevalToNs(ru.ru_stime);
    *userNs = timevalToNs(ru.ru_utime);
    return true;
}
#endif

// ---- Cross-platform terminal/console abstraction ----
// Story 7.6.5: Replaces Win32 console APIs (AllocConsole, SetConsoleTextAttribute,
// FillConsoleOutputCharacter, etc.) with ANSI escape sequences on ALL platforms.
// Windows 10+ supports ANSI natively once ENABLE_VIRTUAL_TERMINAL_PROCESSING is set.
// [VS0-QUAL-WIN32CLEAN-CONSOLE]
#include <cstdio>
#ifdef _WIN32
#include <io.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

// Global TTY state — set by mu_console_init(), read by mu_console_* functions.
inline bool& mu_console_is_tty_ref()
{
    static bool s_isTTY = false;
    return s_isTTY;
}

// mu_console_init() — enable ANSI escape sequences on Windows; check isatty on POSIX.
// Must be called once before any other mu_console_* function.
// Idempotent: safe to call multiple times (only initializes once).
#ifdef _WIN32
inline void mu_console_init()
{
    static bool s_inited = false;
    if (s_inited)
        return;
    s_inited = true;

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
    {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode))
        {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
            SetConsoleMode(hOut, dwMode);
        }
    }
    mu_console_is_tty_ref() = (_isatty(_fileno(stdout)) != 0);
}
#else
inline void mu_console_init()
{
    static bool s_inited = false;
    if (s_inited)
        return;
    s_inited = true;

    mu_console_is_tty_ref() = (isatty(STDOUT_FILENO) != 0);
}
#endif

// mu_set_console_title() — ANSI OSC title escape, same on all platforms.
inline void mu_set_console_title(const wchar_t* title)
{
    // Ignore null pointer
    if (!title)
    {
        return;
    }
    // Guard ANSI output — don't emit escapes to non-TTY (files, pipes)
    if (!mu_console_is_tty_ref())
        return;
    std::string utf8 = mu_wchar_to_utf8(title);
    std::printf("\033]0;%s\007", utf8.c_str());
    std::fflush(stdout);
}

// mu_set_console_text_color() — ANSI SGR colour escape, same on all platforms.
// colorCode is an ANSI SGR parameter (e.g., 31=red, 32=green, 0=reset).
inline void mu_set_console_text_color(int colorCode)
{
    // Guard ANSI output — don't emit escapes to non-TTY (files, pipes)
    if (!mu_console_is_tty_ref())
        return;
    std::printf("\033[%dm", colorCode);
    std::fflush(stdout);
}

// mu_set_console_text_color_with_bg() — ANSI SGR colour escape with background, same on all platforms.
// fgCode = foreground ANSI code (30-37 for dark, 90-97 for bright)
// bgCode = background ANSI code (40-47 for dark, 100-107 for bright)
inline void mu_set_console_text_color_with_bg(int fgCode, int bgCode)
{
    // Guard ANSI output — don't emit escapes to non-TTY (files, pipes)
    if (!mu_console_is_tty_ref())
        return;
    std::printf("\033[%d;%dm", fgCode, bgCode);
    std::fflush(stdout);
}

// mu_get_console_size() — terminal dimensions with 80x24 fallback.
#ifdef _WIN32
inline void mu_get_console_size(int* cols, int* rows)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        if (*cols > 0 && *rows > 0)
        {
            return;
        }
    }
    *cols = 80;
    *rows = 24;
}
#else
inline void mu_get_console_size(int* cols, int* rows)
{
    struct winsize ws;
    if (isatty(STDOUT_FILENO) && ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0)
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
    }
    else
    {
        *cols = 80;
        *rows = 24;
    }
}
#endif

// mu_console_clear() — ANSI clear screen + cursor home, same on all platforms.
inline void mu_console_clear()
{
    // Guard ANSI output — don't emit escapes to non-TTY (files, pipes)
    if (!mu_console_is_tty_ref())
        return;
    std::printf("\033[2J\033[H");
    std::fflush(stdout);
}

// mu_console_set_cursor_position() — ANSI cursor positioning, same on all platforms.
// x = column (0-based), y = row (0-based). ANSI sequences are 1-based.
inline void mu_console_set_cursor_position(int x, int y)
{
    // Guard ANSI output — don't emit escapes to non-TTY (files, pipes)
    if (!mu_console_is_tty_ref())
        return;
    std::printf("\033[%d;%dH", y + 1, x + 1);
    std::fflush(stdout);
}

// ---- Cross-platform credential encryption (all platforms) ----
// Story 7.6.3: mu_encrypt_blob / mu_decrypt_blob replace Windows DPAPI.
// Implementation moved to Platform/PlatformCrypto.cpp to confine OpenSSL headers to a single TU.
// [VS0-QUAL-WIN32CLEAN-DATALAYER]
#include "Platform/PlatformCrypto.h"

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

// Story 7.6.1: MU_C16 — transparent wchar_t* to const char16_t* conversion macro.
// On Windows/MinGW (sizeof(wchar_t)==2): zero-cost reinterpret_cast.
// On macOS/Linux (sizeof(wchar_t)==4): transcode via mu_wchar_to_char16.
// Safe as function argument: the temporary std::u16string lives for the full expression.
// [VS0-QUAL-BUILDCOMP-MACOS]
#if WCHAR_MAX <= 0xFFFF
#define MU_C16(s) reinterpret_cast<const char16_t*>(s)
#else
#define MU_C16(s) mu_wchar_to_char16(s).c_str()
#endif
