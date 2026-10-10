#include "Network/Discord/DiscordPackets.h"

#include "Data/Translation/MultiLanguage.h"

#include <vector>

namespace
{
// DiscordIntegrationInfo: C1 header with sub code, fixed length.
constexpr std::size_t InfoLength = 224;
constexpr std::size_t InfoAccountLinkedOffset = 4;
constexpr std::size_t InfoGuildChatOffset = 5;
constexpr std::size_t InfoAllianceChatOffset = 6;
constexpr std::size_t InfoWorldChatOffset = 7;
constexpr std::size_t InfoApplicationIdOffset = 8;
constexpr std::size_t InfoApplicationIdLength = 20;
constexpr std::size_t InfoLargeImageOffset = 28;
constexpr std::size_t InfoSmallImageOffset = 60;
constexpr std::size_t InfoImageKeyLength = 32;
constexpr std::size_t InfoInviteUrlOffset = 92;
constexpr std::size_t InfoInviteUrlLength = 100;
constexpr std::size_t InfoLinkedUserOffset = 192;
constexpr std::size_t InfoLinkedUserLength = 32;

// DiscordLinkCode: C1 header with sub code, fixed length.
constexpr std::size_t LinkCodeLength = 16;
constexpr std::size_t LinkCodeResultOffset = 4;
constexpr std::size_t LinkCodeCodeOffset = 5;
constexpr std::size_t LinkCodeCodeLength = 10;
constexpr std::size_t LinkCodeMinutesOffset = 15;
constexpr BYTE LinkCodeResultLast = 2;

// ExternalChatMessage: C2 header with sub code; the message fills the rest.
constexpr std::size_t ExternalSourceOffset = 5;
constexpr std::size_t ExternalScopeOffset = 6;
constexpr std::size_t ExternalSenderOffset = 7;
constexpr std::size_t ExternalSenderLength = 48;
constexpr std::size_t ExternalMessageOffset = 55;
constexpr BYTE ExternalSourceDiscord = 0;
constexpr BYTE ExternalScopeLast = 2;

// The strings are UTF-8 and padded with zeros.
std::wstring ReadString(std::span<const BYTE> packet, std::size_t offset, std::size_t length)
{
    std::vector<wchar_t> buffer(length + 1, L'\0');
    CMultiLanguage::ConvertFromUtf8(buffer.data(), reinterpret_cast<const char*>(packet.data() + offset),
                                    static_cast<int>(length));
    buffer[length] = L'\0';
    return std::wstring(buffer.data());
}

bool ReadBool(std::span<const BYTE> packet, std::size_t offset)
{
    return packet[offset] != 0;
}
} // namespace

namespace Network::Discord
{
bool IntegrationInfo::HasAnything() const
{
    return isAccountLinked || isGuildChatBridged || isAllianceChatBridged || isWorldChatBridged ||
           !richPresenceApplicationId.empty() || !inviteUrl.empty();
}

std::optional<IntegrationInfo> ParseIntegrationInfo(std::span<const BYTE> packet)
{
    if (packet.size() < InfoLength)
    {
        return std::nullopt;
    }

    IntegrationInfo info;
    info.isAccountLinked = ReadBool(packet, InfoAccountLinkedOffset);
    info.isGuildChatBridged = ReadBool(packet, InfoGuildChatOffset);
    info.isAllianceChatBridged = ReadBool(packet, InfoAllianceChatOffset);
    info.isWorldChatBridged = ReadBool(packet, InfoWorldChatOffset);
    info.richPresenceApplicationId = ReadString(packet, InfoApplicationIdOffset, InfoApplicationIdLength);
    info.richPresenceLargeImageKey = ReadString(packet, InfoLargeImageOffset, InfoImageKeyLength);
    info.richPresenceSmallImageKey = ReadString(packet, InfoSmallImageOffset, InfoImageKeyLength);
    info.inviteUrl = ReadString(packet, InfoInviteUrlOffset, InfoInviteUrlLength);
    info.linkedUserName = ReadString(packet, InfoLinkedUserOffset, InfoLinkedUserLength);
    return info;
}

std::optional<LinkCode> ParseLinkCode(std::span<const BYTE> packet)
{
    if (packet.size() < LinkCodeLength)
    {
        return std::nullopt;
    }

    // A result this client doesn't know reads as "not available".
    const BYTE result = packet[LinkCodeResultOffset];
    LinkCode linkCode;
    linkCode.result =
        result <= LinkCodeResultLast ? static_cast<LinkCode::Result>(result) : LinkCode::Result::NotAvailable;
    linkCode.code = ReadString(packet, LinkCodeCodeOffset, LinkCodeCodeLength);
    linkCode.validMinutes = packet[LinkCodeMinutesOffset];
    return linkCode;
}

std::optional<ExternalChatMessage> ParseExternalChatMessage(std::span<const BYTE> packet)
{
    if (packet.size() <= ExternalMessageOffset || packet[ExternalSourceOffset] != ExternalSourceDiscord ||
        packet[ExternalScopeOffset] > ExternalScopeLast)
    {
        return std::nullopt;
    }

    ExternalChatMessage message;
    message.scope = static_cast<ExternalChatMessage::Scope>(packet[ExternalScopeOffset]);
    message.senderName = ReadString(packet, ExternalSenderOffset, ExternalSenderLength);
    message.message = ReadString(packet, ExternalMessageOffset, packet.size() - ExternalMessageOffset);
    return message;
}
} // namespace Network::Discord
