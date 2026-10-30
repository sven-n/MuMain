#include "stdafx.h"

#include "MonsterSoundJsonStorage.h"

#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MonsterData/MonsterSoundValidation.h"
#include "Data/GameData/MonsterData/MonsterSoundsJson.h"

namespace Data::Monsters
{
namespace
{
constexpr const char* MonsterSoundsFile = "Data/Monsters/Sounds.json";
} // namespace

std::filesystem::path GetMonsterSoundsFile()
{
    return MonsterSoundsFile;
}

MonsterSoundsLoadResult LoadMonsterSoundsFile(const std::filesystem::path& file)
{
    MonsterSoundsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    ReadMonsterSoundsJson(*text, source, result.sets, result.issues);
    ValidateMonsterSoundSetNames(result.sets, source, result.issues);
    return result;
}

std::optional<std::string> MakeMonsterSoundsFileText(const std::filesystem::path& file,
                                                     std::span<const MonsterSoundSet> sets,
                                                     std::vector<Items::ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    std::string text = WriteMonsterSoundsJson(sets);
    std::vector<MonsterSoundSet> readBack;
    std::vector<Items::ItemDataIssue> found;
    ReadMonsterSoundsJson(text, source, readBack, found);
    ValidateMonsterSoundSetNames(readBack, source, found);
    issues.insert(issues.end(), found.begin(), found.end());
    if (Items::HasFatal(found))
    {
        return std::nullopt;
    }
    return text;
}
} // namespace Data::Monsters
