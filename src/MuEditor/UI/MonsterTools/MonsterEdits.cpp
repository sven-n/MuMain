#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterEdits.h"

#include "MonsterSelection.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/Common/LocalizedString.h"

#include <algorithm>
#include <span>
#include <utility>

namespace MuEditor::Monsters
{
namespace
{
using Data::LocalizedString;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterSnapshot;

// What the monster list's checks refuse to save: a name with the separator
// or a language code that is not one.
bool CanStoreNames(const LocalizedString& names)
{
    if (LocalizedString::ContainsSeparator(names.GetNeutral()))
    {
        return false;
    }
    return std::ranges::all_of(names.GetTranslations(),
                               [](const auto& translation)
                               {
                                   return LocalizedString::IsValidLocaleCode(translation.first) &&
                                          !LocalizedString::ContainsSeparator(translation.second);
                               });
}

bool IsMonsterNumber(int number)
{
    return number >= 0 && number < Data::Monsters::MonsterNumberCount;
}
} // namespace

MonsterEdits& MonsterEdits::GetInstance()
{
    static MonsterEdits instance;
    return instance;
}

MonsterEdits::MonsterEdits()
    : m_history({[](int number) { return g_MonsterDataHandler.ReadMonster(number); },
                 [](std::span<const MonsterSnapshot> snapshots) { g_MonsterDataHandler.RestoreMonsters(snapshots); }})
{
}

bool MonsterEdits::SetMonster(std::string label, const MonsterDefinition& monster)
{
    if (!g_MonsterDataHandler.ReadMonster(monster.number) || !CanStoreNames(monster.names))
    {
        return false;
    }
    Change(std::move(label), monster.number, monster);
    return true;
}

bool MonsterEdits::SetMonsterName(std::string label, int number, std::string_view locale, std::string text)
{
    std::optional<MonsterDefinition> monster = g_MonsterDataHandler.ReadMonster(number);
    const bool emptiesEnglish = locale == LocalizedString::NeutralLocale && text.empty();
    if (!monster || emptiesEnglish || !LocalizedString::IsValidLocaleCode(locale) ||
        LocalizedString::ContainsSeparator(text))
    {
        return false;
    }
    monster->names.Set(locale, std::move(text));
    Change(std::move(label), number, monster);
    return true;
}

bool MonsterEdits::AddMonster(std::string label, const MonsterDefinition& monster)
{
    if (!IsMonsterNumber(monster.number) || g_MonsterDataHandler.ReadMonster(monster.number) ||
        monster.names.GetNeutral().empty() || !CanStoreNames(monster.names))
    {
        return false;
    }
    Change(std::move(label), monster.number, monster);
    return true;
}

bool MonsterEdits::RemoveMonster(std::string label, int number)
{
    if (!g_MonsterDataHandler.ReadMonster(number))
    {
        return false;
    }
    Change(std::move(label), number, std::nullopt);
    return true;
}

void MonsterEdits::Change(std::string label, int number, const std::optional<MonsterDefinition>& after)
{
    std::optional<MonsterDefinition> before = g_MonsterDataHandler.ReadMonster(number);
    if (after == before)
    {
        return;
    }
    const MonsterSnapshot snapshot{number, after};
    g_MonsterDataHandler.RestoreMonsters(std::span<const MonsterSnapshot>(&snapshot, 1));
    // What the table stored, as the undo compares it.
    Record({std::move(label), {{number, std::move(before), g_MonsterDataHandler.ReadMonster(number)}}});
}

void MonsterEdits::BeginGroup(std::string label)
{
    if (!m_group)
    {
        m_group = MonsterEditEntry{std::move(label), {}};
    }
}

void MonsterEdits::EndGroup()
{
    if (m_group)
    {
        MonsterEditEntry entry = std::move(*m_group);
        m_group.reset();
        m_history.Record(std::move(entry));
    }
}

void MonsterEdits::Record(MonsterEditEntry entry)
{
    if (!m_group)
    {
        m_history.Record(std::move(entry));
        return;
    }
    // One step per number: what changed twice keeps its first "before" and its
    // last "after".
    for (MonsterEditStep& step : entry.steps)
    {
        const auto same = std::ranges::find(m_group->steps, step.number, &MonsterEditStep::number);
        if (same == m_group->steps.end())
        {
            m_group->steps.push_back(std::move(step));
        }
        else
        {
            same->after = std::move(step.after);
        }
    }
}

bool MonsterEdits::Undo()
{
    if (!m_history.Undo())
    {
        return false;
    }
    if (const std::optional<int> changed = m_history.GetLastChangedMonster())
    {
        g_MonsterSelection.Select(*changed);
    }
    return true;
}

bool MonsterEdits::Redo()
{
    if (!m_history.Redo())
    {
        return false;
    }
    if (const std::optional<int> changed = m_history.GetLastChangedMonster())
    {
        g_MonsterSelection.Select(*changed);
    }
    return true;
}

bool MonsterEdits::Save(std::vector<Data::Items::ItemDataIssue>& issues)
{
    if (!g_MonsterDataHandler.Save(issues))
    {
        return false;
    }
    m_history.MarkSaved();
    return true;
}

bool MonsterEdits::Revert(std::vector<Data::Items::ItemDataIssue>& issues)
{
    if (!g_MonsterDataHandler.Revert(issues))
    {
        return false;
    }
    OnDataReplaced();
    m_history.MarkSaved();
    return true;
}

void MonsterEdits::OnDataReplaced()
{
    m_history.Clear();
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
