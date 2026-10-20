// doctest unit tests for chat messages written outside the game (Discord,
// #700): how their sender is shown, which chat they belong to, and the chat
// log's Discord view.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "External chat"

#include "App/stdafx.h"
#include "doctest.h"

#include "UI/Chat/ExternalChat.h"
#include "UI/HUD/ChatLogWindow.h"

#include <string>

using namespace UI::Chat::External;
using mu::ui::window::CChatLogWindow;

namespace
{
// Short enough that the chat log keeps it on one line without measuring it.
constexpr const wchar_t* ShortText = L"hi guild";
} // namespace

TEST_CASE("External chat marks the sender with where they wrote [ui][chat]")
{
    CHECK(SenderLabel(Source::Discord, L"Sven") == L"[Discord] Sven");
}

TEST_CASE("External chat keeps a Discord name longer than a character's, up to its limit [ui][chat]")
{
    const std::wstring longName(MaxDisplayNameLength + 8, L'n');
    CHECK(SenderLabel(Source::Discord, longName) == L"[Discord] " + longName.substr(0, MaxDisplayNameLength));
}

TEST_CASE("External chat recognises senders bridged from Discord by their prefix [ui][chat]")
{
    CHECK(IsBridgedSender(L"@Hero"));
    CHECK_FALSE(IsBridgedSender(L"Hero"));
    CHECK_FALSE(IsBridgedSender(L"@"));
    CHECK_FALSE(IsBridgedSender(L""));
}

TEST_CASE("External chat takes the colours of its scope [ui][chat]")
{
    CHECK(ScopeMessageType(Scope::Guild) == mu::ui::window::TYPE_GUILD_MESSAGE);
    CHECK(ScopeMessageType(Scope::Alliance) == mu::ui::window::TYPE_UNION_MESSAGE);
    CHECK(ScopeMessageType(Scope::World) == mu::ui::window::TYPE_CHAT_MESSAGE);
}

TEST_CASE("External chat cuts the text to an in-game chat message [ui][chat]")
{
    const std::wstring longText(500, L'x');
    CHECK(ShownText(longText).size() < longText.size());
    CHECK(ShownText(ShortText) == ShortText);
}

TEST_CASE("External chat is listed in its scope, among all messages and in the Discord view [ui][chat]")
{
    CChatLogWindow chatLog;
    chatLog.AddExternalText(SenderLabel(Source::Discord, L"Sven"), ShortText, mu::ui::window::TYPE_GUILD_MESSAGE);

    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_GUILD_MESSAGE) == 1);
    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_ALL_MESSAGE) == 1);
    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_DISCORD_MESSAGE) == 1);
}

TEST_CASE("In-game chat stays out of the Discord view [ui][chat]")
{
    CChatLogWindow chatLog;
    chatLog.AddText(L"Player", ShortText, mu::ui::window::TYPE_GUILD_MESSAGE);

    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_GUILD_MESSAGE) == 1);
    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_DISCORD_MESSAGE) == 0);
}

TEST_CASE("F2 toggles all messages and whispers while nothing came from Discord [ui][chat]")
{
    CChatLogWindow chatLog;
    CHECK(chatLog.GetNextView() == mu::ui::window::TYPE_WHISPER_MESSAGE);

    chatLog.ChangeMessage(mu::ui::window::TYPE_WHISPER_MESSAGE);
    CHECK(chatLog.GetNextView() == mu::ui::window::TYPE_ALL_MESSAGE);
}

TEST_CASE("F2 also offers the Discord view once something came from Discord [ui][chat]")
{
    CChatLogWindow chatLog;
    chatLog.AddExternalText(SenderLabel(Source::Discord, L"Sven"), ShortText, mu::ui::window::TYPE_CHAT_MESSAGE);

    chatLog.ChangeMessage(mu::ui::window::TYPE_WHISPER_MESSAGE);
    CHECK(chatLog.GetNextView() == mu::ui::window::TYPE_DISCORD_MESSAGE);

    chatLog.ChangeMessage(mu::ui::window::TYPE_DISCORD_MESSAGE);
    CHECK(chatLog.GetNextView() == mu::ui::window::TYPE_ALL_MESSAGE);
}

TEST_CASE("An error is still listed among the messages that caused it [ui][chat]")
{
    CChatLogWindow chatLog;
    chatLog.AddText(L"", L"not online", mu::ui::window::TYPE_ERROR_MESSAGE, mu::ui::window::TYPE_WHISPER_MESSAGE);

    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_ERROR_MESSAGE) == 1);
    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_WHISPER_MESSAGE) == 1);
    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_ALL_MESSAGE) == 1);
    CHECK(chatLog.GetNumberOfLines(mu::ui::window::TYPE_DISCORD_MESSAGE) == 0);
}
