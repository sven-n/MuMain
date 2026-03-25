#pragma once

#ifdef _WIN32
// On Windows, all types come from <windows.h> — this header is a no-op.
#else
#include <cstdint>
#include <cstring>
// Fundamental Windows type aliases
using DWORD = uint32_t;
using BOOL = int;
using BYTE = uint8_t;
using WORD = uint16_t;
using UINT = unsigned int;
using LONG = long;
using ULONG = unsigned long;
using UCHAR = unsigned char;
using USHORT = unsigned short;
using SHORT = short;
using INT = int;
using VOID = void;
using TCHAR = wchar_t;
using WCHAR = wchar_t;
using CHAR = char;
using __int64 = intptr_t;
using ULONGLONG = uint64_t;
using LONGLONG = int64_t;
using LPTSTR = wchar_t*;
using LPCTSTR = const wchar_t*;
using FLOAT = float;
using DOUBLE = double;
using PFLOAT = float*;
using PCHAR = char*;
using PBYTE = unsigned char*;
using LPBYTE = unsigned char*;
using LPWORD = unsigned short*;
using LPDWORD = uint32_t*;
using LPBOOL = int*;

// Handle types (opaque pointers)
using HANDLE = void*;
using HWND = void*;
using HDC = void*;
using HGLRC = void*;
using HINSTANCE = void*;
using HFONT = void*;
using LPVOID = void*;

// Integral parameter types (message loop)
using LPARAM = intptr_t;
using WPARAM = uintptr_t;
using UINT_PTR = uintptr_t;
using LRESULT = intptr_t;
using HRESULT = long;

// Pointer-width unsigned integer (needed for LOWORD/HIWORD macros)
using DWORD_PTR = uintptr_t;

// Story 7.3.0: CONST qualifier alias — used in KeyGenerater.h CONST parameters
// [VS0-QUAL-BUILDCOMPAT-MACOS]
#define CONST const

// Story 7.3.0: CP_UTF8 code page constant — used in encoding call sites
// [VS0-QUAL-BUILDCOMPAT-MACOS]
#define CP_UTF8 65001

// Story 7.3.0: MSVC-specific path buffer constants [VS0-QUAL-BUILDCOMPAT-MACOS]
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256

// Story 7.3.0: MSVC __forceinline keyword [VS0-QUAL-BUILDCOMPAT-MACOS]
#define __forceinline inline __attribute__((always_inline))

// Story 7.3.0: Windows message constants used in game logic [VS0-QUAL-BUILDCOMPAT-MACOS]
#define WM_DESTROY 0x0002

// Constants
#define MAX_PATH 260
#define TRUE 1
#define FALSE 0
#define CALLBACK
#define WM_TIMER 0x0113
#define MB_OK 0x00000000L
#define MB_YESNO 0x00000004L
#define MB_OKCANCEL 0x00000001L
#define MB_ICONERROR 0x00000010L
#define MB_ICONWARNING 0x00000030L
#define MB_ICONSTOP 0x00000010L
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7
#define IN
#define OUT

#define FOREGROUND_BLUE 0x0001
#define FOREGROUND_GREEN 0x0002
#define FOREGROUND_RED 0x0004
#define FOREGROUND_INTENSITY 0x0008
#define BACKGROUND_BLUE 0x0010
#define BACKGROUND_GREEN 0x0020
#define BACKGROUND_RED 0x0040
#define BACKGROUND_INTENSITY 0x0080

// Virtual Key Codes
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_CANCEL 0x03
#define VK_MBUTTON 0x04
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14
#define VK_ESCAPE 0x1B
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SELECT 0x29
#define VK_PRINT 0x2A
#define VK_EXECUTE 0x2B
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HELP 0x2F
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#define VK_SLEEP 0x5F
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5

#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE)                                                                           \
    extern "C++"                                                                                                       \
    {                                                                                                                  \
        inline ENUMTYPE operator|(ENUMTYPE a, ENUMTYPE b)                                                              \
        {                                                                                                              \
            return ENUMTYPE(((int)a) | ((int)b));                                                                      \
        }                                                                                                              \
        inline ENUMTYPE& operator|=(ENUMTYPE& a, ENUMTYPE b)                                                           \
        {                                                                                                              \
            return (ENUMTYPE&)(((int&)a) |= ((int)b));                                                                 \
        }                                                                                                              \
        inline ENUMTYPE operator&(ENUMTYPE a, ENUMTYPE b)                                                              \
        {                                                                                                              \
            return ENUMTYPE(((int)a) & ((int)b));                                                                      \
        }                                                                                                              \
        inline ENUMTYPE& operator&=(ENUMTYPE& a, ENUMTYPE b)                                                           \
        {                                                                                                              \
            return (ENUMTYPE&)(((int&)a) &= ((int)b));                                                                 \
        }                                                                                                              \
        inline ENUMTYPE operator~(ENUMTYPE a)                                                                          \
        {                                                                                                              \
            return ENUMTYPE(~((int)a));                                                                                \
        }                                                                                                              \
        inline ENUMTYPE operator^(ENUMTYPE a, ENUMTYPE b)                                                              \
        {                                                                                                              \
            return ENUMTYPE(((int)a) ^ ((int)b));                                                                      \
        }                                                                                                              \
        inline ENUMTYPE& operator^=(ENUMTYPE& a, ENUMTYPE b)                                                           \
        {                                                                                                              \
            return (ENUMTYPE&)(((int&)a) ^= ((int)b));                                                                 \
        }                                                                                                              \
    }
#endif

// Bit extraction macros
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))

// Memory zeroing macro
#define ZeroMemory(Destination, Length) memset((Destination), 0, (Length))

// ---- Geometric structs ----

struct POINT
{
    long x;
    long y;
};

struct RECT
{
    long left;
    long top;
    long right;
    long bottom;
};
using LPRECT = RECT*;

struct SIZE
{
    long cx;
    long cy;
};
using LPSIZE = SIZE*;

struct FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

struct SYSTEM_INFO
{
    union
    {
        DWORD dwOemId;
        struct
        {
            WORD wProcessorArchitecture;
            WORD wReserved;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
};

inline bool PtInRect(const RECT* prc, POINT pt)
{
    if (prc == nullptr)
    {
        return false;
    }
    return pt.x >= prc->left && pt.x < prc->right && pt.y >= prc->top && pt.y < prc->bottom;
}
#endif // _WIN32
