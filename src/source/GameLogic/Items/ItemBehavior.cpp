#include "stdafx.h"
#include "GameLogic/Items/ItemBehavior.h"

#include "Engine/Object/ZzzInventory.h"

namespace GameLogic::Items
{
    bool IsFlyingMount(ITEM* pItem)
    {
        return pItem->Type == ITEM_HORN_OF_DINORANT
            || pItem->Type == ITEM_DARK_HORSE_ITEM
            || pItem->Type == ITEM_HORN_OF_FENRIR;
    }

    bool HasFlightEquipment(ITEM* pItemHelper, ITEM* pItemWing)
    {
        return IsWingItem(pItemWing) || IsFlyingMount(pItemHelper);
    }
}
