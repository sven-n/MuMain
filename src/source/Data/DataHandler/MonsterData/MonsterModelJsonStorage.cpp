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
} // namespace Data::Monsters
