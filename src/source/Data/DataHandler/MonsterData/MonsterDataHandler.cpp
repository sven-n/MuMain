#include "stdafx.h"

#include "MonsterDataHandler.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"

#include <chrono>

namespace Data::Monsters
{
namespace
{
constexpr std::string_view MonsterDataLabel = "Monster data";
constexpr std::string_view MonsterDataName = "monster data";

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

// The problems from `first` on are problems of the monster list file.
void NameSource(std::vector<Items::ItemDataIssue>& issues, std::size_t first)
{
    const std::string source = GetMonstersFile().generic_string();
    for (std::size_t index = first; index < issues.size(); ++index)
    {
        issues[index].source = source;
    }
}

// The languages check of Load, on top of the checks of the file's reader.
void ValidateLocales(std::span<const MonsterDefinition> monsters, std::vector<Items::ItemDataIssue>& issues)
{
    const std::size_t first = issues.size();
    ValidateMonsterNameLocales(monsters, I18N::GetAvailableLocales(), issues);
    NameSource(issues, first);
}
#endif
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
    ValidateMonsterNameLocales(result.monsters, I18N::GetAvailableLocales(), result.issues);
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
        NotifyNamesChanged(snapshot.number);
    }
}

void MonsterDataHandler::Validate(std::vector<Items::ItemDataIssue>& issues) const
{
    const std::vector<MonsterDefinition> monsters = CollectMonsters();
    const std::size_t first = issues.size();
    ValidateMonsterNames(monsters, issues);
    NameSource(issues, first);
    ValidateLocales(monsters, issues);
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
