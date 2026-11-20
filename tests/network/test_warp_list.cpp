#include "Core/Platform/WinCompat.h"
#include "Data/Translation/MultiLanguage.h"
#include "GameLogic/Warps/ServerWarpList.h"
#include "GameLogic/Warps/WarpListMerge.h"

#include "doctest.h"

#include <cwchar>
#include <string>
#include <vector>

// Only ASCII in these tests, so a byte per character is enough.
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
using GameLogic::Warps::ServerWarp;
using MoveReqInfo = SEASON3B::CMoveCommandData::MOVEREQINFO;

constexpr size_t HeaderSize = 6;
constexpr size_t WarpSize = 44;

void WriteUInt16(std::vector<BYTE>& packet, size_t offset, uint16_t value)
{
    packet[offset] = static_cast<BYTE>(value & 0xFF);
    packet[offset + 1] = static_cast<BYTE>(value >> 8);
}

void WriteUInt32(std::vector<BYTE>& packet, size_t offset, uint32_t value)
{
    for (size_t i = 0; i < 4; ++i)
    {
        packet[offset + i] = static_cast<BYTE>((value >> (8 * i)) & 0xFF);
    }
}

struct PacketWarp
{
    uint16_t index;
    uint16_t mapNumber;
    uint16_t level;
    uint32_t costs;
    std::string name;
};

std::vector<BYTE> MakePacket(const std::vector<PacketWarp>& warps)
{
    std::vector<BYTE> packet(HeaderSize + warps.size() * WarpSize);
    packet[0] = 0xC2;
    packet[1] = static_cast<BYTE>(packet.size() >> 8);
    packet[2] = static_cast<BYTE>(packet.size() & 0xFF);
    packet[3] = 0xF5;
    packet[4] = 0x09;
    packet[5] = static_cast<BYTE>(warps.size());
    for (size_t i = 0; i < warps.size(); ++i)
    {
        const size_t entry = HeaderSize + i * WarpSize;
        WriteUInt16(packet, entry, warps[i].index);
        WriteUInt16(packet, entry + 2, warps[i].mapNumber);
        WriteUInt16(packet, entry + 4, warps[i].level);
        WriteUInt32(packet, entry + 8, warps[i].costs);
        std::copy(warps[i].name.begin(), warps[i].name.end(), packet.begin() + static_cast<std::ptrdiff_t>(entry + 12));
    }

    return packet;
}

MoveReqInfo DataFileWarp(int index, const wchar_t* mainName, const wchar_t* subName, int level, int zen)
{
    MoveReqInfo info{};
    info.index = index;
    std::wcsncpy(info.szMainMapName, mainName, 31);
    std::wcsncpy(info.szSubMapName, subName, 31);
    info.iReqLevel = level;
    info.iReqZen = zen;
    info.iGateNum = 17;
    return info;
}

ServerWarp Warp(int index, const wchar_t* name, int level, uint32_t costs)
{
    return ServerWarp{index, 0, level, costs, name};
}

// The message of OpenMU's test WarpRequirementsAreWrittenAfterTheWarpListAsync: warp 3 needs master level 80
// and 2 resets, warp 300 70000 resets.
std::vector<BYTE> RequirementsPacket()
{
    return {0xC2, 0x00, 0x16, 0xF5, 0x0B, 0x02, 3, 0, 80, 0, 2, 0, 0, 0, 0x2C, 0x01, 0, 0, 0x70, 0x11, 0x01, 0};
}
} // namespace

TEST_CASE("server warp list: the warps are read from the message")
{
    const auto packet =
        MakePacket({{26, 34, 190, 10000, "Crywolf"}, {300, 79, 65535, 4000000000u, std::string(32, 'x')}});

    const auto warps = GameLogic::Warps::ReadServerWarpList(packet.data(), static_cast<int32_t>(packet.size()));

    REQUIRE(warps.has_value());
    REQUIRE(warps->size() == 2);
    CHECK((*warps)[0].index == 26);
    CHECK((*warps)[0].mapNumber == 34);
    CHECK((*warps)[0].levelRequirement == 190);
    CHECK((*warps)[0].costs == 10000u);
    CHECK((*warps)[0].name == L"Crywolf");
    CHECK((*warps)[1].index == 300);
    CHECK((*warps)[1].levelRequirement == 65535);
    CHECK((*warps)[1].costs == 4000000000u);
    CHECK((*warps)[1].name == std::wstring(32, L'x')); // a name may use all of its 32 bytes
}

