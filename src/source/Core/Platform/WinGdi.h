// Portable GDI struct shim (issue #462, Phase 3).
//
// The terrain/UI code reads and builds Windows DIB/BMP structures for file I/O
// and texture uploads. On Windows these come from <wingdi.h> (via <windows.h>);
// elsewhere this provides just the structs and constants used, laid out to match
// the on-disk/in-memory Windows formats. GDI *functions* are a separate concern.
#pragma once

#ifdef _WIN32

#include <windows.h>  // BMP/DIB types (pulled in via <wingdi.h>)

#else  // ---- non-Windows ----------------------------------------------------

#include "Core/Platform/WinCompat.h"  // WORD, DWORD, LONG, BYTE

#ifndef BI_RGB
#define BI_RGB 0
#endif

// The file header is read/written with sizeof == 14, so it must be packed (the
// DWORD bfSize sits at offset 2, not 4).
#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, * LPBITMAPFILEHEADER, * PBITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, * LPBITMAPINFOHEADER, * PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO, * LPBITMAPINFO, * PBITMAPINFO;

typedef struct tagPALETTEENTRY {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
} PALETTEENTRY, * LPPALETTEENTRY, * PPALETTEENTRY;

#endif // _WIN32
