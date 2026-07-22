#include "stdafx.h"

#ifdef _EDITOR

#include "MapEditorSave.h"
#include "MapEditorFileUtil.h"

#include "Render/Terrain/ZzzLodTerrain.h"   // terrain arrays + MapFileEncrypt + TERRAIN_SIZE
#include "UI/Console/MuEditorConsoleUI.h"

#include <cstdio>
#include <cstring>
#include <memory>

namespace Editor::MapSave
{
namespace
{
    // Bytes-per-cell for each mapping array in the .map layout.
    constexpr int CELLS = TERRAIN_SIZE * TERRAIN_SIZE;

    // .map plaintext layout (matches OpenTerrainMapping in ZzzLodTerrain.cpp):
    //   byte version, byte mapNumber, byte Layer1[CELLS], byte Layer2[CELLS], byte Alpha[CELLS]
    constexpr int HEADER_BYTES = 2;
    constexpr int PLAIN_BYTES  = HEADER_BYTES + CELLS * 3;

    constexpr BYTE MAP_VERSION = 0;
}

bool SaveMappingEncrypted(int worldNumber, int mapNumber)
{
    // Build the decrypted byte image exactly as the loader expects to read it.
    auto plain = std::make_unique<BYTE[]>(PLAIN_BYTES);
    int p = 0;
    plain[p++] = MAP_VERSION;
    plain[p++] = static_cast<BYTE>(mapNumber);

    std::memcpy(&plain[p], TerrainMappingLayer1, CELLS);
    p += CELLS;
    std::memcpy(&plain[p], TerrainMappingLayer2, CELLS);
    p += CELLS;
    for (int i = 0; i < CELLS; ++i)
        plain[p++] = static_cast<BYTE>(TerrainMappingAlpha[i] * 255.f);

    // Encrypt with the same rolling-XOR cipher the loader decrypts with.
    auto enc = std::make_unique<BYTE[]>(PLAIN_BYTES);
    const int encBytes = MapFileEncrypt(enc.get(), plain.get(), PLAIN_BYTES);

    wchar_t fileName[128];
    swprintf_s(fileName, L"Data\\World%d\\EncTerrain%d.map", worldNumber, worldNumber);

    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == nullptr)
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] SaveMapping FAILED: could not open file for write");
        return false;
    }

    fwrite(enc.get(), 1, encBytes, fp);
    fclose(fp);

    g_MuEditorConsoleUI.LogEditor("[MapEditor] Saved terrain mapping (encrypted) to EncTerrain.map");
    Editor::Files::MirrorNextToExe(fileName, worldNumber);
    return true;
}

} // namespace Editor::MapSave

#endif // _EDITOR
