#include "Network/Server/IllusionTempleResult.h"

#include "doctest.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <ostream>
#include <span>
#include <string_view>
#include <vector>

namespace
{
using Network::Packets::DecodeIllusionTempleClass;
using Network::Packets::ParseIllusionTempleResult;

// Expected values from OpenMU's ToIllusionTempleCharacterClass and CharacterClassNumber.
constexpr std::array<std::array<std::uint8_t, 2>, 18> Classes{{
    {0x00, 0},
    {0x20, 2},
    {0x30, 3},
    {0x01, 4},
    {0x21, 6},
    {0x31, 7},
    {0x02, 8},
    {0x22, 10},
    {0x32, 11},
    {0x03, 12},
    {0x33, 13},
    {0x04, 16},
    {0x34, 17},
    {0x05, 20},
    {0x25, 22},
    {0x35, 23},
    {0x06, 24},
    {0x36, 25},
}};

// C1 BF04 with one 20-byte Blade Master row, including nonzero alignment bytes.
constexpr std::array<std::uint8_t, 27> BladeMasterPacket{0xC1, 27,   0xBF, 4,    2,    0,    1,    'B',  'l',
                                                         'a',  'd',  'e',  0,    0,    0,    0,    0,    45,
                                                         0,    0x31, 0xA1, 0xB2, 0xC3, 0x87, 0xD6, 0x12, 0};
constexpr std::size_t HeaderSize = 7;
constexpr std::size_t PlayerSize = 20;
constexpr std::size_t CountOffset = 6;
constexpr std::size_t ClassOffset = HeaderSize + 12;
constexpr std::size_t ExperienceOffset = HeaderSize + 16;

std::vector<std::uint8_t> TwoPlayers()
{
    std::vector<std::uint8_t> packet(BladeMasterPacket.begin(), BladeMasterPacket.end());
    const std::array<std::uint8_t, PlayerSize> second{'W', 'w', 'w',  'w',  'w',  'w',  'w',  'w',  'w',  'w',
                                                      56,  1,   0x35, 0xD4, 0xE5, 0xF6, 0x78, 0x56, 0x34, 0x12};
    packet.insert(packet.end(), second.begin(), second.end());
    packet[1] = static_cast<std::uint8_t>(packet.size());
    packet[CountOffset] = 2;
    return packet;
}
} // namespace

TEST_CASE("Temple result decodes all 18 OpenMU packed classes [network][temple-result]")
{
    for (const auto& [wireClass, serverClass] : Classes)
    {
        CAPTURE(static_cast<int>(wireClass));
        const auto decoded = DecodeIllusionTempleClass(wireClass);
        REQUIRE(decoded.has_value());
        CHECK(static_cast<std::uint8_t>(*decoded) == serverClass);

        auto packet = BladeMasterPacket;
        packet[ClassOffset] = wireClass;
        const auto result = ParseIllusionTempleResult(packet);
        REQUIRE(result.has_value());
        REQUIRE(result->playerCount == 1);
        REQUIRE(result->players[0].playerClass.has_value());
        CHECK(static_cast<std::uint8_t>(*result->players[0].playerClass) == serverClass);
    }
}

TEST_CASE("Temple result reports every invalid packed class separately from Wizard [network][temple-result]")
{
    for (unsigned int value = 0; value <= UINT8_MAX; ++value)
    {
        const auto wireClass = static_cast<std::uint8_t>(value);
        if (std::any_of(Classes.begin(), Classes.end(),
                        [wireClass](const auto& entry) { return entry[0] == wireClass; }))
            continue;
        CAPTURE(value);
        CHECK_FALSE(DecodeIllusionTempleClass(wireClass).has_value());
        auto packet = BladeMasterPacket;
        packet[ClassOffset] = wireClass;
        const auto result = ParseIllusionTempleResult(packet);
        REQUIRE(result.has_value());
        CHECK_FALSE(result->players[0].playerClass.has_value());
    }
}

TEST_CASE(
    "Temple result parses complete rows with names teams classes and little endian experience [network][temple-result]")
{
    const auto packet = TwoPlayers();
    const auto result = ParseIllusionTempleResult(packet);
    REQUIRE(result.has_value());
    CHECK(result->alliedPoints == 2);
    CHECK(result->illusionPoints == 0);
    REQUIRE(result->playerCount == 2);
    const auto& first = result->players[0];
    CHECK(std::string_view(first.name.data()) == "Blade");
    CHECK(first.mapNumber == 45);
    CHECK(first.team == 0);
    CHECK(first.playerClass == BladeMaster);
    CHECK(first.addedExperience == 1234567);
    const auto& second = result->players[1];
    CHECK(std::string_view(second.name.data()) == "Wwwwwwwwww");
    CHECK(second.name[10] == '\0');
    CHECK(second.mapNumber == 56);
    CHECK(second.team == 1);
    CHECK(second.playerClass == DimensionMaster);
    CHECK(second.addedExperience == 0x12345678);
}

