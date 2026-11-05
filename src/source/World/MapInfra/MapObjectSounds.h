#pragma once

#include "Audio/DSPlaySound.h"

#include <optional>

class OBJECT;

// Plays the sounds of map objects (Data/Maps/ObjectSounds.json, maps design
// MP1) for the map shown: each frame an object is drawn, the sound of its
// type, as the original client played the map code's sounds (D-MP3).
namespace World::MapObjectSounds
{
// Loads the sounds of the entries that name the map and stage and keeps
// which object type plays which (CMapManager::LoadWorld, after the map's own
// sounds). A change of the table (the Map Editor) loads them again.
void LoadForMap(int serverMap, int stage);

// The last frame drew the object (UpdateObjectVisual): its type's sound, if
// an entry of the map names the type, starts a copy in PlayDrawnSounds, and
// the sounds of the map's entries without objects (any object, Q-MP8) do so
// for the first object drawn in the frame.
void OnObjectDrawn(const OBJECT& object);

// After the objects the last frame drew (UpdateObjectEffects): each start of
// OnObjectDrawn, in their order, plays on a free channel, each sound at the
// level of its nearest object (DistanceLevel), so a far object of a type
// drawn when a channel ends does not quiet the copy of the one beside the
// hero.
void PlayDrawnSounds();

// The sound an object type plays on the map loaded; nullopt for none.
std::optional<ESound> FindSound(int objectType);

// The volume of the copies a drawn object starts, by its distance from the
// hero (maps design, D-MP7, a look-and-feel test): full within 3 tiles, then
// lower to a quarter at 15 tiles and beyond.
float DistanceLevel(float distance);
} // namespace World::MapObjectSounds
