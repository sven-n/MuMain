#include "stdafx.h"

#include "SoundFileStorage.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "Data/GameData/SoundData/SoundFilesJson.h"

#include <chrono>

namespace Data::Sounds
{
namespace
{
constexpr const char* SoundFilesFile = "Data/Audio/SoundFiles.json";
constexpr std::string_view SoundFilesLabel = "Sound files";
constexpr std::string_view SoundFilesName = "sound files";
} // namespace

std::filesystem::path GetSoundFilesFile()
{
    return SoundFilesFile;
}

SoundFilesLoadResult LoadSoundFilesFile(const std::filesystem::path& file)
{
    SoundFilesLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    ReadSoundFilesJson(*text, source, result.sounds, result.issues);
    ValidateSoundFiles(result.sounds, source, result.issues);
    ValidateSoundFilesOnDisk(result.sounds, source, result.issues);
    return result;
}

std::optional<std::string> MakeSoundFilesFileText(const std::filesystem::path& file, std::span<const SoundFile> sounds,
                                                  std::vector<Items::ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    std::string text = WriteSoundFilesJson(sounds);
    std::vector<SoundFile> readBack;
    std::vector<Items::ItemDataIssue> found;
    ReadSoundFilesJson(text, source, readBack, found);
    ValidateSoundFiles(readBack, source, found);
    ValidateSoundFilesOnDisk(readBack, source, found);
    issues.insert(issues.end(), found.begin(), found.end());
    if (Items::HasFatal(found))
    {
        return std::nullopt;
    }
    return text;
}

bool LoadSoundFiles(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    const SoundFilesLoadResult result = LoadSoundFilesFile(GetSoundFilesFile());
    LogDataIssues(SoundFilesLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(SoundFilesName, GetSoundFilesFile(), result.issues);
        return false;
    }
    g_SoundFileDatabase.Build(result.sounds);
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} sound files from {} in {:.1f} ms", result.sounds.size(),
                GetSoundFilesFile().generic_string(), elapsed.count());
    return true;
}

bool SaveSoundFiles(std::vector<Items::ItemDataIssue>& issues)
{
    const std::optional<std::string> text =
        MakeSoundFilesFileText(GetSoundFilesFile(), g_SoundFileDatabase.GetAll(), issues);
    if (!text)
    {
        return false;
    }
    std::string error;
    if (!WriteTextFile(GetSoundFilesFile(), *text, error))
    {
        issues.push_back({Items::ItemDataIssueSeverity::Fatal, GetSoundFilesFile().generic_string(),
                          Items::ItemDataIssue::NoItem, Items::ItemDataIssue::NoItem, "", error});
        return false;
    }
    return true;
}

bool RevertSoundFiles(std::vector<Items::ItemDataIssue>& issues)
{
    SoundFilesLoadResult result = LoadSoundFilesFile(GetSoundFilesFile());
    issues.insert(issues.end(), result.issues.begin(), result.issues.end());
    if (Items::HasFatal(result.issues))
    {
        return false;
    }
    g_SoundFileDatabase.Build(result.sounds);
    return true;
}
} // namespace Data::Sounds
