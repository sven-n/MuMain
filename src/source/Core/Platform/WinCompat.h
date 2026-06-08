// Win32 type compatibility shim (issue #462, Phase 1).
//
// A handful of widely-included engine headers reference Win32 scalar types,
// handles and small structs in their declarations. On Windows this comes from
// <windows.h>; this header funnels that through one place so the engine can be
// parsed by a non-Windows toolchain.
//
//   - On Windows: include <windows.h> unchanged (zero behavioral change).
//   - Elsewhere: provide just the Win32 *types* the engine declarations need.
//
// This only unblocks parsing. Win32 *function* calls and the wchar_t width
// difference are handled in later phases of #462; this header deliberately does
// not try to emulate the Win32 API.
#pragma once

#ifdef _WIN32

#include <windows.h>

#else  // ---- non-Windows: minimal Win32 type shims -------------------------

#include <cstdint>
#include <cstddef>
#include <cstring>  // memset for ZeroMemory

// Fixed-width scalar aliases. Widths match the Windows definitions (DWORD/LONG
// are 32-bit there, unlike `long` on LP64 Linux) so struct layouts stay stable.
typedef uint8_t   BYTE;
typedef uint16_t  WORD;
typedef uint32_t  DWORD;
typedef uint32_t  UINT;
typedef int32_t   INT;
typedef int32_t   LONG;
typedef uint32_t  ULONG;
typedef int16_t   SHORT;
typedef uint16_t  USHORT;
typedef int64_t   LONGLONG;
typedef uint64_t  ULONGLONG;
typedef uint64_t  DWORDLONG;
typedef int       BOOL;
typedef float     FLOAT;
typedef char      CHAR;
typedef unsigned char UCHAR;
typedef wchar_t   WCHAR;  // NOTE: 32-bit here vs 16-bit on Windows; the wchar_t width migration is Phase 2.
typedef void      VOID;
typedef DWORD     COLORREF;

// MSVC fixed-width keyword aliases (gcc/clang lack these). Defined as macros,
// not typedefs, so the common `unsigned __int64` form stays valid.
#ifndef __int64
#define __int64 long long
#endif
#ifndef __int32
#define __int32 int
#endif
#ifndef __int16
#define __int16 short
#endif
#ifndef __int8
#define __int8 char
#endif

// Pointer-sized message/param/int types.
typedef uintptr_t UINT_PTR;
typedef intptr_t  INT_PTR;
typedef intptr_t  LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef uintptr_t DWORD_PTR;
typedef uintptr_t WPARAM;
typedef intptr_t  LPARAM;
typedef intptr_t  LRESULT;
typedef LONG      HRESULT;
typedef BYTE      BOOLEAN;

// TCHAR maps to the wide character set (the engine builds UNICODE).
typedef WCHAR     TCHAR;
typedef WCHAR*    LPTSTR;
typedef const WCHAR* LPCTSTR;
#ifndef _T
#define _T(x) L##x
#endif
#ifndef TEXT
#define TEXT(x) L##x
#endif

// Opaque handles. Each is a distinct incompatible pointer type (as on Windows
// via DECLARE_HANDLE) so overloads on different handle types don't collapse.
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
typedef void* HANDLE;
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HGLRC);
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HMODULE);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HCURSOR);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HGDIOBJ);
DECLARE_HANDLE(HKEY);
DECLARE_HANDLE(HIMC);

// Pointer aliases.
typedef void*           LPVOID;
typedef const void*     LPCVOID;
typedef CHAR*           LPSTR;
typedef const CHAR*     LPCSTR;
typedef WCHAR*          LPWSTR;
typedef const WCHAR*    LPCWSTR;
typedef BYTE*           LPBYTE;
typedef WORD*           LPWORD;
typedef DWORD*          LPDWORD;
typedef LONG*           LPLONG;
typedef BOOL*           LPBOOL;

// Small structs used in declarations.
typedef struct tagPOINT { LONG x, y; } POINT, * LPPOINT;
typedef const POINT* LPCPOINT;
typedef struct tagSIZE { LONG cx, cy; } SIZE, * LPSIZE;
typedef struct tagRECT { LONG left, top, right, bottom; } RECT, * LPRECT;
typedef const RECT* LPCRECT;

// Calling-convention / annotation macros: no-ops off Windows.
#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef CONST
#define CONST const
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// ---- Constants & macros (values match the Win32 SDK) ------------------------