TEST_CASE("server warp list: a message which is shorter than its count is rejected")
{
    auto packet = MakePacket({{26, 34, 190, 10000, "Crywolf"}});
    packet.pop_back();

    CHECK_FALSE(GameLogic::Warps::ReadServerWarpList(packet.data(), static_cast<int32_t>(packet.size())).has_value());
    CHECK_FALSE(GameLogic::Warps::ReadServerWarpList(packet.data(), 5).has_value());
    CHECK_FALSE(GameLogic::Warps::ReadServerWarpList(nullptr, 0).has_value());
}

TEST_CASE("server warp list: an empty list is a valid list")
{
    const auto packet = MakePacket({});

    const auto warps = GameLogic::Warps::ReadServerWarpList(packet.data(), static_cast<int32_t>(packet.size()));

    REQUIRE(warps.has_value());
    CHECK(warps->empty());
}

TEST_CASE("server warp requirements: the entries are read from the message")
{
    const auto packet = RequirementsPacket();

    const auto requirements =
        GameLogic::Warps::ReadServerWarpRequirements(packet.data(), static_cast<int32_t>(packet.size()));

    REQUIRE(requirements.has_value());
    REQUIRE(requirements->size() == 2);
    CHECK((*requirements)[0].index == 3);
    CHECK((*requirements)[0].minMasterLevel == 80);
    CHECK((*requirements)[0].minResets == 2);
    CHECK((*requirements)[1].index == 300);
    CHECK((*requirements)[1].minMasterLevel == 0);
    CHECK((*requirements)[1].minResets == 70000);
}

