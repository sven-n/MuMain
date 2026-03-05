#pragma once

#ifdef _WIN32
// On Windows, all types come from <windows.h> — this header is a no-op.
#else

#include <cstdint>
#include <cstring>
#include <climits>

// Fundamental Windows type aliases
using DWORD = uint32_t;
using BOOL = int;
using BYTE = uint8_t;
using WORD = uint16_t;
using ULONG = unsigned long;
using UCHAR = unsigned char;
using USHORT = unsigned short;

// Handle types (opaque pointers)
using HANDLE = void*;
using HWND = void*;
using HDC = void*;
using HGLRC = void*;
using HINSTANCE = void*;
using HFONT = void*;

// Integral parameter types (message loop)
using LPARAM = intptr_t;
using WPARAM = uintptr_t;
using LRESULT = intptr_t;
using HRESULT = long;

// Pointer-width unsigned integer (needed for LOWORD/HIWORD macros)
using DWORD_PTR = uintptr_t;

// Constants
#define MAX_PATH 260
#define TRUE 1
#define FALSE 0

// Bit extraction macros
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))

// Memory zeroing macro
#define ZeroMemory(Destination, Length) memset((Destination), 0, (Length))

#endif // _WIN32
