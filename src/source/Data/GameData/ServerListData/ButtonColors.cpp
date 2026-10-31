#include "stdafx.h"

#include "ButtonColors.h"

#include <algorithm>

namespace Data::ServerList
{
bool RuleFits(const ButtonColorRule& rule, const ChannelSettings& channel)
{
    if (rule.pvp && *rule.pvp != channel.pvp)
    {
        return false;
    }
    switch (rule.vipMatch)
    {
    case RuleVip::AnyVipType:
        return !channel.vip.empty();
    case RuleVip::NoVipType:
        return channel.vip.empty();
    case RuleVip::VipType:
        return channel.vip == rule.vip;
    case RuleVip::Any:
    default:
        return true;
    }
}

int GetRuleSpecificity(const ButtonColorRule& rule)
{
    // The VIP type weighs more than PvP: any rule that names one beats a rule for PvP only.
    int vip = 0;
    switch (rule.vipMatch)
    {
    case RuleVip::VipType:
        vip = 2;
        break;
    case RuleVip::AnyVipType:
    case RuleVip::NoVipType:
        vip = 1;
        break;
    case RuleVip::Any:
    default:
        vip = 0;
        break;
    }
    return vip * 2 + (rule.pvp ? 1 : 0);
}

const ButtonColorRule* FindButtonColorRule(const ServerListData& list, const ChannelSettings& channel)
{
    const ButtonColorRule* best = nullptr;
    for (const ButtonColorRule& rule : list.buttonColors)
    {
        // Two rules that fit the same channels are refused, so the most specific one is the only one.
        if (RuleFits(rule, channel) && (best == nullptr || GetRuleSpecificity(rule) > GetRuleSpecificity(*best)))
        {
            best = &rule;
        }
    }
    return best;
}

ButtonColor ResolveButtonColor(const ServerListData& list, std::string_view color, std::string_view theme)
{
    ButtonColor resolved;
    if (const std::optional<ThemeColor> themeColor = FindThemeColor(color))
    {
        resolved.theme = *themeColor;
        return resolved;
    }
    if (const CustomColor* custom = list.FindCustomColor(color))
    {
        const auto shade = custom->shades.find(theme);
        if (shade != custom->shades.end() && IsShade(shade->second))
        {
            resolved.shade = shade->second;
        }
    }
    return resolved;
}

ButtonColor GetButtonColor(const ServerListData& list, const ChannelSettings& channel, std::string_view theme)
{
    const ButtonColorRule* rule = FindButtonColorRule(list, channel);
    return rule != nullptr ? ResolveButtonColor(list, rule->color, theme) : ButtonColor{};
}

bool IsShade(std::string_view text)
{
    const auto isHexDigit = [](char character)
    {
        return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f') ||
               (character >= 'A' && character <= 'F');
    };
    return text.size() == 7 && text.front() == '#' && std::all_of(text.begin() + 1, text.end(), isHexDigit);
}

bool IsKnownColor(const ServerListData& list, std::string_view color)
{
    return FindThemeColor(color).has_value() || list.FindCustomColor(color) != nullptr;
}
} // namespace Data::ServerList
