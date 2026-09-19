#include "Core/Platform/WinCompat.h"
#include "Data/Translation/MultiLanguage.h"
#include "GameLogic/Commands/ChatCommandCatalog.h"

#include "doctest.h"

#include <algorithm>
#include <vector>

int32_t CMultiLanguage::ConvertFromUtf8(wchar_t* target, const char* source, int maxSourceLength)
{
    int32_t length = 0;
    while (length < maxSourceLength && source[length] != '\0')
    {
        target[length] = static_cast<unsigned char>(source[length]);
        ++length;
    }

    target[length] = L'\0';
    return length;
}

namespace
{
constexpr size_t PacketSize = 345;
constexpr size_t IndexOffset = 5;
constexpr size_t CountOffset = 6;
constexpr size_t ParameterTypeOffset = 346;
constexpr size_t CommandOffset = 9;

std::vector<BYTE> MakePacket(BYTE index, BYTE count, BYTE parameterCount = 0)
{
    constexpr size_t ParameterSize = 102;
    std::vector<BYTE> packet(PacketSize + static_cast<size_t>(parameterCount) * ParameterSize);
    packet[IndexOffset] = index;
    packet[CountOffset] = count;
    packet[8] = parameterCount;

    constexpr char Command[] = "/test";
    std::copy(std::begin(Command), std::end(Command), packet.begin() + CommandOffset);
    return packet;
}
} // namespace

TEST_CASE("chat command catalog rejects an out-of-order list")
{
    auto& catalog = GameLogic::Commands::Catalog();
    catalog.Reset();
    const auto packet = MakePacket(1, 2);

    CHECK_FALSE(catalog.AddFromPacket(packet.data(), static_cast<int32_t>(packet.size())));
    CHECK(catalog.GetCommands().empty());
    CHECK_FALSE(catalog.IsAvailable());
}

TEST_CASE("chat command catalog accepts a complete ordered list")
{
    auto& catalog = GameLogic::Commands::Catalog();
    catalog.Reset();
    const auto first = MakePacket(0, 2);
    const auto second = MakePacket(1, 2);

    CHECK(catalog.AddFromPacket(first.data(), static_cast<int32_t>(first.size())));
    CHECK_FALSE(catalog.IsAvailable());
    CHECK(catalog.AddFromPacket(second.data(), static_cast<int32_t>(second.size())));
    CHECK(catalog.IsAvailable());
    CHECK(catalog.GetCommands().size() == 2);
}

TEST_CASE("chat command catalog rejects a changed list count")
{
    auto& catalog = GameLogic::Commands::Catalog();
    catalog.Reset();
    const auto first = MakePacket(0, 2);
    const auto second = MakePacket(1, 3);

    CHECK(catalog.AddFromPacket(first.data(), static_cast<int32_t>(first.size())));
    CHECK_FALSE(catalog.AddFromPacket(second.data(), static_cast<int32_t>(second.size())));
    CHECK_FALSE(catalog.IsAvailable());
    CHECK(catalog.GetCommands().size() == 1);
}

TEST_CASE("chat command catalog rejects unknown parameter types")
{
    auto& catalog = GameLogic::Commands::Catalog();
    catalog.Reset();
    auto packet = MakePacket(0, 1, 1);
    packet[ParameterTypeOffset] = 0xFF;

    CHECK_FALSE(catalog.AddFromPacket(packet.data(), static_cast<int32_t>(packet.size())));
    CHECK(catalog.GetCommands().empty());
}
