#pragma once

#include "Data/GameData/Common/LocalizedString.h"

#include <string>

namespace Data::ServerList
{
// A VIP type's name in UTF-16 units, as a server group's.
inline constexpr int MaxVipTypeNameLength = 32;

// A VIP type of channels (Gold, Silver, ...): its name in the channel names.
struct VipType
{
    // What a channel names the type by ("gold"; IsListId).
    std::string id;
    // {vip} in the templates.
    LocalizedString name;

    bool operator==(const VipType&) const = default;
};
} // namespace Data::ServerList
