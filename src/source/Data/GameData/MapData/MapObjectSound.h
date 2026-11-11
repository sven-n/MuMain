#pragma once

#include "Data/GameData/MapData/MapRef.h"

#include <string>
#include <vector>

// The sounds of map objects (Data/Maps/ObjectSounds.json, maps design MP1):
// the object types of some maps that play a sound while they are drawn, see
// docs/map-data.md.
namespace Data::Maps
{
// The object types a map has (MAX_WORLD_OBJECTS): 0 to this.
constexpr int MaxMapObjectType = 159;
// A sound's channels (MAX_CHANNEL): how many copies of it play at once.
constexpr int MaxMapSoundChannels = 4;

// When an object sound plays.
enum class MapObjectSoundPlay
{
    // Asked for each frame an object of the type is drawn (it passes the
    // view test); a channel that has ended starts again, a playing one is
    // left alone (D-MP3); a copy plays quieter the farther its object is
    // from the hero (D-MP7, a look-and-feel test).
    WhileInView,
};

// One entry: the object types of the maps that play the sound.
struct MapObjectSound
{
    // The maps (server numbers, with a stage for one stage), sorted.
    std::vector<MapRef> maps;
    // The object types, sorted; none with anyObject.
    std::vector<int> objects;
    // Plays for any object of the maps (the file names no objects): a sound
    // a map played from every object it moved, its hum (Kanturu, Q-MP8).
    bool anyObject = false;
    // "Data/Sound/aKalimaWaterFall.wav"; its number is its file's
    // (Audio/SoundNumbers.h).
    std::string file;
    int channels = 1;
    // What the Sound Editor shows instead of the file (sound editor design,
    // Q-SE1); "" for none. The game reads and ignores it.
    std::string name;
    MapObjectSoundPlay plays = MapObjectSoundPlay::WhileInView;

    bool operator==(const MapObjectSound&) const = default;
};
} // namespace Data::Maps
