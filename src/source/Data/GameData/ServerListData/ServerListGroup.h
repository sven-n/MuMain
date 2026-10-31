#pragma once

#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Data::ServerList
{
// A button color every theme (legacy, modern, ...) draws in a shade of its own.
enum class ThemeColor : std::uint8_t
{
    Default,
    Gray,
    Orange,
    Silver,
};

inline constexpr int ThemeColorCount = 4;

// A VIP type of channels (Gold, Silver, ...): its name in the channel names.
struct VipType
{
    // What a channel names the type by ("gold"): letters and digits, starting
    // with a lowercase letter.
    std::string id;
    // {vip} in the templates.
    LocalizedString name;

    bool operator==(const VipType&) const = default;
};

// What a channel is: PvP on or off, and a VIP type or none.
struct ChannelSettings
{
    bool pvp = true;
    // The id of a VipType; empty for none.
    std::string vip;

    bool operator==(const ChannelSettings&) const = default;
};

// A button color of the list's own, with a shade for each theme.
struct CustomColor
{
    // What rules name it by ("goldRed"): letters and digits, starting with a
    // lowercase letter, and none of the theme colors' names.
    std::string id;
    // What the editor calls it.
    std::string name;
    // "#RRGGBB" by theme folder ("legacy", "modern"); a theme without one draws its default color.
    std::map<std::string, std::string, std::less<>> shades;

    bool operator==(const CustomColor&) const = default;
};

// The VIP types a button color rule fits.
enum class RuleVip : std::uint8_t
{
    // Every channel.
    Any,
    // A channel with a VIP type, whichever.
    AnyVipType,
    // A channel without one.
    NoVipType,
    // A channel with the rule's VIP type.
    VipType,
};

// Colors the buttons of the channels it fits. Of the rules that fit a channel,
// the most specific one colors it (docs/server-list.md).
struct ButtonColorRule
{
    RuleVip vipMatch = RuleVip::Any;
    // The VIP type's id, with RuleVip::VipType.
    std::string vip;
    // PvP on or off; nothing for either.
    std::optional<bool> pvp;
    // A theme color's name ("orange") or a CustomColor's id.
    std::string color = "default";

    // Whether it fits the same channels as `other`: the same VIP type and PvP.
    bool FitsAsMuchAs(const ButtonColorRule& other) const
    {
        return vipMatch == other.vipMatch && (vipMatch != RuleVip::VipType || vip == other.vip) && pvp == other.pvp;
    }

    bool operator==(const ButtonColorRule&) const = default;
};

// The places that show a channel's name; each has a template of its own.
enum class ChannelNamePlace : std::uint8_t
{
    // The channel's button on the login screen.
    Button,
    // The login window's title.
    Login,
    // The character info window's title.
    CharacterInfo,
    // What the chat message about a saved screenshot ends with.
    Screenshot,
};

inline constexpr int ChannelNamePlaceCount = 4;

// The texts that name a channel, each with its translations.
struct ChannelNames
{
    // {pvp} on a channel with PvP on, and with PvP off.
    LocalizedString pvp;
    LocalizedString nonPvp;
    // By ChannelNamePlace.
    std::array<LocalizedString, ChannelNamePlaceCount> templates;

    LocalizedString& Template(ChannelNamePlace place)
    {
        return templates[static_cast<size_t>(place)];
    }
    const LocalizedString& Template(ChannelNamePlace place) const
    {
        return templates[static_cast<size_t>(place)];
    }

    bool operator==(const ChannelNames&) const = default;
};

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
// A VIP type's name, as a group's.
inline constexpr int MaxVipTypeNameLength = MaxServerGroupNameLength;
// A template, or the PvP and Non-PvP text, in UTF-16 units.
inline constexpr int MaxChannelNameTextLength = MAX_TEXT_LENGTH - 1;

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

// The built-in English texts: what the texts a file leaves out are.
ChannelNames GetDefaultChannelNames();

// A VIP type id: letters and digits, starting with a lowercase letter ("gold", "goldPlus").
bool IsVipTypeId(std::string_view id);
// An id made from a name: "Gold Plus" is "goldPlus"; "vip" when the name has no letter to start with.
std::string MakeVipTypeId(std::string_view name);

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
