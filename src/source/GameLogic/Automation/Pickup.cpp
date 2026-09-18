#include "stdafx.h"
#include "GameLogic/Automation/Pickup.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzObject.h"
#include "Network/Server/WSclient.h"

#include <cmath>

// Movement/target globals are defined in ZzzInterface.cpp.
extern int TargetX;
extern int TargetY;

namespace GameLogic::Automation
{
// Straight-line tile distance, rounded up, as the auto-helper measures it.
static int TileDistance(int fromX, int fromY, int toX, int toY)
{
    const int dx = fromX - toX;
    const int dy = fromY - toY;
    return static_cast<int>(std::ceil(std::sqrt(static_cast<double>(dx * dx + dy * dy))));
}

PickupResult PickUpItem(int itemIndex, int maximumDistance)
{
    if (itemIndex < 0 || itemIndex >= MAX_ITEMS)
    {
        return PickupResult::Gone;
    }

    ITEM_t* drop = &Items[itemIndex];
    if (!drop->Object.Live)
    {
        return PickupResult::Gone;
    }

    TargetX = (int)(drop->Object.Position[0] / TERRAIN_SCALE);
    TargetY = (int)(drop->Object.Position[1] / TERRAIN_SCALE);

    if (TileDistance(Hero->PositionX, Hero->PositionY, TargetX, TargetY) > maximumDistance)
    {
        return PickupResult::TooFar;
    }

    if (!CheckTile(Hero, &Hero->Object, PickupReach))
    {
        if (!PathFinding2(Hero->PositionX, Hero->PositionY, TargetX, TargetY, &Hero->Path))
        {
            return PickupResult::NoPath;
        }

        SendMove(Hero, &Hero->Object);
        return PickupResult::Approaching;
    }

    // One pickup at a time: the server answers with the item, and
    // SendGetItem is what remembers which request is outstanding.
    if (SendGetItem != -1)
    {
        return PickupResult::Busy;
    }

    SendGetItem = itemIndex;
    SocketClient->ToGameServer()->SendPickupItemRequest(itemIndex);
    return PickupResult::Requested;
}
} // namespace GameLogic::Automation
