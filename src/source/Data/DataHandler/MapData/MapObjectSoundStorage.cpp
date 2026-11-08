#include "stdafx.h"

#include "MapObjectSoundStorage.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MapData/MapObjectSoundsJson.h"

#include <chrono>

namespace Data::Maps
{
namespace
{
constexpr const char* MapObjectSoundsFile = "Data/Maps/ObjectSounds.json";
constexpr std::string_view MapSoundsLabel = "Map sounds";
constexpr std::string_view MapSoundsName = "map object sounds";
} // namespace

std::filesystem::path GetMapObjectSoundsFile()
{
    return MapObjectSoundsFile;
}

MapObjectSoundsLoadResult LoadMapObjectSoundsFile(const std::filesystem::path& file)
{
    MapObjectSoundsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    ReadMapObjectSoundsJson(*text, source, result.sounds, result.issues);
    ValidateMapObjectSounds(result.sounds, source, result.issues);
    ValidateMapObjectSoundFiles(result.sounds, source, result.issues);
    return result;
}

std::optional<std::string> MakeMapObjectSoundsFileText(const std::filesystem::path& file,
                                                       std::span<const MapObjectSound> sounds,
                                                       std::vector<Items::ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    std::string text = WriteMapObjectSoundsJson(sounds);
    std::vector<MapObjectSound> readBack;
    std::vector<Items::ItemDataIssue> found;
    ReadMapObjectSoundsJson(text, source, readBack, found);
    ValidateMapObjectSounds(readBack, source, found);
    ValidateMapObjectSoundFiles(readBack, source, found);
    issues.insert(issues.end(), found.begin(), found.end());
    if (Items::HasFatal(found))
    {
        return std::nullopt;
    }
    return text;
}

bool LoadMapObjectSounds(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    const MapObjectSoundsLoadResult result = LoadMapObjectSoundsFile(GetMapObjectSoundsFile());
    LogDataIssues(MapSoundsLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(MapSoundsName, GetMapObjectSoundsFile(), result.issues);
        return false;
    }
    g_MapObjectSoundDatabase.Build(result.sounds);
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} map object sounds from {} in {:.1f} ms", result.sounds.size(),
                GetMapObjectSoundsFile().generic_string(), elapsed.count());
    return true;
}

bool SaveMapObjectSounds(std::vector<Items::ItemDataIssue>& issues)
{
    const std::optional<std::string> text =
        MakeMapObjectSoundsFileText(GetMapObjectSoundsFile(), g_MapObjectSoundDatabase.GetAll(), issues);
    if (!text)
    {
        return false;
    }
    std::string error;
    if (!WriteTextFile(GetMapObjectSoundsFile(), *text, error))
    {
        issues.push_back({Items::ItemDataIssueSeverity::Fatal, GetMapObjectSoundsFile().generic_string(),
                          Items::ItemDataIssue::NoItem, Items::ItemDataIssue::NoItem, "", error});
        return false;
    }
    return true;
}

bool RevertMapObjectSounds(std::vector<Items::ItemDataIssue>& issues)
{
    MapObjectSoundsLoadResult result = LoadMapObjectSoundsFile(GetMapObjectSoundsFile());
    issues.insert(issues.end(), result.issues.begin(), result.issues.end());
    if (Items::HasFatal(result.issues))
    {
        return false;
    }
    g_MapObjectSoundDatabase.Build(result.sounds);
    return true;
}
} // namespace Data::Maps
