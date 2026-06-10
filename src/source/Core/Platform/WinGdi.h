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

// ---- Font creation / text metrics (wingdi.h) --------------------------------
// The text is rendered through CUIRenderText, a Win32 GDI font-DC pipeline that
// is not yet ported (the implementation is still behind the in-game-shop include
// wall). Until that port, the font handle is a no-op and text extents are an
// estimate -- enough for layout code to compile and run approximately. The real
// metrics come with the text-rendering port (issue #462, Phase 4).

#ifndef FW_BOLD
#define FW_BOLD               700
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

// A small fixed cell, used to estimate a string's pixel size until real font
// metrics are available.
inline constexpr int MU_APPROX_CHAR_WIDTH  = 8;
inline constexpr int MU_APPROX_CHAR_HEIGHT = 16;

inline HFONT CreateFontW(int /*cHeight*/, int /*cWidth*/, int /*cEscapement*/, int /*cOrientation*/,
                         int /*cWeight*/, DWORD /*bItalic*/, DWORD /*bUnderline*/, DWORD /*bStrikeOut*/,
                         DWORD /*iCharSet*/, DWORD /*iOutPrecision*/, DWORD /*iClipPrecision*/,
                         DWORD /*iQuality*/, DWORD /*iPitchAndFamily*/, LPCWSTR /*pszFaceName*/)
{
    // Non-null so callers treat the font as valid; it is never selected into a
    // real DC off Windows.
    return reinterpret_cast<HFONT>(1);
}
#ifndef CreateFont
#define CreateFont CreateFontW
#endif

inline BOOL GetTextExtentPoint32W(HDC /*hdc*/, LPCWSTR lpString, int c, LPSIZE psizl)
{
    if (!psizl || (!lpString && c != 0)) return FALSE;
    const int len = (c >= 0) ? c : static_cast<int>(wcslen(lpString));
    psizl->cx = len * MU_APPROX_CHAR_WIDTH;
    psizl->cy = MU_APPROX_CHAR_HEIGHT;
    return TRUE;
}
#ifndef GetTextExtentPoint32
#define GetTextExtentPoint32 GetTextExtentPoint32W
#endif

// Release a GDI object. The engine also declares its own DeleteObject(OBJECT*)
// for game objects; these typed overloads coexist with it (an HFONT/HGDIOBJ
// argument selects one of these, an OBJECT* selects the engine's).
inline BOOL DeleteObject(HFONT)   { return TRUE; }
inline BOOL DeleteObject(HGDIOBJ) { return TRUE; }
inline BOOL DeleteObject(HBITMAP) { return TRUE; }
inline BOOL DeleteObject(HBRUSH)  { return TRUE; }

// ---- GDI device-context / text rendering (wingdi.h) -------------------------
// The legacy text renderer rasterizes glyphs into a DIB via a memory DC. That
// pipeline isn't ported yet, so these are no-ops: CreateDIBSection/
// CreateCompatibleDC return null, which makes the renderer's Create() fail
// gracefully and skip GDI text until the SDL_ttf port (issue #462, Phase 4).
#ifndef DIB_RGB_COLORS
#define DIB_RGB_COLORS 0
#endif

inline HBITMAP CreateDIBSection(HDC, const BITMAPINFO*, UINT, void** ppvBits, HANDLE, DWORD)
{
    if (ppvBits) *ppvBits = nullptr;
    return nullptr;
}
inline HDC  CreateCompatibleDC(HDC) { return nullptr; }
inline BOOL DeleteDC(HDC)           { return TRUE; }

// SelectObject returns the previously selected object (a null stub is fine).
inline HGDIOBJ SelectObject(HDC, HGDIOBJ) { return nullptr; }
inline HGDIOBJ SelectObject(HDC, HBITMAP) { return nullptr; }
inline HGDIOBJ SelectObject(HDC, HFONT)   { return nullptr; }
inline HGDIOBJ GetStockObject(int)        { return nullptr; }

inline BOOL     TextOut(HDC, int, int, LPCWSTR, int) { return TRUE; }
inline COLORREF SetBkColor(HDC, COLORREF)            { return 0; }
inline COLORREF SetTextColor(HDC, COLORREF)          { return 0; }

#endif // _WIN32
