// Portable backend for the GDI text pipeline (WinGdi.h, issue #462 Phase 4).
//
// CUIRenderText draws UI text the Win32 way: TextOut rasterizes white-on-black
// into a 24bpp top-down DIB section, and the engine scans those pixels into a
// GL texture. This file reproduces exactly that contract with stb_truetype:
// CreateFont resolves a system TTF by weight, CreateDIBSection allocates the
// pixel buffer with DIB pitch rules, and TextOut composites antialiased glyph
// coverage between the DC's background and text colors.
#ifndef _WIN32

#include "Core/Platform/WinGdi.h"

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <unistd.h>                    // readlink (executable path)
#include "Core/Platform/WinNls.h"      // WideCharToMultiByte / CP_UTF8
#include "Core/Platform/BundledFonts.h" // curated font list shared with Windows

namespace
{
    enum class MuGdiKind : unsigned { Font = 0x464F4E54, Bitmap = 0x424D5031, Dc = 0x44432020 };

    struct MuGdiObject
    {
        MuGdiKind kind;
    };

    // A loaded TTF face, shared by every font of the same weight class.
    struct MuGdiFace
    {
        std::vector<unsigned char> data;
        stbtt_fontinfo info{};
        bool valid = false;
    };

    // One human-readable line per resolved weight, surfaced to the crash log so
    // a "no UI text" report shows what was found (or not) on the user's system.
    std::string g_fontDiag;
    void RecordFontDiag(const std::string& line) { g_fontDiag += line; g_fontDiag += '\n'; }

    struct MuGdiFont : MuGdiObject
    {
        const MuGdiFace* face = nullptr;
        int cellHeight = 0;   // requested pixel height (GDI cell height)
        float scale = 0.0f;   // stbtt scale factor for cellHeight
        int ascentPx = 0;     // scaled ascent in pixels
    };

    struct MuGdiBitmap : MuGdiObject
    {
        int width = 0;
        int height = 0;       // rows; stored top-down like a negative-height DIB
        int pitch = 0;        // bytes per row, DWORD-aligned like a DIB
        std::vector<BYTE> pixels;
    };

    struct MuGdiDC : MuGdiObject
    {
        MuGdiBitmap* bitmap = nullptr;
        MuGdiFont* font = nullptr;
        COLORREF textColor = RGB(255, 255, 255);
        COLORREF bkColor = RGB(0, 0, 0);
    };

    template <typename T>
    T* AsGdi(void* handle, MuGdiKind kind)
    {
        auto* obj = static_cast<MuGdiObject*>(handle);
        return (obj && obj->kind == kind) ? static_cast<T*>(obj) : nullptr;
    }

    // Candidate faces per weight class, tried in order. MU_FONT/MU_FONT_BOLD
    // override the lookup for systems whose fonts live elsewhere.

