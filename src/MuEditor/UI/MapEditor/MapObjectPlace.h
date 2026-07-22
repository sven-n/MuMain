#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

class OBJECT;

// Map Editor object placement helpers.
//
// The client already loads every world object model (Data\Object{N}\Object{i}.bmd
// -> model type i-1) and can place them with CreateObject() and persist them with
// the engine's SaveObjects() (which encrypts). These helpers wrap that: they list
// which object types a world actually ships, place one on the ground, and save.
namespace Editor::ObjectPlace
{
    struct ModelEntry
    {
        int          type;  // CreateObject type (MODEL_WORLD_OBJECT + type)
        std::wstring file;  // e.g. "Object11.bmd"
    };

    // Lists the object models present in Data\Object{world}\ (files Object*.bmd),
    // sorted by type. These are the types safe to place (their .bmd is loaded).
    std::vector<ModelEntry> EnumerateModels(int world);

    // Places an object of `type` at world position (x,y), with the given yaw
    // (degrees, Z rotation) and scale. When `snap` is set, the position is
    // snapped to the tile centre. Returns false if creation failed.
    bool Place(int type, float x, float y, float yawDeg, float scale, bool snap);

    // Saves all live objects back to Data\World{world}\EncTerrain{world}.obj
    // (encrypted by the engine SaveObjects). Returns false on write failure.
    bool Save(int world);

    // Returns the visible object under the mouse cursor (ray pick), or nullptr.
    OBJECT* PickUnderCursor();

    // Moves `o` to (x,y,z). If the move crosses a 16x16 object-grid block the
    // object is re-created in the correct block (so live culling stays correct);
    // returns the possibly-new object pointer. The saved .obj re-blocks on load
    // regardless, so persistence is always correct.
    OBJECT* Reposition(OBJECT* o, float x, float y, float z);

    // Removes `o` from the world.
    void Remove(OBJECT* o);

    // A minimal serialisable copy of an object, for one-level undo of edits.
    struct SavedObject
    {
        int   type;
        float pos[3];
        float angle[3];
        float scale;
    };

    // Snapshots all live objects (type/pos/angle/scale). RestoreAll clears the
    // current objects and re-creates them from a snapshot; any live OBJECT* the
    // caller holds is invalid afterwards.
    std::vector<SavedObject> SnapshotAll();
    void RestoreAll(const std::vector<SavedObject>& snapshot);
}

#endif // _EDITOR
