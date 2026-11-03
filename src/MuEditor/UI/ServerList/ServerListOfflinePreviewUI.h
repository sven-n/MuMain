#pragma once

#ifdef _EDITOR

#include <map>

// The Server List window's preview of the servers the connect server didn't send (docs/server-list.md): the
// toolbar's switch and load, and each offline channel's box and load in the channel list. Each function returns
// true when the preview changed, for the caller to show it on the login screen. A change is no undo step: the
// preview is not part of the list.
namespace MuEditor::ServerList::OfflinePreview
{
// The IDs of an offline channel's load slider and of the "×" (UTF-8) next to it that drops its own load, in the
// channel's row.
inline constexpr const char* ChannelLoadLabel = "##OfflineLoad";
inline constexpr const char* DropOwnLoadLabel = "\xC3\x97##DropOwnLoad";

// "Show offline servers in the game" and "Their load", which MuEditor.ini keeps.
bool RenderSwitch();
// All and None for the offline channels of the group `groupId`, while the preview is on. `sent`: the channels
// the connect server sent (by channel from 0, with their load), or nullptr for none.
bool RenderGroupButtons(int groupId, const std::map<int, int>* sent);
// The box and the load of the channel with the server ID `serverId`, which the connect server didn't send,
// while the preview is on.
bool RenderChannel(int serverId);
} // namespace MuEditor::ServerList::OfflinePreview

#endif // _EDITOR
