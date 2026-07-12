#include "stdafx.h"

#ifdef _EDITOR

#include "MapObjectPlace.h"

#include "Engine/Object/ZzzObject.h"        // CreateObject / SaveObjects / ObjectBlock
#include "Engine/Object/w_ObjectInfo.h"     // class OBJECT (fields)
#include "Render/Terrain/ZzzLodTerrain.h"   // RequestTerrainHeight / TERRAIN_SCALE
#include "Render/Models/ZzzBMD.h"           // BMD / Models[]
#include "Core/Globals/_enum.h"             // MODEL_WORLD_OBJECT / MAX_WORLD_OBJECTS
#include "UI/Console/MuEditorConsoleUI.h"

#include <cstring>

namespace Editor::ObjectPlace
{

std::vector<ModelEntry> EnumerateModels(int /*world*/)
{
    // Enumerate the world-object model slots the current map actually loaded,
    // rather than scanning filenames. This is map-agnostic: it covers both the
    // generic Object{N}.bmd maps and special maps like Lorencia whose objects use
    // named files (Beer01.bmd, Cannon01.bmd, ...) via a hand-coded model mapping.
    // A slot is loaded when its BMD has meshes.
    std::vector<ModelEntry> out;
    for (int type = 0; type < MAX_WORLD_OBJECTS; ++type)
    {
        const BMD& model = Models[MODEL_WORLD_OBJECT + type];
        if (model.NumMeshs <= 0 || model.Meshs == nullptr)
            continue;

        // BMD.Name is the model's internal name (ASCII). Fall back to the type
        // number if it's blank. Copy into a guaranteed null-terminated buffer.
        char narrow[sizeof(model.Name) + 1] = { 0 };
        std::memcpy(narrow, model.Name, sizeof(model.Name));
        wchar_t name[80];
        if (narrow[0] != '\0')
            swprintf_s(name, L"%hs", narrow);
        else
            swprintf_s(name, L"(type %d)", type);
        out.push_back({ type, name });
    }
    return out;
}

bool Place(int type, float x, float y, float yawDeg, float scale, bool snap)
{
    float px = x;
    float py = y;
    if (snap)
    {
        px = ((int)(x / TERRAIN_SCALE) + 0.5f) * TERRAIN_SCALE;
        py = ((int)(y / TERRAIN_SCALE) + 0.5f) * TERRAIN_SCALE;
    }

    vec3_t position = { px, py, RequestTerrainHeight(px, py) };
    vec3_t angle    = { 0.0f, 0.0f, yawDeg };
    OBJECT* o = CreateObject(type, position, angle, scale);
    return o != nullptr;
}

bool Save(int world)
{
    wchar_t fileName[128];
    swprintf_s(fileName, L"Data\\World%d\\EncTerrain%d.obj", world, world);
    const bool ok = SaveObjects(fileName, world);
    g_MuEditorConsoleUI.LogEditor(ok ? "[MapEditor] Saved objects (encrypted) to EncTerrain.obj"
                                     : "[MapEditor] SaveObjects FAILED");
    return ok;
}

OBJECT* PickUnderCursor()
{
    return CollisionDetectObjects(nullptr);
}

namespace
{
    // World units spanned by one 16x16 object-grid block.
    constexpr float BLOCK_SPAN = 16.0f * TERRAIN_SCALE;

    int BlockOf(float x, float y)
    {
        const int i = (int)(x / BLOCK_SPAN);
        const int j = (int)(y / BLOCK_SPAN);
        if (i < 0 || j < 0 || i >= 16 || j >= 16)
            return -1;
        return i * 16 + j;
    }
}

OBJECT* Reposition(OBJECT* o, float x, float y, float z)
{
    if (o == nullptr)
        return nullptr;

    const int newBlock = BlockOf(x, y);
    if (newBlock < 0)
        return o;  // outside the placeable grid; ignore the move

    if ((BYTE)newBlock == o->Block)
    {
        o->Position[0] = x; o->Position[1] = y; o->Position[2] = z;
        o->StartPosition[0] = x; o->StartPosition[1] = y; o->StartPosition[2] = z;
        return o;
    }

    // Crossed a block boundary: re-create in the correct block, carrying the
    // object's type/angle/scale, then delete the old instance.
    vec3_t pos = { x, y, z };
    vec3_t ang; VectorCopy(o->Angle, ang);
    const int   type  = o->Type;
    const float scale = o->Scale;
    OBJECT* created = CreateObject(type, pos, ang, scale);
    DeleteObject(o, &ObjectBlock[o->Block]);
    return created;
}

void Remove(OBJECT* o)
{
    if (o != nullptr)
        DeleteObject(o, &ObjectBlock[o->Block]);
}

std::vector<SavedObject> SnapshotAll()
{
    std::vector<SavedObject> out;
    for (int b = 0; b < 256; ++b)
    {
        for (OBJECT* o = ObjectBlock[b].Head; o != nullptr; o = o->Next)
        {
            if (!o->Live)
                continue;
            SavedObject s;
            s.type = o->Type;
            s.scale = o->Scale;
            for (int k = 0; k < 3; ++k) { s.pos[k] = o->Position[k]; s.angle[k] = o->Angle[k]; }
            out.push_back(s);
        }
    }
    return out;
}

void RestoreAll(const std::vector<SavedObject>& snapshot)
{
    // Clear every current object, then re-create from the snapshot.
    for (int b = 0; b < 256; ++b)
    {
        OBJECT_BLOCK* ob = &ObjectBlock[b];
        while (ob->Head != nullptr)
            DeleteObject(ob->Head, ob);
    }
    for (const SavedObject& s : snapshot)
    {
        vec3_t pos = { s.pos[0], s.pos[1], s.pos[2] };
        vec3_t ang = { s.angle[0], s.angle[1], s.angle[2] };
        CreateObject(s.type, pos, ang, s.scale);
    }
}

} // namespace Editor::ObjectPlace

#endif // _EDITOR
