#include "stdafx.h"
#include "GameLogic/Events/Cinematic/DirectionTurning.h"
#include "Engine/AI/ZzzAI.h"

namespace GameLogic::Cinematic
{
float CalculateAngleToTile(float x, float y, int tileX, int tileY, float tileSize)
{
    const float targetX = (static_cast<float>(tileX) + 0.5f) * tileSize;
    const float targetY = (static_cast<float>(tileY) + 0.5f) * tileSize;
    return CreateAngle(x, y, targetX, targetY);
}

bool TurnTowards(float& heading, float targetAngle, float step)
{
    if (FarAngle(heading, targetAngle) > kMonsterAlignToleranceDegrees)
    {
        heading = TurnAngle2(heading, targetAngle, step);
        return false;
    }

    heading = targetAngle;
    return true;
}
} // namespace GameLogic::Cinematic