// Virtual-key codes (winuser.h). The input layer maps these onto SDL.
#define VK_LBUTTON   0x01
#define VK_RBUTTON   0x02
#define VK_MBUTTON   0x04
#define VK_BACK      0x08
#define VK_TAB       0x09
#define VK_RETURN    0x0D
#define VK_SHIFT     0x10
#define VK_CONTROL   0x11
#define VK_MENU      0x12
#define VK_ESCAPE    0x1B
#define VK_SPACE     0x20
#define VK_PRIOR     0x21
#define VK_NEXT      0x22
#define VK_END       0x23
#define VK_HOME      0x24
#define VK_LEFT      0x25
#define VK_UP        0x26
#define VK_RIGHT     0x27
#define VK_DOWN      0x28
#define VK_SNAPSHOT  0x2C
#define VK_INSERT    0x2D
#define VK_DELETE    0x2E
#define VK_LCONTROL  0xA2
#define VK_F1        0x70
#define VK_F2        0x71
#define VK_F3        0x72
#define VK_F4        0x73
#define VK_F5        0x74
#define VK_F6        0x75
#define VK_F7        0x76
#define VK_F8        0x77
#define VK_F9        0x78
#define VK_F10       0x79
#define VK_F11       0x7A
#define VK_F12       0x7B

// Window messages (winuser.h). App-defined WM_USER+n messages live elsewhere.
#define WM_USER          0x0400
#define WM_DESTROY       0x0002
#define WM_SIZE          0x0005
#define WM_ACTIVATE      0x0006
#define WM_PAINT         0x000F
#define WM_CLOSE         0x0010
#define WM_ERASEBKGND    0x0014
#define WM_SETCURSOR     0x0020
#define WM_CHAR          0x0102
#define WM_SYSKEYDOWN    0x0104
#define WM_TIMER         0x0113
#define WM_IME_NOTIFY    0x0282
#define WM_IME_CONTROL   0x0283
#define WM_DISPLAYCHANGE 0x007E

// MessageBox flags (winuser.h).
#define MB_OK              0x00000000
#define MB_YESNO           0x00000004
#define MB_ICONERROR       0x00000010
#define MB_ICONSTOP        0x00000010
#define MB_ICONEXCLAMATION 0x00000030

// Console attributes / standard handles (wincon.h).
#define FOREGROUND_BLUE      0x0001
#define FOREGROUND_GREEN     0x0002
#define FOREGROUND_RED       0x0004
#define FOREGROUND_INTENSITY 0x0008
#define STD_INPUT_HANDLE     (static_cast<DWORD>(-10))
#define STD_OUTPUT_HANDLE    (static_cast<DWORD>(-11))
#define STD_ERROR_HANDLE     (static_cast<DWORD>(-12))

// _splitpath component sizes (stdlib.h).
#define _MAX_PATH   260
#define _MAX_DRIVE  3
#define _MAX_DIR    256
#define _MAX_FNAME  256
#define _MAX_EXT    256

// COM-style result codes.
#define S_OK          (static_cast<HRESULT>(0))
#define E_FAIL        (static_cast<HRESULT>(0x80004005))
#define E_INVALIDARG  (static_cast<HRESULT>(0x80070057))
#define SUCCEEDED(hr) (static_cast<HRESULT>(hr) >= 0)
#define FAILED(hr)    (static_cast<HRESULT>(hr) < 0)
#define INVALID_HANDLE_VALUE (reinterpret_cast<HANDLE>(static_cast<LONG_PTR>(-1)))

// Word/byte/color helper macros (minwindef.h / wingdi.h).
#define LOWORD(l)    (static_cast<WORD>(static_cast<uintptr_t>(l) & 0xffff))
#define HIWORD(l)    (static_cast<WORD>((static_cast<uintptr_t>(l) >> 16) & 0xffff))
#define LOBYTE(w)    (static_cast<BYTE>(static_cast<uintptr_t>(w) & 0xff))
#define HIBYTE(w)    (static_cast<BYTE>((static_cast<uintptr_t>(w) >> 8) & 0xff))
#define MAKEWORD(a, b) (static_cast<WORD>((static_cast<BYTE>(a)) | (static_cast<WORD>(static_cast<BYTE>(b)) << 8)))
#define MAKELONG(a, b) (static_cast<LONG>((static_cast<WORD>(a)) | (static_cast<DWORD>(static_cast<WORD>(b)) << 16)))
#define RGB(r, g, b) (static_cast<COLORREF>((static_cast<BYTE>(r)) | (static_cast<WORD>(static_cast<BYTE>(g)) << 8) | (static_cast<DWORD>(static_cast<BYTE>(b)) << 16)))

// Memory fill helper.
#define ZeroMemory(dst, len) memset((dst), 0, (len))

#endif  // _WIN32
