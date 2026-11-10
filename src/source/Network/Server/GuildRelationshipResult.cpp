#include "Network/Server/GuildRelationshipResult.h"

namespace Network::Packets
{
std::optional<GuildRelationshipResult> ParseGuildRelationshipResult(std::span<const std::uint8_t> packet)
{
    constexpr std::size_t SizeOffset = 1;
    constexpr std::size_t RelationshipTypeOffset = 3;
    constexpr std::size_t RequestTypeOffset = 4;
    constexpr std::size_t ResultOffset = 5;
    constexpr std::size_t KeyHighOffset = 6;
    constexpr std::size_t KeyLowOffset = 7;
    constexpr std::size_t PacketSize = 8;

    if (packet.size() < PacketSize || packet[SizeOffset] < PacketSize)
    {
        return std::nullopt;
    }

    return GuildRelationshipResult{
        packet[RelationshipTypeOffset],
        packet[RequestTypeOffset],
        packet[ResultOffset],
        static_cast<std::uint16_t>((packet[KeyHighOffset] << 8) | packet[KeyLowOffset]),
    };
}
} // namespace Network::Packets
