#include "stdafx.h"

#include "ItemJsonStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>

#ifndef _WIN32
#include "Core/Platform/PathResolve.h"
#endif

namespace Data::Items
{
namespace
{
constexpr const char* JsonExtension = ".json";

// Same order as the ITEM_GROUP_* constants.
constexpr std::array<const char*, MAX_ITEM_TYPE> GroupFileNames = {
    "Sword", "Axe",   "Mace",   "Spear", "Bow",  "Staff",  "Shield", "Helm",
    "Armor", "Pants", "Gloves", "Boots", "Wing", "Helper", "Potion", "Etc"};

// The shared model file, whatever the case of its name: Windows does not
// tell them apart, so "sharedModels.json" is the same file there.
bool IsSharedItemModelsFile(std::string_view fileName)
{
    const std::string_view expected = SharedItemModelsFileName;
    return std::equal(
        fileName.begin(), fileName.end(), expected.begin(), expected.end(), [](char left, char right)
        { return std::tolower(static_cast<unsigned char>(left)) == std::tolower(static_cast<unsigned char>(right)); });
}

std::vector<std::filesystem::path> FindJsonFiles(const std::filesystem::path& directory)
{
    std::vector<std::filesystem::path> files;
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(directory, error))
    {
        if (entry.is_regular_file() && entry.path().extension() == JsonExtension)
        {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

void AddFatal(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& message)
{
    issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, "", message});
}

// Reads every *.json file of the folder with readFile(text, source).
template <typename TReadFile>
void ReadJsonFiles(const std::filesystem::path& directory, const char* noFilesMessage,
                   std::vector<ItemDataIssue>& issues, TReadFile&& readFile)
{
    const std::vector<std::filesystem::path> files = FindJsonFiles(directory);
    if (files.empty())
    {
        AddFatal(issues, directory.string(), noFilesMessage);
        return;
    }

    for (const std::filesystem::path& file : files)
    {
        const std::string source = file.filename().string();
        const std::optional<std::string> text = ReadTextFile(file);
        if (!text)
        {
            AddFatal(issues, source, "could not be read");
            continue;
        }
        readFile(*text, source);
    }
}

// A file a save writes, with its new content.
struct FileToSave
{
    std::filesystem::path path;
    std::string text;
};

bool WriteTemporaryFile(const FileToSave& file, std::vector<ItemDataIssue>& issues)
{
    std::string error;
    if (!Data::WriteTemporaryFile(file.path, file.text, error))
    {
        AddFatal(issues, GetTemporaryPath(file.path).string(), error);
        return false;
    }
    return true;
}

bool ReplaceWithTemporaryFile(const std::filesystem::path& path, std::vector<ItemDataIssue>& issues)
{
    std::string error;
    if (!Data::ReplaceWithTemporaryFile(path, error))
    {
        AddFatal(issues, path.string(), error);
        return false;
    }
    return true;
}

// Writes the files whose content changed. No old file is replaced before all
// the new ones are written, so a failed write changes nothing.
ItemDataSaveResult WriteChangedFiles(std::span<const FileToSave> files, std::vector<ItemDataIssue>& issues)
{
    std::vector<const FileToSave*> changedFiles;
    for (const FileToSave& file : files)
    {
        if (ReadTextFile(file.path) != file.text)
        {
            changedFiles.push_back(&file);
        }
    }

    for (size_t index = 0; index < changedFiles.size(); ++index)
    {
        if (WriteTemporaryFile(*changedFiles[index], issues))
        {
            continue;
        }
        std::error_code error;
        for (size_t written = 0; written < index; ++written)
        {
            std::filesystem::remove(GetTemporaryPath(changedFiles[written]->path), error);
        }
        return ItemDataSaveResult::WriteFailed;
    }

    bool success = true;
    for (const FileToSave* file : changedFiles)
    {
        success = ReplaceWithTemporaryFile(file->path, issues) && success;
    }
    return success ? ItemDataSaveResult::Saved : ItemDataSaveResult::WriteFailed;
}

// The items and the entries without a name: what the item files have.
std::vector<ItemDefinition> GetItemEntries(std::span<const ItemDefinition> items)
{
    std::vector<ItemDefinition> entries;
    std::copy_if(items.begin(), items.end(), std::back_inserter(entries),
                 [](const ItemDefinition& definition) { return definition.HasEntry(); });
    return entries;
}

void AddItemGroupFiles(const std::filesystem::path& directory, std::span<const ItemDefinition> entries,
                       std::vector<FileToSave>& files)
{
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        files.push_back({directory / GetItemGroupFileName(group), WriteItemGroupJson(group, entries)});
    }
}

void ValidateExistingItemModels(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues)
{
    std::vector<ItemModelDefinition> existingModels;
    std::copy_if(models.begin(), models.end(), std::back_inserter(existingModels),
                 [](const ItemModelDefinition& model) { return model.Exists(); });
    ValidateItemModels(existingModels, issues);
}

void AddItemModelFiles(const ItemModelSave& models, std::vector<FileToSave>& files)
{
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        if (models.groups.test(static_cast<size_t>(group)))
        {
            files.push_back(
                {models.directory / GetItemGroupFileName(group), WriteItemModelGroupJson(group, models.models)});
        }
    }
    if (models.sharedModelsChanged)
    {
        files.push_back({models.directory / SharedItemModelsFileName, WriteSharedItemModelsJson(models.sharedModels)});
    }
    if (models.glowColorsChanged)
    {
        files.push_back({models.glowColorsFile, Effects::WriteGlowColorsJson(models.glowColors)});
    }
#ifdef _EDITOR
    if (models.statsChanged && models.stats != nullptr)
    {
        files.push_back({models.statsFile, WriteItemStatsJson(*models.stats)});
    }
#endif
}
} // namespace

