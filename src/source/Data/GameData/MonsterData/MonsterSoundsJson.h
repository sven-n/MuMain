#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The sound sets of the monster area (Data/Monsters/Sounds.json, monsters
// design, Q-MN78):
//
//   { "formatVersion": 1,
//     "soundSets": [
//       {
//         "name": "Bull Fighter",
//         "sounds": [
//           {"file": "Data/Sound/mBull1.wav", "channels": 2, "spatial": true}, ...
//         ],
//         "soundEvents": [
//           {"on": ["stand", "walk"], ..., "sounds": ["Data/Sound/mBull1.wav", ...]}, ...
//         ]
//       } ] }
//
// docs/monster-data.md "Sounds" describes the fields. The reader checks the
// form of the values; whether the sound ids exist and the events name sounds
// of their set is checked when the sets are loaded (MonsterSoundValidation.h).
namespace Data::Monsters
{
constexpr int MonsterSoundsFormatVersion = 1;

// Reads the sets of the text into `sets`, in their order. A set with an error
// is left out; a name that is there twice is an error.
void ReadMonsterSoundsJson(std::string_view text, const std::string& source, std::vector<MonsterSoundSet>& sets,
                           std::vector<Items::ItemDataIssue>& issues);

// The text of the file with these sets, sorted by name.
std::string WriteMonsterSoundsJson(std::span<const MonsterSoundSet> sets);

// How a problem of a set names its field: "sound set Bull Fighter sounds[0]".
std::string MonsterSoundSetField(std::string_view name, std::string_view key);
} // namespace Data::Monsters
