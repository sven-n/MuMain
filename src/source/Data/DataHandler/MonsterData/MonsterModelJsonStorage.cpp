#include "stdafx.h"

#include "MonsterModelJsonStorage.h"

#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"
#include "Data/GameData/MonsterData/MonsterModelsJson.h"

#include <optional>
#include <string>

namespace Data::Monsters
{
namespace
{
constexpr const char* MonsterModelsFile = "Data/Monsters/Models.json";
} // namespace

std::filesystem::path GetMonsterModelsFile()
{
    return MonsterModelsFile;
}

MonsterModelsLoadResult LoadMonsterModelsFile(const std::filesystem::path& file)
{
    MonsterModelsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Error, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    ReadMonsterModelsJson(*text, source, result.models, result.issues);
    ValidateMonsterModelNames(result.models, source, result.issues);
    ValidateMonsterModelFiles(result.models, source, result.issues);
    return result;
}

bool SaveMonsterModelsFile(const std::filesystem::path& file, std::span<const MonsterModelDefinition> models,
                           std::vector<Items::ItemDataIssue>& issues)
{
    const std::optional<std::string> text = MakeMonsterModelsFileText(file, models, issues);
    if (!text)
    {
        return false;
    }
    std::string error;
    if (!WriteTextFile(file, *text, error))
    {
        issues.push_back({Items::ItemDataIssueSeverity::Error, file.generic_string(), Items::ItemDataIssue::NoItem,
                          Items::ItemDataIssue::NoItem, "", error});
        return false;
    }
    return true;
}

std::optional<std::string> MakeMonsterModelsFileText(const std::filesystem::path& file,
                                                     std::span<const MonsterModelDefinition> models,
                                                     std::vector<Items::ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    std::string text = WriteMonsterModelsJson(models);
    std::vector<MonsterModelDefinition> readBack;
    std::vector<Items::ItemDataIssue> found;
    ReadMonsterModelsJson(text, source, readBack, found);
    ValidateMonsterModelNames(readBack, source, found);
    ValidateMonsterModelFiles(readBack, source, found);
    issues.insert(issues.end(), found.begin(), found.end());
    if (Items::HasErrors(found))
    {
        return std::nullopt;
    }
    return text;
}
} // namespace Data::Monsters
