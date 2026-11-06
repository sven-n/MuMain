#pragma once

#include "Network/Server/IllusionTempleClass.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace Network::Packets
{
// OpenMU's ServerToClientPackets.xml: C1 BF04, seven-byte header and 20-byte player rows.
// Rows own their ten-byte UTF-8 name plus a terminator; no receive-buffer lifetime escapes parsing.
struct IllusionTempleResult
{
    static constexpr std::size_t HeaderSize = 7;
    static constexpr std::size_t PlayerSize = 20;
    static constexpr std::size_t NameSize = 10;
    static constexpr std::size_t MaximumPlayers = (UINT8_MAX - HeaderSize) / PlayerSize;

    struct Player
    {
        std::array<char, NameSize + 1> name{};
        std::uint8_t mapNumber;
        std::uint8_t team;
        std::optional<SERVER_CLASS_TYPE> playerClass;
        std::int32_t addedExperience;
    };

    std::uint8_t alliedPoints;
    std::uint8_t illusionPoints;
    std::uint8_t playerCount;
    std::array<Player, MaximumPlayers> players;
};

// Rejects an incomplete header or any counted row outside the declared C1 length or supplied span.
// An unknown class remains nullopt in a complete row so the receiver can report its fallback.
[[nodiscard]] std::optional<IllusionTempleResult> ParseIllusionTempleResult(std::span<const std::uint8_t> packet);
} // namespace Network::Packets
