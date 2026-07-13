#include "stdafx.h"

#ifdef _EDITOR

#include "MapAttributeSave.h"
#include "MapEditorFileUtil.h"

#include "Render/Terrain/ZzzLodTerrain.h"   // TerrainWall / MapFileEncrypt / TERRAIN_SIZE
#include "Core/Globals/_crypt.h"            // BuxConvert
#include "UI/Console/MuEditorConsoleUI.h"

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

    // The attribute byte of a tile as it should be PERSISTED.
    //
    // TerrainWall[] is not purely map data at runtime: MoveCharactersClient()
    // clears and re-stamps TW_CHARACTER (0x02) every frame on every tile a live
    // character/NPC stands on. Saving it verbatim bakes "an NPC was standing here
    // when I hit Save" into the map, e.g. a safezone tile becomes 1|2 = 3.
    //
    // That byte is poison for the server: OpenMU decides walkability with an EXACT
    // value test (walkable = v==0 || v==1), so 3 reads as BLOCKED - while the
    // client's bit test still reads it as walkable. Result: players walk onto the
    // tile and get rubber-banded back. Strip the transient bit on save.
    BYTE StaticAttribute(WORD wall)
    {
        return static_cast<BYTE>((wall & 0xFF) & ~TW_CHARACTER);
    }

    // Writes a dead-simple instructions file next to the .sql.
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
"File: terrain_map%d_server.sql   (server map Number = %d)\r\n"
"\r\n"
"Two commands. That's it.\r\n"
"\r\n"
"  docker exec -i -e PGPASSWORD={DB_PASSWORD} {DB_CONTAINER} psql -U {DB_USER} -d {DB_NAME} < {SQL_FILE}\r\n"
"  docker restart {GAMESERVER_CONTAINER}\r\n"
"\r\n"
"By default (OpenMU 'all-in-one' docker compose):\r\n"
"\r\n"
"  docker exec -i -e PGPASSWORD=admin database psql -U postgres -d openmu < terrain_map%d_server.sql\r\n"
"  docker restart openmu-startup\r\n"
"\r\n"
"It should print \"UPDATE 1\". Then relog in the game. Done.\r\n"
"\r\n"
"\r\n"
"IF YOUR SERVER IS REMOTE (cloud / VPS)\r\n"
"--------------------------------------\r\n"
"Copy the file up first, then run the same two commands over SSH:\r\n"
"\r\n"
"  scp terrain_map%d_server.sql youruser@yourserver:/tmp/\r\n"
"  ssh youruser@yourserver\r\n"
"  docker exec -i -e PGPASSWORD=admin database psql -U postgres -d openmu < /tmp/terrain_map%d_server.sql\r\n"
"  docker restart openmu-startup\r\n"
"\r\n"
"\r\n"
"IF YOUR SERVER IS LOCAL (Docker on this PC)\r\n"
"-------------------------------------------\r\n"
"No copying needed - just run the two commands here, in this folder.\r\n"
"\r\n"
"WATCH OUT: PowerShell does NOT support the \"<\" redirect. Use one of:\r\n"
"\r\n"
"  cmd.exe    :  docker exec -i -e PGPASSWORD=admin database psql -U postgres -d openmu < terrain_map%d_server.sql\r\n"
"  PowerShell :  Get-Content terrain_map%d_server.sql | docker exec -i -e PGPASSWORD=admin database psql -U postgres -d openmu\r\n"
"\r\n"
"\r\n"
"DON'T KNOW YOUR CONTAINER NAMES?\r\n"
"--------------------------------\r\n"
"  docker ps\r\n"
"\r\n"
"Look for the postgres one (that's {DB_CONTAINER}) and the OpenMU one\r\n"
"(that's {GAMESERVER_CONTAINER}).\r\n"
"\r\n"
"\r\n"
"TROUBLESHOOTING\r\n"
"---------------\r\n"
"\"UPDATE 0\" instead of \"UPDATE 1\"\r\n"
"    -> Wrong map number. The SERVER keys maps by the world ENUM\r\n"
"       (Lorencia = 0, Arena = 6), NOT the client folder number\r\n"
"       (World1, World7). This file targets Number = %d.\r\n"
"\r\n"
"Players walk through walls, then get snapped back\r\n"
"    -> The client and server disagree. Save the CLIENT file too:\r\n"
"       Map Editor -> Attribute -> \"Save client .att\", and ship it.\r\n"
"       Both sides must match.\r\n",
            mapNumber, mapNumber, mapNumber, mapNumber, mapNumber,
            mapNumber, mapNumber, mapNumber);

        fclose(fp);
    }
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

bool SaveServerSql(int serverMapNumber, std::wstring& outPath)
{
    // Hex-encode the raw attribute block (unencrypted - the DB stores it plain).
    static const char* kHex = "0123456789abcdef";
    std::string hex;
    hex.reserve(static_cast<size_t>(CELLS) * 2);
    for (int i = 0; i < CELLS; ++i)
    {
        const BYTE v = StaticAttribute(TerrainWall[i]);   // never persist TW_CHARACTER
        hex.push_back(kHex[v >> 4]);
        hex.push_back(kHex[v & 0x0F]);
    }

    wchar_t fileName[128];
    swprintf_s(fileName, L"terrain_map%d_server.sql", serverMapNumber);

    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == nullptr)
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] SaveServerSql FAILED: could not open the .sql for write");
        return false;
    }

    // Preserve whatever the existing 3-byte TerrainData header is by slicing it off
    // the current row, then append our full attribute block. One statement, no
    // dependence on knowing the header bytes.
    fprintf(fp,
        "-- Terrain attributes (walk map) for map Number=%d\n"
        "-- Generated by the in-game Map Editor. Run against the OpenMU database,\n"
        "-- then restart the game server so it reloads the map.\n"
        "UPDATE config.\"GameMapDefinition\"\n"
        "SET \"TerrainData\" = substring(\"TerrainData\" from 1 for %d) || decode('%s', 'hex')\n"
        "WHERE \"Number\" = %d;\n",
        serverMapNumber, SERVER_HEADER_BYTES, hex.c_str(), serverMapNumber);
    fclose(fp);

    outPath = fileName;

    // Drop a plain-English HOWTO next to the .sql so anyone can apply it without
    // digging through docs.
    WriteHowTo(serverMapNumber);

    g_MuEditorConsoleUI.LogEditor("[MapEditor] Wrote server SQL + HOWTO next to Main.exe");
    return true;
}

} // namespace Editor::AttrSave

#endif // _EDITOR
