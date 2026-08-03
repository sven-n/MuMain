#include "stdafx.h"

#ifdef _EDITOR

#include "MapMinimapCapture.h"
#include "MapEditorFileUtil.h"
#include "UI/Console/MuEditorConsoleUI.h"

#include "Render/Terrain/ZzzLodTerrain.h"    // TerrainMapping*/TERRAIN_SIZE/TERRAIN_INDEX
#include "Render/Sprites/GlobalBitmap.h"      // Bitmaps / BITMAP_t
#include "Core/Globals/_TextureIndex.h"       // BITMAP_MAPTILE

#include <commdlg.h>   // GetOpenFileNameW (.tga picker)

#include <cstdio>
#include <vector>

extern unsigned char TerrainMappingLayer1[];
extern unsigned char TerrainMappingLayer2[];
extern float         TerrainMappingAlpha[];

namespace Editor::Minimap
{
namespace
{
    constexpr int kOut = 1024;   // every stock world minimap is 1024x1024
    constexpr int kTileSlots = 30;   // BITMAP_MAPTILE + 0..29

    struct RGB { float r, g, b; bool valid; };

    // Average colour of a tile texture, read straight from its GL texture. Core GL,
    // so no extension loading needed. Returns invalid if the slot isn't loaded.
    RGB AverageTileColor(int slot)
    {
        RGB out{ 0, 0, 0, false };
        BITMAP_t& b = Bitmaps[BITMAP_MAPTILE + slot];
        if (b.TextureNumber == 0)
            return out;

        glBindTexture(GL_TEXTURE_2D, b.TextureNumber);
        GLint tw = 0, th = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
        if (tw < 1 || th < 1)
            return out;

        std::vector<BYTE> buf(static_cast<size_t>(tw) * th * 4);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

        double sr = 0, sg = 0, sb = 0;
        const size_t px = static_cast<size_t>(tw) * th;
        for (size_t i = 0; i < px; ++i)
        {
            sr += buf[i * 4 + 0];
            sg += buf[i * 4 + 1];
            sb += buf[i * 4 + 2];
        }
        out.r = static_cast<float>(sr / px);
        out.g = static_cast<float>(sg / px);
        out.b = static_cast<float>(sb / px);
        out.valid = true;
        return out;
    }

    void FillTgaHeader(BYTE hdr[18])
    {
        memset(hdr, 0, 18);
        hdr[2] = 2;                               // uncompressed true-color
        hdr[12] = kOut & 0xFF; hdr[13] = kOut >> 8;
        hdr[14] = kOut & 0xFF; hdr[15] = kOut >> 8;
        hdr[16] = 32;                             // bits per pixel
        hdr[17] = 8;                              // 8 alpha bits, bottom-origin
    }

