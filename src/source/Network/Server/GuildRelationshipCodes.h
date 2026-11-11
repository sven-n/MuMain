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

// The failure results the guild packets share (E1, E6), under the original server's names. 0x10 means a missing
// guild in E1 and the Castle Siege restriction in E6.
enum GUILD_REQ_COMMON_RESULT
{
    GUILD_ANS_NOTEXIST_GUILD = 0x10,
    GUILD_ANS_UNIONFAIL_BY_CASTLE = 0x10,
    GUILD_ANS_NOTEXIST_PERMISSION = 0x11,
    GUILD_ANS_NOTEXIST_EXTRA_STATUS = 0x12,
    GUILD_ANS_NOTEXIST_EXTRA_TYPE = 0x13,
    GUILD_ANS_EXIST_RELATIONSHIP_UNION = 0x15,
    GUILD_ANS_EXIST_RELATIONSHIP_RIVAL = 0x16,
    GUILD_ANS_EXIST_UNION = 0x17,
    GUILD_ANS_EXIST_RIVAL = 0x18,
    GUILD_ANS_NOTEXIST_UNION = 0x19,
    GUILD_ANS_NOTEXIST_RIVAL = 0x1A,
    GUILD_ANS_NOT_UNION_MASTER = 0x1B,
    GUILD_ANS_NOT_GUILD_RIVAL = 0x1C,
    GUILD_ANS_CANNOT_BE_UNION_MASTER_GUILD = 0x1D,
    GUILD_ANS_EXCEED_MAX_UNION_MEMBER = 0x1E,
    GUILD_ANS_CANCEL_REQUEST = 0x20,
    GUILD_ANS_UNION_MASTER_NOT_GENS = 0xA1,
    GUILD_ANS_GUILD_MASTER_NOT_GENS = 0xA2,
    GUILD_ANS_UNION_MASTER_DISAGREE_GENS = 0xA3,
};
