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
#include <cstring>       // memset for ZeroMemory
#include <type_traits>   // underlying_type for DEFINE_ENUM_FLAG_OPERATORS

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
typedef wchar_t   WCHAR;  // 32-bit here vs 16-bit on Windows; width handled at the .NET interop boundary (#462).
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
typedef void*           PVOID;
typedef void*           LPVOID;
typedef const void*     LPCVOID;
typedef CHAR*           LPSTR;
typedef CHAR*           PSTR;
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

// SEH record, only ever used as an opaque pointer parameter off Windows.
struct _EXCEPTION_POINTERS;
typedef struct _EXCEPTION_POINTERS* PEXCEPTION_POINTERS;

// Async-I/O struct referenced by a few function declarations (minwinbase.h).
typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union { struct { DWORD Offset; DWORD OffsetHigh; }; PVOID Pointer; };
    HANDLE    hEvent;
} OVERLAPPED, * LPOVERLAPPED;

// Bitwise operators for scoped flag enums (winnt.h). The engine's flag enums
// invoke this right after their definition; off Windows we provide the same
// operator set, deducing the integer width from the enum's underlying type.
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
inline constexpr ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { using T = std::underlying_type_t<ENUMTYPE>; return static_cast<ENUMTYPE>(static_cast<T>(a) | static_cast<T>(b)); } \
inline ENUMTYPE& operator |= (ENUMTYPE& a, ENUMTYPE b) { a = a | b; return a; } \
inline constexpr ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { using T = std::underlying_type_t<ENUMTYPE>; return static_cast<ENUMTYPE>(static_cast<T>(a) & static_cast<T>(b)); } \
inline ENUMTYPE& operator &= (ENUMTYPE& a, ENUMTYPE b) { a = a & b; return a; } \
inline constexpr ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { using T = std::underlying_type_t<ENUMTYPE>; return static_cast<ENUMTYPE>(static_cast<T>(a) ^ static_cast<T>(b)); } \
inline ENUMTYPE& operator ^= (ENUMTYPE& a, ENUMTYPE b) { a = a ^ b; return a; } \
inline constexpr ENUMTYPE operator ~ (ENUMTYPE a) { using T = std::underlying_type_t<ENUMTYPE>; return static_cast<ENUMTYPE>(~static_cast<T>(a)); }

// Calling-convention / annotation macros: no-ops off Windows.
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __fastcall
#define __fastcall
#endif
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
#define WM_QUIT          0x0012
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

// Window styles (winuser.h). SDL owns the real window; these let the legacy
// Win32 window/resolution code compile (it is stubbed out at the call level).
#ifndef WS_OVERLAPPED
#define WS_OVERLAPPED   0x00000000L
#define WS_POPUP        0x80000000L
#define WS_VISIBLE      0x10000000L
#define WS_CAPTION      0x00C00000L
#define WS_BORDER       0x00800000L
#define WS_SYSMENU      0x00080000L
#define WS_MINIMIZEBOX  0x00020000L
#define WS_CLIPCHILDREN 0x02000000L
#endif

// SetWindowPos flags + insert-after handles (winuser.h).
#ifndef SWP_NOSIZE
#define SWP_NOSIZE      0x0001
#define SWP_NOMOVE      0x0002
#define SWP_NOZORDER    0x0004
#define SWP_FRAMECHANGED 0x0020
#define SWP_SHOWWINDOW  0x0040
#endif
#ifndef HWND_TOP
#define HWND_TOP        (reinterpret_cast<HWND>(0))
#define HWND_BOTTOM     (reinterpret_cast<HWND>(1))
#define HWND_TOPMOST    (reinterpret_cast<HWND>(-1))
#define HWND_NOTOPMOST  (reinterpret_cast<HWND>(-2))
#endif
#ifndef GWL_STYLE
#define GWL_STYLE       (-16)
#endif
#ifndef SW_SHOW
#define SW_HIDE         0
#define SW_NORMAL       1
#define SW_SHOW         5
#endif

// ChangeDisplaySettings / DEVMODE (winuser.h / wingdi.h).
#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN  0x00000004
#endif
#ifndef DISP_CHANGE_SUCCESSFUL
#define DISP_CHANGE_SUCCESSFUL 0
#define DISP_CHANGE_FAILED     (-1)
#endif
#ifndef DM_BITSPERPEL
#define DM_BITSPERPEL       0x00040000L
#define DM_PELSWIDTH        0x00080000L
#define DM_PELSHEIGHT       0x00100000L
#define DM_DISPLAYFREQUENCY 0x00400000L
#endif

// PeekMessage flag, stock-object id (winuser.h / wingdi.h).
#ifndef IMN_SETCONVERSIONMODE
#define IMN_SETCONVERSIONMODE 0x0006
#define IMN_SETSENTENCEMODE   0x0007
#endif
#ifndef PM_REMOVE
#define PM_REMOVE       0x0001
#endif
#ifndef BLACK_BRUSH
#define BLACK_BRUSH     4
#endif

// MIDI sequencer mapper (mmsystem.h).
#ifndef MCI_SEQ_MAPPER
#define MCI_SEQ_MAPPER 0xFFFFu
#endif

// MSVC keyword / fixed-width aliases.
#ifndef __forceinline
#define __forceinline inline
#endif
typedef uint64_t u_int64;
typedef int64_t  INT64;

