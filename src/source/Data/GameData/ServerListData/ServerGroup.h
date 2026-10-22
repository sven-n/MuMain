#pragma once

#include "Core/Globals/_define.h"

#include <array>
#include <cstdint>
#include <string>

namespace Data::ServerList
{
// What the login screen writes after a channel's number, and its color there.
// The values are the ones the client always had: bit 0 is "no PvP", bit 1 "gold".
enum class ChannelType : std::uint8_t
{
    Normal = 0,  // "Valhalla-1", gray
    NonPvp = 1,  // "Valhalla-1(Non-PVP)"
    GoldPvp = 2, // "Valhalla-1(Gold PVP)", orange
    Gold = 3,    // "Valhalla-1(Gold)", orange
};

inline constexpr int ChannelTypeCount = 4;

// What the login screen writes after "<group name>-<channel>" for the type.
constexpr const char* GetChannelSuffix(ChannelType type)
{
    switch (type)
    {
    case ChannelType::NonPvp:
        return "(Non-PVP)";
    case ChannelType::GoldPvp:
        return "(Gold PVP)";
    case ChannelType::Gold:
        return "(Gold)";
    case ChannelType::Normal:
    default:
        return "";
    }
}

// A server sends its ID; the group is ID / MAX_SERVER_PER_GROUP and the
// channel ID % MAX_SERVER_PER_GROUP + 1.
inline constexpr int MaxChannelsPerGroup = MAX_SERVER_PER_GROUP;
// Server IDs are 16 bit.
inline constexpr int MaxServerGroupId = 0xFFFF / MAX_SERVER_PER_GROUP;
// In UTF-16 units, as the client's wide strings hold them on Windows (an emoji
// takes two): the name goes into fixed wide buffers, e.g. the screenshot
// message's 64 with the character name. 32 is also the width of the group
// buttons on the login screen.
inline constexpr int MaxServerGroupNameLength = 32;
inline constexpr int MaxServerGroupDescriptionLength = MAX_TEXT_LENGTH - 1;

// One group of game servers on the login screen: its button, and the name
// its channels get.
struct ServerGroup
{
    int id = 0;
    std::string name;        // UTF-8
    std::string description; // UTF-8, shown below the channels of the selected group
    // A test server group: the login screen shows only the first one, at its
    // place in the list.
    bool testServer = false;
    // Index: channel number - 1.
    std::array<ChannelType, MaxChannelsPerGroup> channels{};

    bool operator==(const ServerGroup&) const = default;
};
} // namespace Data::ServerList
