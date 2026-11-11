// Tests of reading OpenMU's E6 guild relationship result.

#include "Network/Server/GuildRelationshipResult.h"

#include "doctest.h"

#include <array>
#include <cstdint>

using Network::Packets::ParseGuildRelationshipResult;

TEST_CASE("an E6 result reads one byte per field and a big-endian character key [network][guild]")
{
    constexpr std::array<std::uint8_t, 8> packet{0xC1, 0x08, 0xE6, 0x01, 0x02, 0x01, 0x12, 0x34};

    const auto result = ParseGuildRelationshipResult(packet);

    REQUIRE(result.has_value());
    CHECK(result->relationshipType == 1); // Alliance
    CHECK(result->requestType == 2);      // Leave
    CHECK(result->result == 1);           // Success
    CHECK(result->characterKey == 0x1234);
}

TEST_CASE("a short E6 packet is rejected [network][guild]")
{
    constexpr std::array<std::uint8_t, 7> truncated{0xC1, 0x07, 0xE6, 0x01, 0x02, 0x01, 0x12};
    CHECK_FALSE(ParseGuildRelationshipResult(truncated).has_value());

    constexpr std::array<std::uint8_t, 8> declaredShort{0xC1, 0x07, 0xE6, 0x01, 0x02, 0x01, 0x12, 0x34};
    CHECK_FALSE(ParseGuildRelationshipResult(declaredShort).has_value());
}

TEST_CASE("a packet with another head code is not read as E6 [network][guild]")
{
    constexpr std::array<std::uint8_t, 8> packet{0xC1, 0x08, 0xE5, 0x01, 0x02, 0x01, 0x12, 0x34};
    CHECK_FALSE(ParseGuildRelationshipResult(packet).has_value());
}
