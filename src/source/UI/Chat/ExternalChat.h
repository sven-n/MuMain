// Chat messages written outside the game - today in a Discord channel that
// the server bridges into guild, alliance or world chat (#700).
//
// They are shown as such: the sender is marked with where they wrote, keeps
// a name longer than a character's, and can't be whispered, because there
// is no character behind it.
#pragma once

#include "UI/NewUI/HUD/NewUIChatLogWindow.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace UI::Chat::External
{
enum class Source
{
    Discord,
};

// Which in-game chat the message belongs to; it takes that chat's colours.
enum class Scope
{
    Guild,
    Alliance,
    World,
};

// Discord's display names are at most 32 characters long.
inline constexpr std::size_t MaxDisplayNameLength = 32;

struct Message
{
    Source source = Source::Discord;
    Scope scope = Scope::World;
    std::wstring displayName;
    std::wstring text;
};

// What the chat log shows as the sender: "[Discord] name", the name cut to
// MaxDisplayNameLength characters.
[[nodiscard]] std::wstring SenderLabel(Source source, std::wstring_view displayName);

// The chat log type whose colours the scope uses.
[[nodiscard]] SEASON3B::MESSAGE_TYPE ScopeMessageType(Scope scope);

// The text as shown: cut to the length of an in-game chat message.
[[nodiscard]] std::wstring ShownText(std::wstring_view text);

// Shows the message in the chat log, in its scope and in the Discord view.
void Show(const Message& message);

// OpenMU's chat bridge (MUnique/OpenMU#1161) sends messages written in Discord
// through the ordinary guild and alliance chat, as the linked character with
// this prefix. Character names can't contain it, so a sender with it is a
// Discord user and not a character.
inline constexpr wchar_t BridgedSenderPrefix = L'@';

[[nodiscard]] bool IsBridgedSender(std::wstring_view sender);

// Shows a guild or alliance chat line of a bridged sender as a message from
// Discord, named after the character without the prefix.
void ShowBridged(Scope scope, std::wstring_view sender, std::wstring_view text);
} // namespace UI::Chat::External
