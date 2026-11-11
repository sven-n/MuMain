#pragma once

#include <cstdint>

// The codes in the E5, E6 and EB guild relationship packets, named as in OpenMU's ServerToClientPackets.xml.
namespace Network::Packets::GuildRelationship
{
enum Type : std::uint8_t
{
    Alliance = 0x01,
    Hostility = 0x02,
};

enum Request : std::uint8_t
{
    Join = 0x01,
    Leave = 0x02,
    // The original server's alliance disband; OpenMU doesn't send it.
    DisbandAlliance = 0x10,
};

enum Result : std::uint8_t
{
    Failed = 0x00,
    Success = 0x01,
};
} // namespace Network::Packets::GuildRelationship
