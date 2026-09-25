#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"

#include <filesystem>
#include <span>
#include <string>
#include <vector>

// Reads and writes the item data folder (Data/Items): one JSON file per
// item group, see ItemJsonFormat.h.
namespace Data::Items
{
struct ItemDataLoadResult
{
    std::vector<ItemDefinition> items;
    std::vector<ItemDataIssue> issues;
};

// Data/Items, relative to the client folder.
std::filesystem::path GetItemDataDirectory();

// "Group00_Sword.json" ... "Group15_Etc.json".
std::string GetItemGroupFileName(int group);

// Reads every *.json file in the folder and validates the items.
ItemDataLoadResult LoadItemDataDirectory(const std::filesystem::path& directory);

// Validates the items and, when there are no errors, writes one file per
// group. Each file is written to a temporary file first and then replaces
// the old one; files whose content did not change are not touched.
// Returns false (and writes nothing) when validation finds errors.
bool SaveItemDataDirectory(const std::filesystem::path& directory, std::span<const ItemDefinition> items,
                           std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