std::filesystem::path GetItemDataDirectory()
{
    return std::filesystem::path("Data") / "Items";
}

std::string GetItemGroupFileName(int group)
{
    const std::string number = (group < 10 ? "0" : "") + std::to_string(group);
    return "Group" + number + "_" + GroupFileNames[group] + JsonExtension;
}

ItemDataLoadResult LoadItemDataDirectory(const std::filesystem::path& directory)
{
    ItemDataLoadResult result;
    ReadJsonFiles(directory, "no item data files found", result.issues,
                  [&](std::string_view text, const std::string& source)
                  { ReadItemGroupJson(text, source, result.items, result.issues); });
    ValidateItems(result.items, result.issues);
    return result;
}

std::filesystem::path GetItemModelDataDirectory()
{
    return GetItemDataDirectory() / "Models";
}

ItemModelDataLoadResult LoadItemModelDataDirectory(const std::filesystem::path& directory)
{
    ItemModelDataLoadResult result;
    ReadJsonFiles(directory, "no item model files found", result.issues,
                  [&](std::string_view text, const std::string& source)
                  {
                      if (IsSharedItemModelsFile(source))
                      {
                          ReadSharedItemModelsJson(text, source, result.sharedModels, result.issues);
                      }
                      else
                      {
                          ReadItemModelGroupJson(text, source, result.models, result.issues);
                      }
                  });
    ApplySharedItemModels(result.models, result.sharedModels, SharedItemModelsFileName, result.issues);
    ValidateItemModels(result.models, result.issues);
    // These checks go across files; the group file of the item is where to fix it.
    for (ItemDataIssue& issue : result.issues)
    {
        if (issue.source.empty() && issue.group != ItemDataIssue::NoItem)
        {
            issue.source = GetItemGroupFileName(issue.group);
        }
    }
    return result;
}

std::filesystem::path GetItemStatsFile()
{
    return GetItemDataDirectory() / "Stats" / ItemStatsFileName;
}

ItemStatsLoadResult LoadItemStatsFile(const std::filesystem::path& file)
{
    ItemStatsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFatal(result.issues, source, "could not be read");
        return result;
    }
    ReadItemStatsJson(*text, source, result.file, result.issues);
    return result;
}

std::filesystem::path GetItemTooltipFile()
{
    return GetItemDataDirectory() / "Tooltips" / ItemTooltipFileName;
}

ItemTooltipLoadResult LoadItemTooltipFile(const std::filesystem::path& file)
{
    ItemTooltipLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFatal(result.issues, source, "could not be read");
        return result;
    }
    ReadItemTooltipJson(*text, source, result.file, result.issues);
    return result;
}

std::filesystem::path GetGlowColorsFile()
{
    return std::filesystem::path(Effects::GlowColorsFile);
}

GlowColorsLoadResult LoadGlowColorsFile(const std::filesystem::path& file)
{
    GlowColorsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFatal(result.issues, source, "could not be read");
        return result;
    }
    Effects::ReadGlowColorsJson(*text, source, result.colors, result.issues);
    return result;
}

