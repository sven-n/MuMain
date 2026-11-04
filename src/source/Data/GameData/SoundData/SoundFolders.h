#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The Sound Editor's folders (Data/Audio/SoundFolders.json, sound editor
// design Q-SE4): a tree of named folders that holds the sounds by their
// keys, only to find where a sound belongs. The game never reads it.
//
//   { "formatVersion": 1,
//     "folders": [
//       { "name": "Monsters", "folders": [
//           { "name": "Death Tree",
//             "sounds": ["set:Death Tree:Data/Sound/w34/dt_idle1.wav"] } ] },
//       { "name": "Interface", "sounds": ["SOUND_CLICK01"] } ] }
//
// A sound is named by its key (MuEditor::SoundEditor::SoundRow::key): the
// SOUND_ id of a code sound, "set:<set>:<id, else file>" of a set's sound,
// "objects:<file>" of a map object sound; "#2", "#3" after it for the second
// and third sound with the same one.
namespace Data::Sounds
{
constexpr int SoundFoldersFormatVersion = 1;

struct SoundFolder
{
    std::string name;
    std::vector<SoundFolder> folders;
    std::vector<std::string> sounds;

    bool operator==(const SoundFolder&) const = default;
};

// Reads the folders into `folders`. A folder without a name, a name that a
// sibling has too, or a value of the wrong form is fatal ("folders[1].
// folders[0].name"); a sound named twice is a warning and stays only in the
// first folder; unknown fields are warnings.
void ReadSoundFoldersJson(std::string_view text, const std::string& source, std::vector<SoundFolder>& folders,
                          std::vector<Items::ItemDataIssue>& issues);

// The file's text.
std::string WriteSoundFoldersJson(std::span<const SoundFolder> folders);

// The folders' names from the top to the folder that holds the sound;
// nullopt for none.
std::optional<std::vector<std::string>> FindSoundFolder(std::span<const SoundFolder> folders, std::string_view key);
} // namespace Data::Sounds
