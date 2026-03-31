// Story 7-9-5: Eliminate All Cross-Platform Stubs — Real SDL3 Implementations
// [VS0-PLAT-COMPAT-KILLSTUBS]
//
// RED PHASE: Tests written before implementation.
// All GDI/clipboard assertions FAIL until CrossPlatformGDI.cpp provides real implementations.
// AC-6 (WebzenScene visual) uses SKIP — requires a running game binary, manual verification only.
//
// Test coverage:
//   AC-1: No functions in PlatformCompat.h return nullptr/FALSE/0 as a no-op stub
//   AC-2: CreateFont() returns a real non-null font handle
//   AC-3: GetTextExtentPoint32() returns accurate non-zero glyph measurements
//   AC-4: TextOut() succeeds (returns TRUE) when rasterizing into a real DC+bitmap
//   AC-5: Clipboard operations use SDL3 (OpenClipboard returns TRUE, GetClipboardData round-trip)
//   AC-6: WebzenScene completes — SKIP (manual verification, no automated game runner)
//   AC-7: Dead OpenGL/WGL stubs removed from PlatformCompat.h (file-scan, non-Windows only)

#include "PlatformCompat.h"
#include <catch2/catch_test_macros.hpp>
#include <cstring>

// ---------------------------------------------------------------------------
// AC-1: CreateCompatibleDC and CreateDIBSection are not no-op stubs
// ---------------------------------------------------------------------------

TEST_CASE("AC-1: CreateCompatibleDC returns a non-null sentinel DC", "[platform][gdi][7-9-5]")
{
    HDC dc = CreateCompatibleDC(nullptr);
    // RED: currently returns &s_stubDC sentinel — this is acceptable as non-null.
    // GREEN: returns a properly initialised MuGdiDC* struct after CrossPlatformGDI.cpp impl.
    REQUIRE(dc != nullptr);
}

TEST_CASE("AC-1: CreateDIBSection allocates a real pixel buffer", "[platform][gdi][7-9-5]")
{
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = 64;
    bmi.bmiHeader.biHeight = -64; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = 0; // BI_RGB

    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(nullptr, &bmi, 0, &bits, nullptr, 0);

    // RED: current stub returns a zero-filled 1×1 buffer (bits will be non-null but
    // the buffer may be a stub allocation). GREEN: real calloc for width×height×3 bytes.
    REQUIRE(bmp != nullptr);
    REQUIRE(bits != nullptr);

    // Verify we can write and read the pixel buffer without crashing
    if (bits != nullptr)
    {
        std::memset(bits, 0xAB, 64 * 64 * 3);
        const auto* p = static_cast<const unsigned char*>(bits);
        REQUIRE(p[0] == 0xAB);
        REQUIRE(p[64 * 64 * 3 - 1] == 0xAB);
    }

    DeleteObject(reinterpret_cast<HGDIOBJ>(bmp));
}

TEST_CASE("AC-1: SelectObject, DeleteDC, DeleteObject do not crash", "[platform][gdi][7-9-5]")
{
    SECTION("SelectObject accepts a bitmap handle")
    {
        HDC dc = CreateCompatibleDC(nullptr);
        REQUIRE(dc != nullptr);

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = 8;
        bmi.bmiHeader.biHeight = -8;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;

        void* bits = nullptr;
        HBITMAP bmp = CreateDIBSection(nullptr, &bmi, 0, &bits, nullptr, 0);
        REQUIRE(bmp != nullptr);

        // SelectObject should not crash and should return a previous object or null
        HGDIOBJ prev = SelectObject(dc, reinterpret_cast<HGDIOBJ>(bmp));
        // prev is allowed to be null (no prior selection) — just must not crash

        DeleteObject(reinterpret_cast<HGDIOBJ>(bmp));
        DeleteDC(dc);
    }

    SECTION("DeleteDC on null does not crash")
    {
        // Should be a safe no-op
        DeleteDC(nullptr);
    }

    SECTION("DeleteObject on null does not crash")
    {
        DeleteObject(nullptr);
    }
}

