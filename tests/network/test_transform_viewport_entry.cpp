// doctest unit tests for the bounds of head code 45 entries, the transformed
// players that come into view.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "transform_viewport"

#include "doctest.h"

#include "Network/Server/TransformViewportEntry.h"

#include <cstddef>
#include <vector>

using Network::Viewport::TransformViewportEntryLength;

namespace
{
constexpr std::size_t HeaderLength = sizeof(PWHEADER_DEFAULT_WORD);
constexpr std::size_t FixedEntryLength = offsetof(PCREATE_TRANSFORM_EXTENDED, s_BuffEffectState);
constexpr std::size_t BuffCountIndex = offsetof(PCREATE_TRANSFORM_EXTENDED, s_BuffCount);

// A packet with one entry per buff count, each carrying only its own buffs.
std::vector<BYTE> TransformPacket(const std::vector<BYTE>& buffCounts)
{
    std::vector<BYTE> packet(HeaderLength);
    for (const BYTE buffCount : buffCounts)
    {
        std::vector<BYTE> entry(FixedEntryLength + buffCount);
        entry[BuffCountIndex] = buffCount;
        packet.insert(packet.end(), entry.begin(), entry.end());
    }

    return packet;
}
} // namespace

TEST_CASE("An entry is as long as its fixed fields and the buffs it carries [network][transform_viewport]")
{
    for (const BYTE buffCount : {0, 1, 15, MAX_BUFF_SLOT_INDEX})
    {
        CAPTURE(static_cast<int>(buffCount));
        const std::vector<BYTE> packet = TransformPacket({buffCount});

        const auto length = TransformViewportEntryLength(packet, HeaderLength);

        REQUIRE(length.has_value());
        CHECK(*length == FixedEntryLength + buffCount);
        CHECK(HeaderLength + *length == packet.size());
    }
}

TEST_CASE("Entries follow each other by their lengths [network][transform_viewport]")
{
    const std::vector<BYTE> packet = TransformPacket({2, 0, 5});
    std::size_t offset = HeaderLength;

    for (const BYTE buffCount : {2, 0, 5})
    {
        const auto length = TransformViewportEntryLength(packet, offset);
        REQUIRE(length.has_value());
        CHECK(*length == FixedEntryLength + buffCount);
        offset += *length;
    }

    CHECK(offset == packet.size());
    CHECK_FALSE(TransformViewportEntryLength(packet, offset).has_value());
}

TEST_CASE("A truncated entry is refused [network][transform_viewport]")
{
    const std::vector<BYTE> packet = TransformPacket({3});

    const std::vector<BYTE> withoutLastBuff(packet.begin(), packet.end() - 1);
    CHECK_FALSE(TransformViewportEntryLength(withoutLastBuff, HeaderLength).has_value());

    const std::vector<BYTE> withoutBuffs(packet.begin(), packet.begin() + HeaderLength + FixedEntryLength - 1);
    CHECK_FALSE(TransformViewportEntryLength(withoutBuffs, HeaderLength).has_value());

    CHECK_FALSE(TransformViewportEntryLength(packet, packet.size() + 1).has_value());
}

TEST_CASE("An entry with more buffs than a character can have is refused [network][transform_viewport]")
{
    const std::vector<BYTE> packet = TransformPacket({MAX_BUFF_SLOT_INDEX + 1});

    CHECK_FALSE(TransformViewportEntryLength(packet, HeaderLength).has_value());
}
