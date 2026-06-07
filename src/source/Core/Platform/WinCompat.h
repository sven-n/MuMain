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

// MSVC fixed-width keyword aliases (gcc/clang on Linux lack these).
typedef int64_t   __int64;
typedef int32_t   __int32;
typedef int16_t   __int16;
typedef int8_t    __int8;

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

// Opaque handles. Real identity is irrelevant to the portable declarations that
// only store or pass them through.
typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HGLRC;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HMENU;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HBRUSH;
typedef void* HFONT;
typedef void* HBITMAP;
typedef void* HGDIOBJ;
typedef void* HKEY;
typedef void* HIMC;

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
typedef struct tagSIZE { LONG cx, cy; } SIZE, * LPSIZE;
typedef struct tagRECT { LONG left, top, right, bottom; } RECT, * LPRECT;

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