TEST_CASE("server warp requirements: resets above the client's range are capped")
{
    const std::vector<BYTE> packet{0xC2, 0x00, 0x0E, 0xF5, 0x0B, 0x01, 0x34, 0x12, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    const auto requirements =
        GameLogic::Warps::ReadServerWarpRequirements(packet.data(), static_cast<int32_t>(packet.size()));

    REQUIRE(requirements.has_value());
    REQUIRE(requirements->size() == 1);
    CHECK((*requirements)[0].index == 0x1234);
    CHECK((*requirements)[0].minMasterLevel == 65535);
    CHECK((*requirements)[0].minResets == INT32_MAX);
}

TEST_CASE("server warp requirements: a message which is shorter than its count is rejected")
{
    auto packet = RequirementsPacket();
    packet.pop_back();

    CHECK_FALSE(
        GameLogic::Warps::ReadServerWarpRequirements(packet.data(), static_cast<int32_t>(packet.size())).has_value());
    CHECK_FALSE(GameLogic::Warps::ReadServerWarpRequirements(packet.data(), 5).has_value());
    CHECK_FALSE(GameLogic::Warps::ReadServerWarpRequirements(nullptr, 0).has_value());
}

TEST_CASE("warp list merge: the master level and resets are the server's, 0 where it has none")
{
    auto dungeon = DataFileWarp(8, L"Dungeon", L"Dungeon", 30, 3000);
    dungeon.iReqMasterLevel = 5;
    dungeon.iReqResets = 2;
    auto lorencia = DataFileWarp(2, L"Lorencia", L"Lorencia", 10, 2000);
    lorencia.iReqMasterLevel = 5;
    auto withRequirements = Warp(2, L"Lorencia", 10, 2000);
    withRequirements.minMasterLevel = 40;
    withRequirements.minResets = 1;
    auto serverOnly = Warp(300, L"Market", 1, 0);
    serverOnly.minResets = 70000;

    const auto merged =
        GameLogic::Warps::MergeWarpList({dungeon, lorencia}, {Warp(8, L"Dungeon", 30, 3000), withRequirements, serverOnly});

    REQUIRE(merged.size() == 3);
    CHECK(merged[0].iReqMasterLevel == 0);
    CHECK(merged[0].iReqResets == 0);
    CHECK(merged[1].iReqMasterLevel == 40);
    CHECK(merged[1].iReqResets == 1);
    CHECK(merged[2].iReqMasterLevel == 0);
    CHECK(merged[2].iReqResets == 70000);
}

TEST_CASE("warp list merge: known warps keep their names and position, with the values of the server")
{
    const std::vector<MoveReqInfo> dataFile{
        DataFileWarp(2, L"Lorencia", L"Lorencia", 10, 2000),
        DataFileWarp(1, L"Noria", L"Noria", 10, 2000),
        DataFileWarp(3, L"Devias", L"Devias", 20, 2000),
    };
    // The server lists them in another order and with other values.
    const std::vector<ServerWarp> server{Warp(3, L"Devias (server)", 15, 3000), Warp(2, L"Lorencia (server)", 1, 0)};

    const auto merged = GameLogic::Warps::MergeWarpList(dataFile, server);

    REQUIRE(merged.size() == 2);
    CHECK(merged[0].index == 2);
    CHECK(std::wstring(merged[0].szMainMapName) == L"Lorencia");
    CHECK(merged[0].iReqLevel == 1);
    CHECK(merged[0].iReqZen == 0);
    CHECK(merged[0].iGateNum == 17); // the rest of the entry of the data files stays
    CHECK(merged[1].index == 3);
    CHECK(std::wstring(merged[1].szMainMapName) == L"Devias");
    CHECK(merged[1].iReqLevel == 15);
    CHECK(merged[1].iReqZen == 3000);
}

TEST_CASE("warp list merge: warps which only the server has are added after the known ones, in its order")
{
    const std::vector<MoveReqInfo> dataFile{DataFileWarp(2, L"Lorencia", L"Lorencia", 10, 2000)};
    const std::vector<ServerWarp> server{Warp(44, L"LorenMarket", 200, 18000), Warp(2, L"Lorencia", 10, 2000),
                                         Warp(26, L"Crywolf", 190, 10000)};

    const auto merged = GameLogic::Warps::MergeWarpList(dataFile, server);

    REQUIRE(merged.size() == 3);
    CHECK(merged[0].index == 2);
    CHECK(merged[1].index == 44);
    CHECK(std::wstring(merged[1].szMainMapName) == L"LorenMarket");
    CHECK(std::wstring(merged[1].szSubMapName) == L"LorenMarket"); // so that the warp chat command finds it
    CHECK(merged[1].iReqLevel == 200);
    CHECK(merged[1].iReqZen == 18000);
    CHECK(merged[2].index == 26);
}

TEST_CASE("warp list merge: of warps with the same index the first one counts")
{
    const std::vector<MoveReqInfo> dataFile{DataFileWarp(2, L"Lorencia", L"Lorencia", 10, 2000)};
    const std::vector<ServerWarp> server{Warp(26, L"Crywolf", 190, 10000), Warp(26, L"Second", 1, 1),
                                         Warp(2, L"Lorencia", 10, 2000), Warp(2, L"Second", 1, 1)};

    const auto merged = GameLogic::Warps::MergeWarpList(dataFile, server);

    REQUIRE(merged.size() == 2);
    CHECK(merged[0].iReqLevel == 10);
    CHECK(std::wstring(merged[1].szMainMapName) == L"Crywolf");
}

TEST_CASE("warp list merge: a name longer than the field is cut, and costs above the range are capped")
{
    const std::vector<ServerWarp> server{Warp(50, std::wstring(40, L'n').c_str(), 10, 4000000000u)};

    const auto merged = GameLogic::Warps::MergeWarpList({}, server);

    REQUIRE(merged.size() == 1);
    CHECK(std::wstring(merged[0].szMainMapName) == std::wstring(31, L'n'));
    CHECK(merged[0].iReqZen == INT32_MAX);
}
