#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListStepLabel.h"

#include "I18N/All.h"

#include <algorithm>

namespace MuEditor::ServerList::StepLabel
{
namespace
{
using Data::ServerList::ServerGroup;

// The first channel whose type differs, as its number; 0 when none does.
int FindChangedChannel(const ServerGroup& before, const ServerGroup& after)
{
    const auto changed = std::mismatch(before.channels.begin(), before.channels.end(), after.channels.begin());
    return changed.first == before.channels.end()
               ? 0
               : static_cast<int>(std::distance(before.channels.begin(), changed.first)) + 1;
}

std::string DescribeGroupChange(const ServerGroup& before, const ServerGroup& after)
{
    const std::string& name = before.name.empty() ? after.name : before.name;
    if (before.id != after.id)
    {
        return I18N::Format(I18N::Editor::StepID, {name});
    }
    if (before.name != after.name)
    {
        return I18N::Format(I18N::Editor::StepName, {name});
    }
    if (before.description != after.description)
    {
        return I18N::Format(I18N::Editor::StepDescription, {name});
    }
    if (before.testServer != after.testServer)
    {
        return I18N::Format(I18N::Editor::StepTestServer, {name});
    }
    return I18N::Format(I18N::Editor::StepChannel, {name, std::to_string(FindChangedChannel(before, after))});
}
} // namespace

std::string Add(const std::string& groupName)
{
    return I18N::Format(I18N::Editor::StepAdd, {groupName});
}

std::string Remove(const std::string& groupName)
{
    return I18N::Format(I18N::Editor::StepRemove, {groupName});
}

std::string Move(const std::string& groupName)
{
    return I18N::Format(I18N::Editor::StepMove, {groupName});
}

std::string DescribeFieldChange(std::span<const ServerGroup> before, std::span<const ServerGroup> after)
{
    const size_t count = std::min(before.size(), after.size());
    for (size_t index = 0; index < count; ++index)
    {
        if (before[index] != after[index])
        {
            return DescribeGroupChange(before[index], after[index]);
        }
    }
    return I18N::Editor::ServerList;
}
} // namespace MuEditor::ServerList::StepLabel

#endif // _EDITOR