ItemDataSaveResult SaveItemDataDirectory(const std::filesystem::path& directory, std::span<const ItemDefinition> items,
                                         std::vector<ItemDataIssue>& issues)
{
    return SaveItemAndModelDataDirectories(directory, items, ItemModelSave{}, issues);
}

ItemDataSaveResult WriteItemTextFile(const std::filesystem::path& file, const std::string& text,
                                     std::vector<ItemDataIssue>& issues)
{
    const FileToSave files[] = {{file, text}};
    return WriteChangedFiles(files, issues);
}

ItemDataSaveResult SaveItemAndModelDataDirectories(const std::filesystem::path& itemDirectory,
                                                   std::span<const ItemDefinition> items, const ItemModelSave& models,
                                                   std::vector<ItemDataIssue>& issues)
{
    ValidateItems(GetItemEntries(items), issues);
    ValidateExistingItemModels(models.models, issues);
    ValidateModelsHaveItemEntries(items, models.models, issues);
    if (HasFatal(issues))
    {
        return ItemDataSaveResult::InvalidData;
    }
    return WriteItemAndModelDataDirectories(itemDirectory, items, models, issues);
}

ItemDataSaveResult WriteItemAndModelDataDirectories(const std::filesystem::path& itemDirectory,
                                                    std::span<const ItemDefinition> items, const ItemModelSave& models,
                                                    std::vector<ItemDataIssue>& issues)
{
    std::vector<FileToSave> files;
    AddItemGroupFiles(itemDirectory, GetItemEntries(items), files);
    AddItemModelFiles(models, files);
    return WriteChangedFiles(files, issues);
}

void ValidateItemModelsAsWritten(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues)
{
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        std::vector<ItemModelDefinition> readBack;
        std::vector<ItemDataIssue> readIssues;
        ReadItemModelGroupJson(WriteItemModelGroupJson(group, models), GetItemGroupFileName(group), readBack,
                               readIssues);
        std::copy_if(readIssues.begin(), readIssues.end(), std::back_inserter(issues),
                     [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Fatal; });
    }
}

void ValidateSharedItemModelsAsWritten(std::span<const SharedItemModel> sharedModels,
                                       std::vector<ItemDataIssue>& issues)
{
    std::vector<SharedItemModel> readBack;
    std::vector<ItemDataIssue> readIssues;
    ReadSharedItemModelsJson(WriteSharedItemModelsJson(sharedModels), SharedItemModelsFileName, readBack, readIssues);
    std::copy_if(readIssues.begin(), readIssues.end(), std::back_inserter(issues),
                 [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Fatal; });
}

void ValidateGlowColorsAsWritten(std::span<const Effects::GlowColor> colors, std::vector<ItemDataIssue>& issues)
{
    std::vector<Effects::GlowColor> readBack;
    std::vector<ItemDataIssue> readIssues;
    Effects::ReadGlowColorsJson(Effects::WriteGlowColorsJson(colors), Effects::GlowColorsFile, readBack, readIssues);
    std::copy_if(readIssues.begin(), readIssues.end(), std::back_inserter(issues),
                 [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Fatal; });
}

#ifdef _EDITOR
void ValidateItemStatsAsWritten(const ItemStatFile& file, std::vector<ItemDataIssue>& issues)
{
    ItemStatFile readBack;
    std::vector<ItemDataIssue> readIssues;
    ReadItemStatsJson(WriteItemStatsJson(file), GetItemStatsFile().generic_string(), readBack, readIssues);
    std::copy_if(readIssues.begin(), readIssues.end(), std::back_inserter(issues),
                 [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Fatal; });
}
#endif

void ValidateItemModelFilesExist(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues)
{
    for (const ItemModelDefinition& model : models)
    {
        if (model.file.empty())
        {
            continue;
        }
#ifdef _WIN32
        const std::filesystem::path path = std::filesystem::u8path(model.file);
#else
        // The data does not always spell the files in the case they have on disk.
        const std::filesystem::path path = MuResolvePath(model.file.c_str());
#endif
        std::error_code ignored;
        if (!std::filesystem::is_regular_file(path, ignored))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, GetItemGroupFileName(model.group), model.group,
                              model.number, "file",
                              model.file + " is not in the client folder; the item is drawn without a model"});
        }
    }
}
} // namespace Data::Items
