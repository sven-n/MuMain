#pragma once

#ifdef _EDITOR

#include <string>

// Generates a top-down minimap image directly from the map's terrain tiles.
//
// This does NOT screenshot the screen - it averages each tile texture's colour and
// composes an image where pixel (x, y) maps to terrain tile (x, y), the exact same
// mapping the game uses to place the position marker (NewUIMiniMap.cpp:141). That
// makes the in-game marker land on the player by construction, with no HUD, no
// perspective and no dependence on the camera.
//
// Writes two files for world folder N (and mirrors them next to Main.exe):
//   Data\World{N}\mini_map.tga  - a plain 32-bit TGA to edit / touch up
//   Data\World{N}\mini_map.OZT  - the drop-in the game loads
// Both are 1024x1024, the size every stock world minimap uses.
namespace Editor::Minimap
{
    bool GenerateFromTiles(int world, std::string& outMsg);

    // Opens a file dialog for a .tga (e.g. the generated one with a screenshot painted
    // over it) and wraps it into Data\World{N}\mini_map.OZT in the exact format the
    // game loads - normalising orientation, bit depth and (un)compression along the
    // way. Mirrors the result next to Main.exe too.
    bool WrapTgaToOzt(int world, std::string& outMsg);
}

#endif // _EDITOR
