#pragma once

#include "Data/GameData/ServerListData/ServerListData.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>

// Builds a channel's name from a template of Data/Local/ServerList.json:
// "[{name}-{channel} ({vip} {pvp})]" is "[Valhalla-1 (Gold Non-PvP)]" (docs/server-list.md).
namespace Data::ServerList
{
// The key of a place's template in the file ("button").
const char* GetChannelNamePlaceKey(ChannelNamePlace place);
// The placeholders a place's template can use, without braces ("name", "channel", ...).
std::span<const std::string_view> GetChannelNamePlaceholders(ChannelNamePlace place);

// What a template's placeholders are filled with.
struct ChannelNameValues
{
    std::string_view name; // the group's
    int channel = 0;
    std::string_view vip;       // the VIP type's name; empty without one
    std::string_view pvp;       // the PvP or Non-PvP text
    std::string_view load;      // the list's load text ((Full), Overloaded); empty while joinable; Button only
    std::string_view character; // the player's character; Screenshot only
};

// Fills the template `text` for `place`. Each placeholder of the place gets its
// value; an empty value goes together with one space next to it (the one before,
// else the one after), and brackets, () or [], that are left with nothing in
// them go the same way. Anything else stays as written, a placeholder the place
// does not have too.
std::string FillChannelName(std::string_view text, ChannelNamePlace place, const ChannelNameValues& values);

// A channel's name at `place` in the language `locale`: the list's template,
// PvP text and VIP type name in it, or in English where it has none.
// `values.vip` and `values.pvp` come from `channel`; a VIP type the list does
// not have is none.
std::string BuildChannelName(const ServerListData& list, ChannelNamePlace place, const ChannelSettings& channel,
                             ChannelNameValues values, std::string_view locale);

// A channel's button text in three parts, so that its {load} text can look different.
struct ButtonTextParts
{
    std::string before;
    // `values.load`; empty when it is, or the template has no {load}.
    std::string load;
    std::string after;

    bool operator==(const ButtonTextParts&) const = default;
};

// The button's name as BuildChannelName builds it, split at its {load}. A second {load} in the
// template shows the text without a look of its own.
ButtonTextParts BuildButtonTextParts(const ServerListData& list, const ChannelSettings& channel,
                                     ChannelNameValues values, std::string_view locale);
} // namespace Data::ServerList
