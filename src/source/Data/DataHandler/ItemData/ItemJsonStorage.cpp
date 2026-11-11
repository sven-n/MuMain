#include "stdafx.h"

#include "ItemJsonStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemOptionValidation.h"
#include "Data/GameData/ItemData/ItemOptionsJson.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"
#include "Data/GameData/ItemData/OptionRulesJson.h"
#include "Data/GameData/ItemData/OptionTextsJson.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>

#include "Core/Text/Utf8Path.h"

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

// Whether the file is the one of this name, whatever the case of its name: Windows does not tell them apart, so
// "sharedModels.json" is the shared model file there.
bool IsFileNamed(std::string_view fileName, std::string_view expected)
{
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
    if (models.partModelsChanged)
    {
        files.push_back({models.directory / PartModelsFileName, WritePartModelsJson(models.partModels)});
    }
    if (models.glowColorsChanged)
    {
        files.push_back(
            {models.glowColorsFile, Effects::WriteGlowColorsJson(models.glowColors, models.glowColorFolders)});
    }
#ifdef _EDITOR
    if (models.statsChanged && models.stats != nullptr)
    {
        files.push_back({models.statsFile, WriteItemStatsJson(*models.stats)});
    }
    if (models.groundLabelsChanged && models.groundLabels != nullptr)
    {
        files.push_back({models.groundLabelsFile, WriteGroundLabelsJson(*models.groundLabels)});
    }
    for (const LookFileToSave& lookFile : models.lookFiles)
    {
        files.push_back({lookFile.file, lookFile.text});
    }
    if (models.nameCasesChanged && models.nameCases != nullptr)
    {
        files.push_back({models.nameCasesFile, WriteItemNameCasesJson(*models.nameCases)});
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
                      if (IsFileNamed(source, SharedItemModelsFileName))
                      {
                          ReadSharedItemModelsJson(text, source, result.sharedModels, result.issues);
                      }
                      else if (IsFileNamed(source, PartModelsFileName))
                      {
                          ReadPartModelsJson(text, source, result.partModels, result.issues);
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

std::filesystem::path GetItemOptionsDirectory()
{
    return GetItemDataDirectory() / "Options";
}

ItemOptionsLoadResult LoadItemOptionsDirectory(const std::filesystem::path& directory)
{
    ItemOptionsLoadResult result;
    std::vector<std::filesystem::path> files = FindJsonFiles(directory);
    std::erase_if(files, [](const std::filesystem::path& file) { return file.filename() == OptionRulesFileName; });
    if (files.empty())
    {
        AddFatal(result.issues, directory.generic_string(), "no option definition files found");
        return result;
    }
    for (const std::filesystem::path& file : files)
    {
        const std::string fileName = file.filename().string();
        const std::string source = "Options/" + fileName;
        const std::optional<std::string> text = ReadTextFile(file);
        if (!text)
        {
            AddFatal(result.issues, source, "could not be read");
            continue;
        }
        ReadItemOptionsJson(*text, source, fileName, result.definitions, result.issues);
    }
    return result;
}

std::filesystem::path GetOptionTextsFile()
{
    return GetItemDataDirectory() / "Tooltips" / OptionTextsFileName;
}

OptionTextsLoadResult LoadOptionTextsFile(const std::filesystem::path& file)
{
    OptionTextsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFatal(result.issues, source, "could not be read");
        return result;
    }
    ReadOptionTextsJson(*text, source, result.texts, result.issues);
    return result;
}

std::filesystem::path GetOptionRulesFile()
{
    return GetItemOptionsDirectory() / OptionRulesFileName;
}

OptionRulesLoadResult LoadOptionRulesFile(const std::filesystem::path& file)
{
    OptionRulesLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFatal(result.issues, source, "could not be read");
        return result;
    }
    ReadOptionRulesJson(*text, source, result.rules, result.issues);
    return result;
}

ItemOptionDatabase LoadItemOptionFiles(bool withRules, std::span<const char* const> uiLocales,
                                       std::vector<ItemDataIssue>& issues)
{
    ItemOptionsLoadResult definitions = LoadItemOptionsDirectory(GetItemOptionsDirectory());
    OptionTextsLoadResult texts = LoadOptionTextsFile(GetOptionTextsFile());
    const std::string textsSource = GetOptionTextsFile().generic_string();
    ValidateOptionTextLocales(texts.texts, uiLocales, textsSource, texts.issues);
    issues.insert(issues.end(), definitions.issues.begin(), definitions.issues.end());
    issues.insert(issues.end(), texts.issues.begin(), texts.issues.end());
    std::vector<OptionRule> rules;
    if (withRules)
    {
        OptionRulesLoadResult ruleFile = LoadOptionRulesFile(GetOptionRulesFile());
        issues.insert(issues.end(), ruleFile.issues.begin(), ruleFile.issues.end());
        if (!HasFatal(ruleFile.issues) && !HasFatal(definitions.issues))
        {
            ValidateOptionRules(ruleFile.rules, definitions.definitions, GetOptionRulesFile().generic_string(), issues);
        }
        rules = std::move(ruleFile.rules);
    }
    if (HasFatal(definitions.issues) || HasFatal(texts.issues))
    {
        return {};
    }
    ValidateItemOptions(definitions.definitions, texts.texts, textsSource, issues);
    return ItemOptionDatabase(std::move(definitions.definitions), std::move(texts.texts), std::move(rules));
}

std::filesystem::path GetGroundLabelsFile()
{
    return GetItemDataDirectory() / "Tooltips" / GroundLabelsFileName;
}

GroundLabelsLoadResult LoadGroundLabelsFile(const std::filesystem::path& file)
{
    GroundLabelsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFatal(result.issues, source, "could not be read");
        return result;
    }
    ReadGroundLabelsJson(*text, source, result.file, result.issues);
    return result;
}

