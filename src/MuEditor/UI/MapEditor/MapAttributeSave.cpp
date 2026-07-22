#include "stdafx.h"

#ifdef _EDITOR

#include "MapAttributeSave.h"
#include "MapEditorFileUtil.h"

#include "Render/Terrain/ZzzLodTerrain.h"   // TerrainWall / MapFileEncrypt / TERRAIN_SIZE
#include "Core/Globals/_crypt.h"            // BuxConvert
#include "UI/Console/MuEditorConsoleUI.h"

#include <commdlg.h>   // GetOpenFileNameW (server base picker)

#include <cstdio>
#include <memory>
#include <vector>

namespace Editor::AttrSave
{
namespace
{
    constexpr int CELLS        = TERRAIN_SIZE * TERRAIN_SIZE;   // 65536
    constexpr int HEADER_BYTES = 4;                             // version, mapId, w, h
    constexpr int PLAIN_BYTES  = HEADER_BYTES + CELLS;          // 65540

    constexpr BYTE ATT_VERSION = 0;
    constexpr BYTE ATT_EXTENT  = 255;   // width/height bytes are 255 (=256 tiles)

    // The server's TerrainData keeps a 3-byte header before the attribute block.
    constexpr int SERVER_HEADER_BYTES = 3;

    // Writes a dead-simple instructions file next to the server .att.
    void WriteHowTo(int mapNumber)
    {
        wchar_t howToName[128];
        swprintf_s(howToName, L"terrain_map%d_server_HOWTO.txt", mapNumber);

        FILE* fp = _wfopen(howToName, L"wb");
        if (fp == nullptr)
            return;

        fprintf(fp,
"HOW TO APPLY THIS TERRAIN FILE TO YOUR SERVER\r\n"
"=============================================\r\n"
"File: terrain_map%d_server.att   (server map Number = %d)\r\n"
"\r\n"
"Four steps, no command line.\r\n"
"\r\n"
"  1. Open the OpenMU Admin Panel in your browser.\r\n"
"  2. Go to the map list and edit the map with Number = %d.\r\n"
"  3. Find the \"Terrain Data\" field and upload terrain_map%d_server.att\r\n"
"     (the file in THIS folder).\r\n"
"  4. Save, then restart the game server.\r\n"
"\r\n"
"Then relog in the game. Done.\r\n"
"\r\n"
"\r\n"
"IMPORTANT: UPLOAD THE RIGHT FILE\r\n"
"--------------------------------\r\n"
"Upload terrain_map%d_server.att from this folder.\r\n"
"\r\n"
"Do NOT upload the client's Data\\World*\\EncTerrain*.att. The Admin Panel stores\r\n"
"whatever bytes you give it, as-is - it does not decrypt anything. The client's\r\n"
"file is encrypted and has a different header, so uploading it would corrupt the\r\n"
"map completely.\r\n"
"\r\n"
"\r\n"
"WHAT THIS FILE CONTAINS\r\n"
"-----------------------\r\n"
"It is the server's OWN terrain data (the one you downloaded), with ONLY the tiles\r\n"
"you edited changed. Every other tile is byte-for-byte identical to what the server\r\n"
"already had.\r\n"
"\r\n"
"That matters: the client and the server do not agree on the whole map. The client\r\n"
"blocks every tree/rock footprint; the server does not (over 1000 such tiles on some\r\n"
"maps). Uploading the client's map wholesale would block all of them server-side and\r\n"
"could break monster spawns and pathing. This file does not do that.\r\n"
"\r\n"
"\r\n"
"TROUBLESHOOTING\r\n"
"---------------\r\n"
"Can't find the map / edited the wrong one\r\n"
"    -> The SERVER keys maps by the world ENUM (Lorencia = 0, Arena = 6), NOT the\r\n"
"       client folder number (World1, World7). This file targets Number = %d.\r\n"
"\r\n"
"Nothing changed in game\r\n"
"    -> The game server caches the map. Restart it after saving.\r\n"
"\r\n"
"Players walk through walls, then get snapped back\r\n"
"    -> The client and server disagree. Save the CLIENT file too:\r\n"
"       Map Editor -> Attribute -> \"Save client .att\", and ship it.\r\n"
"       Both sides must match.\r\n",
            mapNumber, mapNumber, mapNumber, mapNumber, mapNumber, mapNumber);

        fclose(fp);
    }
}

// The attribute byte of a tile as it should be PERSISTED.
//
// TerrainWall[] is not purely map data at runtime: MoveCharactersClient() clears and
// re-stamps TW_CHARACTER (0x02) every frame on every tile a live character/NPC stands
// on. Saving it verbatim bakes "an NPC was standing here when I hit Save" into the
// map, e.g. a safezone tile becomes 1|2 = 3.
//
// That byte is poison for the server: OpenMU decides walkability with an EXACT value
// test (walkable = v==0 || v==1), so 3 reads as BLOCKED - while the client's bit test
// still reads it as walkable. Result: players walk onto the tile and get rubber-banded
// back. Strip the transient bit on save.
BYTE StaticAttribute(WORD wall)
{
    return static_cast<BYTE>((wall & 0xFF) & ~TW_CHARACTER);
}

bool SaveClientAtt(int world, int mapNumber)
{
    auto plain = std::make_unique<BYTE[]>(PLAIN_BYTES);
    plain[0] = ATT_VERSION;
    plain[1] = static_cast<BYTE>(mapNumber);
    plain[2] = ATT_EXTENT;
    plain[3] = ATT_EXTENT;

    // TerrainWall is a WORD array but the on-disk format for these maps is one
    // byte per cell (the loader masks to 0xFF), so write the low byte - minus the
    // runtime-only character bit (see StaticAttribute).
    for (int i = 0; i < CELLS; ++i)
        plain[HEADER_BYTES + i] = StaticAttribute(TerrainWall[i]);

    // On-disk = MapFileEncrypt(BuxConvert(plain)) - the inverse of how it loads.
    BuxConvert(plain.get(), PLAIN_BYTES);

    auto enc = std::make_unique<BYTE[]>(PLAIN_BYTES);
    const int encBytes = MapFileEncrypt(enc.get(), plain.get(), PLAIN_BYTES);

    wchar_t fileName[128];
    swprintf_s(fileName, L"Data\\World%d\\EncTerrain%d.att", world, world);

    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == nullptr)
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] SaveClientAtt FAILED: could not open the .att for write");
        return false;
    }
    fwrite(enc.get(), 1, encBytes, fp);
    fclose(fp);

    g_MuEditorConsoleUI.LogEditor("[MapEditor] Saved terrain attributes (encrypted) to EncTerrain.att");
    Editor::Files::MirrorNextToExe(fileName, world);
    return true;
}

