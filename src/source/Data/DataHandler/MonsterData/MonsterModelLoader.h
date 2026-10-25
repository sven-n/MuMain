#pragma once

#include <vector>

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

#ifdef _EDITOR
// Opens the slot's model again from its entry when the game has it open
// (the monster editor changed the entry, D55), with its parts, also those of
// the loaded map that are open; a slot without an entry any more is
// released. Returns the slots opened again or released.
std::vector<int> Reopen(int slot);
#endif

// Opens the part models that the entries list for the world (the Totem
// Golem's pieces in Elbeland, the Kundun parts in Kalima); CMapManager::Load
// calls it. They are never closed, as before.
void OpenMapParts(int world);
} // namespace Data::Monsters::ModelLoader
