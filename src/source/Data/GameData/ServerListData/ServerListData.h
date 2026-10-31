#pragma once

#include "Data/GameData/ServerListData/ButtonColorRule.h"
#include "Data/GameData/ServerListData/ChannelNames.h"
#include "Data/GameData/ServerListData/CustomColor.h"
#include "Data/GameData/ServerListData/ServerListGroup.h"
#include "Data/GameData/ServerListData/VipType.h"

#include <string_view>
#include <vector>

namespace Data::ServerList
{
// All that Data/Local/ServerList.json holds.
struct ServerListData
{
    // In the order the editor lists them.
    std::vector<VipType> vipTypes;
    // In the order the editor lists them.
    std::vector<CustomColor> colors;
    // Their order does not matter.
    std::vector<ButtonColorRule> buttonColors;
    ChannelNames names = GetDefaultChannelNames();
    // In the order of their buttons.
    std::vector<ServerGroup> groups;

    // The VIP type with the id, or nullptr.
    const VipType* FindVipType(std::string_view id) const;
    // The color of the list's own with the id, or nullptr.
    const CustomColor* FindCustomColor(std::string_view id) const;

    bool operator==(const ServerListData&) const = default;
};
} // namespace Data::ServerList
