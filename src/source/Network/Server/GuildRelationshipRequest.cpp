#include "Network/Server/GuildRelationshipRequest.h"

#include "Network/Server/FixedC1Packet.h"

namespace Network::Packets
{
std::optional<GuildRelationshipRequest> ParseGuildRelationshipRequest(std::span<const std::uint8_t> packet)
{
    constexpr std::uint8_t HeadCode = 0xE5;
    constexpr std::size_t RelationshipTypeOffset = 3;
    constexpr std::size_t RequestTypeOffset = 4;
    constexpr std::size_t KeyOffset = 5;

    if (!IsFixedC1Packet(packet, HeadCode, GuildRelationshipRequest::PacketSize))
    {
        return std::nullopt;
    }

    return GuildRelationshipRequest{
        packet[RelationshipTypeOffset],
        packet[RequestTypeOffset],
        ReadBigEndianU16(packet, KeyOffset),
    };
}
} // namespace Network::Packets
