#include "stdafx.h"

#include "ServerListData.h"

#include <algorithm>

namespace Data::ServerList
{
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
