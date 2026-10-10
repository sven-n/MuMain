#include "stdafx.h"
#include "GameLogic/Items/EquipmentRestrictions.h"

#include "GameLogic/Items/ItemCategories.h"
#include "World/MapInfra/MapManager.h"

namespace GameLogic::Items
{
bool RequiresFlight(int world)
{
    return world == WD_10HEAVEN;
}

bool CanTakeOff(int equipmentSlot, int world, const ITEM* pItemHelper, const ITEM* pItemWing)
{
    if (!RequiresFlight(world) || !HasFlightEquipment(pItemHelper, pItemWing))
        return true;

    ITEM empty{};
    empty.Type = -1;
    const ITEM* remainingHelper = equipmentSlot == EQUIPMENT_HELPER ? &empty : pItemHelper;
    const ITEM* remainingWing = equipmentSlot == EQUIPMENT_WING ? &empty : pItemWing;
    return HasFlightEquipment(remainingHelper, remainingWing);
}
} // namespace GameLogic::Items
