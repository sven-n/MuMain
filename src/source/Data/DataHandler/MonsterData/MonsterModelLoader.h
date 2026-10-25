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
} // namespace Data::Monsters::ModelLoader