// Console attributes / standard handles (wincon.h).
#define FOREGROUND_BLUE      0x0001
#define FOREGROUND_GREEN     0x0002
#define FOREGROUND_RED       0x0004
#define FOREGROUND_INTENSITY 0x0008
#define STD_INPUT_HANDLE     (static_cast<DWORD>(-10))
#define STD_OUTPUT_HANDLE    (static_cast<DWORD>(-11))
#define STD_ERROR_HANDLE     (static_cast<DWORD>(-12))

// _splitpath component sizes (stdlib.h).
#ifndef _MAX_PATH
#define _MAX_PATH   260
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE  3
#endif
#ifndef _MAX_DIR
#define _MAX_DIR    256
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME  256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT    256
#endif

// COM-style result codes.
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif
#ifndef S_OK
#define S_OK          (static_cast<HRESULT>(0))
#endif
#ifndef E_FAIL
#define E_FAIL        (static_cast<HRESULT>(0x80004005))
#endif
#ifndef E_INVALIDARG
#define E_INVALIDARG  (static_cast<HRESULT>(0x80070057))
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (static_cast<HRESULT>(hr) >= 0)
#endif
#ifndef FAILED
#define FAILED(hr)    (static_cast<HRESULT>(hr) < 0)
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (reinterpret_cast<HANDLE>(static_cast<LONG_PTR>(-1)))
#endif

// Word/byte/color helper macros (minwindef.h / wingdi.h).
#ifndef LOWORD
#define LOWORD(l)    (static_cast<WORD>(static_cast<uintptr_t>(l) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l)    (static_cast<WORD>((static_cast<uintptr_t>(l) >> 16) & 0xffff))
#endif
#ifndef LOBYTE
#define LOBYTE(w)    (static_cast<BYTE>(static_cast<uintptr_t>(w) & 0xff))
#endif
#ifndef HIBYTE
#define HIBYTE(w)    (static_cast<BYTE>((static_cast<uintptr_t>(w) >> 8) & 0xff))
#endif
#ifndef MAKEWORD
#define MAKEWORD(a, b) (static_cast<WORD>((static_cast<BYTE>(a)) | (static_cast<WORD>(static_cast<BYTE>(b)) << 8)))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) (static_cast<LONG>((static_cast<WORD>(a)) | (static_cast<DWORD>(static_cast<WORD>(b)) << 16)))
#endif
#ifndef RGB
#define RGB(r, g, b) (static_cast<COLORREF>((static_cast<BYTE>(r)) | (static_cast<WORD>(static_cast<BYTE>(g)) << 8) | (static_cast<DWORD>(static_cast<BYTE>(b)) << 16)))
#define GetRValue(rgb) (static_cast<BYTE>(rgb))
#define GetGValue(rgb) (static_cast<BYTE>((rgb) >> 8))
#define GetBValue(rgb) (static_cast<BYTE>((rgb) >> 16))
#endif

// Memory fill helper.
#ifndef ZeroMemory
#define ZeroMemory(dst, len) std::memset((dst), 0, (len))
#endif

// Element count of a stack array (MSVC _countof). Type-safe: a pointer argument
// fails to match the array helper and is a compile error, not a silent miscount.
#ifndef _countof
template <typename T, std::size_t N> char (&mu_countof_helper(T (&)[N]))[N];
#define _countof(arr) (sizeof(mu_countof_helper(arr)))
#endif

// ---- GDI rect/point helpers (winuser.h) -------------------------------------
// Pure value-type helpers with no platform dependency; same semantics as Win32.

inline BOOL SetRect(RECT* lprc, int left, int top, int right, int bottom)
{
    if (!lprc) return FALSE;
    lprc->left = left; lprc->top = top; lprc->right = right; lprc->bottom = bottom;
    return TRUE;
}

// Point is inside the rectangle's [left,right) x [top,bottom) half-open range.
inline BOOL PtInRect(const RECT* lprc, POINT pt)
{
    if (!lprc) return FALSE;
    return (pt.x >= lprc->left && pt.x < lprc->right &&
            pt.y >= lprc->top  && pt.y < lprc->bottom) ? TRUE : FALSE;
}

// Intersection of two rects into dst; returns FALSE (and empties dst) if none.
inline BOOL IntersectRect(RECT* dst, const RECT* a, const RECT* b)
{
    if (!dst || !a || !b) return FALSE;
    const LONG left   = (a->left   > b->left)   ? a->left   : b->left;
    const LONG top    = (a->top    > b->top)    ? a->top    : b->top;
    const LONG right  = (a->right  < b->right)  ? a->right  : b->right;
    const LONG bottom = (a->bottom < b->bottom) ? a->bottom : b->bottom;
    if (left < right && top < bottom)
    {
        dst->left = left; dst->top = top; dst->right = right; dst->bottom = bottom;
        return TRUE;
    }
    dst->left = dst->top = dst->right = dst->bottom = 0;
    return FALSE;
}

// Legacy Win32 pointer-validity probe. There is no portable equivalent, and the
// API is deprecated even on Windows; the realistic failure the callers guard
// against is a null pointer, so report only that as "bad".
inline BOOL IsBadReadPtr(const void* lp, UINT_PTR /*ucb*/) { return lp ? FALSE : TRUE; }

#endif  // _WIN32
