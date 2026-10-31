#pragma once

#include "Core/Globals/_define.h"
#include "Data/GameData/ServerListData/ChannelSettings.h"

#include <array>
#include <string>

namespace Data::ServerList
{
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
    std::array<ChannelSettings, MaxChannelsPerGroup> channels{};

    bool operator==(const ServerGroup&) const = default;
};
} // namespace Data::ServerList
