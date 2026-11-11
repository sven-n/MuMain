#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace Network::Packets
{
// Checks for fixed-size C1 packets: byte 0 is 0xC1, byte 1 the declared size, byte 2 the head code and,
// when the packet has one, byte 3 the subcode.

// True when the packet is a C1 packet with this head code and both the span and the declared size hold
// at least `size` bytes.
[[nodiscard]] constexpr bool IsFixedC1Packet(std::span<const std::uint8_t> packet, std::uint8_t headCode,
                                             std::size_t size)
{
    constexpr std::uint8_t C1 = 0xC1;
    constexpr std::size_t SizeOffset = 1;
    constexpr std::size_t HeadCodeOffset = 2;
    return size > HeadCodeOffset && packet.size() >= size && packet[0] == C1 && packet[SizeOffset] >= size &&
           packet[HeadCodeOffset] == headCode;
}

// The same, for a packet whose byte 3 is a subcode.
[[nodiscard]] constexpr bool IsFixedC1Packet(std::span<const std::uint8_t> packet, std::uint8_t headCode,
                                             std::uint8_t subCode, std::size_t size)
{
    constexpr std::size_t SubCodeOffset = 3;
    return size > SubCodeOffset && IsFixedC1Packet(packet, headCode, size) && packet[SubCodeOffset] == subCode;
}

// Reads the two bytes at `offset` as a big-endian value. The caller has checked the length.
[[nodiscard]] constexpr std::uint16_t ReadBigEndianU16(std::span<const std::uint8_t> packet, std::size_t offset)
{
    return static_cast<std::uint16_t>((packet[offset] << 8) | packet[offset + 1]);
}
} // namespace Network::Packets
