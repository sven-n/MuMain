#pragma once

#ifdef _EDITOR

#include "Data/GameData/ServerListData/ServerListData.h"

#include <string>

// What an undo step of the Server List window did, for the editor bar's
// tooltips: "Remove Helheim", "Valhalla: channel 3", "Text: Login window".
namespace MuEditor::ServerList::StepLabel
{
std::string Add(const std::string& groupName);
std::string Remove(const std::string& groupName);
std::string Move(const std::string& groupName);
std::string AddVipType(const std::string& name);
std::string RemoveVipType(const std::string& name);
std::string MoveVipType(const std::string& name);
std::string AddColor(const std::string& name);
std::string RemoveColor(const std::string& name);
std::string AddButtonColorRule();
std::string RemoveButtonColorRule();
// A change of fields, by the first group, VIP type or text that differs; a
// group or VIP type is named as it was before.
std::string DescribeFieldChange(const Data::ServerList::ServerListData& before,
                                const Data::ServerList::ServerListData& after);

// What the window calls a place's template ("Login window").
const char* GetPlaceLabel(Data::ServerList::ChannelNamePlace place);
} // namespace MuEditor::ServerList::StepLabel

#endif // _EDITOR