TEST_CASE("Temple result owns row values after the receive buffer changes [network][temple-result]")
{
    auto packet = TwoPlayers();
    const auto result = ParseIllusionTempleResult(packet);
    REQUIRE(result.has_value());
    std::fill(packet.begin(), packet.end(), 0);
    CHECK(std::string_view(result->players[0].name.data()) == "Blade");
    CHECK(result->players[1].mapNumber == 56);
    CHECK(result->players[1].addedExperience == 0x12345678);
}

TEST_CASE("Temple result ignores row padding and preserves the signed experience field [network][temple-result]")
{
    auto packet = BladeMasterPacket;
    std::fill_n(packet.begin() + ExperienceOffset, sizeof(std::int32_t), 0xFF);
    const auto result = ParseIllusionTempleResult(packet);
    REQUIRE(result.has_value());
    CHECK(result->players[0].addedExperience == -1);
    CHECK(result->players[0].playerClass == BladeMaster);
}

TEST_CASE("Temple result accepts an empty scoreboard [network][temple-result]")
{
    constexpr std::array<std::uint8_t, HeaderSize> packet{0xC1, HeaderSize, 0xBF, 4, 3, 2, 0};
    const auto result = ParseIllusionTempleResult(packet);
    REQUIRE(result.has_value());
    CHECK(result->alliedPoints == 3);
    CHECK(result->illusionPoints == 2);
    CHECK(result->playerCount == 0);
}

TEST_CASE("Temple result rejects every truncated header and row [network][temple-result]")
{
    for (std::size_t size = 0; size < BladeMasterPacket.size(); ++size)
    {
        CAPTURE(size);
        CHECK_FALSE(ParseIllusionTempleResult(std::span(BladeMasterPacket).first(size)).has_value());
        if (size >= HeaderSize)
        {
            auto packet = BladeMasterPacket;
            packet[1] = static_cast<std::uint8_t>(size);
            CHECK_FALSE(ParseIllusionTempleResult(std::span(packet).first(size)).has_value());
        }
    }
}

TEST_CASE("Temple result rejects a partial second row without returning the first [network][temple-result]")
{
    const auto packet = TwoPlayers();
    for (std::size_t size = BladeMasterPacket.size(); size < packet.size(); ++size)
    {
        CAPTURE(size);
        auto truncated = packet;
        truncated.resize(size);
        truncated[1] = static_cast<std::uint8_t>(size);
        CHECK_FALSE(ParseIllusionTempleResult(truncated).has_value());
    }
}

TEST_CASE("Temple result rejects count 255 in a seven byte packet [network][temple-result]")
{
    constexpr std::array<std::uint8_t, HeaderSize> packet{0xC1, HeaderSize, 0xBF, 4, 2, 0, UINT8_MAX};
    CHECK_FALSE(ParseIllusionTempleResult(packet).has_value());
}

TEST_CASE("Temple result bounds counted rows by the declared length not backing storage [network][temple-result]")
{
    auto packet = TwoPlayers();
    packet[1] = static_cast<std::uint8_t>(BladeMasterPacket.size());
    CHECK_FALSE(ParseIllusionTempleResult(packet).has_value());
    packet[CountOffset] = 1;
    const auto result = ParseIllusionTempleResult(packet);
    REQUIRE(result.has_value());
    CHECK(result->playerCount == 1);
    CHECK(result->players[0].playerClass == BladeMaster);
}

TEST_CASE("Temple result rejects declared lengths outside the supplied packet [network][temple-result]")
{
    auto packet = BladeMasterPacket;
    for (const auto size : {0, 1, 6, 28, 255})
    {
        CAPTURE(size);
        packet[1] = static_cast<std::uint8_t>(size);
        CHECK_FALSE(ParseIllusionTempleResult(packet).has_value());
    }
}

TEST_CASE("Temple result rejects other packet framing and opcodes [network][temple-result]")
{
    for (const std::size_t offset : {0, 2, 3})
    {
        auto packet = BladeMasterPacket;
        packet[offset] = 0;
        CHECK_FALSE(ParseIllusionTempleResult(packet).has_value());
    }
}

TEST_CASE("Temple result accepts the largest complete C1 table and rejects one more row [network][temple-result]")
{
    constexpr std::size_t MaximumPlayers = 12;
    std::vector<std::uint8_t> packet(HeaderSize + MaximumPlayers * PlayerSize);
    std::copy_n(BladeMasterPacket.begin(), HeaderSize, packet.begin());
    packet[1] = static_cast<std::uint8_t>(packet.size());
    packet[CountOffset] = MaximumPlayers;
    for (std::size_t i = 0; i < MaximumPlayers; ++i)
        std::copy(BladeMasterPacket.begin() + HeaderSize, BladeMasterPacket.end(),
                  packet.begin() + HeaderSize + i * PlayerSize);
    const auto result = ParseIllusionTempleResult(packet);
    REQUIRE(result.has_value());
    CHECK(result->playerCount == MaximumPlayers);
    CHECK(result->players[MaximumPlayers - 1].playerClass == BladeMaster);
    packet[CountOffset] = MaximumPlayers + 1;
    CHECK_FALSE(ParseIllusionTempleResult(packet).has_value());
}
