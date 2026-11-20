#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/Warp.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// Reads and writes Data/Maps/Warps.json and keeps the list (g_WarpDatabase) in
// step with it.
namespace Data::Maps
{
// "Data/Maps/Warps.json", from the game folder.
std::filesystem::path GetWarpsFile();

struct WarpsLoadResult
{
    WarpList list;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file as the game does: the warps and every problem of the file and
// its checks (ValidateWarps).
WarpsLoadResult LoadWarpsFile(const std::filesystem::path& file);

// The file's text for the list, after reading it back as the game would:
// nothing, with the problems in `issues`, when that finds a fatal one.
std::optional<std::string> MakeWarpsFileText(const std::filesystem::path& file, const WarpList& list,
                                             std::vector<Items::ItemDataIssue>& issues);

// The items the rules name that the game's item list does not have, as errors
// (ValidateWarpItems).
void ValidateWarpItemsInGame(const WarpList& list, const std::string& source,
                             std::vector<Items::ItemDataIssue>& issues);

// At start (after the item list): the file into the list, its problems and
// those of ValidateWarpItemsInGame logged. False, with the list
// left as it was and `errorMessage` describing them, on fatal ones.
bool LoadWarps(std::string& errorMessage);

// The editor's Save: the list into the file after its checks; false, with
// nothing written, when they find a fatal problem or the file cannot be
// written (also in `issues`).
bool SaveWarps(std::vector<Items::ItemDataIssue>& issues);

// The editor's Revert: the file into the list again; false, with the list left
// as it was, on fatal problems.
bool RevertWarps(std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Maps
