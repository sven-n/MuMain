#include "stdafx.h"

#include "doctest.h"

#include "Engine/Object/ZzzObject.h"
#include "Network/Server/SocketSystem.h"
#include "Network/Server/WSclient.h"
#include "UI/Inventory/ItemMng.h"

#include <array>
#include <span>

// An item as the server sends it (OpenMU's ItemSerializerExtended): five fixed bytes (group and number, level,
// durability, the option flags), then one byte for each option the flags name, in bit order, and the sockets. The
// items here are a Bone Blade (0/22) +9.

// The socket byte's low half has room for 15 sockets; an item has at most five.
TEST_CASE("An item from the server keeps at most five sockets [network][items]")
{
    // Bonus option 3 and 15 sockets, and 15 socket bytes after it.
    std::array<BYTE, 21> itemData{0x00, 22, 9, 40, static_cast<BYTE>(ItemOptionFlags::HasSockets), 0x3F};
    for (size_t i = 6; i < itemData.size(); ++i)
    {
        itemData[i] = static_cast<BYTE>(0x10 + i);
    }
    // The next item of a packet starts after all 15.
    CHECK(CalcItemLength(itemData) == static_cast<int>(itemData.size()));

    const ItemCreationParams params = ParseItemData(itemData);
    CHECK(params.SocketCount == MAX_SOCKETS);
    for (size_t i = 0; i < MAX_SOCKETS; ++i)
    {
        CAPTURE(i);
        CHECK(params.SocketOptions[i] == itemData[6 + i]);
    }
    // A sixth socket byte would land on the bonus option, the next ones on the fields after it.
    CHECK(params.SocketBonusOption == 3);
    CHECK_FALSE(params.WithExpiration);
    CHECK_FALSE(params.IsExpired);
}

// The flags name the additional option, the excellent options and sockets, but the item ends after the option's
// byte. The bytes after it in memory belong to something else.
TEST_CASE("An item cut short reads no byte past its end [network][items]")
{
    const auto flags = ItemOptionFlags::HasOption | ItemOptionFlags::HasExcellent | ItemOptionFlags::HasSockets;
    const std::array<BYTE, 9> memory{0x00, 22, 9, 40, static_cast<BYTE>(flags), 0x02, 0x3F, 0xEE, 0xEE};
    const std::span<const BYTE> itemData(memory.data(), 6);

    const ItemCreationParams params = ParseItemData(itemData);
    CHECK(params.OptionLevel == 2);
    CHECK(params.ExcellentFlags == 0);
    CHECK(params.SocketCount == 0);
    CHECK(params.SocketBonusOption == 0);

    // The callers cut the item out of its packet with this length (itemData.subspan(0, length)): it stays inside the
    // bytes there are, also when they end inside the socket bytes.
    CHECK(CalcItemLength(itemData) == 6);
    CHECK(CalcItemLength(std::span<const BYTE>(memory.data(), 8)) == 8);
}

// Fewer bytes than the five fixed ones, so not even the flags byte is there: the length is what there is.
TEST_CASE("An item shorter than its five fixed bytes has the length of what there is [network][items]")
{
    const std::array<BYTE, 3> itemStart{0x00, 22, 9};
    CHECK(CalcItemLength(itemStart) == 3);
    CHECK(CalcItemLength(std::span<const BYTE>()) == 0);
}

// The level byte carries the period state of a time-limited (rented) item in its two highest bits: 0x40 for an item
// which expires, 0x80 once it expired. The level is in the low 6 bits.
TEST_CASE("The level byte of an item carries the period state of a rented item [network][items]")
{
    const std::array<BYTE, 5> permanent{0x00, 22, 9, 40, 0};
    const ItemCreationParams permanentParams = ParseItemData(permanent);
    CHECK(permanentParams.Level == 9);
    CHECK_FALSE(permanentParams.WithExpiration);
    CHECK_FALSE(permanentParams.IsExpired);

    const std::array<BYTE, 5> rented{0x00, 22, 9 | 0x40, 40, 0};
    const ItemCreationParams rentedParams = ParseItemData(rented);
    CHECK(rentedParams.Level == 9);
    CHECK(rentedParams.WithExpiration);
    CHECK_FALSE(rentedParams.IsExpired);

    const std::array<BYTE, 5> expired{0x00, 22, 9 | 0x40 | 0x80, 40, 0};
    const ItemCreationParams expiredParams = ParseItemData(expired);
    CHECK(expiredParams.Level == 9);
    CHECK(expiredParams.WithExpiration);
    CHECK(expiredParams.IsExpired);
}

// A drop on the ground keeps the sockets of its item data, and a reused drop slot doesn't keep the previous drop's.
TEST_CASE("A dropped item keeps the sockets of its item data [network][items]")
{
    // Two sockets: the first one empty, the second one with a seed sphere.
    constexpr BYTE SeedSphere = 2 * SEASON4A::MAX_SOCKET_OPTION + 7;
    const std::array<BYTE, 8> itemData{0x00, 22, 9, 40, static_cast<BYTE>(ItemOptionFlags::HasSockets), 0x02,
                                       SOCKET_EMPTY, SeedSphere};
    const ItemCreationParams params = ParseItemData(itemData);

    ITEM_t drop{};
    drop.Item.SocketCount = MAX_SOCKETS;
    drop.Item.SocketSeedID[0] = 3;
    drop.Item.SocketSphereLv[0] = 4;
    vec3_t position{};
    CreateItemDrop(&drop, params, position, false);

    CHECK(drop.Item.SocketCount == 2);
    CHECK(drop.Item.SocketSeedID[0] == SOCKET_EMPTY);
    CHECK(drop.Item.SocketSphereLv[0] == 0);
    CHECK(drop.Item.SocketSeedID[1] == 7);
    CHECK(drop.Item.SocketSphereLv[1] == 3);
}
