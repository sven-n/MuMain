#include "stdafx.h"
#include "UI/Dialogs/DiscordMsgBox.h"

#include "Core/Text/Utf8.h"
#include "GameLogic/Discord/ServerIntegration.h"
#include "I18N/All.h"
#include "Integration/Discord/Invite.h"
#include "UI/Chat/ChatMessages.h"
#include "UI/Dialogs/GenericMenuDialog.h"

#include <SDL3/SDL_clipboard.h>

#include <string>
#include <utility>

namespace
{
using mu::ui::window::GenericMenuConfig;

const DWORD CodeColor = RGBA(255, 220, 120, 255);

std::wstring Formatted(const wchar_t* format, const std::wstring& value)
{
    wchar_t text[MAX_TEXT_LENGTH]{};
    mu_swprintf_s(text, MAX_TEXT_LENGTH, format, value.c_str());
    return text;
}

std::wstring Formatted(const wchar_t* format, int value)
{
    wchar_t text[MAX_TEXT_LENGTH]{};
    mu_swprintf_s(text, MAX_TEXT_LENGTH, format, value);
    return text;
}

void CopyCode(const std::wstring& code)
{
    SDL_SetClipboardText(Core::Text::ToUtf8(code.c_str()).c_str());
}

GenericMenuConfig::MenuButton Button(const wchar_t* label, std::function<void()> onClick)
{
    GenericMenuConfig::MenuButton button;
    button.label = label;
    button.onClick = std::move(onClick);
    return button;
}

// Join (when there is an invite) and Close after the dialog's own action.
void Show(GenericMenuConfig cfg)
{
    const auto& server = GameLogic::Discord::ServerIntegration::Instance();
    if (Integration::Discord::Invite::IsInviteUrl(server.InviteUrl()))
    {
        cfg.buttons.push_back(Button(I18N::Game::DiscordJoin, [] {
            Integration::Discord::Invite::Open(GameLogic::Discord::ServerIntegration::Instance().InviteUrl());
        }));
    }

    GenericMenuConfig::MenuButton close;
    close.label = I18N::Game::Close388;
    close.compact = true;
    close.dismiss = true;
    cfg.buttons.push_back(std::move(close));

    mu::ui::window::g_pGenericMenuDialog->Show(std::move(cfg));
}
} // namespace

namespace UI::Discord
{
void ShowAccount()
{
    // Shows what is known and asks again, so the next opening reflects a link
    // made in Discord meanwhile.
    auto& server = GameLogic::Discord::ServerIntegration::Instance();
    server.Request();

    GenericMenuConfig cfg;
    cfg.title = I18N::Game::DiscordPresence;
    cfg.highlightTitle = true;
    cfg.lines.push_back({server.IsAccountLinked() ? Formatted(I18N::Game::DiscordLinkedAs, server.LinkedUserName())
                                                  : std::wstring(I18N::Game::DiscordNotLinked)});
    if (server.IsGuildChatBridged())
    {
        cfg.lines.push_back({I18N::Game::DiscordGuildChatMirrored});
    }
    if (server.IsAllianceChatBridged())
    {
        cfg.lines.push_back({I18N::Game::DiscordAllianceChatMirrored});
    }
    if (server.IsWorldChatBridged())
    {
        cfg.lines.push_back({I18N::Game::DiscordWorldChatMirrored});
    }

    // Asks for a code, or unlinks; the server answers with the updated info.
    if (server.IsAccountLinked())
    {
        cfg.buttons.push_back(Button(I18N::Game::DiscordUnlink,
                                     [] { GameLogic::Discord::ServerIntegration::Instance().RequestUnlink(); }));
    }
    else
    {
        cfg.buttons.push_back(Button(I18N::Game::DiscordLink,
                                     [] { GameLogic::Discord::ServerIntegration::Instance().RequestLinkCode(); }));
    }

    Show(std::move(cfg));
}

void ShowLinkCode(const Network::Discord::LinkCode& linkCode)
{
    if (linkCode.result == Network::Discord::LinkCode::Result::TooSoon)
    {
        UI::Chat::PostSystem(I18N::Game::DiscordLinkCodeTooSoon, mu::ui::window::TYPE_ERROR_MESSAGE);
        return;
    }

    if (linkCode.result != Network::Discord::LinkCode::Result::Success || linkCode.code.empty())
    {
        UI::Chat::PostSystem(I18N::Game::DiscordLinkNotAvailable, mu::ui::window::TYPE_ERROR_MESSAGE);
        return;
    }

    CopyCode(linkCode.code);

    GenericMenuConfig cfg;
    cfg.title = I18N::Game::DiscordLinkCodeTitle;
    cfg.highlightTitle = true;
    cfg.lines.push_back({linkCode.code, true, CodeColor});
    cfg.lines.push_back({Formatted(I18N::Game::DiscordLinkCodeHint, linkCode.code)});
    cfg.lines.push_back({Formatted(I18N::Game::DiscordLinkCodeValidity, linkCode.validMinutes)});
    cfg.lines.push_back({I18N::Game::DiscordLinkCodeCopied});

    // Copies the code again, in case the clipboard was used meanwhile.
    cfg.buttons.push_back(Button(I18N::Game::DiscordCopy, [code = linkCode.code] { CopyCode(code); }));

    Show(std::move(cfg));
}
} // namespace UI::Discord
