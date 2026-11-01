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

// The object is drawn this frame (RenderObjectVisual): plays its type's
// sound on a free channel, if an entry of the map names the type.
void OnObjectDrawn(const OBJECT& object);

// The sound an object type plays on the map loaded; nullopt for none.
std::optional<ESound> FindSound(int objectType);

// The volume of a copy a drawn object starts, by its distance from the hero
// (maps design, D-MP7, a look-and-feel test): full within 3 tiles, then
// lower to a quarter at 15 tiles and beyond.
float DistanceLevel(float distance);
} // namespace World::MapObjectSounds
