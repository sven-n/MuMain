#pragma once

class CHARACTER;

namespace GameLogic::Monsters::Setup
{
struct SpawnWorld;
} // namespace GameLogic::Monsters::Setup

// How a monster that the server sends (0x13) faces (monsters design,
// Q-MN38): from the facing fields of its entry, on the map it is on and in
// the states that are on (the world says which).
namespace GameLogic::Monsters::Facing
{
// Object.Angle[2] 0x13 gives the character for the server's direction: its
// entry's (below).
float AngleOnArrival(const CHARACTER& character, int direction, const Setup::SpawnWorld& world);

// Object.Angle[2] of the number at the tile for the server's direction (the
// packet's Path >> 4, 1 to 8): its fixed facing, else (direction - 1) times
// its step at the tile.
float AngleOnArrival(int number, int tileX, int tileY, int direction, const Setup::SpawnWorld& world);
} // namespace GameLogic::Monsters::Facing
