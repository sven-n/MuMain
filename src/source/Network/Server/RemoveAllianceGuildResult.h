#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace Network::Packets
{
// OpenMU's ServerToClientPackets.xml: C1 EB 01, the result of a guild master removing a guild from the alliance.
// The client only needs whether it worked; the request and relationship type bytes after it aren't read.
struct RemoveAllianceGuildResult
{
    static constexpr std::size_t PacketSize = 7;

    bool succeeded;
};

// Returns nullopt unless it's a C1 packet with this packet's codes and both the span and the declared size
// hold PacketSize bytes.
[[nodiscard]] std::optional<RemoveAllianceGuildResult>
ParseRemoveAllianceGuildResult(std::span<const std::uint8_t> packet);
} // namespace Network::Packets
