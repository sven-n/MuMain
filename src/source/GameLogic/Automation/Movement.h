// Mouse-free movement of the controlled character.
//
// Extracted from MUHelper/MuHelper.cpp so the auto-helper and the control
// socket walk through the same code the mouse does: the client's own path
// finder fills Hero->Path and the walk packet goes out unchanged.
#pragma once

#include <cstdint>
#include <string_view>

namespace GameLogic::Automation
{
enum class MoveResult : std::uint8_t
{
    // Already standing on (or within a tile of) the target.
    Arrived,
    // A path was found and the walk packet was sent.
    Walking,
    // The path finder found no way there.
    NoPath,
};

// Distance, in tiles, at which the character counts as arrived. The value
// the auto-helper has always used for its own regroup move.
inline constexpr float ArrivalTolerance = 1.5f;

[[nodiscard]] constexpr std::string_view ResultName(MoveResult result)
{
    switch (result)
    {
    case MoveResult::Arrived:
        return "arrived";
    case MoveResult::Walking:
        return "walking";
    case MoveResult::NoPath:
        return "no_path";
    }
    return "unknown";
}

// Walks towards the map tile, one path per call, exactly as a click there
// would. Call again on later frames until it reports Arrived.
MoveResult WalkTo(int tileX, int tileY);
} // namespace GameLogic::Automation
