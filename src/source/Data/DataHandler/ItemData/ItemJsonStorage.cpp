#include "stdafx.h"

#include "ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <optional>

namespace Data::Items
{
namespace
{
constexpr const char* JsonExtension = ".json";
constexpr const char* TemporaryExtension = ".tmp";
constexpr std::string_view Utf8ByteOrderMark = "\xEF\xBB\xBF";

// Same order as the ITEM_GROUP_* constants.
constexpr std::array<const char*, MAX_ITEM_TYPE> GroupFileNames = {
    "Sword", "Axe", "Mace", "Spear", "Bow", "Staff", "Shield", "Helm",
    "Armor", "Pants", "Gloves", "Boots", "Wing", "Helper", "Potion", "Etc"};

std::optional<std::string> ReadTextFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return std::nullopt;
    }

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (text.starts_with(Utf8ByteOrderMark))
    {
        text.erase(0, Utf8ByteOrderMark.size());
    }
    return text;
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

void AddError(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& message)
{
    issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, "", message});
}

bool WriteFileIfChanged(const std::filesystem::path& path, const std::string& text, std::vector<ItemDataIssue>& issues)
{
    const std::optional<std::string> current = ReadTextFile(path);
    if (current == text)
    {
        return true;
    }

    std::filesystem::path temporaryPath = path;
    temporaryPath += TemporaryExtension;
    {
        std::ofstream file(temporaryPath, std::ios::binary | std::ios::trunc);
        file << text;
        if (!file)
        {
            AddError(issues, temporaryPath.string(), "could not be written");
            return false;
        }
    }

    std::error_code error;
    std::filesystem::rename(temporaryPath, path, error);
    if (error)
    {
        AddError(issues, path.string(), "could not be replaced: " + error.message());
        std::filesystem::remove(temporaryPath, error);
        return false;
    }
    return true;
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
    const std::vector<std::filesystem::path> files = FindJsonFiles(directory);
    if (files.empty())
    {
        AddError(result.issues, directory.string(), "no item data files found");
        return result;
    }

    for (const std::filesystem::path& file : files)
    {
        const std::string source = file.filename().string();
        const std::optional<std::string> text = ReadTextFile(file);
        if (!text)
        {
            AddError(result.issues, source, "could not be read");
            continue;
        }
        ReadItemGroupJson(*text, source, result.items, result.issues);
    }

    ValidateItems(result.items, result.issues);
    return result;
}

bool SaveItemDataDirectory(const std::filesystem::path& directory, std::span<const ItemDefinition> items,
                           std::vector<ItemDataIssue>& issues)
{
    std::vector<ItemDefinition> existingItems;
    std::copy_if(items.begin(), items.end(), std::back_inserter(existingItems),
                 [](const ItemDefinition& definition) { return definition.Exists(); });

    ValidateItems(existingItems, issues);
    if (HasErrors(issues))
    {
        return false;
    }

    std::error_code error;
    std::filesystem::create_directories(directory, error);

    bool success = true;
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        const std::string text = WriteItemGroupJson(group, existingItems);
        success = WriteFileIfChanged(directory / GetItemGroupFileName(group), text, issues) && success;
    }
    return success;
}
} // namespace Data::Items
