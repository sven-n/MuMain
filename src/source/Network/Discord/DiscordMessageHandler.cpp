#include "stdafx.h"
#include "Network/Discord/DiscordMessageHandler.h"

#include "GameLogic/Discord/ServerIntegration.h"
#include "Network/Discord/DiscordPackets.h"
#include "UI/Chat/ExternalChat.h"
#include "UI/NewUI/Dialogs/DiscordMsgBox.h"

namespace
{
UI::Chat::External::Scope ToChatScope(Network::Discord::ExternalChatMessage::Scope scope)
{
    switch (scope)
    {
    case Network::Discord::ExternalChatMessage::Scope::Guild:
        return UI::Chat::External::Scope::Guild;
    case Network::Discord::ExternalChatMessage::Scope::Alliance:
        return UI::Chat::External::Scope::Alliance;
    case Network::Discord::ExternalChatMessage::Scope::World:
    default:
        return UI::Chat::External::Scope::World;
    }
}

void ShowExternalChatMessage(const Network::Discord::ExternalChatMessage& received)
{
    UI::Chat::External::Message message;
    message.source = UI::Chat::External::Source::Discord;
    message.scope = ToChatScope(received.scope);
    message.displayName = received.senderName;
    message.text = received.message;
    UI::Chat::External::Show(message);
}
} // namespace

namespace Network::Discord
{
bool HandleMessage(BYTE subCode, std::span<const BYTE> packet)
{
    switch (subCode)
    {
    case IntegrationInfoSubCode:
        if (const auto info = ParseIntegrationInfo(packet))
        {
            GameLogic::Discord::ServerIntegration::Instance().Apply(*info);
        }
        return true;
    case LinkCodeSubCode:
        if (const auto linkCode = ParseLinkCode(packet))
        {
            UI::Discord::ShowLinkCode(*linkCode);
        }
        return true;
    case ExternalChatMessageSubCode:
        if (const auto message = ParseExternalChatMessage(packet))
        {
            ShowExternalChatMessage(*message);
        }
        return true;
    default:
        return false;
    }
}
} // namespace Network::Discord
