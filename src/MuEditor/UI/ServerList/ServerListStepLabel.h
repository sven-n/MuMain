#pragma once

#ifdef _EDITOR

#include "Data/GameData/ServerListData/ServerListGroup.h"

#include <span>
#include <string>

// What an undo step of the Server List window did, for the editor bar's
// tooltips: "Remove Helheim", "Valhalla: channel 3".
namespace MuEditor::ServerList::StepLabel
{
std::string Add(const std::string& groupName);
std::string Remove(const std::string& groupName);
std::string Move(const std::string& groupName);
// A change of one group's fields, by the first group and field that differ;
// the group is named as it was before.
std::string DescribeFieldChange(std::span<const Data::ServerList::ServerGroup> before,
                                std::span<const Data::ServerList::ServerGroup> after);
} // namespace MuEditor::ServerList::StepLabel

#endif // _EDITOR
