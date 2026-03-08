#pragma once

#ifdef _WIN32
// On Windows, all types come from <windows.h> — this header is a no-op.
#else
// Fundamental Windows type aliases
using DWORD = uint32_t;
using BOOL = int;
using BYTE = uint8_t;
using WORD = uint16_t;
using UINT = unsigned int;
using ULONG = unsigned long;
using UCHAR = unsigned char;
using USHORT = unsigned short;
using SHORT = short;
using INT = int;
using VOID = void;
using TCHAR = wchar_t;
using __int64 = intptr_t;
using ULONGLONG = uint64_t;
using LONGLONG = int64_t;

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

// Constants
#define MAX_PATH 260
#define TRUE 1
#define FALSE 0
#define CALLBACK
#define WM_TIMER 0x0113
#define MB_OK 0x00000000L
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
