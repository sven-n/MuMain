// What the game server told about its Discord integration, with config.ini
// as the fallback for servers which don't tell (older servers, or ones
// without the Discord integration).
//
// Asked for after the character entered the world; the answer configures
// the Rich Presence, the Join button, the account dialog and the notice in
// the guild window.
#pragma once

#include "Network/Discord/DiscordPackets.h"

#include <optional>
#include <string>
#include <string_view>

namespace GameLogic::Discord
{
class ServerIntegration
{
public:
    [[nodiscard]] static ServerIntegration& Instance();

    // Forgets what the server told; on entering the world, so nothing of a
    // previous server is left over when the new one doesn't answer.
    void Forget();

    // Asks the server, keeping its previous answer until the new one arrives,
    // so a refresh doesn't show "not linked" for a round trip. Servers
    // without the Discord integration don't answer.
    void Request();

    // Takes the server's answer.
    void Apply(const Network::Discord::IntegrationInfo& info);

    // Whether the server answered with anything to offer, so the account
    // dialog has something to show.
    [[nodiscard]] bool IsAvailable() const;

    // The server's value, else the one of config.ini.
    [[nodiscard]] std::wstring InviteUrl() const;
    // Only a Discord application id (a snowflake: 17 to 20 digits) is taken
    // from the server; anything else falls back to config.ini, so a server
    // can't hand the handshake something that isn't one.
    [[nodiscard]] std::wstring RichPresenceApplicationId() const;
    [[nodiscard]] std::wstring RichPresenceLargeImageKey() const;
    [[nodiscard]] std::wstring RichPresenceSmallImageKey() const;

    // Known from the server only; false/empty without its answer.
    [[nodiscard]] bool IsAccountLinked() const;
    [[nodiscard]] std::wstring LinkedUserName() const;
    [[nodiscard]] bool IsGuildChatBridged() const;
    [[nodiscard]] bool IsAllianceChatBridged() const;
    [[nodiscard]] bool IsWorldChatBridged() const;

    // Account link, from the account dialog.
    void RequestLinkCode() const;
    void RequestUnlink() const;

    [[nodiscard]] static bool IsApplicationId(std::wstring_view id);

private:
    std::optional<Network::Discord::IntegrationInfo> m_info;
};
} // namespace GameLogic::Discord
