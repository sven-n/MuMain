#include "stdafx.h"

#include "ServerListGroup.h"

#include <algorithm>

namespace Data::ServerList
{
ChannelNames GetDefaultChannelNames()
{
    ChannelNames names;
    names.pvp.Set(LocalizedString::NeutralLocale, "PvP");
    names.nonPvp.Set(LocalizedString::NeutralLocale, "Non-PvP");
    names.Template(ChannelNamePlace::Button)
        .Set(LocalizedString::NeutralLocale, "{name}-{channel} ({vip} {pvp}) {load}");
    names.Template(ChannelNamePlace::Login).Set(LocalizedString::NeutralLocale, "[{name}-{channel} ({vip} {pvp})]");
    names.Template(ChannelNamePlace::CharacterInfo)
        .Set(LocalizedString::NeutralLocale, "[{name}-{channel} ({vip} {pvp})]");
    names.Template(ChannelNamePlace::Screenshot).Set(LocalizedString::NeutralLocale, "[{name} / {character}]");
    return names;
}

const CustomColor* ServerListData::FindCustomColor(std::string_view id) const
{
    const auto found =
        std::find_if(colors.begin(), colors.end(), [id](const CustomColor& color) { return color.id == id; });
    return found == colors.end() ? nullptr : &*found;
}

const VipType* ServerListData::FindVipType(std::string_view id) const
{
    const auto found =
        std::find_if(vipTypes.begin(), vipTypes.end(), [id](const VipType& vipType) { return vipType.id == id; });
    return found == vipTypes.end() ? nullptr : &*found;
}
} // namespace Data::ServerList
