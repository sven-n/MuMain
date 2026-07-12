#pragma once

#ifdef _EDITOR

// In-game map editor: save helpers that write the live terrain globals back to
// the game's own on-disk format.
//
// The plain SaveTerrain*() functions in ZzzLodTerrain.cpp write DECRYPTED bytes,
// but the client loads EncTerrain{N}.* through MapFileDecrypt (and BuxConvert for
// .att). Saving with the plain functions would therefore produce files the game
// can't reload. These helpers build the exact byte layout the loader expects and
// encrypt it the same way, so an edit made in the editor survives a map reload.
namespace Editor::MapSave
{
    // Encrypts the live TerrainMappingLayer1/2/Alpha arrays and overwrites
    // Data\World{worldNumber}\EncTerrain{worldNumber}.map. Returns false if the
    // file could not be written. `mapNumber` is the map id byte stored in the
    // header (conventionally == worldNumber).
    bool SaveMappingEncrypted(int worldNumber, int mapNumber);
}

#endif // _EDITOR
