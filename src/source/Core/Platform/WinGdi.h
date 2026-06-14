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

#include <cwchar>                      // wcslen
#include <string>
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

// ---- Font creation / text metrics / DIB text rasterization (wingdi.h) -------
// The text is rendered through CUIRenderText, a Win32 GDI font-DC pipeline:
// TextOut rasterizes white-on-black into a 24bpp DIB section, the engine scans
// the pixels into a GL texture. Off Windows the same pipeline is backed by a
// stb_truetype rasterizer (GdiText.cpp, issue #462 Phase 4): CreateFont picks a
// system TTF by weight, CreateDIBSection allocates the pixel buffer with DIB
// pitch rules, and TextOut draws antialiased glyphs into it.

#ifndef FW_BOLD
#define FW_BOLD               700
#endif
#ifndef FW_NORMAL
#define FW_NORMAL             400
#endif
#ifndef FW_SEMIBOLD
#define FW_SEMIBOLD           600
#endif
#ifndef CLEARTYPE_NATURAL_QUALITY
#define CLEARTYPE_NATURAL_QUALITY 6
#endif
#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET       1
#endif
#ifndef OUT_DEFAULT_PRECIS
#define OUT_DEFAULT_PRECIS    0
#endif
#ifndef CLIP_DEFAULT_PRECIS
#define CLIP_DEFAULT_PRECIS   0
#endif
#ifndef DEFAULT_QUALITY
#define DEFAULT_QUALITY       0
#endif
#ifndef NONANTIALIASED_QUALITY
#define NONANTIALIASED_QUALITY 3
#endif
#ifndef ANTIALIASED_QUALITY
#define ANTIALIASED_QUALITY   4
#endif
#ifndef DEFAULT_PITCH
#define DEFAULT_PITCH         0
#endif
#ifndef FF_DONTCARE
#define FF_DONTCARE           0
#endif

HFONT CreateFontW(int cHeight, int cWidth, int cEscapement, int cOrientation,
                  int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut,
                  DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision,
                  DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName);
#ifndef CreateFont
#define CreateFont CreateFontW
#endif

BOOL GetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int c, LPSIZE psizl);
#ifndef GetTextExtentPoint32
#define GetTextExtentPoint32 GetTextExtentPoint32W
#endif

// Release a GDI object. The engine also declares its own DeleteObject(OBJECT*)
// for game objects; these typed overloads coexist with it (an HFONT/HGDIOBJ
// argument selects one of these, an OBJECT* selects the engine's).
BOOL DeleteObject(HFONT hObject);
BOOL DeleteObject(HGDIOBJ hObject);
BOOL DeleteObject(HBITMAP hObject);
BOOL DeleteObject(HBRUSH hObject);

#ifndef DIB_RGB_COLORS
#define DIB_RGB_COLORS 0
#endif

HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO* pbmi, UINT usage, void** ppvBits, HANDLE hSection, DWORD offset);
HDC     CreateCompatibleDC(HDC hdc);
BOOL    DeleteDC(HDC hdc);

// SelectObject returns the previously selected object of the same kind.
HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h);
HGDIOBJ SelectObject(HDC hdc, HBITMAP h);
HGDIOBJ SelectObject(HDC hdc, HFONT h);
inline HGDIOBJ GetStockObject(int) { return nullptr; }

BOOL     TextOut(HDC hdc, int x, int y, LPCWSTR lpString, int c);
COLORREF SetBkColor(HDC hdc, COLORREF color);
COLORREF SetTextColor(HDC hdc, COLORREF color);

// UI-font discovery diagnostics for the crash log: one line per resolved font
// weight (which file loaded, or that none was found). Empty until the first
// font is created. Implemented in GdiText.cpp.
std::string MuFontDiagnostics();

#endif // _WIN32
