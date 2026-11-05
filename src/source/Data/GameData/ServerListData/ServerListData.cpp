#include "stdafx.h"

#include "ServerListData.h"

#include "LoadBar.h"
#include "ThemeColor.h"

#include <algorithm>

namespace Data::ServerList
{
const char* GetThemeColorEntry(int index)
{
    if (index < ThemeColorCount)
    {
        return GetThemeColorName(static_cast<ThemeColor>(index));
    }
    return GetLoadBarToken(static_cast<LoadBarPart>(index - ThemeColorCount));
}

const char* GetThemeColorEntryToken(std::string_view name)
{
    if (const std::optional<ThemeColor> color = FindThemeColor(name))
    {
        return GetThemeColorToken(*color);
    }
    for (const LoadBarPart part : {LoadBarPart::Base, LoadBarPart::Full, LoadBarPart::Empty})
    {
        if (name == GetLoadBarToken(part))
        {
            return GetLoadBarToken(part);
        }
    }
    return nullptr;
}

bool IsThemeColorEntry(std::string_view name)
{
    return GetThemeColorEntryToken(name) != nullptr;
}

bool ServerListData::IsColorEntry(std::string_view name) const
{
    return FindCustomColor(name) != nullptr || IsThemeColorEntry(name);
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
