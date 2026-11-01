#pragma once

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ServerListData/ButtonColorRule.h"
#include "Data/GameData/ServerListData/ChannelNames.h"
#include "Data/GameData/ServerListData/CustomColor.h"
#include "Data/GameData/ServerListData/LoadSettings.h"
#include "Data/GameData/ServerListData/ServerListGroup.h"
#include "Data/GameData/ServerListData/VipType.h"

#include <string_view>
#include <vector>

namespace Data::ServerList
{
// The colors of the themes the list uses, each a token of every theme's tokens.ini: the theme
// colors of the buttons and the load texts ("default", "gray", "orange", "silver") and the load
// bar's own ("server-load-base", "server-load-full", "server-load-empty"). MuEditor's Colors
// window lists them in the Server list root, and its folders may hold them.
inline constexpr int ThemeColorEntryCount = 7;
const char* GetThemeColorEntry(int index);
// The token of the theme color entry, or nullptr for any other name.
const char* GetThemeColorEntryToken(std::string_view name);
bool IsThemeColorEntry(std::string_view name);

// All that Data/Local/ServerList.json holds.
struct ServerListData
{
    // In the order the editor lists them.
    std::vector<VipType> vipTypes;
    // In the order the editor lists them.
    std::vector<CustomColor> colors;
    // The folders MuEditor's Colors window sorts the colors in ("colorFolders", ColorFolders.h); the game skips them.
    std::vector<Colors::ColorFolder> colorFolders;
    // Their order does not matter.
    std::vector<ButtonColorRule> buttonColors;
    ChannelNames names = GetDefaultChannelNames();
    LoadSettings load = GetDefaultLoadSettings();
    // In the order of their buttons.
    std::vector<ServerGroup> groups;

    // The VIP type with the id, or nullptr.
    const VipType* FindVipType(std::string_view id) const;
    // The color of the list's own with the id, or nullptr.
    const CustomColor* FindCustomColor(std::string_view id) const;
    // Whether the name is one of the list's colors for the editor's Colors window: one of its own,
    // or a theme color it uses (IsThemeColorEntry).
    bool IsColorEntry(std::string_view name) const;

    bool operator==(const ServerListData&) const = default;
};
} // namespace Data::ServerList
