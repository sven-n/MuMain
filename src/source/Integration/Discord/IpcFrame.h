// Framing of Discord's local RPC channel (the `discord-ipc-N` pipe/socket).
//
// Every message is an 8-byte header - opcode and payload length, both 32-bit
// little-endian - followed by a UTF-8 JSON payload of that length.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace Integration::Discord::Ipc
{
enum class Opcode : std::uint32_t
{
    Handshake = 0,
    Frame = 1,
    Close = 2,
    Ping = 3,
    Pong = 4,
};

inline constexpr std::size_t HeaderBytes = 8;

// Discord's answers to the few commands the client sends are a few hundred
// bytes. A header announcing more than this is not a peer we understand, and
// trusting it would let it make the client allocate whatever it claims.
inline constexpr std::uint32_t MaxPayloadBytes = 64 * 1024;

struct FrameHeader
{
    Opcode opcode = Opcode::Frame;
    std::uint32_t length = 0;
};

[[nodiscard]] std::string EncodeFrame(Opcode opcode, std::string_view payload);

// Empty for an unknown opcode or a payload above MaxPayloadBytes.
[[nodiscard]] std::optional<FrameHeader> DecodeHeader(const std::array<std::uint8_t, HeaderBytes>& bytes);
} // namespace Integration::Discord::Ipc
