// Tests of reading OpenMU's E5 guild relationship request.

#include "Network/Server/GuildRelationshipRequest.h"

#include "doctest.h"

#include <array>
#include <cstdint>

using Network::Packets::ParseGuildRelationshipRequest;

TEST_CASE("an E5 alliance request reads one byte per field and a big-endian requester key [network][guild]")
{
    constexpr std::array<std::uint8_t, 7> packet{0xC1, 0x07, 0xE5, 0x01, 0x01, 0x12, 0x34};

    const auto request = ParseGuildRelationshipRequest(packet);

    REQUIRE(request.has_value());
    CHECK(request->relationshipType == 1); // Alliance
    CHECK(request->requestType == 1);      // Join
    CHECK(request->requesterKey == 0x1234);
}

TEST_CASE("a short E5 packet is rejected [network][guild]")
{
    constexpr std::array<std::uint8_t, 6> truncated{0xC1, 0x06, 0xE5, 0x02, 0x02, 0x12};
    CHECK_FALSE(ParseGuildRelationshipRequest(truncated).has_value());

    constexpr std::array<std::uint8_t, 7> declaredShort{0xC1, 0x06, 0xE5, 0x02, 0x02, 0x12, 0x34};
    CHECK_FALSE(ParseGuildRelationshipRequest(declaredShort).has_value());
}

TEST_CASE("a packet with another head code is not read as E5 [network][guild]")
{
    constexpr std::array<std::uint8_t, 7> packet{0xC1, 0x07, 0xE6, 0x01, 0x01, 0x12, 0x34};
    CHECK_FALSE(ParseGuildRelationshipRequest(packet).has_value());
}