bool PickServerBaseAtt(std::wstring& outPath)
{
    wchar_t file[MAX_PATH] = { 0 };
    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Server TerrainData (*.att)\0*.att\0All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"Select the server's current TerrainData (downloaded from the Admin Panel)";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn))
        return false;
    outPath = file;
    return true;
}

bool LoadServerBaseAtt(const std::wstring& path, std::vector<BYTE>& outBase, std::string& outError)
{
    outBase.clear();

    FILE* fp = _wfopen(path.c_str(), L"rb");
    if (fp == nullptr)
    {
        outError = "Could not open that file.";
        return false;
    }

    fseek(fp, 0, SEEK_END);
    const long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size != SERVER_HEADER_BYTES + CELLS)
    {
        fclose(fp);
        // The overwhelmingly likely mistake is grabbing the client's .att, which is
        // encrypted and 65540 bytes - say so instead of just "wrong size".
        char buf[192];
        snprintf(buf, sizeof(buf),
                 "Wrong size: %ld bytes, expected 65539. This must be the server's TerrainData "
                 "(downloaded from the Admin Panel) - NOT the client's EncTerrain*.att.",
                 size);
        outError = buf;
        return false;
    }

    outBase.resize(static_cast<size_t>(size));
    const size_t read = fread(outBase.data(), 1, outBase.size(), fp);
    fclose(fp);
    if (read != outBase.size())
    {
        outBase.clear();
        outError = "Could not read the whole file.";
        return false;
    }

    // The client's .att is encrypted, so its bytes look like noise; the server's is
    // plain, and every attribute byte is a small flag combination (< 128 - the client
    // loader rejects anything above that too). A single high byte means it is not
    // plain server data, which is worth catching before it corrupts the map.
    for (int i = 0; i < CELLS; ++i)
    {
        if (outBase[SERVER_HEADER_BYTES + i] >= 128)
        {
            outBase.clear();
            outError = "That file is not plain server TerrainData (it looks encrypted). "
                       "Download it from the Admin Panel's \"Terrain Data\" field.";
            return false;
        }
    }

    return true;
}

bool SaveServerAtt(int serverMapNumber,
                   const std::vector<BYTE>& serverBase,
                   const std::vector<BYTE>& baseline,
                   const std::vector<bool>& edited,
                   std::wstring& outPath,
                   int& outChanged)
{
    outChanged = 0;

    if (serverBase.size() != static_cast<size_t>(SERVER_HEADER_BYTES + CELLS)
        || baseline.size() != static_cast<size_t>(CELLS)
        || edited.size()   != static_cast<size_t>(CELLS))
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] SaveServerAtt FAILED: server base not loaded");
        return false;
    }

    // Start from the server's own bytes - header included, so whatever it had is kept.
    std::vector<BYTE> data = serverBase;

    // ...and touch ONLY the tiles the user actually changed. "Painted" is not enough:
    // a stroke that was undone leaves the tile back at its baseline value, and writing
    // the client's value there would push a client-only object block onto the server.
    for (int i = 0; i < CELLS; ++i)
    {
        if (!edited[i])
            continue;

        const BYTE now = StaticAttribute(TerrainWall[i]);
        if (now == baseline[i])
            continue;

        data[SERVER_HEADER_BYTES + i] = now;
        ++outChanged;
    }

    wchar_t fileName[128];
    swprintf_s(fileName, L"terrain_map%d_server.att", serverMapNumber);

    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == nullptr)
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] SaveServerAtt FAILED: could not open the .att for write");
        return false;
    }
    fwrite(data.data(), 1, data.size(), fp);
    fclose(fp);

    outPath = fileName;

    // Drop a plain-English HOWTO next to it so anyone can apply it without docs.
    WriteHowTo(serverMapNumber);

    char msg[128];
    snprintf(msg, sizeof(msg), "[MapEditor] Wrote server .att + HOWTO next to Main.exe (%d tile(s) changed)", outChanged);
    g_MuEditorConsoleUI.LogEditor(msg);
    return true;
}

} // namespace Editor::AttrSave

#endif // _EDITOR
