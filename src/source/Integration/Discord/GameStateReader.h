// Reads the presence snapshot from the running game.
#pragma once

#include "Integration/Discord/PresenceSnapshot.h"

#include <optional>

namespace Integration::Discord::GameState
{
// Empty while the client is between scenes or loading a map, when the
// state it would read is not settled yet.
[[nodiscard]] std::optional<PresenceSnapshot> Read();
} // namespace Integration::Discord::GameState
