#include "stdafx.h"

#include "SoundFolderStorage.h"

#include "Core/Content/ContentStore.h"

#include "Data/DataHandler/TextFile.h"

#include <optional>
#include <string>

namespace Data::Sounds
{
std::filesystem::path GetSoundFoldersFile()
{
    return "Data/Audio/SoundFolders.json";
}

SoundFoldersLoadResult LoadSoundFoldersFile(const std::filesystem::path& file)
{
    SoundFoldersLoadResult result;
    if (const Content::Found found = Content::Probe(file);
        found == Content::Found::Nothing || found == Content::Found::Unknown)
    {
        return result;
    }
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, file.generic_string(),
                                 Items::ItemDataIssue::NoItem, Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    ReadSoundFoldersJson(*text, file.generic_string(), result.folders, result.issues);
    return result;
}

bool SaveSoundFolders(const std::filesystem::path& file, std::span<const SoundFolder> folders,
                      std::vector<Items::ItemDataIssue>& issues)
{
    std::string error;
    if (!WriteTextFile(file, WriteSoundFoldersJson(folders), error))
    {
        issues.push_back({Items::ItemDataIssueSeverity::Fatal, file.generic_string(), Items::ItemDataIssue::NoItem,
                          Items::ItemDataIssue::NoItem, "", error});
        return false;
    }
    return true;
}
} // namespace Data::Sounds
