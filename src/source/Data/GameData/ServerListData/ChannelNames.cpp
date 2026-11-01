#include "stdafx.h"

#include "ChannelNames.h"

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
} // namespace Data::ServerList
