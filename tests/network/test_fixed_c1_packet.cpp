// Tests of the shared check for fixed-size C1 packets.

#include "Network/Server/FixedC1Packet.h"

#include "doctest.h"

#include <array>
#include <cstdint>

using Network::Packets::IsFixedC1Packet;
using Network::Packets::ReadBigEndianU16;

TEST_CASE("a C1 packet with the head code and enough bytes passes [network]")
{
    constexpr std::array<std::uint8_t, 5> packet{0xC1, 0x05, 0xE6, 0x01, 0x02};

    CHECK(IsFixedC1Packet(packet, 0xE6, 5));
    CHECK(IsFixedC1Packet(packet, 0xE6, 0x01, 5));
}

TEST_CASE("a packet that is short in the span or in its declared size fails [network]")
{
    constexpr std::array<std::uint8_t, 4> truncated{0xC1, 0x05, 0xE6, 0x01};
    CHECK_FALSE(IsFixedC1Packet(truncated, 0xE6, 5));

    constexpr std::array<std::uint8_t, 5> declaredShort{0xC1, 0x04, 0xE6, 0x01, 0x02};
    CHECK_FALSE(IsFixedC1Packet(declaredShort, 0xE6, 5));

    constexpr std::array<std::uint8_t, 3> noSubCode{0xC1, 0x03, 0xEB};
    CHECK_FALSE(IsFixedC1Packet(noSubCode, 0xEB, 0x01, 3));
}

TEST_CASE("a packet with another header type, head code or subcode fails [network]")
{
    constexpr std::array<std::uint8_t, 5> c3{0xC3, 0x05, 0xE6, 0x01, 0x02};
    CHECK_FALSE(IsFixedC1Packet(c3, 0xE6, 5));

    constexpr std::array<std::uint8_t, 5> otherHeadCode{0xC1, 0x05, 0xE5, 0x01, 0x02};
    CHECK_FALSE(IsFixedC1Packet(otherHeadCode, 0xE6, 5));

    constexpr std::array<std::uint8_t, 5> otherSubCode{0xC1, 0x05, 0xEB, 0x02, 0x02};
    CHECK_FALSE(IsFixedC1Packet(otherSubCode, 0xEB, 0x01, 5));
}

TEST_CASE("ReadBigEndianU16 reads the high byte first [network]")
{
    constexpr std::array<std::uint8_t, 3> bytes{0x00, 0x12, 0x34};
    CHECK(ReadBigEndianU16(bytes, 1) == 0x1234);
}
