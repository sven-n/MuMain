#include "Network/Server/RemoveAllianceGuildResult.h"

#include "Network/Server/FixedC1Packet.h"
#include "Network/Server/GuildRelationshipCodes.h"

namespace Network::Packets
{
std::optional<RemoveAllianceGuildResult> ParseRemoveAllianceGuildResult(std::span<const std::uint8_t> packet)
{
    constexpr std::uint8_t HeadCode = 0xEB;
    constexpr std::uint8_t SubCode = 0x01;
    constexpr std::size_t ResultOffset = 4;

    if (!IsFixedC1Packet(packet, HeadCode, SubCode, RemoveAllianceGuildResult::PacketSize))
    {
        return std::nullopt;
    }

    return RemoveAllianceGuildResult{packet[ResultOffset] == GuildRelationship::Success};
}
} // namespace Network::Packets
