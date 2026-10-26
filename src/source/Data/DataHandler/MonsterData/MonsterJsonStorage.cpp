#include "stdafx.h"

#include "MonsterJsonStorage.h"

#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonstersJson.h"

#include <optional>
#include <string>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;

constexpr const char* MonstersFile = "Data/Monsters/Monsters.json";

void AddFileError(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& message)
{
    issues.push_back(
        {Items::ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, "", message});
}
} // namespace

std::filesystem::path GetMonstersFile()
{
    return MonstersFile;
}

MonsterDataLoadResult LoadMonstersFile(const std::filesystem::path& file)
{
    MonsterDataLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        AddFileError(result.issues, source, "could not be read");
        return result;
    }
    ReadMonstersJson(*text, source, result.monsters, result.issues);
    const std::size_t firstValidationIssue = result.issues.size();
    ValidateMonsterNames(result.monsters, result.issues);
    NameIssueSource(result.issues, firstValidationIssue, file);
    return result;
}

bool SaveMonstersFile(const std::filesystem::path& file, std::span<const MonsterDefinition> monsters,
                      std::vector<ItemDataIssue>& issues)
{
    const std::optional<std::string> text = MakeMonstersFileText(file, monsters, issues);
    if (!text)
    {
        return false;
    }
    std::string error;
    if (!WriteTextFile(file, *text, error))
    {
        AddFileError(issues, file.generic_string(), error);
        return false;
    }
    return true;
}

std::optional<std::string> MakeMonstersFileText(const std::filesystem::path& file,
                                                std::span<const MonsterDefinition> monsters,
                                                std::vector<ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    std::string text = WriteMonstersJson(monsters);
    std::vector<MonsterDefinition> readBack;
    ReadMonstersJson(text, source, readBack, issues);
    const std::size_t firstValidationIssue = issues.size();
    ValidateMonsterNames(readBack, issues);
    NameIssueSource(issues, firstValidationIssue, file);
    if (Items::HasErrors(issues))
    {
        return std::nullopt;
    }
    return text;
}

void NameIssueSource(std::vector<ItemDataIssue>& issues, std::size_t first, const std::filesystem::path& file)
{
    const std::string source = file.generic_string();
    for (std::size_t index = first; index < issues.size(); ++index)
    {
        issues[index].source = source;
    }
}
} // namespace Data::Monsters
