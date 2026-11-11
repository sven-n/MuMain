#pragma once

#ifdef _EDITOR

#include <string>

// Map Editor texture import: makes a texture usable on the current map by placing
// it into one of that map's free ExtTile slots and loading it live.
//
// The terrain can only paint the 30 tile slots loaded for the active world, so a
// texture from another map (or the user's disk) has to become one of those slots.
// The 16 ExtTile slots are the spare ones; we drop the source in as the next free
// ExtTile file (OZJ) and LoadBitmap it into the matching tile slot so it shows in
// the palette immediately and survives a map reload.
namespace Editor::TextureImport
{
    // Places `sourcePath` (a .jpg/.jpeg or .ozj file) into the next free ExtTile
    // slot of `world`, loads it into that terrain slot, and returns the tile-slot
    // index (14..29) on success, or -1 on failure (no free slot / bad file).
    int UseTextureFile(int world, const std::wstring& sourcePath);

    // Opens a Windows file-picker for an image to import. Returns false if the
    // user cancelled. Accepts JPEG (wrapped as OZJ) and OZJ files.
    bool PickImageFile(std::wstring& outPath);
}

#endif // _EDITOR
