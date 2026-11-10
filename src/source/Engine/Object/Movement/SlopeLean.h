#pragma once

// Slope lean: on a slope a walking player leans a little into a climb, and a mount (and its rider) follows the slope.
namespace Engine::Characters
{
// A slope under a player (each frame, everyone): on foot it leans a little into a climb while it walks; riding, the
// mount (and the rider on it) follows the slope, nose up uphill.
void UpdateSlopeLean(CHARACTER* c);
} // namespace Engine::Characters
