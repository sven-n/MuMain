#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/MapObjectSound.h"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

// Reads and writes Data/Maps/ObjectSounds.json (maps design, MP1) and keeps
// the table (g_MapObjectSoundDatabase) in step with it.
namespace Data::Maps
{
// "Data/Maps/ObjectSounds.json", from the game folder.
std::filesystem::path GetMapObjectSoundsFile();

struct MapObjectSoundsLoadResult
{
    std::vector<MapObjectSound> sounds;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file as the game does: the entries and every problem of the file
// and its checks (ValidateMapObjectSounds, ValidateMapObjectSoundFiles).
MapObjectSoundsLoadResult LoadMapObjectSoundsFile(const std::filesystem::path& file);

// The file's text for the entries, after reading it back as the game would:
// nothing, with the problems in `issues`, when that finds a fatal one.
std::optional<std::string> MakeMapObjectSoundsFileText(const std::filesystem::path& file,
                                                       std::span<const MapObjectSound> sounds,
                                                       std::vector<Items::ItemDataIssue>& issues);

// At start: the file into the table, its problems logged. False, with the
// table left as it was and `errorMessage` describing them, on fatal ones.
bool LoadMapObjectSounds(std::string& errorMessage);

// The editor's Save: the table into the file after its checks; false, with
// nothing written, when they find a fatal problem or the file cannot be
// written (also in `issues`).
bool SaveMapObjectSounds(std::vector<Items::ItemDataIssue>& issues);

// The editor's Revert: the file into the table again; false, with the table
// left as it was, on fatal problems.
bool RevertMapObjectSounds(std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Maps