    bool WriteFile(const wchar_t* path, const BYTE* prefix, int prefixLen,
                   const BYTE* hdr, const std::vector<BYTE>& pixels)
    {
        FILE* fp = _wfopen(path, L"wb");
        if (fp == nullptr)
            return false;
        if (prefixLen > 0)
            fwrite(prefix, 1, prefixLen, fp);
        fwrite(hdr, 1, 18, fp);
        fwrite(pixels.data(), 1, pixels.size(), fp);
        fclose(fp);
        return true;
    }
}

bool GenerateFromTiles(int world, std::string& outMsg)
{
    // One representative colour per tile slot (averaged once).
    RGB slotColor[kTileSlots];
    for (int i = 0; i < kTileSlots; ++i)
        slotColor[i] = AverageTileColor(i);
    glBindTexture(GL_TEXTURE_2D, 0);

    auto colorOf = [&](int idx, float& r, float& g, float& b)
    {
        const int l1 = TerrainMappingLayer1[idx] % kTileSlots;
        const int l2 = TerrainMappingLayer2[idx] % kTileSlots;
        float a = TerrainMappingAlpha[idx];
        if (a < 0.f) a = 0.f; else if (a > 1.f) a = 1.f;

        const RGB& c1 = slotColor[l1];
        const RGB& c2 = slotColor[l2];
        // Blend layer2 over layer1 by the tile's alpha, matching how it renders.
        const float r1 = c1.valid ? c1.r : 0.f, g1 = c1.valid ? c1.g : 0.f, b1 = c1.valid ? c1.b : 0.f;
        const float r2 = c2.valid ? c2.r : r1,  g2 = c2.valid ? c2.g : g1,  b2 = c2.valid ? c2.b : b1;
        r = r1 * (1.f - a) + r2 * a;
        g = g1 * (1.f - a) + g2 * a;
        b = b1 * (1.f - a) + b2 * a;
    };

    // Compose a bottom-origin BGRA image. The game indexes the minimap by
    // (horizontal = world Y, vertical = world X) - see NewUIMiniMap.cpp:141 - and
    // draws it as a bottom-origin TGA. So for file row f (0 = bottom) and column c:
    //   display row (from top) = kOut-1-f  ->  world X
    //   column c                            ->  world Y
    // Building the image from the SAME (x,y) mapping the marker uses guarantees the
    // in-game position marker lands exactly on the player. No screen capture, no HUD,
    // no perspective.
    std::vector<BYTE> pixels(static_cast<size_t>(kOut) * kOut * 4);
    for (int f = 0; f < kOut; ++f)
    {
        const int wx = (kOut - 1 - f) * TERRAIN_SIZE / kOut;   // world X (0..255)
        for (int c = 0; c < kOut; ++c)
        {
            const int wy = c * TERRAIN_SIZE / kOut;            // world Y (0..255)
            const int idx = TERRAIN_INDEX(wx, wy);             // y*256 + x
            float r, g, b;
            colorOf(idx, r, g, b);
            BYTE* out = pixels.data() + (static_cast<size_t>(f) * kOut + c) * 4;
            out[0] = static_cast<BYTE>(b + 0.5f);   // TGA is BGRA
            out[1] = static_cast<BYTE>(g + 0.5f);
            out[2] = static_cast<BYTE>(r + 0.5f);
            out[3] = 255;
        }
    }

    BYTE hdr[18];
    FillTgaHeader(hdr);
    const BYTE oztPrefix[4] = { 0x00, 0x00, 0x02, 0x00 };

    wchar_t tgaPath[128], oztPath[128];
    swprintf_s(tgaPath, L"Data\\World%d\\mini_map.tga", world);
    swprintf_s(oztPath, L"Data\\World%d\\mini_map.OZT", world);

    const bool okTga = WriteFile(tgaPath, nullptr, 0, hdr, pixels);
    const bool okOzt = WriteFile(oztPath, oztPrefix, 4, hdr, pixels);
    if (!okTga || !okOzt)
    {
        outMsg = "Generate FAILED: could not write the minimap file(s).";
        g_MuEditorConsoleUI.LogEditor("[MapEditor] Minimap generate failed to write files");
        return false;
    }

    Editor::Files::MirrorNextToExe(tgaPath, world);
    Editor::Files::MirrorNextToExe(oztPath, world);

    char buf[160];
    snprintf(buf, sizeof(buf),
             "Generated mini_map.OZT + .tga (1024x1024) from the map tiles, in Data\\World%d\\ and next to Main.exe.",
             world);
    outMsg = buf;
    g_MuEditorConsoleUI.LogEditor("[MapEditor] Generated minimap from tiles (OZT + TGA)");
    return true;
}

bool WrapTgaToOzt(int world, std::string& outMsg)
{
    wchar_t file[MAX_PATH] = { 0 };
    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"TGA image (*.tga)\0*.tga\0All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"Select your edited minimap .tga";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn))
        return false;   // cancelled

    FILE* fp = _wfopen(file, L"rb");
    if (fp == nullptr)
    {
        outMsg = "Could not open that .tga.";
        return false;
    }
    std::vector<BYTE> raw;
    fseek(fp, 0, SEEK_END);
    const long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz > 18)
    {
        raw.resize(static_cast<size_t>(sz));
        fread(raw.data(), 1, raw.size(), fp);
    }
    fclose(fp);
    if (raw.size() <= 18)
    {
        outMsg = "That file is too small to be a TGA.";
        return false;
    }

