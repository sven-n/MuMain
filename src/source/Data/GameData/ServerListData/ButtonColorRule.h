#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace Data::ServerList
{
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
} // namespace Data::ServerList
