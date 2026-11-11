#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace Network::Packets
{
// OpenMU's ServerToClientPackets.xml: C1 E6, the result of an alliance or hostility change. A failed request goes to
// the requester with the target's character key; after the other guild master responds, both masters get it with
// the requester's key. One byte per field; the key is big-endian.
struct GuildRelationshipResult
{
    static constexpr std::size_t PacketSize = 8;

    std::uint8_t relationshipType;
    std::uint8_t requestType;
    std::uint8_t result;
    std::uint16_t characterKey;
};

// Returns nullopt unless it's a C1 packet with this packet's codes and both the span and the declared size
// hold PacketSize bytes.
[[nodiscard]] std::optional<GuildRelationshipResult> ParseGuildRelationshipResult(std::span<const std::uint8_t> packet);
} // namespace Network::Packets
