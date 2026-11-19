#include "Network/Server/GuildRelationshipResult.h"

#include "Network/Server/FixedC1Packet.h"

namespace Network::Packets
{
std::optional<GuildRelationshipResult> ParseGuildRelationshipResult(std::span<const std::uint8_t> packet)
{
    constexpr std::uint8_t HeadCode = 0xE6;
    constexpr std::size_t RelationshipTypeOffset = 3;
    constexpr std::size_t RequestTypeOffset = 4;
    constexpr std::size_t ResultOffset = 5;
    constexpr std::size_t KeyOffset = 6;

    if (!IsFixedC1Packet(packet, HeadCode, GuildRelationshipResult::PacketSize))
    {
        return std::nullopt;
    }

    return GuildRelationshipResult{
        packet[RelationshipTypeOffset],
        packet[RequestTypeOffset],
        packet[ResultOffset],
        ReadBigEndianU16(packet, KeyOffset),
    };
}
} // namespace Network::Packets
