#pragma once

// Flight feel and the mount lean: winged players flying or hovering bank into turns and drift slowly up and down;
// riders and their mounts lean into turns.
namespace Engine::Characters
{
// Whether a player rides a mount now (outside the safe zones, where it's put away).
bool IsRiding(const CHARACTER* c);
// Whether a player flies on or hovers in place (the fly actions).
bool IsFlying(const OBJECT* o);

// A flying winged player's bank into a turn and slow up-and-down drift, faded in and out (each frame, everyone).
void UpdateFlightFeel(CHARACTER* c);
// A mount's lean into a turn from its rider's (UpdateFlightFeel leans a rider a share of the mount's).
float MountLeanFromRider(float riderLean);
} // namespace Engine::Characters
