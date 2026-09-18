#include "stdafx.h"
#include "GameLogic/Automation/Movement.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"

// Movement globals are defined in ZzzInterface.cpp.
extern int TargetX;
extern int TargetY;

namespace GameLogic::Automation
{
MoveResult WalkTo(int tileX, int tileY)
{
    Hero->MovementType = MOVEMENT_MOVE;
    TargetX = tileX;
    TargetY = tileY;

    if (CheckTile(Hero, &Hero->Object, ArrivalTolerance))
    {
        return MoveResult::Arrived;
    }

    if (!PathFinding2(Hero->PositionX, Hero->PositionY, TargetX, TargetY, &Hero->Path))
    {
        return MoveResult::NoPath;
    }

    SendMove(Hero, &Hero->Object);
    return MoveResult::Walking;
}
} // namespace GameLogic::Automation
