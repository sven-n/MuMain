#pragma once

// Opens monster and NPC models from their entries in the model table
// (g_MonsterModelDatabase, Data/Monsters/Models.json): the file and its
// textures, the part models, the animation speeds and held last frames, the
// head bone, stream mesh and none-blend meshes, the sounds and sound slots,
// and the bitmaps of fixed slots. OpenMonsterModel and OpenNpc call it.
namespace Data::Monsters::ModelLoader
{
// Opens the model of the slot from its entry; false when the slot has no
// entry. The slot must not be open yet (the callers check it).
bool Open(int slot);

// Whether some entry has part models that a map opens (the Totem Golem's
// pieces in Elbeland, the Kundun parts in Kalima); CMapManager::Load opens
// them from the table then.
bool HasMapParts();

// Opens the part models that the entries list for the world, as a map
// load did. They are never closed, as before.
void OpenMapParts(int world);
} // namespace Data::Monsters::ModelLoader
