#pragma once

#include <cstdint>
#include <optional>
#include <span>

namespace Network::Packets
{
// OpenMU's ServerToClientPackets.xml: C1 EB 01, the result of a guild master removing a guild from the alliance.
// The client only needs whether it worked; the request and relationship type bytes after it aren't read.
struct RemoveAllianceGuildResult
{
    bool succeeded;
};

// Returns nullopt when the span or the size declared in the header is shorter than the 7-byte packet.
[[nodiscard]] std::optional<RemoveAllianceGuildResult>
ParseRemoveAllianceGuildResult(std::span<const std::uint8_t> packet);
} // namespace Network::Packets
