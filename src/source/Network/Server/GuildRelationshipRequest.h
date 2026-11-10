#pragma once

#include <cstdint>
#include <optional>
#include <span>

namespace Network::Packets
{
// OpenMU's ServerToClientPackets.xml: C1 E5, a guild master asks for an alliance or hostility change.
// One byte per field; the requester's character key is big-endian.
struct GuildRelationshipRequest
{
    std::uint8_t relationshipType;
    std::uint8_t requestType;
    std::uint16_t requesterKey;
};

// Returns nullopt when the span or the size declared in the header is shorter than the 7-byte packet.
[[nodiscard]] std::optional<GuildRelationshipRequest>
ParseGuildRelationshipRequest(std::span<const std::uint8_t> packet);
} // namespace Network::Packets
