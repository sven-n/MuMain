#pragma once

#ifdef _EDITOR

#include <string>

// Shared file helpers for the Map Editor's save paths.
namespace Editor::Files
{
    // Drops a copy of a just-saved map file into a World{N}\ folder next to
    // Main.exe, keeping the original file name - so the whole folder can be copied
    // straight into src\bin\Data\ to make the edit permanent:
    //   Data\World7\EncTerrain7.map   ->  World7\EncTerrain7.map
    //   Data\World7\TerrainHeight.OZB ->  World7\TerrainHeight.OZB
    // (The per-world folder also stops different maps' TerrainHeight.OZB from
    // clobbering each other.)
    // `savedPath` is the path the file was written to (relative to the working dir).
    // Returns the copy's path, or an empty string if the copy failed.
    std::wstring MirrorNextToExe(const std::wstring& savedPath, int world);
}

#endif // _EDITOR