    // Ask fontconfig for a font file via fc-match. This is the distro-agnostic
    // way to locate a system font (paths differ across Debian/Fedora/Arch/etc.),
    // and works without linking libfontconfig. `family` is the requested UI font
    // name; empty falls back to the generic "sans-serif". Empty result if
    // fc-match is unavailable.
    std::string FontconfigMatch(const std::string& family, bool bold)
    {
        std::string fam = family.empty() ? std::string("sans-serif") : family;
        // Drop single quotes so the name can't break out of the shell-quoted
        // pattern; real font family names never contain them.
        fam.erase(std::remove(fam.begin(), fam.end(), '\''), fam.end());
        const std::string cmd = "fc-match -f '%{file}' '" + fam + (bold ? ":bold" : "")
                              + "' 2>/dev/null";
        FILE* p = ::popen(cmd.c_str(), "r");
        if (!p) return {};
        std::string out;
        char buf[4096];
        size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf), p)) > 0)
            out.append(buf, n);
        ::pclose(p);
        while (!out.empty() && (out.back() == '\n' || out.back() == '\r' || out.back() == ' '))
            out.pop_back();
        return out;
    }

    // UTF-8 copy of a wide string via the WideCharToMultiByte shim - the same
    // conversion the rest of the engine uses, rather than a hand-rolled encoder.
    std::string ToUtf8(const wchar_t* ws)
    {
        if (!ws) return {};
        const int n = WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);
        if (n <= 0) return {};
        std::vector<char> buf(static_cast<size_t>(n));
        WideCharToMultiByte(CP_UTF8, 0, ws, -1, buf.data(), n, nullptr, nullptr);
        return std::string(buf.data());   // stops at the trailing NUL
    }

    // Directory of the running executable (with a trailing '/'), so bundled fonts
    // resolve regardless of the working directory. Read via /proc/self/exe, the
    // same way Dotnet/Connection.h locates the client library. Empty on failure.
    const std::string& ExeDir()
    {
        static const std::string dir = []() -> std::string {
            char buf[4096];
            const ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
            if (n <= 0) return {};
            buf[n] = '\0';
            const std::string path(buf);
            const auto slash = path.find_last_of('/');
            return slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
        }();
        return dir;
    }

    // Curated fonts shipped in the client's ./fonts directory. Resolving these by
    // name lets a chosen font work even when it is not installed system-wide, so
    // every option the UI offers is guaranteed present. Names match what the
    // options UI lists and what config stores.
    std::string BundledFontPath(const std::string& family, bool bold)
    {
        for (const auto& e : kBundledFonts)
            if (family == e.family)
                return ExeDir() + (bold ? e.bold : e.regular);
        return {};
    }

    // Loads (and caches) the TTF face for a UI font family + weight. The cache is
    // a std::map so node pointers stay stable: when the user switches fonts a new
    // face loads under a new key, and MuGdiFont objects created earlier keep
    // pointing at their still-alive face. `family` empty -> system sans-serif.
    const MuGdiFace* LoadFace(const std::string& family, bool bold)
    {
        static std::map<std::pair<std::string, bool>, MuGdiFace> s_cache;
        const auto key = std::make_pair(family, bold);
        if (auto it = s_cache.find(key); it != s_cache.end())
        {
            if (it->second.valid)
                return &it->second;
            // Already scanned and failed: bold reuses regular, regular gives up.
            return bold ? LoadFace(family, false) : nullptr;
        }
        MuGdiFace& face = s_cache[key];   // default-inserts the (empty) entry

        // Order: explicit override, then fontconfig for the requested family,
        // then well-known paths as a last resort if fontconfig is missing.
        std::vector<std::string> candidates;
        if (const char* envPath = std::getenv(bold ? "MU_FONT_BOLD" : "MU_FONT"))
            candidates.emplace_back(envPath);
        // Prefer a bundled curated font (./fonts) so a chosen name always works,
        // even without a system install. Falls through to fontconfig if absent.
        if (std::string bundled = BundledFontPath(family, bold); !bundled.empty())
            candidates.push_back(std::move(bundled));
        if (std::string fc = FontconfigMatch(family, bold); !fc.empty())
            candidates.push_back(std::move(fc));
        const char* fallbacks[] = {
            bold ? "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
                 : "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            bold ? "/usr/share/fonts/dejavu/DejaVuSans-Bold.ttf"
                 : "/usr/share/fonts/dejavu/DejaVuSans.ttf",
            bold ? "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf"
                 : "/usr/share/fonts/TTF/DejaVuSans.ttf",
            bold ? "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"
                 : "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            bold ? "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf"
                 : "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        };
        for (const char* f : fallbacks)
            candidates.emplace_back(f);

        for (const std::string& path : candidates)
        {
            FILE* fp = std::fopen(path.c_str(), "rb");
            if (!fp) continue;
            std::fseek(fp, 0, SEEK_END);
            const long size = std::ftell(fp);
            std::fseek(fp, 0, SEEK_SET);
            if (size > 0)
            {
                face.data.resize(static_cast<size_t>(size));
                if (std::fread(face.data.data(), 1, face.data.size(), fp) == face.data.size() &&
                    stbtt_InitFont(&face.info, face.data.data(),
                                   stbtt_GetFontOffsetForIndex(face.data.data(), 0)))
                {
                    face.valid = true;
                }
            }
            std::fclose(fp);
            if (face.valid)
            {
                RecordFontDiag((bold ? "bold:    " : "regular: ") + path);
                return &face;
            }
            face.data.clear();
        }

        // No bold face found: fall back to the regular one rather than no text.
        if (bold)
        {
            RecordFontDiag("bold:    not found, using regular");
            return LoadFace(family, false);
        }

        RecordFontDiag("regular: NOT FOUND (fontconfig and known paths failed)");
        std::fprintf(stderr, "[GdiText] No usable TTF found; UI text disabled. "
                             "Install a sans-serif font (e.g. fonts-dejavu-core) "
                             "or set MU_FONT to a .ttf path.\n");
        return nullptr;
    }

    // Width of `text` in pixels at the font's scale, accumulating advances the
    // way TextOut steps the pen so measurement and rendering agree.
    int MeasureWidth(const MuGdiFont* font, LPCWSTR text, int len)
    {
        float pen = 0.0f;
        for (int i = 0; i < len; ++i)
        {
            int advance = 0, lsb = 0;
            stbtt_GetCodepointHMetrics(&font->face->info, static_cast<int>(text[i]), &advance, &lsb);
            pen += advance * font->scale;
        }
        return static_cast<int>(std::ceil(pen));
    }
}

