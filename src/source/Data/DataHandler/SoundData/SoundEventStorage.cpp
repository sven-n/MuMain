#include "stdafx.h"

#include "SoundEventStorage.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/SoundData/SoundEventDatabase.h"

namespace Data::Sounds
{
namespace
{
constexpr std::string_view SoundEventsLabel = "Sound events";
constexpr std::string_view SoundEventsName = "sound events";
} // namespace

std::filesystem::path GetSoundEventsFile()
{
    return "Data/Audio/SoundEvents.json";
}

SoundEventsLoadResult LoadSoundEventsFile(const std::filesystem::path& file)
{
    SoundEventsLoadResult result;
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, file.generic_string(),
                                 Items::ItemDataIssue::NoItem, Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    ReadSoundEventsJson(*text, file.generic_string(), result.events, result.issues);
    return result;
}

std::optional<std::string> MakeSoundEventsFileText(const std::filesystem::path& file,
                                                   std::span<const SoundEvent> events,
                                                   std::vector<Items::ItemDataIssue>& issues)
{
    std::string text = WriteSoundEventsJson(events);
    std::vector<SoundEvent> readBack;
    std::vector<Items::ItemDataIssue> found;
    ReadSoundEventsJson(text, file.generic_string(), readBack, found);
    issues.insert(issues.end(), found.begin(), found.end());
    if (Items::HasFatal(found))
    {
        return std::nullopt;
    }
    return text;
}

bool LoadSoundEvents(std::string& errorMessage)
{
    const SoundEventsLoadResult result = LoadSoundEventsFile(GetSoundEventsFile());
    LogDataIssues(SoundEventsLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(SoundEventsName, GetSoundEventsFile(), result.issues);
        return false;
    }
    g_SoundEventDatabase.Build(result.events);
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} sound events from {}", result.events.size(),
                GetSoundEventsFile().generic_string());
    return true;
}

bool SaveSoundEvents(std::vector<Items::ItemDataIssue>& issues)
{
    const std::optional<std::string> text =
        MakeSoundEventsFileText(GetSoundEventsFile(), g_SoundEventDatabase.GetAll(), issues);
    if (!text)
    {
        return false;
    }
    std::string error;
    if (!WriteTextFile(GetSoundEventsFile(), *text, error))
    {
        issues.push_back({Items::ItemDataIssueSeverity::Fatal, GetSoundEventsFile().generic_string(),
                          Items::ItemDataIssue::NoItem, Items::ItemDataIssue::NoItem, "", error});
        return false;
    }
    return true;
}

bool RevertSoundEvents(std::vector<Items::ItemDataIssue>& issues)
{
    SoundEventsLoadResult result = LoadSoundEventsFile(GetSoundEventsFile());
    issues.insert(issues.end(), result.issues.begin(), result.issues.end());
    if (Items::HasFatal(result.issues))
    {
        return false;
    }
    g_SoundEventDatabase.Build(result.events);
    return true;
}
} // namespace Data::Sounds
