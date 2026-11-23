#include "stdafx.h"

#include "MasterSkillStorage.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Data/GameData/SkillData/MasterSkillDatabase.h"
#include "Data/GameData/SkillData/MasterSkillsJson.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Skills/SkillManager.h"

#include <algorithm>

namespace Data::Skills
{
namespace
{
constexpr const char* MasterSkillsFile = "Data/Skills/MasterSkills.json";
constexpr std::string_view MasterSkillsLabel = "MasterSkills";
constexpr std::string_view MasterSkillsName = "master skills";
} // namespace

std::filesystem::path GetMasterSkillsFile()
{
    return MasterSkillsFile;
}

MasterSkillsLoadResult LoadMasterSkillsFile(const std::filesystem::path& file)
{
    MasterSkillsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    std::vector<size_t> entries;
    ReadMasterSkillsJson(*text, source, result.skills, result.issues, &entries);
    ValidateMasterSkills(result.skills, entries, source, result.issues);
    return result;
}

int FindSkillDataBuff(int skill)
{
    if (SkillAttribute == nullptr)
    {
        return 0;
    }
    auto at = static_cast<ActionSkillType>(skill);
    // A master skill and its base skills; the replacement table has no loops, the bound only guards against one.
    for (int step = 0; step < MAX_SKILLS && at >= 0 && at < MAX_SKILLS; ++step)
    {
        if (SkillAttribute[at].Effect != 0)
        {
            return SkillAttribute[at].Effect;
        }
        const auto next = SKILL_REPLACEMENTS.find(at);
        if (next == SKILL_REPLACEMENTS.end())
        {
            break;
        }
        at = next->second;
    }
    return 0;
}

int FindMasterSkillBuff(int skill, std::span<const MasterSkillDefinition> skills)
{
    auto at = static_cast<ActionSkillType>(skill);
    for (int step = 0; step < MAX_SKILLS && at >= 0 && at < MAX_SKILLS; ++step)
    {
        const auto master = std::find_if(skills.begin(), skills.end(),
                                         [at](const MasterSkillDefinition& entry) { return entry.skill == at; });
        if (master != skills.end() && master->castsBuff != 0)
        {
            return master->castsBuff;
        }
        const auto next = SKILL_REPLACEMENTS.find(at);
        if (next == SKILL_REPLACEMENTS.end())
        {
            break;
        }
        at = next->second;
    }
    return FindSkillDataBuff(skill);
}

bool LoadMasterSkills(std::string& errorMessage)
{
    MasterSkillsLoadResult result = LoadMasterSkillsFile(GetMasterSkillsFile());
    if (!Items::HasFatal(result.issues))
    {
        ValidateMasterSkillBuffs(
            result.skills, {}, g_BuffDatabase.GetAll(),
            [&](int skill) { return FindMasterSkillBuff(skill, result.skills); },
            GetMasterSkillsFile().generic_string(), result.issues);
    }
    LogDataIssues(MasterSkillsLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(MasterSkillsName, GetMasterSkillsFile(), result.issues);
        return false;
    }
    g_MasterSkillDatabase.Build(result.skills);
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} master skills from {}", result.skills.size(),
                GetMasterSkillsFile().generic_string());
    return true;
}
} // namespace Data::Skills
