#include "stdafx.h"
#include "GameLogic/Discord/ServerIntegration.h"

#include "Data/GameConfig/GameConfig.h"
#include "Network/Server/WSclient.h"

namespace
{
std::wstring ServerValueOr(const std::optional<Network::Discord::IntegrationInfo>& info,
                           std::wstring Network::Discord::IntegrationInfo::* field, const std::wstring& fallback)
{
    if (info.has_value() && !((*info).*field).empty())
    {
        return (*info).*field;
    }
    return fallback;
}
} // namespace

namespace GameLogic::Discord
{
ServerIntegration& ServerIntegration::Instance()
{
    static ServerIntegration instance;
    return instance;
}

void ServerIntegration::Request()
{
    m_info.reset();
    if (SocketClient != nullptr)
    {
        SocketClient->ToGameServer()->SendDiscordIntegrationInfoRequest();
    }
}

void ServerIntegration::Apply(const Network::Discord::IntegrationInfo& info)
{
    m_info = info;
}

bool ServerIntegration::IsAvailable() const
{
    return m_info.has_value() && m_info->HasAnything();
}

std::wstring ServerIntegration::InviteUrl() const
{
    return ServerValueOr(m_info, &Network::Discord::IntegrationInfo::inviteUrl,
                         GameConfig::GetInstance().GetDiscordInviteUrl());
}

std::wstring ServerIntegration::RichPresenceApplicationId() const
{
    return ServerValueOr(m_info, &Network::Discord::IntegrationInfo::richPresenceApplicationId,
                         GameConfig::GetInstance().GetDiscordApplicationId());
}

std::wstring ServerIntegration::RichPresenceLargeImageKey() const
{
    return ServerValueOr(m_info, &Network::Discord::IntegrationInfo::richPresenceLargeImageKey,
                         GameConfig::GetInstance().GetDiscordLargeImageKey());
}

std::wstring ServerIntegration::RichPresenceSmallImageKey() const
{
    return ServerValueOr(m_info, &Network::Discord::IntegrationInfo::richPresenceSmallImageKey,
                         GameConfig::GetInstance().GetDiscordSmallImageKey());
}

bool ServerIntegration::IsAccountLinked() const
{
    return m_info.has_value() && m_info->isAccountLinked;
}

std::wstring ServerIntegration::LinkedUserName() const
{
    return m_info.has_value() ? m_info->linkedUserName : std::wstring();
}

bool ServerIntegration::IsGuildChatBridged() const
{
    return m_info.has_value() && m_info->isGuildChatBridged;
}

bool ServerIntegration::IsAllianceChatBridged() const
{
    return m_info.has_value() && m_info->isAllianceChatBridged;
}

bool ServerIntegration::IsWorldChatBridged() const
{
    return m_info.has_value() && m_info->isWorldChatBridged;
}

void ServerIntegration::RequestLinkCode() const
{
    if (SocketClient != nullptr)
    {
        SocketClient->ToGameServer()->SendDiscordLinkCodeRequest();
    }
}

void ServerIntegration::RequestUnlink() const
{
    if (SocketClient != nullptr)
    {
        SocketClient->ToGameServer()->SendDiscordUnlinkRequest();
    }
}
} // namespace GameLogic::Discord
