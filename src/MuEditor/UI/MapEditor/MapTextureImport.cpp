#include "stdafx.h"

#ifdef _EDITOR

#include "MapTextureImport.h"

#include "Core/Globals/_TextureIndex.h"     // BITMAP_MAPTILE
#include "Render/Sprites/GlobalBitmap.h"     // GL_LINEAR / GL_REPEAT
#include "Render/Textures/ZzzTexture.h"      // LoadBitmap
#include "UI/Console/MuEditorConsoleUI.h"

#include <algorithm>
#include <cstdio>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <vector>

#include <windows.h>
#include <commdlg.h>

namespace fs = std::filesystem;

namespace Editor::TextureImport
{
namespace
{
    // ExtTile slots occupy tile indices 14..29 (ExtTile01..16); index = 13 + i.
    constexpr int EXT_FIRST_TILE_INDEX = 14;
    constexpr int EXT_COUNT            = 16;

    // OZJ = a 24-byte header the loader skips, followed by the raw JPEG stream.
    constexpr int OZJ_HEADER_BYTES = 24;

    std::wstring ToLower(std::wstring s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](wchar_t c) { return (wchar_t)towlower(c); });
        return s;
    }

    // ExtTile01..09 are zero-padded, ExtTile10..16 are not (matches the client's
    // CreateTerrain naming).
    std::wstring ExtTileBaseName(int i)
    {
        wchar_t buf[16];
        if (i < 10)
            swprintf_s(buf, L"ExtTile0%d", i);
        else
            swprintf_s(buf, L"ExtTile%d", i);
        return buf;
    }

    std::wstring WorldFolder(int world)
    {
        return L"Data\\World" + std::to_wstring(world);
    }

    // First ExtTile index (1..16) with no file yet, or -1 if all are taken.
    int FindFreeExtIndex(int world)
    {
        const std::wstring folder = WorldFolder(world);
        for (int i = 1; i <= EXT_COUNT; ++i)
        {
            const std::wstring base = folder + L"\\" + ExtTileBaseName(i);
            std::error_code ec;
            if (!fs::exists(base + L".OZJ", ec) && !fs::exists(base + L".jpg", ec) &&
                !fs::exists(base + L".ozj", ec))
                return i;
        }
        return -1;
    }

    std::vector<unsigned char> ReadFile(const std::wstring& path)
    {
        std::ifstream f(path, std::ios::binary);
        if (!f)
            return {};
        return std::vector<unsigned char>((std::istreambuf_iterator<char>(f)),
                                          std::istreambuf_iterator<char>());
    }

    // Produces the ExtTile OZJ file for the target slot from a .jpg/.jpeg (wrapped
    // with a 24-byte header) or a .ozj (copied verbatim). Returns false on any
    // unsupported input or write error.
    bool WriteExtTileOzj(const std::wstring& destOzj, const std::wstring& sourcePath)
    {
        const std::wstring ext = ToLower(fs::path(sourcePath).extension().wstring());
        const std::vector<unsigned char> src = ReadFile(sourcePath);
        if (src.empty())
            return false;

        std::ofstream out(destOzj, std::ios::binary);
        if (!out)
            return false;

        if (ext == L".ozj")
        {
            out.write(reinterpret_cast<const char*>(src.data()), src.size());
        }
        else if (ext == L".jpg" || ext == L".jpeg")
        {
            // Wrap the raw JPEG in an OZJ container: 24 zero header bytes (the
            // loader only special-cases a "MUHD" stamp) + the JPEG payload.
            const char header[OZJ_HEADER_BYTES] = { 0 };
            out.write(header, OZJ_HEADER_BYTES);
            out.write(reinterpret_cast<const char*>(src.data()), src.size());
        }
        else
        {
            return false;
        }
        return out.good();
    }
}

int UseTextureFile(int world, const std::wstring& sourcePath)
{
    const int i = FindFreeExtIndex(world);
    if (i < 0)
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] Import failed: all 16 ExtTile slots are used on this map");
        return -1;
    }

    std::error_code ec;
    fs::create_directories(WorldFolder(world), ec);

    const std::wstring destOzj = WorldFolder(world) + L"\\" + ExtTileBaseName(i) + L".OZJ";
    if (!WriteExtTileOzj(destOzj, sourcePath))
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] Import failed: source must be a .jpg/.jpeg or .ozj file");
        return -1;
    }

    // Load it live into the matching tile slot (LoadBitmap prepends Data\\ and
    // swaps .jpg -> .OZJ, so pass the world-relative .jpg name).
    const int tileIndex = EXT_FIRST_TILE_INDEX + (i - 1);
    const std::wstring relative = L"World" + std::to_wstring(world) + L"\\" + ExtTileBaseName(i) + L".jpg";
    if (!LoadBitmap(relative.c_str(), BITMAP_MAPTILE + tileIndex, GL_LINEAR, GL_REPEAT, false))
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] Import: file written but failed to load as a texture");
        return -1;
    }

    // The palette/terrain read this slot through a quick-cache that may have cached
    // it as "empty" (it was polled every frame while unused). Invalidate that entry
    // so the freshly loaded texture shows immediately instead of only after a reload.
    Bitmaps.RefreshCacheEntry(BITMAP_MAPTILE + tileIndex);

    char msg[128];
    snprintf(msg, sizeof(msg), "[MapEditor] Imported texture into slot %d (ExtTile%02d)", tileIndex, i);
    g_MuEditorConsoleUI.LogEditor(msg);
    return tileIndex;
}

bool PickImageFile(std::wstring& outPath)
{
    wchar_t file[MAX_PATH] = { 0 };
    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Textures (*.jpg;*.jpeg;*.ozj)\0*.jpg;*.jpeg;*.ozj\0All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"Select a texture to import (JPEG or OZJ)";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn))
        return false;
    outPath = file;
    return true;
}

} // namespace Editor::TextureImport

#endif // _EDITOR
