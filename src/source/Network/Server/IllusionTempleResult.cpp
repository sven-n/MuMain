#include "Network/Server/IllusionTempleResult.h"

#include <bit>
#include <cstring>

namespace Network::Packets
{
namespace
{
constexpr std::size_t SizeOffset = 1;
constexpr std::size_t HeadCodeOffset = 2;
constexpr std::size_t SubCodeOffset = 3;
constexpr std::size_t AlliedPointsOffset = 4;
constexpr std::size_t IllusionPointsOffset = 5;
constexpr std::size_t PlayerCountOffset = 6;

IllusionTempleResult::Player ParsePlayer(std::span<const std::uint8_t, IllusionTempleResult::PlayerSize> row)
{
    constexpr std::size_t MapOffset = 10;
    constexpr std::size_t TeamOffset = 11;
    constexpr std::size_t ClassOffset = 12;
    constexpr std::size_t ExperienceOffset = 16; // Bytes 13..15 are alignment padding on the wire.
    constexpr int ByteBits = 8;
    std::uint32_t experience = 0;
    for (std::size_t i = 0; i < sizeof(experience); ++i)
        experience |= static_cast<std::uint32_t>(row[ExperienceOffset + i]) << (i * ByteBits);

    IllusionTempleResult::Player player{};
    std::memcpy(player.name.data(), row.data(), IllusionTempleResult::NameSize);
    player.mapNumber = row[MapOffset];
    player.team = row[TeamOffset];
    player.playerClass = DecodeIllusionTempleClass(row[ClassOffset]);
    player.addedExperience = std::bit_cast<std::int32_t>(experience);
    return player;
}
} // namespace

std::optional<IllusionTempleResult> ParseIllusionTempleResult(std::span<const std::uint8_t> packet)
{
    constexpr std::uint8_t HeaderType = 0xC1;
    constexpr std::uint8_t HeadCode = 0xBF;
    constexpr std::uint8_t SubCode = 0x04;
    if (packet.size() < IllusionTempleResult::HeaderSize || packet[0] != HeaderType ||
        packet[HeadCodeOffset] != HeadCode || packet[SubCodeOffset] != SubCode)
        return std::nullopt;

    const std::size_t declaredSize = packet[SizeOffset];
    if (declaredSize < IllusionTempleResult::HeaderSize || declaredSize > packet.size())
        return std::nullopt;

    const std::uint8_t playerCount = packet[PlayerCountOffset];
    const std::size_t completeRows =
        (declaredSize - IllusionTempleResult::HeaderSize) / IllusionTempleResult::PlayerSize;
    if (playerCount > completeRows || playerCount > IllusionTempleResult::MaximumPlayers)
        return std::nullopt;

    IllusionTempleResult result{};
    result.alliedPoints = packet[AlliedPointsOffset];
    result.illusionPoints = packet[IllusionPointsOffset];
    result.playerCount = playerCount;
    for (std::size_t i = 0; i < playerCount; ++i)
    {
        const std::size_t offset = IllusionTempleResult::HeaderSize + i * IllusionTempleResult::PlayerSize;
        result.players[i] = ParsePlayer(
            packet.subspan(offset, IllusionTempleResult::PlayerSize).first<IllusionTempleResult::PlayerSize>());
    }
    return result;
}
} // namespace Network::Packets
