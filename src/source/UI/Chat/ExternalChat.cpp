#include "stdafx.h"
#include "UI/Chat/ExternalChat.h"

#include "App/Control/ControlTaps.h"
#include "Network/Server/WSclient.h"
#include "UI/NewUI/NewUISystem.h"

namespace
{
constexpr const wchar_t* DiscordBadge = L"[Discord] ";
constexpr const char* DiscordChatKind = "discord";
} // namespace

namespace UI::Chat::External
{
std::wstring SenderLabel(Source source, std::wstring_view displayName)
{
    switch (source)
    {
    case Source::Discord:
    default:
        return DiscordBadge + std::wstring(displayName.substr(0, MaxDisplayNameLength));
    }
}

SEASON3B::MESSAGE_TYPE ScopeMessageType(Scope scope)
{
    switch (scope)
    {
    case Scope::Guild:
        return SEASON3B::TYPE_GUILD_MESSAGE;
    case Scope::Alliance:
        return SEASON3B::TYPE_UNION_MESSAGE;
    case Scope::World:
    default:
        return SEASON3B::TYPE_CHAT_MESSAGE;
    }
}

std::wstring ShownText(std::wstring_view text)
{
    return std::wstring(text.substr(0, MAX_CHAT_SIZE));
}

// Unlike an in-game message, no speech balloon (nobody stands there) and no
// entry in the whisper allow-list (nobody to whisper).
void Show(const Message& message)
{
    const std::wstring sender = SenderLabel(message.source, message.displayName);
    const std::wstring text = ShownText(message.text);
    if (text.empty())
    {
        return;
    }

    g_pChatListBox->AddExternalText(sender, text, ScopeMessageType(message.scope));
    App::Control::Events::RecordChatLine(sender.c_str(), text.c_str(), DiscordChatKind);
}
} // namespace UI::Chat::External