// External linkage so the crash-log writer can read it. One line per resolved
// font weight; empty until the first font is created.
std::string MuFontDiagnostics()
{
    return g_fontDiag;
}

HFONT CreateFontW(int cHeight, int /*cWidth*/, int /*cEscapement*/, int /*cOrientation*/,
                  int cWeight, DWORD /*bItalic*/, DWORD /*bUnderline*/, DWORD /*bStrikeOut*/,
                  DWORD /*iCharSet*/, DWORD /*iOutPrecision*/, DWORD /*iClipPrecision*/,
                  DWORD /*iQuality*/, DWORD /*iPitchAndFamily*/, LPCWSTR pszFaceName)
{
    // The requested GDI face name is the font selector's channel: the shared font
    // factory passes the configured UI font name here. "Tahoma" is the built-in
    // default and has no Linux equivalent, so map it to the empty family and let
    // fontconfig pick the system sans-serif (unchanged default look). Any other
    // name is resolved as-is via fontconfig.
    std::string family = ToUtf8(pszFaceName);
    if (family == "Tahoma")
        family.clear();

    const MuGdiFace* face = LoadFace(family, cWeight >= FW_SEMIBOLD);
    if (!face)
        return nullptr;

    auto* font = new MuGdiFont();
    font->kind = MuGdiKind::Font;
    font->face = face;
    font->cellHeight = std::max(1, std::abs(cHeight));
    font->scale = stbtt_ScaleForPixelHeight(&face->info, static_cast<float>(font->cellHeight));

    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&face->info, &ascent, &descent, &lineGap);
    font->ascentPx = static_cast<int>(std::lround(ascent * font->scale));

    return reinterpret_cast<HFONT>(font);
}

HBITMAP CreateDIBSection(HDC /*hdc*/, const BITMAPINFO* pbmi, UINT /*usage*/,
                         void** ppvBits, HANDLE /*hSection*/, DWORD /*offset*/)
{
    if (ppvBits) *ppvBits = nullptr;
    if (!pbmi || pbmi->bmiHeader.biBitCount != 24)
        return nullptr;

    const int width = static_cast<int>(pbmi->bmiHeader.biWidth);
    const int height = static_cast<int>(std::abs(pbmi->bmiHeader.biHeight));
    if (width <= 0 || height <= 0)
        return nullptr;

    auto* bitmap = new MuGdiBitmap();
    bitmap->kind = MuGdiKind::Bitmap;
    bitmap->width = width;
    bitmap->height = height;
    bitmap->pitch = ((width * 24 + 31) & ~31) >> 3;
    bitmap->pixels.assign(static_cast<size_t>(bitmap->pitch) * height, 0);

    if (ppvBits) *ppvBits = bitmap->pixels.data();
    return reinterpret_cast<HBITMAP>(bitmap);
}

