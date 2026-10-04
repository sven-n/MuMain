#pragma once

#include <cstdint>
#include <span>
#include <string_view>

// Social changes the server reports, applied to the friend, letter and chat-room owners. Strings
// are borrowed for the call only.
namespace UI::Social
{
struct FriendEntry
{
    std::wstring_view name;
    std::uint8_t server;
};

// Logout or return to login: closes the social windows and drops every friend and letter.
void Reset();

void ReplaceFriendList(std::span<const FriendEntry> friends);
void FriendAdded(std::wstring_view name, std::uint8_t server);
void FriendRemoved(std::wstring_view name);
// `offline` locks an open chat room with that friend; otherwise it is unlocked.
void FriendStateChanged(std::wstring_view name, std::uint8_t server, bool offline);
// The friend server is gone: every friend takes `server`, chat rooms lock, sending is disabled.
void FriendServerLost(std::uint8_t server);

// Opens the friend window if it is closed and asks whether to accept `requester` as a friend.
void ShowFriendRequest(std::wstring_view prompt, std::wstring_view requester);
void ShowNotice(std::wstring_view text);
}