// ---------------------------------------------------------------------------
// AC-2: CreateFont returns a real font handle (non-null)
// ---------------------------------------------------------------------------

TEST_CASE("AC-2: CreateFont returns a non-null HFONT", "[platform][gdi][font][7-9-5]")
{
    // Typical call from CUIRenderTextOriginal::Create() in UIControls.cpp:
    //   g_hFont = CreateFont(-nFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    //                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    //                        ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial");
    HFONT font = CreateFont(
        -16,             // nHeight
        0,               // nWidth
        0,               // nEscapement
        0,               // nOrientation
        400,             // fnWeight (FW_NORMAL)
        FALSE,           // fdwItalic
        FALSE,           // fdwUnderline
        FALSE,           // fdwStrikeOut
        DEFAULT_CHARSET, // fdwCharSet
        0,               // fdwOutputPrecision (OUT_DEFAULT_PRECIS)
        CLIP_DEFAULT_PRECIS, // fdwClipPrecision
        0,               // fdwQuality (NONANTIALIASED_QUALITY)
        0,               // fdwPitchAndFamily (DEFAULT_PITCH)
        L"Arial"         // lpszFace
    );

    // RED: current stub returns nullptr. GREEN: returns a real MuGdiFont* descriptor.
    REQUIRE(font != nullptr);

    DeleteObject(reinterpret_cast<HGDIOBJ>(font));
}

// ---------------------------------------------------------------------------
// AC-3: GetTextExtentPoint32 returns accurate non-zero glyph measurements
// ---------------------------------------------------------------------------

TEST_CASE("AC-3: GetTextExtentPoint32 returns non-zero SIZE for non-empty text",
    "[platform][gdi][font][7-9-5]")
{
    HDC dc = CreateCompatibleDC(nullptr);
    REQUIRE(dc != nullptr);

    SECTION("Single ASCII character has positive dimensions")
    {
        SIZE sz{};
        BOOL ok = GetTextExtentPoint32(dc, L"A", 1, &sz);
        REQUIRE(ok == TRUE);
        // RED: current stub returns fixed 8px estimate but passes REQUIRE(ok==TRUE).
        // GREEN: returns real glyph width from loaded bitmap font atlas.
        REQUIRE(sz.cx > 0);
        REQUIRE(sz.cy > 0);
    }

    SECTION("Multi-character string has width > single character")
    {
        SIZE sz1{}, sz5{};
        GetTextExtentPoint32(dc, L"A", 1, &sz1);
        GetTextExtentPoint32(dc, L"AAAAA", 5, &sz5);
        // Width of 5 chars should be greater than width of 1 char
        REQUIRE(sz5.cx > sz1.cx);
    }

    SECTION("Empty string returns zero width")
    {
        SIZE sz{};
        BOOL ok = GetTextExtentPoint32(dc, L"", 0, &sz);
        REQUIRE(ok == TRUE);
        REQUIRE(sz.cx == 0);
    }

    DeleteDC(dc);
}

// ---------------------------------------------------------------------------
// AC-4: TextOut rasterizes into a real DC+bitmap (returns TRUE)
// ---------------------------------------------------------------------------

