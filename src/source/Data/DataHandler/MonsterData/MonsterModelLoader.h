#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Data::Monsters
{
struct MonsterModelDefinition;
struct MonsterModelSound;
} // namespace Data::Monsters

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

// Whether the slot holds an opened model (meshes or animations).
bool IsOpen(int slot);

// The number the game plays a sound by that a sound slot or a sound event of
// the entry names: a sound of the model (by its id, or by its file when it
// has none), else a SOUND_* id; nullopt when it names neither.
std::optional<int> FindSoundNumber(const MonsterModelDefinition& model, const std::string& value);

// The number the sound of an entry is loaded and played with: its SOUND_* id,
// or the number of its file.
int SoundNumber(const MonsterModelSound& sound);

// Loads the sound of an entry as its model loads it, unless another model or
// the map loaded it from the same file already: a load starts it over and
// stops it where it plays.
void LoadSound(const MonsterModelSound& sound);

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
