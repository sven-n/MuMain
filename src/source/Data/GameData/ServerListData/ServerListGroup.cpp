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

namespace
{
bool IsLower(char character)
{
    return character >= 'a' && character <= 'z';
}

bool IsUpper(char character)
{
    return character >= 'A' && character <= 'Z';
}

bool IsDigit(char character)
{
    return character >= '0' && character <= '9';
}

char ToLower(char character)
{
    return IsUpper(character) ? static_cast<char>(character - 'A' + 'a') : character;
}

char ToUpper(char character)
{
    return IsLower(character) ? static_cast<char>(character - 'a' + 'A') : character;
}
} // namespace

bool IsVipTypeId(std::string_view id)
{
    return !id.empty() && IsLower(id.front()) &&
           std::all_of(id.begin(), id.end(),
                       [](char character) { return IsLower(character) || IsUpper(character) || IsDigit(character); });
}

std::string MakeVipTypeId(std::string_view name)
{
    // Each word of letters and digits; the first in lowercase, each other one capitalized.
    std::string id;
    bool wordStart = true;
    for (const char character : name)
    {
        if (!IsLower(character) && !IsUpper(character) && !IsDigit(character))
        {
            wordStart = true;
            continue;
        }
        if (id.empty())
        {
            if (IsDigit(character))
            {
                continue;
            }
            id += ToLower(character);
        }
        else
        {
            id += wordStart ? ToUpper(character) : ToLower(character);
        }
        wordStart = false;
    }
    return id.empty() ? std::string("vip") : id;
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
