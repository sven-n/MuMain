// What the control socket reports about the character and its surroundings.
//
// Reading the engine's tables lives here, apart from the command handlers, so
// `state`, `nearby` and the event recorders share one description of the world.
#pragma once

#include <string>

namespace App::Control
{
// Encoded JSON object with every field the `state` command promises for a
// character standing in the world.
[[nodiscard]] std::string WorldStateObject();

// Encoded JSON array of the objects the client currently knows: players,
// monsters, NPCs and dropped items.
[[nodiscard]] std::string NearbyArray();
} // namespace App::Control
