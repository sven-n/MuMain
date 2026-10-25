#include "stdafx.h"

#include "MonsterDataHandler.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"
#include "I18N/All.h"

#include <chrono>

namespace Data::Monsters
{
namespace
{
constexpr std::string_view MonsterDataLabel = "Monster data";
constexpr std::string_view MonsterDataName = "monster data";
constexpr std::string_view MonsterModelsLabel = "Monster models";
constexpr std::string_view MonsterModelsName = "monster model list";

#ifdef _EDITOR
// The entries of the table, sorted by number.
std::vector<MonsterDefinition> CollectMonsters()
{
    std::vector<MonsterDefinition> monsters;
    for (int number = 0; number < MonsterNumberCount; ++number)
    {
        if (const MonsterDefinition* monster = g_MonsterDatabase.Find(number))
        {
            monsters.push_back(*monster);
        }
    }
    return monsters;
}
#endif

// The languages check, on top of the checks of the file's reader: names in
// languages the game has no texts for, as problems of the monster list file.
void ValidateLocales(std::span<const MonsterDefinition> monsters, std::vector<Items::ItemDataIssue>& issues)
{
    const std::size_t first = issues.size();
    ValidateMonsterNameLocales(monsters, I18N::GetAvailableLocales(), issues);
    NameIssueSource(issues, first, GetMonstersFile());
}
} // namespace

MonsterDataHandler& MonsterDataHandler::GetInstance()
{
    static MonsterDataHandler instance;
    return instance;
}

bool MonsterDataHandler::Load(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    MonsterDataLoadResult result = LoadMonstersFile(GetMonstersFile());
    ValidateLocales(result.monsters, result.issues);
    LogDataIssues(MonsterDataLabel, result.issues);
    if (Items::HasErrors(result.issues))
    {
        errorMessage = DescribeDataErrors(MonsterDataName, GetMonstersFile(), result.issues);
        return false;
    }

    g_MonsterDatabase.Build(result.monsters, I18N::GetCurrentLocale());
    RegisterLocaleObserver();
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} monsters from {} in {:.1f} ms", result.monsters.size(),
                GetMonstersFile().generic_string(), elapsed.count());
    return true;
}

bool MonsterDataHandler::LoadModels(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    const MonsterModelsLoadResult result = LoadMonsterModelsFile(GetMonsterModelsFile());
    LogDataIssues(MonsterModelsLabel, result.issues);
    if (Items::HasErrors(result.issues))
    {
        errorMessage = DescribeDataErrors(MonsterModelsName, GetMonsterModelsFile(), result.issues);
        return false;
    }

    g_MonsterModelDatabase.Build(result.models);
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} monster models from {} in {:.1f} ms", result.models.size(),
                GetMonsterModelsFile().generic_string(), elapsed.count());
    return true;
}

void MonsterDataHandler::RegisterLocaleObserver()
{
    if (m_localeObserverRegistered)
    {
        return;
    }
    I18N::RegisterLocaleObserver(&MonsterDataHandler::OnLocaleChanged, this);
    m_localeObserverRegistered = true;
}

void MonsterDataHandler::OnLocaleChanged(void* context) noexcept
{
    auto* handler = static_cast<MonsterDataHandler*>(context);
    g_MonsterDatabase.SetDisplayLocale(I18N::GetCurrentLocale());
    if (handler->m_namesChanged != nullptr)
    {
        handler->m_namesChanged(AllNumbers);
    }
}

#ifdef _EDITOR
std::optional<MonsterDefinition> MonsterDataHandler::ReadMonster(int number) const
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    return monster != nullptr ? std::optional<MonsterDefinition>(*monster) : std::nullopt;
}

void MonsterDataHandler::RestoreMonsters(std::span<const MonsterSnapshot> snapshots)
{
    // The callback is told once: the number restored, or AllNumbers when more
    // were. The game walks every character for each call.
    std::optional<int> changed;
    for (const MonsterSnapshot& snapshot : snapshots)
    {
        if (snapshot.number < 0 || snapshot.number >= MonsterNumberCount)
        {
            continue;
        }
        if (snapshot.monster)
        {
            MonsterDefinition monster = *snapshot.monster;
            monster.number = snapshot.number;
            g_MonsterDatabase.Set(monster);
        }
        else
        {
            g_MonsterDatabase.Remove(snapshot.number);
        }
        const bool sameNumber = !changed || *changed == snapshot.number;
        changed = sameNumber ? snapshot.number : AllNumbers;
    }
    if (changed)
    {
        NotifyNamesChanged(*changed);
    }
}

void MonsterDataHandler::Validate(std::vector<Items::ItemDataIssue>& issues) const
{
    const std::vector<MonsterDefinition> monsters = CollectMonsters();
    const std::size_t first = issues.size();
    ValidateMonsterNames(monsters, issues);
    NameIssueSource(issues, first, GetMonstersFile());
    ValidateLocales(monsters, issues);
}

void MonsterDataHandler::ValidateModels(std::vector<Items::ItemDataIssue>& issues) const
{
    std::vector<MonsterModelDefinition> models;
    for (const MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
    {
        models.push_back(*model);
    }
    const std::string source = GetMonsterModelsFile().generic_string();
    ValidateMonsterModelNames(models, source, issues);
    ValidateMonsterModelFiles(models, source, issues);
}

bool MonsterDataHandler::Save(std::vector<Items::ItemDataIssue>& issues)
{
    const std::vector<MonsterDefinition> monsters = CollectMonsters();
    ValidateLocales(monsters, issues);
    return SaveMonstersFile(GetMonstersFile(), monsters, issues);
}

bool MonsterDataHandler::Revert(std::vector<Items::ItemDataIssue>& issues)
{
    MonsterDataLoadResult result = LoadMonstersFile(GetMonstersFile());
    ValidateLocales(result.monsters, result.issues);
    issues.insert(issues.end(), result.issues.begin(), result.issues.end());
    if (Items::HasErrors(result.issues))
    {
        return false;
    }

    // The data changes, not the language the names are shown in.
    const std::string displayLocale = g_MonsterDatabase.GetDisplayLocale();
    g_MonsterDatabase.Build(result.monsters, displayLocale);
    RegisterLocaleObserver();
    NotifyNamesChanged(AllNumbers);
    return true;
}

void MonsterDataHandler::NotifyNamesChanged(int number) const
{
    if (m_namesChanged != nullptr)
    {
        m_namesChanged(number);
    }
}
#endif
} // namespace Data::Monsters
