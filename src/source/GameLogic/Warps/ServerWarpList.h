#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace GameLogic::Warps
{
// A warp of the warp list which the server sends (WarpListExtended, C2 F5 09). Only OpenMU sends it,
// to the open source client.
struct ServerWarp
{
    int32_t index = 0;
    int32_t mapNumber = 0;
    // As configured on the server; the classes with a reduced requirement get it on top, like with
    // the warp list of the data files.
    int32_t levelRequirement = 0;
    uint32_t costs = 0;
    std::wstring name;
};

// Reads the warps of a WarpListExtended message. Returns nothing if the message is shorter than its
// count of warps says.
std::optional<std::vector<ServerWarp>> ReadServerWarpList(const uint8_t* data, int32_t size);
} // namespace GameLogic::Warps
