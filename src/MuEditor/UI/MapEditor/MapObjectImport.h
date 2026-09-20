#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

// Cross-map object import for the Map Editor ("O. Browse").
//
// To place an object from another map so it survives a reload, its model + the
// textures the model references must live in the current map's Object folder and
// be loaded into one of the world-object model slots. This is the object analog
// of the ExtTile texture import: pick a free slot, copy Object<file>.bmd (and its
// textures) into Data\Object{current}\, and load it live so it's placeable now.
//
// Note: a few special maps (e.g. Lorencia / World1) don't load objects via the
// generic Object{N}.bmd scheme, so imported objects there are live-only and won't
// persist across a reload. Most maps use the generic scheme and persist fine.
namespace Editor::ObjectImport
{
    // Lists the *.bmd model files present in Data\Object{sourceWorld}\.
    std::vector<std::wstring> ListModelFiles(int sourceWorld);

    // Loads `bmdFile` from Data\Object{sourceWorld}\ (model + textures) into a
    // reserved scratch model slot for previewing, and returns that slot index.
    // Overwrites the previous scratch model each call. Returns -1 on failure.
    int LoadForPreview(int sourceWorld, const std::wstring& bmdFile);

    // Imports `bmdFile` from Data\Object{sourceWorld}\ onto the current map:
    // loads it into a free model slot (live) and copies the model + textures into
    // Data\Object{currentWorld}\ for persistence. Returns the object type (model
    // slot) to place, or -1 on failure.
    int UseModelOnCurrentMap(int currentWorld, int sourceWorld, const std::wstring& bmdFile);
}

#endif // _EDITOR
