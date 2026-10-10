// The Discord integration messages of the server (OpenMU's opt-in F5 group,
// MUnique/OpenMU#1174), read into plain values.
#pragma once

#include "Core/Platform/WinCompat.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace Network::Discord
{
// Sub codes within the F5 group.
inline constexpr BYTE IntegrationInfoSubCode = 0x03;
inline constexpr BYTE LinkCodeSubCode = 0x05;
inline constexpr BYTE ExternalChatMessageSubCode = 0x07;

// How the server is connected to Discord (DiscordIntegrationInfo).
struct IntegrationInfo
{
    bool isAccountLinked = false;
    bool isGuildChatBridged = false;
    bool isAllianceChatBridged = false;
    bool isWorldChatBridged = false;
    std::wstring richPresenceApplicationId;
    std::wstring richPresenceLargeImageKey;
    std::wstring richPresenceSmallImageKey;
    std::wstring inviteUrl;
    std::wstring linkedUserName;

    // Whether the server has anything to offer: a server with the integration
    // switched off answers with an empty info.
    [[nodiscard]] bool HasAnything() const;
};

// The one-time code to link the account (DiscordLinkCode).
struct LinkCode
{
    bool isAvailable = false;
    std::wstring code;
    int validMinutes = 0;
};

// A message written outside the game (ExternalChatMessage).
struct ExternalChatMessage
{
    enum class Scope : std::uint8_t
    {
        Guild = 0,
        Alliance = 1,
        World = 2,
    };

    Scope scope = Scope::World;
    std::wstring senderName;
    std::wstring message;
};

// Each returns nothing for a message too short for its fields, or with a
// value this client doesn't know.
[[nodiscard]] std::optional<IntegrationInfo> ParseIntegrationInfo(std::span<const BYTE> packet);
[[nodiscard]] std::optional<LinkCode> ParseLinkCode(std::span<const BYTE> packet);
[[nodiscard]] std::optional<ExternalChatMessage> ParseExternalChatMessage(std::span<const BYTE> packet);
} // namespace Network::Discord
