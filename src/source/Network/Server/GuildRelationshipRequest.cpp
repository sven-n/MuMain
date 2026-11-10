#include "Network/Server/GuildRelationshipRequest.h"

namespace Network::Packets
{
std::optional<GuildRelationshipRequest> ParseGuildRelationshipRequest(std::span<const std::uint8_t> packet)
{
    constexpr std::size_t SizeOffset = 1;
    constexpr std::size_t RelationshipTypeOffset = 3;
    constexpr std::size_t RequestTypeOffset = 4;
    constexpr std::size_t KeyHighOffset = 5;
    constexpr std::size_t KeyLowOffset = 6;
    constexpr std::size_t PacketSize = 7;

    if (packet.size() < PacketSize || packet[SizeOffset] < PacketSize)
    {
        return std::nullopt;
    }

    return GuildRelationshipRequest{
        packet[RelationshipTypeOffset],
        packet[RequestTypeOffset],
        static_cast<std::uint16_t>((packet[KeyHighOffset] << 8) | packet[KeyLowOffset]),
    };
}
} // namespace Network::Packets
