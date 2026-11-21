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
    // From the WarpRequirementsExtended message which follows the list (0 = none).
    int32_t minMasterLevel = 0;
    int32_t minResets = 0;
};

// The minimum master level and resets of a warp of the server's list (WarpRequirementsExtended,
// C2 F5 0B). OpenMU sends it right after the WarpListExtended, only for the warps which need one of
// them, and not at all when no warp does.
struct ServerWarpRequirement
{
    int32_t index = 0;
    int32_t minMasterLevel = 0;
    // The message has 32 bits; more than the client's int holds is capped.
    int32_t minResets = 0;
};

// Reads the warps of a WarpListExtended message. Returns nothing if the message is shorter than its
// count of warps says.
std::optional<std::vector<ServerWarp>> ReadServerWarpList(const uint8_t* data, int32_t size);

// Reads the entries of a WarpRequirementsExtended message. Returns nothing if the message is shorter
// than its count of entries says.
std::optional<std::vector<ServerWarpRequirement>> ReadServerWarpRequirements(const uint8_t* data, int32_t size);
} // namespace GameLogic::Warps