    const int idLen  = raw[0];
    const int imgType = raw[2];                       // 2 = uncompressed, 10 = RLE
    const int width  = raw[12] | (raw[13] << 8);
    const int height = raw[14] | (raw[15] << 8);
    const int bpp    = raw[16];
    const bool topOrigin = (raw[17] & 0x20) != 0;     // descriptor bit 5

    if ((imgType != 2 && imgType != 10) || (bpp != 24 && bpp != 32) || width <= 0 || height <= 0)
    {
        outMsg = "Unsupported TGA. Export it as 24- or 32-bit uncompressed true-color.";
        return false;
    }

    const int channels = bpp / 8;
    const size_t pxCount = static_cast<size_t>(width) * height;
    std::vector<BYTE> src(pxCount * channels);        // decoded, in file scan order

    size_t p = 18 + idLen;                             // skip id field (no colormap for true-color)
    if (imgType == 2)
    {
        if (p + pxCount * channels > raw.size()) { outMsg = "TGA pixel data is truncated."; return false; }
        memcpy(src.data(), &raw[p], pxCount * channels);
    }
    else // RLE (type 10)
    {
        size_t out = 0;
        while (out < pxCount && p < raw.size())
        {
            const BYTE packet = raw[p++];
            const int count = (packet & 0x7F) + 1;
            if (packet & 0x80)                         // run-length packet
            {
                if (p + channels > raw.size()) break;
                for (int i = 0; i < count && out < pxCount; ++i, ++out)
                    memcpy(&src[out * channels], &raw[p], channels);
                p += channels;
            }
            else                                       // raw packet
            {
                for (int i = 0; i < count && out < pxCount; ++i, ++out)
                {
                    if (p + channels > raw.size()) break;
                    memcpy(&src[out * channels], &raw[p], channels);
                    p += channels;
                }
            }
        }
        if (out < pxCount) { outMsg = "TGA RLE data is truncated."; return false; }
    }

    // Normalise to a bottom-origin BGRA buffer - the exact layout the OZT loader wants
    // (and what GenerateFromTiles writes). Source rows go top->bottom if the origin bit
    // is set, else bottom->top.
    std::vector<BYTE> pixels(pxCount * 4);
    for (int f = 0; f < height; ++f)                   // f = destination row, 0 = bottom
    {
        const int srcRow = topOrigin ? (height - 1 - f) : f;
        const BYTE* s = &src[static_cast<size_t>(srcRow) * width * channels];
        BYTE* d = &pixels[static_cast<size_t>(f) * width * 4];
        for (int c = 0; c < width; ++c)
        {
            d[0] = s[0];                               // B
            d[1] = s[1];                               // G
            d[2] = s[2];                               // R
            d[3] = (channels == 4) ? s[3] : 255;       // A
            s += channels;
            d += 4;
        }
    }

    // Header with this image's real dimensions (not forced to 1024).
    BYTE hdr[18];
    memset(hdr, 0, 18);
    hdr[2] = 2;
    hdr[12] = width & 0xFF;  hdr[13] = (width >> 8) & 0xFF;
    hdr[14] = height & 0xFF; hdr[15] = (height >> 8) & 0xFF;
    hdr[16] = 32;
    hdr[17] = 8;

    const BYTE oztPrefix[4] = { 0x00, 0x00, 0x02, 0x00 };
    wchar_t oztPath[128];
    swprintf_s(oztPath, L"Data\\World%d\\mini_map.OZT", world);
    if (!WriteFile(oztPath, oztPrefix, 4, hdr, pixels))
    {
        outMsg = "Could not write mini_map.OZT.";
        return false;
    }
    Editor::Files::MirrorNextToExe(oztPath, world);

    char buf[160];
    snprintf(buf, sizeof(buf),
             "Wrote mini_map.OZT (%dx%d) to Data\\World%d\\ and next to Main.exe. Relog to see it.",
             width, height, world);
    outMsg = buf;
    g_MuEditorConsoleUI.LogEditor("[MapEditor] Wrapped .tga into mini_map.OZT");
    return true;
}

} // namespace Editor::Minimap

#endif // _EDITOR
