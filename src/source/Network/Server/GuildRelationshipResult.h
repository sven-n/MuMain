#pragma once

#include <cstdint>
#include <optional>
#include <span>

namespace Network::Packets
{
// OpenMU's ServerToClientPackets.xml: C1 E6, the result of an alliance or hostility change, sent back to the
// requester. One byte per field; the guild master's character key is big-endian.
struct GuildRelationshipResult
{
    std::uint8_t relationshipType;
    std::uint8_t requestType;
    std::uint8_t result;
    std::uint16_t guildMasterKey;
};

// Returns nullopt when the span or the size declared in the header is shorter than the 8-byte packet.
[[nodiscard]] std::optional<GuildRelationshipResult> ParseGuildRelationshipResult(std::span<const std::uint8_t> packet);
} // namespace Network::Packets
