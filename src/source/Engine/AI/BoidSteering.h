#pragma once

namespace Engine::AI::BoidSteering
{
// Accumulate a unit direction; a coincident target contributes nothing.
void AddDirection(float x, float y, float& totalX, float& totalY);

// The turn limit is authored per 25-FPS reference frame, not per rendered frame.
float TurnTowards(float heading, float targetHeading, float referenceTurnDegrees, float referenceFrames);
} // namespace Engine::AI::BoidSteering
