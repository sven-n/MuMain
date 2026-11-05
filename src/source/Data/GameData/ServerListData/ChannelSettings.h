#pragma once

#include <string>

namespace Data::ServerList
{
// What a channel is: PvP on or off, and a VIP type or none.
struct ChannelSettings
{
    bool pvp = true;
    // The id of a VipType; empty for none.
    std::string vip;

    bool operator==(const ChannelSettings&) const = default;
};
} // namespace Data::ServerList
