#include "Integration/Discord/IpcFrame.h"

namespace
{
constexpr int BitsPerByte = 8;
constexpr std::uint32_t ByteMask = 0xFF;

void AppendLittleEndian(std::string& out, std::uint32_t value)
{
    for (std::size_t byte = 0; byte < sizeof(value); ++byte)
    {
        out.push_back(static_cast<char>((value >> (byte * BitsPerByte)) & ByteMask));
    }
}

std::uint32_t ReadLittleEndian(const std::uint8_t* bytes)
{
    std::uint32_t value = 0;
    for (std::size_t byte = 0; byte < sizeof(value); ++byte)
    {
        value |= static_cast<std::uint32_t>(bytes[byte]) << (byte * BitsPerByte);
    }
    return value;
}
} // namespace

namespace Integration::Discord::Ipc
{
std::string EncodeFrame(Opcode opcode, std::string_view payload)
{
    std::string frame;
    frame.reserve(HeaderBytes + payload.size());
    AppendLittleEndian(frame, static_cast<std::uint32_t>(opcode));
    AppendLittleEndian(frame, static_cast<std::uint32_t>(payload.size()));
    frame.append(payload);
    return frame;
}

std::optional<FrameHeader> DecodeHeader(const std::array<std::uint8_t, HeaderBytes>& bytes)
{
    const std::uint32_t opcode = ReadLittleEndian(bytes.data());
    const std::uint32_t length = ReadLittleEndian(bytes.data() + sizeof(std::uint32_t));
    if (opcode > static_cast<std::uint32_t>(Opcode::Pong) || length > MaxPayloadBytes)
    {
        return std::nullopt;
    }

    return FrameHeader{static_cast<Opcode>(opcode), length};
}
} // namespace Integration::Discord::Ipc
