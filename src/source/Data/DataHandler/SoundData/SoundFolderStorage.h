#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/SoundData/SoundFolders.h"

#include <filesystem>
#include <span>
#include <vector>

// Reads and writes Data/Audio/SoundFolders.json, the Sound Editor's folders
// (sound editor design, Q-SE4). Only the editor reads it.
namespace Data::Sounds
{
// "Data/Audio/SoundFolders.json", from the game folder.
std::filesystem::path GetSoundFoldersFile();

struct SoundFoldersLoadResult
{
    std::vector<SoundFolder> folders;
    std::vector<Items::ItemDataIssue> issues;
};

// The folders of the file; none, and no problem, when there is no file.
SoundFoldersLoadResult LoadSoundFoldersFile(const std::filesystem::path& file);

// Writes the folders; false, with the problem in `issues`, when the file
// cannot be written.
bool SaveSoundFolders(const std::filesystem::path& file, std::span<const SoundFolder> folders,
                      std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Sounds
