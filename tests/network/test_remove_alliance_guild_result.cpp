// Tests of reading OpenMU's EB 01 remove-alliance-guild result.

#include "Network/Server/RemoveAllianceGuildResult.h"

#include "doctest.h"

#include <array>
#include <cstdint>

using Network::Packets::ParseRemoveAllianceGuildResult;

TEST_CASE("an EB 01 result reads whether the guild was removed [network][guild]")
{
    constexpr std::array<std::uint8_t, 7> succeeded{0xC1, 0x07, 0xEB, 0x01, 0x01, 0x02, 0x01};
    const auto removed = ParseRemoveAllianceGuildResult(succeeded);
    REQUIRE(removed.has_value());
    CHECK(removed->succeeded);

    constexpr std::array<std::uint8_t, 7> failed{0xC1, 0x07, 0xEB, 0x01, 0x00, 0x02, 0x01};
    const auto notRemoved = ParseRemoveAllianceGuildResult(failed);
    REQUIRE(notRemoved.has_value());
    CHECK_FALSE(notRemoved->succeeded);
}

TEST_CASE("a short EB 01 packet is rejected [network][guild]")
{
    constexpr std::array<std::uint8_t, 5> truncated{0xC1, 0x05, 0xEB, 0x01, 0x01};
    CHECK_FALSE(ParseRemoveAllianceGuildResult(truncated).has_value());

    constexpr std::array<std::uint8_t, 7> declaredShort{0xC1, 0x05, 0xEB, 0x01, 0x01, 0x02, 0x01};
    CHECK_FALSE(ParseRemoveAllianceGuildResult(declaredShort).has_value());
}

TEST_CASE("a packet with another subcode is not read as EB 01 [network][guild]")
{
    constexpr std::array<std::uint8_t, 7> packet{0xC1, 0x07, 0xEB, 0x02, 0x01, 0x02, 0x01};
    CHECK_FALSE(ParseRemoveAllianceGuildResult(packet).has_value());
}