HDC CreateCompatibleDC(HDC /*hdc*/)
{
    auto* dc = new MuGdiDC();
    dc->kind = MuGdiKind::Dc;
    return reinterpret_cast<HDC>(dc);
}

BOOL DeleteDC(HDC hdc)
{
    auto* dc = AsGdi<MuGdiDC>(hdc, MuGdiKind::Dc);
    if (!dc) return FALSE;
    delete dc;
    return TRUE;
}

namespace
{
    BOOL DeleteGdiObject(void* handle)
    {
        auto* obj = static_cast<MuGdiObject*>(handle);
        if (!obj) return FALSE;
        switch (obj->kind)
        {
            case MuGdiKind::Font:   delete static_cast<MuGdiFont*>(obj);   return TRUE;
            case MuGdiKind::Bitmap: delete static_cast<MuGdiBitmap*>(obj); return TRUE;
            // A DC is not a GDI object; deleting it here would be a caller bug.
            case MuGdiKind::Dc:     return FALSE;
        }
        return FALSE;
    }

    HGDIOBJ SelectIntoDC(HDC hdc, void* handle)
    {
        auto* dc = AsGdi<MuGdiDC>(hdc, MuGdiKind::Dc);
        auto* obj = static_cast<MuGdiObject*>(handle);
        if (!dc || !obj) return nullptr;
        switch (obj->kind)
        {
            case MuGdiKind::Font:
            {
                auto* prev = dc->font;
                dc->font = static_cast<MuGdiFont*>(obj);
                return reinterpret_cast<HGDIOBJ>(prev);
            }
            case MuGdiKind::Bitmap:
            {
                auto* prev = dc->bitmap;
                dc->bitmap = static_cast<MuGdiBitmap*>(obj);
                return reinterpret_cast<HGDIOBJ>(prev);
            }
            case MuGdiKind::Dc:
                return nullptr;
        }
        return nullptr;
    }
}

BOOL DeleteObject(HFONT hObject)   { return DeleteGdiObject(hObject); }
BOOL DeleteObject(HGDIOBJ hObject) { return DeleteGdiObject(hObject); }
BOOL DeleteObject(HBITMAP hObject) { return DeleteGdiObject(hObject); }
BOOL DeleteObject(HBRUSH hObject)  { return DeleteGdiObject(hObject); }

HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h) { return SelectIntoDC(hdc, h); }
HGDIOBJ SelectObject(HDC hdc, HBITMAP h) { return SelectIntoDC(hdc, h); }
HGDIOBJ SelectObject(HDC hdc, HFONT h)   { return SelectIntoDC(hdc, h); }

COLORREF SetBkColor(HDC hdc, COLORREF color)
{
    auto* dc = AsGdi<MuGdiDC>(hdc, MuGdiKind::Dc);
    if (!dc) return 0;
    const COLORREF prev = dc->bkColor;
    dc->bkColor = color;
    return prev;
}

COLORREF SetTextColor(HDC hdc, COLORREF color)
{
    auto* dc = AsGdi<MuGdiDC>(hdc, MuGdiKind::Dc);
    if (!dc) return 0;
    const COLORREF prev = dc->textColor;
    dc->textColor = color;
    return prev;
}

BOOL GetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int c, LPSIZE psizl)
{
    if (!psizl || (!lpString && c != 0)) return FALSE;

    auto* dc = AsGdi<MuGdiDC>(hdc, MuGdiKind::Dc);
    const MuGdiFont* font = dc ? dc->font : nullptr;
    const int len = (c >= 0) ? c : static_cast<int>(wcslen(lpString));
    if (!font || !font->face)
    {
        // No font selected yet: a small fixed cell keeps layout code moving.
        psizl->cx = len * 8;
        psizl->cy = 16;
        return TRUE;
    }

    psizl->cx = MeasureWidth(font, lpString, len);
    psizl->cy = font->cellHeight;
    return TRUE;
}

