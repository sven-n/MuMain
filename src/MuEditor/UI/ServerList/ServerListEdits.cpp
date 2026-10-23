#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListEdits.h"

#include "ServerListStepLabel.h"

#include <algorithm>
#include <utility>

namespace MuEditor::ServerList
{
void ServerListEdits::Reset(std::vector<ServerGroup> groups, bool complete)
{
    m_complete = complete;
    m_groups = std::move(groups);
    m_committed = m_groups;
    m_saved = m_groups;
    m_undo.clear();
    m_redo.clear();
    m_pendingLabel.clear();
}

void ServerListEdits::Commit()
{
    if (m_groups == m_committed)
    {
        m_pendingLabel.clear();
        return;
    }
    std::string label = GetPendingLabel();
    m_undo.push_back({std::move(m_committed), std::move(label)});
    if (m_undo.size() > MaxUndoSteps)
    {
        m_undo.erase(m_undo.begin());
    }
    m_redo.clear();
    m_committed = m_groups;
    m_pendingLabel.clear();
}

void ServerListEdits::Undo()
{
    Commit();
    if (m_undo.empty())
    {
        return;
    }
    Step step = std::move(m_undo.back());
    m_undo.pop_back();
    m_redo.push_back({std::move(m_groups), std::move(step.label)});
    m_groups = std::move(step.groups);
    m_committed = m_groups;
}

void ServerListEdits::Redo()
{
    Commit();
    if (m_redo.empty())
    {
        return;
    }
    Step step = std::move(m_redo.back());
    m_redo.pop_back();
    m_undo.push_back({std::move(m_groups), std::move(step.label)});
    m_groups = std::move(step.groups);
    m_committed = m_groups;
}

std::string ServerListEdits::GetUndoLabel() const
{
    if (m_groups != m_committed)
    {
        return GetPendingLabel();
    }
    return m_undo.empty() ? std::string() : m_undo.back().label;
}

std::string ServerListEdits::GetRedoLabel() const
{
    return m_redo.empty() ? std::string() : m_redo.back().label;
}

std::string ServerListEdits::GetPendingLabel() const
{
    return m_pendingLabel.empty() ? StepLabel::DescribeFieldChange(m_committed, m_groups) : m_pendingLabel;
}

std::optional<size_t> ServerListEdits::AddGroup(const std::string& name, std::optional<int> id)
{
    if (!id || IsIdUsed(*id))
    {
        id = FindFreeId();
    }
    if (!id)
    {
        return std::nullopt;
    }
    ServerGroup group;
    group.id = *id;
    group.name = name;
    m_groups.push_back(std::move(group));
    m_pendingLabel = StepLabel::Add(name);
    return m_groups.size() - 1;
}

void ServerListEdits::RemoveGroup(size_t index)
{
    if (index < m_groups.size())
    {
        m_pendingLabel = StepLabel::Remove(m_groups[index].name);
        m_groups.erase(m_groups.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

bool ServerListEdits::MoveGroup(size_t index, int direction)
{
    const long long target = static_cast<long long>(index) + direction;
    if (index >= m_groups.size() || target < 0 || target >= static_cast<long long>(m_groups.size()))
    {
        return false;
    }
    m_pendingLabel = StepLabel::Move(m_groups[index].name);
    std::swap(m_groups[index], m_groups[static_cast<size_t>(target)]);
    return true;
}

bool ServerListEdits::IsIdUsed(int id) const
{
    return std::any_of(m_groups.begin(), m_groups.end(), [id](const ServerGroup& group) { return group.id == id; });
}

std::optional<int> ServerListEdits::FindFreeId() const
{
    for (int id = 0; id <= Data::ServerList::MaxServerGroupId; ++id)
    {
        if (!IsIdUsed(id))
        {
            return id;
        }
    }
    return std::nullopt;
}
} // namespace MuEditor::ServerList

#endif // _EDITOR
