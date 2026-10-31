#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListEdits.h"

#include "ServerListStepLabel.h"

#include <algorithm>
#include <utility>

namespace MuEditor::ServerList
{
namespace
{
// Moves the entry one place up (-1) or down (+1); false at the end of the list.
template <typename TEntry> bool MoveEntry(std::vector<TEntry>& entries, size_t index, int direction)
{
    const long long target = static_cast<long long>(index) + direction;
    if (index >= entries.size() || target < 0 || target >= static_cast<long long>(entries.size()))
    {
        return false;
    }
    std::swap(entries[index], entries[static_cast<size_t>(target)]);
    return true;
}
} // namespace

void ServerListEdits::Reset(ServerListData list, bool complete)
{
    m_complete = complete;
    m_list = std::move(list);
    m_committed = m_list;
    m_saved = m_list;
    m_undo.clear();
    m_redo.clear();
    m_pendingLabel.clear();
}

void ServerListEdits::Commit()
{
    if (m_list == m_committed)
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
    m_committed = m_list;
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
    m_redo.push_back({std::move(m_list), std::move(step.label)});
    m_list = std::move(step.list);
    m_committed = m_list;
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
    m_undo.push_back({std::move(m_list), std::move(step.label)});
    m_list = std::move(step.list);
    m_committed = m_list;
}

std::string ServerListEdits::GetUndoLabel() const
{
    if (m_list != m_committed)
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
    return m_pendingLabel.empty() ? StepLabel::DescribeFieldChange(m_committed, m_list) : m_pendingLabel;
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
    m_list.groups.push_back(std::move(group));
    m_pendingLabel = StepLabel::Add(name);
    return m_list.groups.size() - 1;
}

void ServerListEdits::RemoveGroup(size_t index)
{
    if (index < m_list.groups.size())
    {
        m_pendingLabel = StepLabel::Remove(m_list.groups[index].name);
        m_list.groups.erase(m_list.groups.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

bool ServerListEdits::MoveGroup(size_t index, int direction)
{
    if (index >= m_list.groups.size())
    {
        return false;
    }
    const std::string name = m_list.groups[index].name;
    if (!MoveEntry(m_list.groups, index, direction))
    {
        return false;
    }
    m_pendingLabel = StepLabel::Move(name);
    return true;
}

bool ServerListEdits::IsIdUsed(int id) const
{
    return std::any_of(m_list.groups.begin(), m_list.groups.end(),
                       [id](const ServerGroup& group) { return group.id == id; });
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

size_t ServerListEdits::AddVipType(const std::string& name)
{
    const std::string base = Data::ServerList::MakeVipTypeId(name);
    std::string id = base;
    for (int number = 2; IsVipTypeIdUsed(id); ++number)
    {
        id = base + std::to_string(number);
    }
    VipType vipType;
    vipType.id = std::move(id);
    vipType.name.Set(Data::LocalizedString::NeutralLocale, name);
    m_list.vipTypes.push_back(std::move(vipType));
    m_pendingLabel = StepLabel::AddVipType(name);
    return m_list.vipTypes.size() - 1;
}

void ServerListEdits::RemoveVipType(size_t index)
{
    if (index >= m_list.vipTypes.size())
    {
        return;
    }
    const VipType& vipType = m_list.vipTypes[index];
    m_pendingLabel = StepLabel::RemoveVipType(vipType.name.GetNeutral());
    for (ServerGroup& group : m_list.groups)
    {
        for (Data::ServerList::ChannelSettings& channel : group.channels)
        {
            if (channel.vip == vipType.id)
            {
                channel.vip.clear();
            }
        }
    }
    m_list.vipTypes.erase(m_list.vipTypes.begin() + static_cast<std::ptrdiff_t>(index));
}

bool ServerListEdits::MoveVipType(size_t index, int direction)
{
    if (index >= m_list.vipTypes.size())
    {
        return false;
    }
    const std::string name = m_list.vipTypes[index].name.GetNeutral();
    if (!MoveEntry(m_list.vipTypes, index, direction))
    {
        return false;
    }
    m_pendingLabel = StepLabel::MoveVipType(name);
    return true;
}

bool ServerListEdits::ChangeVipTypeId(size_t index, const std::string& id)
{
    if (index >= m_list.vipTypes.size() || !Data::ServerList::IsVipTypeId(id))
    {
        return false;
    }
    VipType& vipType = m_list.vipTypes[index];
    if (id == vipType.id)
    {
        return true;
    }
    if (IsVipTypeIdUsed(id))
    {
        return false;
    }
    for (ServerGroup& group : m_list.groups)
    {
        for (Data::ServerList::ChannelSettings& channel : group.channels)
        {
            if (channel.vip == vipType.id)
            {
                channel.vip = id;
            }
        }
    }
    vipType.id = id;
    return true;
}

bool ServerListEdits::IsVipTypeIdUsed(std::string_view id) const
{
    return m_list.FindVipType(id) != nullptr;
}

std::vector<std::string> ServerListEdits::FindChannelsWithVipType(std::string_view id) const
{
    std::vector<std::string> channels;
    for (const ServerGroup& group : m_list.groups)
    {
        for (size_t index = 0; index < group.channels.size(); ++index)
        {
            if (group.channels[index].vip == id)
            {
                channels.push_back(group.name + "-" + std::to_string(index + 1));
            }
        }
    }
    return channels;
}
} // namespace MuEditor::ServerList

#endif // _EDITOR
