#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListStepLabel.h"

#include "I18N/All.h"

#include <algorithm>

namespace MuEditor::ServerList::StepLabel
{
namespace
{
using Data::ServerList::ChannelNamePlace;
using Data::ServerList::ServerGroup;
using Data::ServerList::VipType;

// The first channel whose settings differ, as its number; 0 when none does.
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

std::string DescribeVipTypeChange(const VipType& before, const VipType& after)
{
    const std::string& name = before.name.GetNeutral().empty() ? after.name.GetNeutral() : before.name.GetNeutral();
    if (before.id != after.id)
    {
        return I18N::Format(I18N::Editor::StepID, {name});
    }
    if (before.name != after.name)
    {
        return I18N::Format(I18N::Editor::StepName, {name});
    }
    return I18N::Format(I18N::Editor::StepThemeColor, {name});
}

// The first text that differs, or nothing.
std::string DescribeNamesChange(const Data::ServerList::ChannelNames& before,
                                const Data::ServerList::ChannelNames& after)
{
    if (before.pvp != after.pvp)
    {
        return I18N::Format(I18N::Editor::StepText, {I18N::Editor::PvPText});
    }
    if (before.nonPvp != after.nonPvp)
    {
        return I18N::Format(I18N::Editor::StepText, {I18N::Editor::NonPvPText});
    }
    for (int value = 0; value < Data::ServerList::ChannelNamePlaceCount; ++value)
    {
        const ChannelNamePlace place = static_cast<ChannelNamePlace>(value);
        if (before.Template(place) != after.Template(place))
        {
            return I18N::Format(I18N::Editor::StepText, {GetPlaceLabel(place)});
        }
    }
    return {};
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

std::string AddVipType(const std::string& name)
{
    return I18N::Format(I18N::Editor::StepAddVIPType, {name});
}

std::string RemoveVipType(const std::string& name)
{
    return I18N::Format(I18N::Editor::StepRemoveVIPType, {name});
}

std::string MoveVipType(const std::string& name)
{
    return I18N::Format(I18N::Editor::StepMoveVIPType, {name});
}

std::string DescribeFieldChange(const Data::ServerList::ServerListData& before,
                                const Data::ServerList::ServerListData& after)
{
    const size_t groups = std::min(before.groups.size(), after.groups.size());
    for (size_t index = 0; index < groups; ++index)
    {
        if (before.groups[index] != after.groups[index])
        {
            return DescribeGroupChange(before.groups[index], after.groups[index]);
        }
    }
    const size_t vipTypes = std::min(before.vipTypes.size(), after.vipTypes.size());
    for (size_t index = 0; index < vipTypes; ++index)
    {
        if (before.vipTypes[index] != after.vipTypes[index])
        {
            return DescribeVipTypeChange(before.vipTypes[index], after.vipTypes[index]);
        }
    }
    std::string names = DescribeNamesChange(before.names, after.names);
    return names.empty() ? std::string(I18N::Editor::ServerList) : names;
}

const char* GetPlaceLabel(ChannelNamePlace place)
{
    switch (place)
    {
    case ChannelNamePlace::Button:
        return I18N::Editor::ServerListButton;
    case ChannelNamePlace::Login:
        return I18N::Editor::LoginWindow;
    case ChannelNamePlace::CharacterInfo:
        return I18N::Editor::CharacterInfo;
    case ChannelNamePlace::Screenshot:
    default:
        return I18N::Editor::ScreenshotMessage;
    }
}
} // namespace MuEditor::ServerList::StepLabel

#endif // _EDITOR
