#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterEditHistory.h"

#include <algorithm>
#include <ranges>
#include <utility>

namespace MuEditor::Monsters
{
namespace
{
const std::string NoLabel;
} // namespace

MonsterEditHistory::MonsterEditHistory(Hooks hooks) : m_hooks(std::move(hooks)) {}

void MonsterEditHistory::Record(MonsterEditEntry entry)
{
    if (entry.steps.empty() && entry.models.empty())
    {
        return;
    }
    m_entries.erase(m_entries.begin() + static_cast<std::ptrdiff_t>(m_done), m_entries.end());
    if (m_saved && *m_saved > m_done)
    {
        m_saved.reset();
    }
    m_entries.push_back(std::move(entry));
    m_done = m_entries.size();
    if (m_entries.size() > MaxEntries)
    {
        m_entries.erase(m_entries.begin());
        --m_done;
        if (m_saved)
        {
            m_saved = *m_saved > 0 ? std::optional<std::size_t>(*m_saved - 1) : std::nullopt;
        }
    }
}

bool MonsterEditHistory::CanUndo() const
{
    return m_done > 0;
}

bool MonsterEditHistory::CanRedo() const
{
    return m_done < m_entries.size();
}

const std::string& MonsterEditHistory::GetUndoLabel() const
{
    return CanUndo() ? m_entries[m_done - 1].label : NoLabel;
}

const std::string& MonsterEditHistory::GetRedoLabel() const
{
    return CanRedo() ? m_entries[m_done].label : NoLabel;
}

bool MonsterEditHistory::Undo()
{
    if (!CanUndo())
    {
        m_lastRefusal = Refusal::NothingToDo;
        return false;
    }
    if (!TryApply(m_entries[m_done - 1], true))
    {
        return false;
    }
    --m_done;
    return true;
}

bool MonsterEditHistory::Redo()
{
    if (!CanRedo())
    {
        m_lastRefusal = Refusal::NothingToDo;
        return false;
    }
    if (!TryApply(m_entries[m_done], false))
    {
        return false;
    }
    ++m_done;
    return true;
}

bool MonsterEditHistory::TryApply(const MonsterEditEntry& entry, bool undoing)
{
    if (!IsAsLeft(entry, undoing))
    {
        m_lastRefusal = Refusal::MonstersChanged;
        return false;
    }
    m_lastRefusal = Refusal::None;
    Apply(entry, undoing);
    return true;
}

void MonsterEditHistory::Clear()
{
    m_entries.clear();
    m_done = 0;
    m_saved.reset();
    m_lastChangedMonster.reset();
}

void MonsterEditHistory::MarkSaved()
{
    m_saved = m_done;
}

bool MonsterEditHistory::IsModifiedSinceSave() const
{
    return m_saved != m_done;
}

// Undoing: the monsters and models are as the entry's steps left them.
// Redoing: as they were before them.
bool MonsterEditHistory::IsAsLeft(const MonsterEditEntry& entry, bool undoing) const
{
    const bool monstersAsLeft =
        std::ranges::all_of(entry.steps, [&](const MonsterEditStep& step)
                            { return m_hooks.readMonster(step.number) == (undoing ? step.after : step.before); });
    return monstersAsLeft &&
           std::ranges::all_of(entry.models, [&](const MonsterModelEditStep& step)
                               { return m_hooks.readModel(step.slot) == (undoing ? step.after : step.before); });
}

// Undoing puts the steps back last first.
void MonsterEditHistory::Apply(const MonsterEditEntry& entry, bool undoing)
{
    if (!entry.steps.empty())
    {
        std::vector<Data::Monsters::MonsterSnapshot> snapshots;
        snapshots.reserve(entry.steps.size());
        if (undoing)
        {
            for (const MonsterEditStep& step : std::views::reverse(entry.steps))
            {
                snapshots.push_back({step.number, step.before});
            }
        }
        else
        {
            for (const MonsterEditStep& step : entry.steps)
            {
                snapshots.push_back({step.number, step.after});
            }
        }
        m_hooks.restoreMonsters(snapshots);
    }
    if (!entry.models.empty())
    {
        std::vector<Data::Monsters::MonsterModelSnapshot> snapshots;
        snapshots.reserve(entry.models.size());
        if (undoing)
        {
            for (const MonsterModelEditStep& step : std::views::reverse(entry.models))
            {
                snapshots.push_back({step.slot, step.before});
            }
        }
        else
        {
            for (const MonsterModelEditStep& step : entry.models)
            {
                snapshots.push_back({step.slot, step.after});
            }
        }
        m_hooks.restoreModels(snapshots);
    }
    if (!entry.steps.empty())
    {
        m_lastChangedMonster = entry.steps.front().number;
    }
    else if (entry.models.front().monster >= 0)
    {
        m_lastChangedMonster = entry.models.front().monster;
    }
    else
    {
        m_lastChangedMonster.reset();
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
