#pragma once

namespace GameLogic::Cinematic
{
// The degrees which a monster of a direction turns in one frame at the reference frame rate, like in the
// original client. The caller scales it by the animation factor of the current frame rate.
inline constexpr float kMonsterTurnStepDegrees = 3.0f;

// The deviation from the direction to its target, within which a monster of a direction stops turning.
inline constexpr float kMonsterAlignToleranceDegrees = 3.0f;

// Calculates the angle, in which an object at the position (x, y) walks to the center of the tile.
// It's the angle of the engine (see CreateAngle): the object walks along (sin a, -cos a).
float CalculateAngleToTile(float x, float y, int tileX, int tileY, float tileSize);

// Turns the heading towards the target angle, the short way and by the step at most.
// Returns true when the heading is within kMonsterAlignToleranceDegrees. Then the heading is set exactly
// to the target angle, so that a monster which walks along it can't pass beside its target tile.
bool TurnTowards(float& heading, float targetAngle, float step);
} // namespace GameLogic::Cinematic
