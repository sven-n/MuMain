#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/SoundData/SoundEvents.h"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

// Reads and writes Data/Audio/SoundEvents.json (sound editor design, Q-SE1)
// and keeps the table (g_SoundEventDatabase) in step with it.
namespace Data::Sounds
{
// "Data/Audio/SoundEvents.json", from the game folder.
std::filesystem::path GetSoundEventsFile();

struct SoundEventsLoadResult
{
    std::vector<SoundEvent> events;
    std::vector<Items::ItemDataIssue> issues;
};

SoundEventsLoadResult LoadSoundEventsFile(const std::filesystem::path& file);

// The file's text, after reading it back as the game would: nothing, with
// the problems in `issues`, when that finds a fatal one.
std::optional<std::string> MakeSoundEventsFileText(const std::filesystem::path& file,
                                                   std::span<const SoundEvent> events,
                                                   std::vector<Items::ItemDataIssue>& issues);

// At start: the file into the table, its problems logged; false, with the
// table left as it was and `errorMessage` describing them, on fatal ones.
bool LoadSoundEvents(std::string& errorMessage);

// The editor's Save and Revert.
bool SaveSoundEvents(std::vector<Items::ItemDataIssue>& issues);
bool RevertSoundEvents(std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Sounds
