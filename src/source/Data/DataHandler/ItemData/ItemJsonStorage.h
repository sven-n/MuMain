#pragma once

#include "Data/GameData/EffectData/GlowColors.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <filesystem>
#include <span>
#include <string>
#include <vector>

// Reads and writes the item data folder (Data/Items): one JSON file per
// item group, see ItemJsonFormat.h, and reads the item model folder
// (Data/Items/Models).
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

struct ItemModelDataLoadResult
{
    std::vector<ItemModelDefinition> models;
    std::vector<ItemDataIssue> issues;
};

// Data/Items/Models: the item model files, see ItemModelJsonFormat.h. They
// use the same file names as the item data files.
std::filesystem::path GetItemModelDataDirectory();

// Reads every *.json file in the folder and validates the models.
ItemModelDataLoadResult LoadItemModelDataDirectory(const std::filesystem::path& directory);

struct GlowColorsLoadResult
{
    std::vector<Effects::GlowColor> colors;
    std::vector<ItemDataIssue> issues;
};

// Data/Effects/GlowColors.json, relative to the client folder: the named
// colors of the item glow, see GlowColors.h.
std::filesystem::path GetGlowColorsFile();

GlowColorsLoadResult LoadGlowColorsFile(const std::filesystem::path& file);

enum class ItemDataSaveResult
{
    Saved,
    // Validation found errors; nothing was written.
    InvalidData,
    // The data is valid, but a file could not be written or replaced.
    WriteFailed,
};

// Validates the items and, when there are no errors, writes one file per
// group. Each file is written to a temporary file first and then replaces
// the old one; files whose content did not change are not touched.
ItemDataSaveResult SaveItemDataDirectory(const std::filesystem::path& directory, std::span<const ItemDefinition> items,
                                         std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
