#include "Network/Server/RemoveAllianceGuildResult.h"

namespace Network::Packets
{
std::optional<RemoveAllianceGuildResult> ParseRemoveAllianceGuildResult(std::span<const std::uint8_t> packet)
{
    constexpr std::size_t SizeOffset = 1;
    constexpr std::size_t ResultOffset = 4;
    constexpr std::size_t PacketSize = 7;

    if (packet.size() < PacketSize || packet[SizeOffset] < PacketSize)
    {
        return std::nullopt;
    }

    return RemoveAllianceGuildResult{packet[ResultOffset] == 0x01};
}
} // namespace Network::Packets
