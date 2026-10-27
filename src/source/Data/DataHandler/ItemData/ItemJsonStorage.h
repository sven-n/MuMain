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

// Writes `text` to `file` as the saves do: by a temporary file that replaces it, so a failed
// write leaves the old file. An unchanged file is not touched.
ItemDataSaveResult WriteItemTextFile(const std::filesystem::path& file, const std::string& text,
                                     std::vector<ItemDataIssue>& issues);

// The item models a save writes with the items.
struct ItemModelSave
{
    std::filesystem::path directory;
    // The slots of the item model database.
    std::span<const ItemModelDefinition> models;
    // The groups whose model files are written.
    ItemGroupSet groups;
    // Written to SharedModels.json when sharedModelsChanged.
    std::span<const SharedItemModel> sharedModels;
    bool sharedModelsChanged = false;
    // Written to glowColorsFile (GetGlowColorsFile) when glowColorsChanged.
    std::filesystem::path glowColorsFile;
    std::span<const Effects::GlowColor> glowColors;
    bool glowColorsChanged = false;
#ifdef _EDITOR
    // Written to statsFile (GetItemStatsFile) when statsChanged.
    std::filesystem::path statsFile;
    const ItemStatFile* stats = nullptr;
    bool statsChanged = false;
#endif
};

// Like SaveItemDataDirectory, and the models are checked and written with the
// items: one model file for each of the groups, and the shared model file when
// it changed. A failed write leaves the item files and the model files as
// they were.
ItemDataSaveResult SaveItemAndModelDataDirectories(const std::filesystem::path& itemDirectory,
                                                   std::span<const ItemDefinition> items, const ItemModelSave& models,
                                                   std::vector<ItemDataIssue>& issues);

// Like SaveItemAndModelDataDirectories without the checks, for data that was
// already checked; a failed write is reported in `issues`.
ItemDataSaveResult WriteItemAndModelDataDirectories(const std::filesystem::path& itemDirectory,
                                                    std::span<const ItemDefinition> items, const ItemModelSave& models,
                                                    std::vector<ItemDataIssue>& issues);

// The rules the model file reader applies to a value (scales above 0,
// specular 0 to 1, glow levels, mesh lists, file and folder paths, names):
// writes the models of every group as the save would and reads them back, so
// a value the next start would refuse is an error now. The problems name the
// group file.
void ValidateItemModelsAsWritten(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues);

// The same for the shared models: the group files name them only, so their
// file, texture folders and meshes are read back from SharedModels.json as the
// save would write it.
void ValidateSharedItemModelsAsWritten(std::span<const SharedItemModel> sharedModels,
                                       std::vector<ItemDataIssue>& issues);

// The same for the glow colors (a name or value the item editor gave one),
// read back from GlowColors.json as the save would write it.
void ValidateGlowColorsAsWritten(std::span<const Effects::GlowColor> colors, std::vector<ItemDataIssue>& issues);

#ifdef _EDITOR
// The same for the item stat file (the changes of Item Formulas), read back
// from ItemStats.json as the save would write it.
void ValidateItemStatsAsWritten(const ItemStatFile& file, std::vector<ItemDataIssue>& issues);
#endif

// A model file that is not in the client folder (relative to the working
// directory): a warning for each model, since the game starts without it and
// draws nothing for the item.
void ValidateItemModelFilesExist(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
