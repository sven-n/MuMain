#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/SoundData/SoundFile.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Data/Audio/SoundFiles.json (sound editor design, Q-SE1):
//
//   { "formatVersion": 1,
//     "sounds": [
//       { "id": "SOUND_CLICK01", "name": "Button click",
//         "file": "Data/Sound/iButtonClick.wav", "channels": 1,
//         "spatial": true, "type": "interface" } ] }
//
// "name", "spatial" (false) and "type" are optional.
namespace Data::Sounds
{
constexpr int SoundFilesFormatVersion = 1;

// Reads the entries into `sounds`. An entry with a fatal problem is left out
// and its problem added to `issues`, with the field "sounds[i].key"; unknown
// fields are warnings.
void ReadSoundFilesJson(std::string_view text, const std::string& source, std::vector<SoundFile>& sounds,
                        std::vector<Items::ItemDataIssue>& issues);

// The file's text, the entries in their order, each on its own line.
std::string WriteSoundFilesJson(std::span<const SoundFile> sounds);

// What the reader cannot see in one entry: an id that two entries name
// (fatal: which file would it load?).
void ValidateSoundFiles(std::span<const SoundFile> sounds, const std::string& source,
                        std::vector<Items::ItemDataIssue>& issues);

// A file that is not in the client folder (the working folder): a warning,
// the sound is silent.
void ValidateSoundFilesOnDisk(std::span<const SoundFile> sounds, const std::string& source,
                              std::vector<Items::ItemDataIssue>& issues);

// "sounds[2].file", as the problems name a field of an entry.
std::string SoundFileField(size_t entry, std::string_view key);

// The type's word in the file ("interface"), and back; nullopt for a word
// that is none.
std::string_view SoundTypeWord(::Audio::SoundKind type);
std::optional<::Audio::SoundKind> FindSoundType(std::string_view word);
} // namespace Data::Sounds
