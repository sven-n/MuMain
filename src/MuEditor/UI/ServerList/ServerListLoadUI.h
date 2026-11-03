#pragma once

#ifdef _EDITOR

#include "Data/GameData/ServerListData/ServerListData.h"

#include <functional>
#include <span>
#include <string>
#include <string_view>

// The Load part of the Server List window's Channel names tab (docs/server-list.md): the load
// bar's colors, the look of the {load} texts, and their preview.
namespace MuEditor::ServerList::LoadUI
{
// The load bar's three colors and the two load texts' color, bold and italic. `beforeChange`
// runs before each change, so what was typed before is a step of its own. True when the list
// changed.
bool RenderLoadSettings(Data::ServerList::ServerListData& list, const std::function<void()>& beforeChange);
// The button of `channel` (channel 1 of the group `groupName`) at 35 % load, full and
// overloaded, in `locale`, with its load bar in each of the theme folders `themes`.
void RenderLoadPreview(const Data::ServerList::ServerListData& list, const Data::ServerList::ChannelSettings& channel,
                       const std::string& groupName, std::string_view locale, std::span<const std::string> themes);
} // namespace MuEditor::ServerList::LoadUI

#endif // _EDITOR
