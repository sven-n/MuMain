#include "stdafx.h"
#include "GameLogic/Items/TradeRestrictions.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Items/ItemCategories.h"
#include "GameLogic/Items/ItemLevelVariants.h"

// The flags tradable, droppable and storable come from the item data; the
// exceptions below depend on the item's state (level, rental time,
// durability) or on the player.
namespace
{
using Data::Items::ItemAction;

bool IsAllowed(const ITEM* pItem, ItemAction action)
{
    return g_ItemDatabase.IsAllowed(pItem->Type, action);
}

bool IsGameMaster()
{
    return g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) || Hero->CtlCode == CTLCODE_20OPERATOR ||
           Hero->CtlCode == CTLCODE_08OPERATOR;
}

// These items can be dropped when bought, but not while rented.
bool IsDropBannedWhileRented(const ITEM* pItem)
{
    return pItem->Type == ITEM_TALISMAN_OF_CHAOS_ASSEMBLY || pItem->Type == ITEM_CHAOS_CARD ||
           GameLogic::Items::IsDemonOrSpiritOfGuardian(pItem) || GameLogic::Items::IsPandaOrSkeletonItem(pItem);
}
} // namespace

namespace GameLogic::Items
{
    bool IsTradeBan(const ITEM* pItem)
    {
        if (pItem->Type == ITEM_GM_GIFT && IsGameMaster())
        {
            return false;
        }
        if (IsSignOfLord(pItem))
        {
            return false;
        }
        if (IsHeartOfDarkLord(pItem) || IsWizardsRingVariant(pItem))
        {
            return true;
        }

        return !IsAllowed(pItem, ItemAction::Trade);
    }

    bool IsDropBan(const ITEM* pItem)
    {
        if (pItem->bPeriodItem && IsDropBannedWhileRented(pItem))
        {
            return true;
        }

        return !IsAllowed(pItem, ItemAction::Drop);
    }

    bool IsStoreBan(const ITEM* pItem)
    {
        // Rented items cannot be stored.
        if (pItem->bPeriodItem)
        {
            return true;
        }
        if (IsSignOfLord(pItem))
        {
            return false;
        }
        if (IsHeartOfDarkLord(pItem) || IsWizardsRingVariant(pItem))
        {
            return true;
        }
        if (pItem->Type == ITEM_TALISMAN_OF_MOBILITY && pItem->Durability == 1)
        {
            return true;
        }

        return !IsAllowed(pItem, ItemAction::Store);
    }
}
