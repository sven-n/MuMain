#pragma once

#include "Data/GameData/EffectData/GlowColors.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"
#include "Data/GameData/ItemData/SharedItemModel.h"

#include <bitset>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

// Reads and writes the item data folder (Data/Items): one JSON file per
// item group, see ItemJsonFormat.h, and the item model folder
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
    // With the file, texture folders and none-blend meshes of their shared
    // model filled in.
    std::vector<ItemModelDefinition> models;
    std::vector<SharedItemModel> sharedModels;
    std::vector<ItemDataIssue> issues;
};

// Data/Items/Models: the item model files, see ItemModelJsonFormat.h. They
// use the same file names as the item data files.
std::filesystem::path GetItemModelDataDirectory();

// Reads every *.json file in the folder (the group files and the shared
// models, SharedModels.json) and validates the models.
ItemModelDataLoadResult LoadItemModelDataDirectory(const std::filesystem::path& directory);

struct ItemStatsLoadResult
{
    ItemStatFile file;
    std::vector<ItemDataIssue> issues;
};

// Data/Items/Stats/ItemStats.json, relative to the client folder: what the
// + level, excellent options, ancient sets and option level add to the item
// stats and requirements, see
// ItemStatsJson.h. It is in a folder of its own because every *.json file in
// Data/Items is read as an item group file.
std::filesystem::path GetItemStatsFile();

ItemStatsLoadResult LoadItemStatsFile(const std::filesystem::path& file);

struct ItemTooltipLoadResult
{
    ItemTooltipFile file;
    std::vector<ItemDataIssue> issues;
};

// Data/Items/Tooltips/Tooltip.json, relative to the client folder: the shared texts and the layouts of the item
// tooltips, see ItemTooltipLayouts.h. In a folder of its own like the stat file.
std::filesystem::path GetItemTooltipFile();

ItemTooltipLoadResult LoadItemTooltipFile(const std::filesystem::path& file);

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

// Item groups by number, e.g. the groups whose model files a save writes.
using ItemGroupSet = std::bitset<MAX_ITEM_TYPE>;

// Validates the items and, when there are no errors, writes one file per
// group. Each changed file is written to a temporary file first; only when
// all of them could be written do they replace the old files, so a full disk
// or a folder that cannot be written leaves every file as it was. Files
// whose content did not change are not touched.
ItemDataSaveResult SaveItemDataDirectory(const std::filesystem::path& directory, std::span<const ItemDefinition> items,
                                         std::vector<ItemDataIssue>& issues);

// Like SaveItemDataDirectory, and the models (the slots of the item model
// database) are checked and written with the items, one model file for each
// of `modelGroups`; a failed write leaves the item files and the model files
// as they were. The shared model file is not written, nothing changes it yet.
ItemDataSaveResult SaveItemAndModelDataDirectories(const std::filesystem::path& itemDirectory,
                                                   std::span<const ItemDefinition> items,
                                                   const std::filesystem::path& modelDirectory,
                                                   std::span<const ItemModelDefinition> models,
                                                   const ItemGroupSet& modelGroups, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
