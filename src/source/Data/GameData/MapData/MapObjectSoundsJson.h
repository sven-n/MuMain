#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/MapObjectSound.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// Data/Maps/ObjectSounds.json (maps design, D-MP2):
//
//   { "formatVersion": 1,
//     "objectSounds": [
//       { "maps": [24, 25, 26, 27, 28, 29, 36], "objects": [37, 38],
//         "sound": { "file": "Data/Sound/aKalimaWaterFall.wav", "channels": 3 },
//         "plays": "whileInView" } ] }
namespace Data::Maps
{
constexpr int MapObjectSoundsFormatVersion = 1;

// Reads the entries into `sounds`. An entry with a fatal problem is left out
// and its problem added to `issues`, with the field "objectSounds[i].key";
// unknown fields are warnings.
void ReadMapObjectSoundsJson(std::string_view text, const std::string& source, std::vector<MapObjectSound>& sounds,
                             std::vector<Items::ItemDataIssue>& issues);

// The file's text, the entries in their order, each on its own line.
std::string WriteMapObjectSoundsJson(std::span<const MapObjectSound> sounds);

// What the reader cannot see in one entry: an object type that two entries
// name on one map (fatal: which would play?).
void ValidateMapObjectSounds(std::span<const MapObjectSound> sounds, const std::string& source,
                             std::vector<Items::ItemDataIssue>& issues);

// A file that is not in the client folder (the working folder): a warning,
// the sound is silent.
void ValidateMapObjectSoundFiles(std::span<const MapObjectSound> sounds, const std::string& source,
                                 std::vector<Items::ItemDataIssue>& issues);

// "objectSounds[2].sound.file", as the problems name a field of an entry.
std::string MapObjectSoundField(size_t entry, std::string_view key);
} // namespace Data::Maps
