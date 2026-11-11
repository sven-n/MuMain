#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace Network::Packets
{
// OpenMU's ServerToClientPackets.xml: C1 E5, a guild master asks for an alliance or hostility change.
// One byte per field; the requester's character key is big-endian.
struct GuildRelationshipRequest
{
    static constexpr std::size_t PacketSize = 7;

    std::uint8_t relationshipType;
    std::uint8_t requestType;
    std::uint16_t requesterKey;
};

// Returns nullopt unless it's a C1 packet with this packet's codes and both the span and the declared size
// hold PacketSize bytes.
[[nodiscard]] std::optional<GuildRelationshipRequest>
ParseGuildRelationshipRequest(std::span<const std::uint8_t> packet);
} // namespace Network::Packets
