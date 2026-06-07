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

#endif  // _WIN32
