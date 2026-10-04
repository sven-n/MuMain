#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
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

// `writeWindow` is the letter-writing window the send came from, 0 if none.
void LetterSent(std::uint32_t writeWindow);
// Re-enables that window's Send and shows why the letter wasn't sent.
void LetterSendFailed(std::uint32_t writeWindow, std::wstring_view notice);

struct LetterSummary
{
    std::uint32_t index;
    std::wstring_view sender;
    std::wstring_view subject;
    std::wstring_view date;
    std::wstring_view time;
};

// A letter that just arrived: added unread, and the mail button blinks.
void NewLetterArrived(const LetterSummary& letter);
// A letter already in the mailbox, listed at login; `readState` as the server sent it.
void LetterListed(const LetterSummary& letter, std::uint8_t readState);
void LetterDeleted(std::uint32_t index);
int LetterCount();

// A letter's text and its sender's portrait, decoded. Kept by the letter list so reopening the
// letter needs no new request.
struct LetterBody
{
    std::uint32_t index = 0;
    std::wstring text;
    std::uint8_t classType = 0; // CLASS_TYPE
    std::array<std::uint8_t, 25> equipment{};
    int animation = 0;
    float angleDegrees = 0.f;
    float zoom = 1.f;
};

// Keeps the body and opens the letter.
void LetterBodyReceived(const LetterBody& body);
// Opens a letter-reading window for a listed letter; does nothing if the letter isn't listed.
void ShowLetter(const LetterBody& body);

// Where to join a chat room the server opened.
struct ChatRoomTicket
{
    std::wstring_view server;
    std::uint32_t room;
    std::uint32_t ticket;
};

enum class ChatRoomArrival
{
    // A room this player asked for: a chat window opens now.
    Requested,
    // A friend's room: rejoins an open room with that friend, or waits hidden behind the others.
    FromFriend,
    // A room this player was invited to: waits at the bottom of the window stack.
    Invited,
};

// Ends the pending request to `peer` and joins the room.
void ChatRoomOpened(std::wstring_view peer, ChatRoomArrival arrival, const ChatRoomTicket& ticket);
// Ends the pending request to `peer` and shows why no room opened.
void ChatRoomRefused(std::wstring_view peer, std::wstring_view notice);
// Ends the pending request to `peer` without opening a room.
void EndChatRoomRequest(std::wstring_view peer);

enum class ChatInviteOutcome
{
    Offline,
    Invited,
    ListFull,
};

// Reports an invitation's outcome in the chat room window it was sent from.
void ChatInviteAnswered(std::uint32_t chatWindow, ChatInviteOutcome outcome);
}
