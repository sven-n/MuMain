#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterEdits.h"

#include "MonsterSelection.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterModelsJson.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"

#include <algorithm>
#include <span>
#include <utility>

namespace MuEditor::Monsters
{
namespace
{
using Data::LocalizedString;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterModelSnapshot;
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

// Whether the model list's save would take the model: written and read back
// as the save does, with the checks of its names, without fatal problems.
bool CanStoreModel(const MonsterModelDefinition& model)
{
    const std::string text = Data::Monsters::WriteMonsterModelsJson(std::span<const MonsterModelDefinition>(&model, 1));
    std::vector<MonsterModelDefinition> readBack;
    std::vector<Data::Items::ItemDataIssue> issues;
    Data::Monsters::ReadMonsterModelsJson(text, "", readBack, issues);
    Data::Monsters::ValidateMonsterModelNames(readBack, "", issues);
    return !Data::Items::HasFatal(issues);
}
} // namespace

MonsterEdits& MonsterEdits::GetInstance()
{
    static MonsterEdits instance;
    return instance;
}

MonsterEdits::MonsterEdits()
    : m_history({[](int number) { return g_MonsterDataHandler.ReadMonster(number); },
                 [](std::span<const MonsterSnapshot> snapshots) { g_MonsterDataHandler.RestoreMonsters(snapshots); },
                 [](int slot) { return g_MonsterDataHandler.ReadModel(slot); },
                 [](std::span<const MonsterModelSnapshot> snapshots)
                 { g_MonsterDataHandler.RestoreModels(snapshots); }})
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

bool MonsterEdits::SetModel(std::string label, const MonsterModelDefinition& model, int monster)
{
    const std::optional<int> slot = Data::Monsters::FindMonsterModelSlot(model.slot);
    if (!slot || !g_MonsterDataHandler.ReadModel(*slot) || !CanStoreModel(model))
    {
        return false;
    }
    ChangeModel(std::move(label), *slot, model, monster);
    return true;
}

void MonsterEdits::ChangeModel(std::string label, int slot, const std::optional<MonsterModelDefinition>& after,
                               int monster)
{
    std::optional<MonsterModelDefinition> before = g_MonsterDataHandler.ReadModel(slot);
    if (after == before)
    {
        return;
    }
    const MonsterModelSnapshot snapshot{slot, after};
    g_MonsterDataHandler.RestoreModels(std::span<const MonsterModelSnapshot>(&snapshot, 1));
    // What the table stored, as the undo compares it.
    MonsterEditEntry entry{std::move(label), {}, {}};
    entry.models.push_back({slot, std::move(before), g_MonsterDataHandler.ReadModel(slot), monster});
    Record(std::move(entry));
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
    // One step per number and per slot: what changed twice keeps its first
    // "before" and its last "after".
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
    for (MonsterModelEditStep& step : entry.models)
    {
        const auto same = std::ranges::find(m_group->models, step.slot, &MonsterModelEditStep::slot);
        if (same == m_group->models.end())
        {
            m_group->models.push_back(std::move(step));
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

// Both files or neither: a save that stopped after the first would leave a
// monster list that names models the model list does not have.
bool MonsterEdits::Save(std::vector<Data::Items::ItemDataIssue>& issues)
{
    if (!g_MonsterDataHandler.SaveAll(issues))
    {
        return false;
    }
    m_history.MarkSaved();
    return true;
}

// Both files or neither: when one has fatal problems, the edits and their history
// stay.
bool MonsterEdits::Revert(std::vector<Data::Items::ItemDataIssue>& issues)
{
    if (!g_MonsterDataHandler.RevertAll(issues))
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
