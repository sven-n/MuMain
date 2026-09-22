// Mouse-free item pickup.
//
// Extracted from MUHelper/MuHelper.cpp. Which item is worth taking stays with
// the caller (the auto-helper has its own filters); this walks to one item and
// asks the server for it.
#pragma once

#include <cstdint>
#include <string_view>

namespace GameLogic::Automation
{
enum class PickupResult : std::uint8_t
{
    // The pickup request went out.
    Requested,
    // Out of reach: a step towards the item was taken instead.
    Approaching,
    // Further away than the caller allows.
    TooFar,
    // The drop is no longer on the ground.
    Gone,
    // No way to walk to the drop from here.
    NoPath,
    // Another pickup is still waiting for its answer.
    Busy,
};

// Distance, in tiles, at which the character can reach a drop.
inline constexpr float PickupReach = 2.0f;

[[nodiscard]] constexpr std::string_view ResultName(PickupResult result)
{
    switch (result)
    {
    case PickupResult::Requested:
        return "requested";
    case PickupResult::Approaching:
        return "approaching";
    case PickupResult::TooFar:
        return "too_far";
    case PickupResult::Gone:
        return "gone";
    case PickupResult::NoPath:
        return "no_path";
    case PickupResult::Busy:
        return "busy";
    }
    return "unknown";
}

// Walks to the drop and picks it up. `itemIndex` is the client's slot in
// the dropped-item table, `maximumDistance` the caller's own limit in
// tiles.
PickupResult PickUpItem(int itemIndex, int maximumDistance);
} // namespace GameLogic::Automation
