#pragma once

// CrossPlatformGDI.h — Real cross-platform GDI text rendering for SDL3 builds
// Story 7-9-5: Replaces all GDI stubs with functional implementations
//
// Architecture:
//   MuGdiDC   — lightweight device context holding bitmap/font/color state
//   MuGdiFont — font descriptor (height, weight, face name)
//   MuGdiBitmap — wraps a calloc'd pixel buffer with dimension metadata
//
// The original Win32 pipeline: CreateDIBSection → CreateCompatibleDC → SelectObject
// → TextOut (white-on-black) → WriteText (RGB→RGBA color replace) → UploadText (GPU)
// Steps after TextOut are unchanged. This file replaces steps 1-4 with a software
// bitmap font rasterizer that writes directly into the pixel buffer.

#ifndef _WIN32

#include <cstdint>

// Object type tags — first field of every GDI object struct.
// SelectObject and DeleteObject use these to dispatch correctly.
enum MuGdiObjType : uint32_t
{
    MU_GDI_OBJ_BITMAP = 0x47424D50, // 'GBMP'
    MU_GDI_OBJ_FONT = 0x47464E54,   // 'GFNT'
    MU_GDI_OBJ_DC = 0x47444330,     // 'GDC0'
};

struct MuGdiBitmap
{
    MuGdiObjType type; // MU_GDI_OBJ_BITMAP
    unsigned char* pBits;
    int width;
    int height;
    int bytesPerPixel;
};

struct MuGdiFont
{
    MuGdiObjType type; // MU_GDI_OBJ_FONT
    int nHeight;       // Requested character height (absolute value of CreateFont nHeight)
    int fnWeight;
    wchar_t szFaceName[64];
};

struct MuGdiDC
{
    MuGdiObjType type; // MU_GDI_OBJ_DC
    MuGdiBitmap* pBitmap;
    MuGdiFont* pFont;
    unsigned int textColor; // COLORREF (0x00BBGGRR)
    unsigned int bkColor;
    int bkMode; // TRANSPARENT=1, OPAQUE=2
};

// Embedded 8x16 bitmap font — ASCII 32..127 (96 characters, 1536 bytes)
extern const unsigned char g_muBitmapFont8x16[];

// Constants for the embedded font
constexpr int MU_FONT_CHAR_WIDTH = 8;
constexpr int MU_FONT_CHAR_HEIGHT = 16;
constexpr int MU_FONT_FIRST_CHAR = 32;
constexpr int MU_FONT_LAST_CHAR = 127;

#endif // !_WIN32