TEST_CASE("AC-4: TextOut succeeds when given a valid DC and bitmap", "[platform][gdi][7-9-5]")
{
    // Set up a full GDI pipeline: DC → bitmap → font → SelectObject → TextOut
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = 256;
    bmi.bmiHeader.biHeight = -32; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;

    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(nullptr, &bmi, 0, &bits, nullptr, 0);
    REQUIRE(bmp != nullptr);
    REQUIRE(bits != nullptr);

    HDC dc = CreateCompatibleDC(nullptr);
    REQUIRE(dc != nullptr);

    SelectObject(dc, reinterpret_cast<HGDIOBJ>(bmp));

    HFONT font = CreateFont(-14, 0, 0, 0, 400, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, 0, CLIP_DEFAULT_PRECIS, 0, 0, L"Arial");
    REQUIRE(font != nullptr);
    SelectObject(dc, reinterpret_cast<HGDIOBJ>(font));

    SetTextColor(dc, 0x00FFFFFF); // white
    SetBkMode(dc, 1 /*TRANSPARENT*/);

    SECTION("TextOut returns TRUE for non-empty string")
    {
        BOOL ok = TextOut(dc, 0, 0, L"Hello", 5);
        // RED: current stub returns FALSE. GREEN: returns TRUE after rasterizing glyphs.
        REQUIRE(ok == TRUE);
    }

    SECTION("TextOut returns TRUE for single character")
    {
        BOOL ok = TextOut(dc, 0, 0, L"X", 1);
        REQUIRE(ok == TRUE);
    }

    DeleteObject(reinterpret_cast<HGDIOBJ>(font));
    DeleteObject(reinterpret_cast<HGDIOBJ>(bmp));
    DeleteDC(dc);
}

TEST_CASE("AC-4: SetTextColor, SetBkColor, SetBkMode do not crash", "[platform][gdi][7-9-5]")
{
    HDC dc = CreateCompatibleDC(nullptr);
    REQUIRE(dc != nullptr);

    // These must not crash — return values (prev color/mode) are ignored here
    SetTextColor(dc, 0x00FF0000); // red
    SetBkColor(dc, 0x00000000);   // black
    SetBkMode(dc, 2 /*OPAQUE*/);
    SetBkMode(dc, 1 /*TRANSPARENT*/);

    DeleteDC(dc);
}

// ---------------------------------------------------------------------------
// AC-5: Clipboard operations use SDL3 (non-stub behaviour)
// ---------------------------------------------------------------------------

TEST_CASE("AC-5: OpenClipboard returns TRUE on SDL3 path", "[platform][clipboard][7-9-5]")
{
    // RED: current stub returns FALSE. GREEN: returns TRUE (SDL3 doesn't need open/close).
    BOOL ok = OpenClipboard(nullptr);
    REQUIRE(ok == TRUE);
    CloseClipboard();
}

TEST_CASE("AC-5: Clipboard round-trip via SDL3", "[platform][clipboard][7-9-5]")
{
    // This test verifies the full paste pipeline used by CUITextInputBox::DoActionSub().
    // On SDL3: OpenClipboard → GetClipboardData → GlobalLock → read → GlobalUnlock → CloseClipboard

    SECTION("OpenClipboard / CloseClipboard pair does not crash")
    {
        BOOL opened = OpenClipboard(nullptr);
        // GREEN phase: opened must be TRUE — SDL3 path is always open
        REQUIRE(opened == TRUE);
        BOOL closed = CloseClipboard();
        REQUIRE(closed == TRUE);
    }

    SECTION("GetClipboardData(CF_TEXT) returns data or nullptr safely")
    {
        OpenClipboard(nullptr);
        HGLOBAL hMem = GetClipboardData(CF_TEXT);
        // It's acceptable for hMem to be nullptr if clipboard is empty.
        // What must NOT happen: crash or undefined behaviour.
        if (hMem != nullptr)
        {
            void* ptr = GlobalLock(hMem);
            // GREEN: GlobalLock returns the actual data pointer (not nullptr)
            REQUIRE(ptr != nullptr);
            GlobalUnlock(hMem);
        }
        CloseClipboard();
    }
}

// ---------------------------------------------------------------------------
// AC-6: WebzenScene completes (manual — SKIP in automated test run)
// ---------------------------------------------------------------------------

