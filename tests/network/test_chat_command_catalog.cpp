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

constexpr size_t ParameterSize = 102;

std::vector<BYTE> MakePacket(BYTE index, BYTE count, BYTE parameterCount = 0, size_t parameterSize = ParameterSize)
{
    std::vector<BYTE> packet(PacketSize + static_cast<size_t>(parameterCount) * parameterSize);
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

namespace
{
constexpr size_t ParameterWithHintsSize = 152;

struct Hints
{
    const char* Name = "";
    BYTE ValueReference = 0;
    bool HasRange = false;
    int64_t Minimum = 0;
    int64_t Maximum = 0;
    const char* GroupWith = "";
};

void WriteInt64LittleEndian(BYTE* target, int64_t value)
{
    auto bits = static_cast<uint64_t>(value);
    for (size_t i = 0; i < sizeof(bits); ++i)
    {
        target[i] = static_cast<BYTE>(bits & 0xFF);
        bits >>= 8;
    }
}

void WriteString(BYTE* target, const char* text)
{
    for (size_t i = 0; text[i] != '\0'; ++i)
    {
        target[i] = static_cast<BYTE>(text[i]);
    }
}

std::vector<BYTE> MakePacketWithHints(const std::vector<Hints>& parameters)
{
    auto packet = MakePacket(0, 1, static_cast<BYTE>(parameters.size()), ParameterWithHintsSize);
    for (size_t i = 0; i < parameters.size(); ++i)
    {
        auto* entry = packet.data() + PacketSize + i * ParameterWithHintsSize;
        WriteString(entry + 2, parameters[i].Name);
        entry[102] = parameters[i].ValueReference;
        entry[103] = parameters[i].HasRange ? 1 : 0;
        WriteInt64LittleEndian(entry + 104, parameters[i].Minimum);
        WriteInt64LittleEndian(entry + 112, parameters[i].Maximum);
        WriteString(entry + 120, parameters[i].GroupWith);
    }

    return packet;
}

const GameLogic::Commands::ChatCommand& AddSingleCommand(const std::vector<BYTE>& packet)
{
    auto& catalog = GameLogic::Commands::Catalog();
    catalog.Reset();
    REQUIRE(catalog.AddFromPacket(packet.data(), static_cast<int32_t>(packet.size())));
    return catalog.GetCommands().front();
}
} // namespace

TEST_CASE("chat command catalog reads the hints of the parameters")
{
    using GameLogic::Commands::ChatCommandParameter;
    using GameLogic::Commands::ChatCommandValueReference;

    const auto packet = MakePacketWithHints({
        {"Group", 7, true, 0, 255, ""},
        {"Number", 8, true, -32768, 32767, "Group"},
    });

    const auto& parameters = AddSingleCommand(packet).Parameters;
    REQUIRE(parameters.size() == 2);
    CHECK(parameters[0].Name == L"Group");
    CHECK(parameters[0].ValueReference == ChatCommandValueReference::ItemGroup);
    CHECK(parameters[0].GroupWithIndex == ChatCommandParameter::NoGroup);
    CHECK(parameters[0].HasRange);
    CHECK(parameters[0].Maximum == 255);
    CHECK(parameters[1].ValueReference == ChatCommandValueReference::ItemNumber);
    CHECK(parameters[1].GroupWithIndex == 0);
    CHECK(parameters[1].Minimum == -32768);
    CHECK(parameters[1].Maximum == 32767);
}

TEST_CASE("chat command catalog treats unknown value references and groups as none")
{
    using GameLogic::Commands::ChatCommandParameter;
    using GameLogic::Commands::ChatCommandValueReference;

    // The parameter can't be grouped with itself or a missing one, and a
    // minimum above the maximum isn't a range.
    const auto packet = MakePacketWithHints({
        {"Self", 0xEE, true, 5, 1, "Self"},
        {"Other", 0, false, 0, 0, "Missing"},
    });

    const auto& parameters = AddSingleCommand(packet).Parameters;
    CHECK(parameters[0].ValueReference == ChatCommandValueReference::None);
    CHECK(parameters[0].GroupWithIndex == ChatCommandParameter::NoGroup);
    CHECK_FALSE(parameters[0].HasRange);
    CHECK(parameters[1].GroupWithIndex == ChatCommandParameter::NoGroup);
}

TEST_CASE("chat command catalog still reads parameters without hints")
{
    using GameLogic::Commands::ChatCommandValueReference;

    // Older servers send the entries without the hints at their end.
    auto packet = MakePacket(0, 1, 2);
    packet[PacketSize + ParameterSize + 1] = 1;

    const auto& parameters = AddSingleCommand(packet).Parameters;
    REQUIRE(parameters.size() == 2);
    CHECK(parameters[1].Type == GameLogic::Commands::ChatCommandParameterType::Number);
    CHECK(parameters[1].ValueReference == ChatCommandValueReference::None);
    CHECK_FALSE(parameters[1].HasRange);
}

TEST_CASE("chat command catalog rejects parameters which don't fit into the message")
{
    auto& catalog = GameLogic::Commands::Catalog();
    catalog.Reset();
    auto packet = MakePacket(0, 1, 2);
    packet.pop_back();

    CHECK_FALSE(catalog.AddFromPacket(packet.data(), static_cast<int32_t>(packet.size())));
}

TEST_CASE("chat command parameter accepts only numbers within its range")
{
    GameLogic::Commands::ChatCommandParameter parameter;
    CHECK(parameter.Accepts(L"anything"));

    parameter.HasRange = true;
    parameter.Minimum = 1;
    parameter.Maximum = 5;
    CHECK(parameter.Accepts(L""));
    CHECK(parameter.Accepts(L"1"));
    CHECK(parameter.Accepts(L"5"));
    CHECK_FALSE(parameter.Accepts(L"0"));
    CHECK_FALSE(parameter.Accepts(L"6"));
    CHECK_FALSE(parameter.Accepts(L"3a"));
    CHECK_FALSE(parameter.Accepts(L"99999999999999999999"));
}
