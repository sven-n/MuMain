#include "GameLogic/Warps/ServerWarpList.h"

#include "Core/Platform/WinCompat.h"
#include "Data/Translation/MultiLanguage.h"

namespace GameLogic::Warps
{
namespace
{
// Offsets of the WarpListExtended message (C2 header with sub code).
constexpr int32_t CountOffset = 5;
constexpr int32_t WarpsOffset = 6;

// Offsets within one warp entry.
constexpr int32_t WarpSize = 44;
constexpr int32_t IndexOffset = 0;
constexpr int32_t MapNumberOffset = 2;
constexpr int32_t LevelRequirementOffset = 4;
constexpr int32_t CostsOffset = 8;
constexpr int32_t NameOffset = 12;
constexpr int32_t NameLength = 32;

uint16_t ReadUInt16LittleEndian(const uint8_t* data)
{
    return static_cast<uint16_t>(data[0] | (data[1] << 8));
}

uint32_t ReadUInt32LittleEndian(const uint8_t* data)
{
    return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
}

// The name is UTF-8 and padded with zeros; all of its 32 bytes may be used.
std::wstring ReadName(const uint8_t* data)
{
    std::vector<wchar_t> buffer(static_cast<size_t>(NameLength) + 1, L'\0');
    CMultiLanguage::ConvertFromUtf8(buffer.data(), reinterpret_cast<const char*>(data), NameLength);
    buffer[NameLength] = L'\0';
    return std::wstring(buffer.data());
}
} // namespace

std::optional<std::vector<ServerWarp>> ReadServerWarpList(const uint8_t* data, int32_t size)
{
    if (data == nullptr || size < WarpsOffset)
    {
        return std::nullopt;
    }

    const int32_t count = data[CountOffset];
    if (size < WarpsOffset + count * WarpSize)
    {
        return std::nullopt;
    }

    std::vector<ServerWarp> warps;
    warps.reserve(static_cast<size_t>(count));
    for (int32_t i = 0; i < count; ++i)
    {
        const uint8_t* entry = data + WarpsOffset + i * WarpSize;
        ServerWarp warp;
        warp.index = ReadUInt16LittleEndian(entry + IndexOffset);
        warp.mapNumber = ReadUInt16LittleEndian(entry + MapNumberOffset);
        warp.levelRequirement = ReadUInt16LittleEndian(entry + LevelRequirementOffset);
        warp.costs = ReadUInt32LittleEndian(entry + CostsOffset);
        warp.name = ReadName(entry + NameOffset);
        warps.push_back(std::move(warp));
    }

    return warps;
}
} // namespace GameLogic::Warps
