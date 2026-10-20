#pragma once

namespace Engine::AI::BoidSteering
{
// Accumulate a unit direction; a coincident target contributes nothing.
void AddDirection(float x, float y, float& totalX, float& totalY);

// The turn limit is authored per 25-FPS reference frame, not per rendered frame.
float TurnTowards(float heading, float targetHeading, float referenceTurnDegrees, float referenceFrames);

// Reference frames to steer by this frame; 0 skips steering. A rate-gated member steers at random times,
// about every four reference frames: the gate follows time, so each steer it takes is a whole reference step.
float SteerFrames(bool rateGated, float frameFactor);
} // namespace Engine::AI::BoidSteering
