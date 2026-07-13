#pragma once

#ifdef _EDITOR

#include <string>

// Saving terrain attributes (walkability) edited in the Map Editor.
//
// Two outputs are needed, because the client and the server each keep their own
// copy of the walk map:
//
//  * CLIENT: Data\World{world}\EncTerrain{world}.att
//      plaintext = [version, mapId, 255, 255] + attr[256*256]  (1 byte per cell,
//      index = y*256 + x, same order as TerrainWall[]).
//      On disk it is BuxConvert'd and then MapFileEncrypt'd (the engine's own
//      SaveTerrainAttribute writes PLAINTEXT and 2 bytes/cell, so we can't use it).
//
//  * SERVER (OpenMU): a .sql file the user runs against the game database. The
//      walk map lives in config."GameMapDefinition"."TerrainData" (bytea), where
//      attribute byte i sits at offset 3 + i. NOTE the server keys maps by
//      "Number" = the world ENUM value (Arena = 6), while the client's folder is
//      World{enum + 1} (World7) - don't mix them up.
namespace Editor::AttrSave
{
    // Writes the encrypted client .att for `world` (folder number). `mapNumber` is
    // the map id byte in the header (conventionally == world).
    bool SaveClientAtt(int world, int mapNumber);

    // Writes an UPDATE statement for the OpenMU database that replaces the whole
    // attribute block while preserving the existing 3-byte TerrainData header.
    // `serverMapNumber` is the world ENUM value. Returns the file path written.
    bool SaveServerSql(int serverMapNumber, std::wstring& outPath);
}

#endif // _EDITOR
