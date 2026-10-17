#include "stdafx.h"
#include "Engine/AI/BoidSteering.h"
#include "Engine/AI/ZzzAI.h"

#include <algorithm>
#include <cmath>

namespace Engine::AI::BoidSteering
{
void AddDirection(float x, float y, float& totalX, float& totalY)
{
    const float lengthSquared = x * x + y * y;
    if (lengthSquared == 0.f || !std::isfinite(lengthSquared))
    {
        return;
    }

    const float length = std::sqrt(lengthSquared);
    totalX += x / length;
    totalY += y / length;
}

float TurnTowards(float heading, float targetHeading, float referenceTurnDegrees, float referenceFrames)
{
    const float turnStep = referenceTurnDegrees * std::clamp(referenceFrames, 0.f, 1.f);
    return TurnAngle2(heading, targetHeading, turnStep);
}
} // namespace Engine::AI::BoidSteering
