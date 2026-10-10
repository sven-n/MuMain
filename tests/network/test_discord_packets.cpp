// doctest unit tests for the Discord integration messages of the server
// (MUnique/OpenMU#1174): reading them, and the server values taking over
// from config.ini.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Discord packet|Discord server"

#include "App/stdafx.h"
#include "doctest.h"

#include "GameLogic/Discord/ServerIntegration.h"
#include "Network/Discord/DiscordPackets.h"

#include <cstring>
#include <string>
#include <vector>

using namespace Network::Discord;

namespace
{
constexpr BYTE C1 = 0xC1;
constexpr BYTE C2 = 0xC2;
constexpr BYTE GroupCode = 0xF5;

void Write(std::vector<BYTE>& packet, std::size_t offset, const std::string& text)
{
    std::memcpy(packet.data() + offset, text.data(), text.size());
}

std::vector<BYTE> IntegrationInfoPacket()
{
    std::vector<BYTE> packet(224, 0);
    packet[0] = C1;
    packet[1] = static_cast<BYTE>(packet.size());
    packet[2] = GroupCode;
    packet[3] = IntegrationInfoSubCode;
    packet[4] = 1; // linked
    packet[5] = 1; // guild chat bridged
    packet[6] = 0; // alliance chat
    packet[7] = 1; // world chat
    Write(packet, 8, "123456789012345678");
    Write(packet, 28, "logo");
    Write(packet, 92, "https://discord.gg/abc123");
    Write(packet, 192, "sven");
    return packet;
}

std::vector<BYTE> ExternalChatPacket(BYTE scope, const std::string& sender, const std::string& message)
{
    std::vector<BYTE> packet(55 + message.size(), 0);
    packet[0] = C2;
    packet[1] = static_cast<BYTE>(packet.size() >> 8);
    packet[2] = static_cast<BYTE>(packet.size() & 0xFF);
    packet[3] = GroupCode;
    packet[4] = ExternalChatMessageSubCode;
    packet[5] = 0; // Discord
    packet[6] = scope;
    Write(packet, 7, sender);
    Write(packet, 55, message);
    return packet;
}
} // namespace

TEST_CASE("Discord packet: the integration info is read field by field [network][discord]")
{
    const auto packet = IntegrationInfoPacket();
    const auto info = ParseIntegrationInfo(packet);

    REQUIRE(info.has_value());
    CHECK(info->isAccountLinked);
    CHECK(info->isGuildChatBridged);
    CHECK_FALSE(info->isAllianceChatBridged);
    CHECK(info->isWorldChatBridged);
    CHECK(info->richPresenceApplicationId == L"123456789012345678");
    CHECK(info->richPresenceLargeImageKey == L"logo");
    CHECK(info->richPresenceSmallImageKey.empty());
    CHECK(info->inviteUrl == L"https://discord.gg/abc123");
    CHECK(info->linkedUserName == L"sven");
    CHECK(info->HasAnything());
}

TEST_CASE("Discord packet: a truncated integration info is refused [network][discord]")
{
    auto packet = IntegrationInfoPacket();
    packet.resize(100);
    CHECK_FALSE(ParseIntegrationInfo(packet).has_value());
}

TEST_CASE("Discord packet: an empty integration info offers nothing [network][discord]")
{
    std::vector<BYTE> packet(224, 0);
    const auto info = ParseIntegrationInfo(packet);
    REQUIRE(info.has_value());
    CHECK_FALSE(info->HasAnything());
}

TEST_CASE("Discord packet: the link code carries its code and validity [network][discord]")
{
    std::vector<BYTE> packet(16, 0);
    packet[0] = C1;
    packet[4] = 0; // success
    Write(packet, 5, "ABCD-EFGH");
    packet[15] = 10;

    const auto linkCode = ParseLinkCode(packet);
    REQUIRE(linkCode.has_value());
    CHECK(linkCode->isAvailable);
    CHECK(linkCode->code == L"ABCD-EFGH");
    CHECK(linkCode->validMinutes == 10);

    packet[4] = 1; // not available
    CHECK_FALSE(ParseLinkCode(packet)->isAvailable);
}

TEST_CASE("Discord packet: an external chat message is read with its scope [network][discord]")
{
    const auto packet = ExternalChatPacket(1, "Sven Nicolai", "hello alliance");
    const auto message = ParseExternalChatMessage(packet);

    REQUIRE(message.has_value());
    CHECK(message->scope == ExternalChatMessage::Scope::Alliance);
    CHECK(message->senderName == L"Sven Nicolai");
    CHECK(message->message == L"hello alliance");
}

TEST_CASE("Discord packet: an external chat message of an unknown scope is refused [network][discord]")
{
    const auto packet = ExternalChatPacket(7, "Sven", "hi");
    CHECK_FALSE(ParseExternalChatMessage(packet).has_value());
}

TEST_CASE(
    "Discord server: its values take over from config.ini, and are forgotten on the next request [network][discord]")
{
    auto& server = GameLogic::Discord::ServerIntegration::Instance();
    const auto info = ParseIntegrationInfo(IntegrationInfoPacket());
    REQUIRE(info.has_value());

    server.Apply(*info);
    CHECK(server.IsAvailable());
    CHECK(server.InviteUrl() == L"https://discord.gg/abc123");
    CHECK(server.RichPresenceApplicationId() == L"123456789012345678");
    CHECK(server.IsAccountLinked());
    CHECK(server.LinkedUserName() == L"sven");
    CHECK(server.IsGuildChatBridged());

    // Asking again keeps the answer until the new one arrives (without a
    // connection nothing is sent).
    server.Request();
    CHECK(server.IsAccountLinked());

    // Entering the world forgets it, in case the new server doesn't answer.
    server.Forget();
    CHECK_FALSE(server.IsAvailable());
    CHECK_FALSE(server.IsAccountLinked());
    CHECK_FALSE(server.IsGuildChatBridged());
}

TEST_CASE("Discord server: only a Discord application id is taken from the server [network][discord]")
{
    using GameLogic::Discord::ServerIntegration;
    CHECK(ServerIntegration::IsApplicationId(L"123456789012345678"));
    CHECK(ServerIntegration::IsApplicationId(L"12345678901234567"));
    CHECK_FALSE(ServerIntegration::IsApplicationId(L"1234567890123456"));
    CHECK_FALSE(ServerIntegration::IsApplicationId(L"123456789012345678901"));
    CHECK_FALSE(ServerIntegration::IsApplicationId(L"12345678901234567a"));
    CHECK_FALSE(ServerIntegration::IsApplicationId(L""));

    auto info = ParseIntegrationInfo(IntegrationInfoPacket());
    REQUIRE(info.has_value());
    info->richPresenceApplicationId = L"not-an-id";
    auto& server = ServerIntegration::Instance();
    server.Apply(*info);
    CHECK(server.RichPresenceApplicationId() != L"not-an-id");
    server.Forget();
}