std::filesystem::path GetItemNameCasesFile()
{
    return GetItemDataDirectory() / "Tooltips" / ItemNameCasesFileName;
}

ItemNameCasesLoadResult LoadItemNameCasesFile(const std::filesystem::path& file)
{
    ItemNameCasesLoadResult result;
    std::error_code error;
    if (!std::filesystem::exists(file, error))
    {
        return result;
    }
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFatal(result.issues, source, "could not be read");
        return result;
    }
    ReadItemNameCasesJson(*text, source, result.cases, result.issues);
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
    Effects::ReadGlowColorsJson(*text, source, result.colors, result.folders, result.issues);
    return result;
}

std::filesystem::path GetShinesFile()
{
    return std::filesystem::path(Effects::ShinesFile);
}

ShinesLoadResult LoadShinesFile(const std::filesystem::path& file)
{
    ShinesLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFatal(result.issues, source, "could not be read");
        return result;
    }
    Effects::ReadShinesJson(*text, source, result.shines, result.issues);
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
#ifdef _EDITOR
    for (const TextFileToWrite& file : models.alsoWrite)
    {
        files.push_back({file.path, std::string(file.text)});
    }
#endif
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

void ValidateGroundLabelsAsWritten(const GroundLabelFile& file, std::vector<ItemDataIssue>& issues)
{
    GroundLabelFile readBack;
    std::vector<ItemDataIssue> readIssues;
    ReadGroundLabelsJson(WriteGroundLabelsJson(file), GetGroundLabelsFile().generic_string(), readBack, readIssues);
    std::copy_if(readIssues.begin(), readIssues.end(), std::back_inserter(issues),
                 [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Fatal; });
}
#endif

namespace
{
// Whether a model file of the data is in the client folder.
bool ModelFileExists(const std::string& file)
{
#ifdef _WIN32
    const std::filesystem::path path = Core::Text::PathFromUtf8(file);
#else
    // The data does not always spell the files in the case they have on disk.
    const std::filesystem::path path = MuResolvePath(file.c_str());
#endif
    std::error_code ignored;
    return std::filesystem::is_regular_file(path, ignored);
}
} // namespace

void ValidateItemModelFilesExist(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues)
{
    for (const ItemModelDefinition& model : models)
    {
        if (model.file.empty())
        {
            continue;
        }
        if (!ModelFileExists(model.file))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, GetItemGroupFileName(model.group), model.group,
                              model.number, "file",
                              model.file + " is not in the client folder; the item is drawn without a model"});
        }
    }
}

void ValidatePartModelFilesExist(std::span<const PartModel> parts, std::vector<ItemDataIssue>& issues)
{
    for (const PartModel& part : parts)
    {
        if (!part.model.file.empty() && !ModelFileExists(part.model.file))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, PartModelsFileName, ItemDataIssue::NoItem,
                              ItemDataIssue::NoItem, part.model.name + ".file",
                              part.model.file + " is not in the client folder; the part model is not drawn"});
        }
    }
}
} // namespace Data::Items