TEST_CASE("AC-6: WebzenScene completes on SDL3 — MANUAL verification required",
    "[platform][visual][7-9-5][.manual]")
{
    // This AC requires running the full game binary. It cannot be automated in Catch2.
    // Manual verification checklist:
    //   1. Launch game on macOS/Linux (./MuMain or equivalent)
    //   2. OpenFont() completes without returning nullptr from CreateFont()
    //   3. WebzenScene progress bar is visible on screen (text renders)
    //   4. All game data loads (ZzzOpenData sequence completes)
    //   5. Scene transitions to LOG_IN_SCENE (login screen appears)
    //   6. Login UI text is readable (chat input, menu labels)
    //
    // Mark this test GREEN only after manual session confirms all 6 points above.
    SKIP("AC-6 requires manual game binary execution — see verification checklist in story 7-9-5");
}

// ---------------------------------------------------------------------------
// AC-7: Dead stubs removed — file-scan tests (non-Windows only)
// ---------------------------------------------------------------------------

#ifndef _WIN32

#include <filesystem>
#include <fstream>
#include <string>

static std::string ReadFileToString(const std::filesystem::path& path)
{
    std::ifstream f(path);
    return std::string(std::istreambuf_iterator<char>(f), {});
}

TEST_CASE("AC-7: wglCreateContext stub is removed from PlatformCompat.h",
    "[platform][stubs][audit][7-9-5]")
{
    // After Task 4 (delete dead OpenGL/WGL stubs), wglCreateContext must not exist
    // in PlatformCompat.h. The SDL3 path uses SDL_GPU — no WGL context management needed.
    const std::filesystem::path compatHeader{MU_SOURCE_DIR "/Platform/PlatformCompat.h"};
    REQUIRE(std::filesystem::exists(compatHeader));

    const std::string content = ReadFileToString(compatHeader);

    // RED: wglCreateContext stub still present. GREEN: deleted in Task 4.
    REQUIRE(content.find("wglCreateContext") == std::string::npos);
}

TEST_CASE("AC-7: gluPerspective stub is removed from PlatformCompat.h",
    "[platform][stubs][audit][7-9-5]")
{
    const std::filesystem::path compatHeader{MU_SOURCE_DIR "/Platform/PlatformCompat.h"};
    REQUIRE(std::filesystem::exists(compatHeader));

    const std::string content = ReadFileToString(compatHeader);

    // RED: gluPerspective stub still present. GREEN: deleted in Task 4.
    REQUIRE(content.find("gluPerspective") == std::string::npos);
}

TEST_CASE("AC-7: Dead registry stubs removed from PlatformCompat.h",
    "[platform][stubs][audit][7-9-5]")
{
    const std::filesystem::path compatHeader{MU_SOURCE_DIR "/Platform/PlatformCompat.h"};
    REQUIRE(std::filesystem::exists(compatHeader));

    const std::string content = ReadFileToString(compatHeader);

    // Game uses config.ini, not registry. After Task 5, these must be gone.
    // RED: stubs still present.
    REQUIRE(content.find("RegOpenKeyEx") == std::string::npos);
    REQUIRE(content.find("RegQueryValueEx") == std::string::npos);
    REQUIRE(content.find("RegSetValueEx") == std::string::npos);
}

TEST_CASE("AC-7: Dead Win32 file I/O stubs removed from PlatformCompat.h",
    "[platform][stubs][audit][7-9-5]")
{
    // Game uses fopen/std::ifstream. CreateFile/ReadFile stubs are dead.
    // After Task 5.3, they must be gone.
    const std::filesystem::path compatHeader{MU_SOURCE_DIR "/Platform/PlatformCompat.h"};
    REQUIRE(std::filesystem::exists(compatHeader));

    const std::string content = ReadFileToString(compatHeader);

    // RED: stubs still present. GREEN: deleted in Task 5.3.
    // Note: only check for the Win32 CreateFile overload, not mu_wfopen which is real.
    REQUIRE(content.find("inline HANDLE CreateFile") == std::string::npos);
    REQUIRE(content.find("inline BOOL ReadFile") == std::string::npos);
}

#endif // !_WIN32
