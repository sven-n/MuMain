#pragma once

#include "Core/Globals/_enum.h"
#include "Core/Globals/_struct.h"

// Some items are a different item at each item level: Rena +3 is the Sign of
// Lord, Box of Luck +13 the Heart of Dark Lord. The server sends them as the
// same item with another level, so the item data cannot tell them apart and
// the rule code checks them here. Phase 12 of the data-driven items plan
// makes them items of their own.
namespace GameLogic::Items
{
inline bool IsSignOfLord(const ITEM* pItem)
{
    return pItem->Type == ITEM_RENA && pItem->Level == 3;
}

inline bool IsHeartOfDarkLord(const ITEM* pItem)
{
    return pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13;
}

// The Wizard's Ring above +0.
inline bool IsWizardsRingVariant(const ITEM* pItem)
{
    return pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0;
}
} // namespace GameLogic::Items
