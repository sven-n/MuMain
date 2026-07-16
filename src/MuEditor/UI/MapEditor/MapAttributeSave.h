#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

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
//  * SERVER (OpenMU): the walk map lives in config."GameMapDefinition"."TerrainData"
//      (bytea), where attribute byte i sits at offset 3 + i. It is uploaded through
//      the Admin Panel's "Terrain Data" field. NOTE the server keys maps by
//      "Number" = the world ENUM value (Arena = 6), while the client's folder is
//      World{enum + 1} (World7) - don't mix them up.
namespace Editor::AttrSave
{
    // Writes the encrypted client .att for `world` (folder number). `mapNumber` is
    // the map id byte in the header (conventionally == world).
    bool SaveClientAtt(int world, int mapNumber);

    // --- Server export -------------------------------------------------------
    //
    // The server file is a MERGE, not a dump of the client map, and that matters:
    // the client and server walk maps legitimately DISAGREE. The client blocks every
    // object footprint (trees, rocks); the server does not. On Tarkan that is 1084
    // tiles, on Devias 511, on Lorencia 234. Writing the client's map wholesale would
    // silently block all of those server-side - breaking monster spawns and pathing -
    // even if the user only meant to edit 12 tiles.
    //
    // So: take the server's CURRENT TerrainData as the base and overwrite only the
    // tiles the user actually edited this session. Everything else stays byte-for-byte
    // identical.

    // Opens a file dialog for the server's current TerrainData, downloaded from the
    // Admin Panel's "Terrain Data" field (its "Download" link).
    bool PickServerBaseAtt(std::wstring& outPath);

    // Reads + validates that base: must be exactly 65539 bytes (3-byte header + 65536
    // attribute bytes) and plain (never the client's encrypted .att). On failure,
    // outError explains why in plain English.
    bool LoadServerBaseAtt(const std::wstring& path, std::vector<BYTE>& outBase, std::string& outError);

    // Writes the merged server file (+ a HOWTO) next to Main.exe, ready to upload.
    //   serverBase  - the 65539 bytes from LoadServerBaseAtt.
    //   baseline    - the 65536 client attribute bytes as of the start of the session.
    //   edited      - which tiles the user painted (65536 flags).
    // A tile is written only if it was painted AND actually differs from the baseline,
    // so an undone stroke correctly writes nothing. outChanged = tiles written.
    // `serverMapNumber` is the world ENUM value.
    bool SaveServerAtt(int serverMapNumber,
                       const std::vector<BYTE>& serverBase,
                       const std::vector<BYTE>& baseline,
                       const std::vector<bool>& edited,
                       std::wstring& outPath,
                       int& outChanged);

    // The attribute byte of a tile as it should be persisted (strips the runtime-only
    // TW_CHARACTER bit). Exposed so the UI can snapshot a baseline the same way.
    BYTE StaticAttribute(WORD wall);
}

#endif // _EDITOR