BOOL TextOut(HDC hdc, int x, int y, LPCWSTR lpString, int c)
{
    auto* dc = AsGdi<MuGdiDC>(hdc, MuGdiKind::Dc);
    if (!dc || !dc->bitmap || !dc->font || !dc->font->face || !lpString)
        return FALSE;

    MuGdiBitmap* bmp = dc->bitmap;
    const MuGdiFont* font = dc->font;
    const stbtt_fontinfo* info = &font->face->info;
    const int len = (c >= 0) ? c : static_cast<int>(wcslen(lpString));

    // GDI's default OPAQUE mode paints the text cell with the background color
    // first; the engine scans exactly this rectangle out of the buffer.
    const int cellWidth = MeasureWidth(font, lpString, len);
    const BYTE bk[3] = { GetBValue(dc->bkColor), GetGValue(dc->bkColor), GetRValue(dc->bkColor) };
    const BYTE tx[3] = { GetBValue(dc->textColor), GetGValue(dc->textColor), GetRValue(dc->textColor) };

    const int clearX1 = std::min(std::max(x, 0), bmp->width);
    const int clearX2 = std::min(x + cellWidth, bmp->width);
    const int clearY1 = std::min(std::max(y, 0), bmp->height);
    const int clearY2 = std::min(y + font->cellHeight, bmp->height);
    for (int py = clearY1; py < clearY2; ++py)
    {
        BYTE* row = bmp->pixels.data() + static_cast<size_t>(py) * bmp->pitch;
        for (int px = clearX1; px < clearX2; ++px)
            std::memcpy(row + px * 3, bk, 3);
    }

    // Rasterize each glyph and blend its coverage between background and text
    // color. Scratch buffer is reused across glyphs.
    static thread_local std::vector<unsigned char> coverage;
    const int baseline = y + font->ascentPx;
    float pen = static_cast<float>(x);
    for (int i = 0; i < len; ++i)
    {
        const int cp = static_cast<int>(lpString[i]);
        int advance = 0, lsb = 0;
        stbtt_GetCodepointHMetrics(info, cp, &advance, &lsb);

        const float xShift = pen - std::floor(pen);
        int gx1, gy1, gx2, gy2;
        stbtt_GetCodepointBitmapBoxSubpixel(info, cp, font->scale, font->scale, xShift, 0.0f,
                                            &gx1, &gy1, &gx2, &gy2);
        const int gw = gx2 - gx1, gh = gy2 - gy1;
        if (gw > 0 && gh > 0)
        {
            coverage.assign(static_cast<size_t>(gw) * gh, 0);
            stbtt_MakeCodepointBitmapSubpixel(info, coverage.data(), gw, gh, gw,
                                              font->scale, font->scale, xShift, 0.0f, cp);

            const int originX = static_cast<int>(std::floor(pen)) + gx1;
            const int originY = baseline + gy1;
            for (int gy = 0; gy < gh; ++gy)
            {
                const int py = originY + gy;
                if (py < 0 || py >= bmp->height) continue;
                BYTE* row = bmp->pixels.data() + static_cast<size_t>(py) * bmp->pitch;
                for (int gx = 0; gx < gw; ++gx)
                {
                    const int px = originX + gx;
                    if (px < 0 || px >= bmp->width) continue;
                    const unsigned cov = coverage[static_cast<size_t>(gy) * gw + gx];
                    if (cov == 0) continue;
                    BYTE* p = row + px * 3;
                    for (int ch = 0; ch < 3; ++ch)
                        p[ch] = static_cast<BYTE>((tx[ch] * cov + bk[ch] * (255u - cov)) / 255u);
                }
            }
        }

        // No kerning: GDI TextOut does not kern, and MeasureWidth must step the
        // pen identically so the cleared cell always covers the glyphs.
        pen += advance * font->scale;
    }

    return TRUE;
}

#endif // !_WIN32
