#pragma once

#include "Data/GameData/ServerListData/ServerListGroup.h"

#include <string>
#include <string_view>

// The colors of the channel buttons on the login screen, from the list's button
// color rules (docs/server-list.md).
namespace Data::ServerList
{
// A button's color: a theme color, or a shade of the list's own ("#RRGGBB").
struct ButtonColor
{
    ThemeColor theme = ThemeColor::Default;
    // When set, it is the color; `theme` is Default then.
    std::string shade;

    bool operator==(const ButtonColor&) const = default;
};

// Whether the rule fits the channel.
bool RuleFits(const ButtonColorRule& rule, const ChannelSettings& channel);
// How specific a rule is: of the rules that fit a channel, the highest colors it.
int GetRuleSpecificity(const ButtonColorRule& rule);
// The rule that colors the channel's button, or nullptr when none fits.
const ButtonColorRule* FindButtonColorRule(const ServerListData& list, const ChannelSettings& channel);
// What a rule's color is in the theme folder `theme`; one the list does not
// have, or a shade the color has not for the theme, is the default color.
ButtonColor ResolveButtonColor(const ServerListData& list, std::string_view color, std::string_view theme);
// The channel's button color in the theme folder `theme`.
ButtonColor GetButtonColor(const ServerListData& list, const ChannelSettings& channel, std::string_view theme);

// "#RRGGBB".
bool IsShade(std::string_view text);
// A theme color's name or one of the list's colors.
bool IsKnownColor(const ServerListData& list, std::string_view color);
} // namespace Data::ServerList
