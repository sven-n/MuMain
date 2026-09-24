#pragma once

typedef struct tagITEM ITEM;

namespace GameLogic::Items
{
    bool IsFlyingMount(ITEM* pItem);
    bool HasFlightEquipment(ITEM* pItemHelper, ITEM* pItemWing);
}
