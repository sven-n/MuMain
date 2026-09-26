#include "stdafx.h"
#include "GameLogic/Items/ShopRestrictions.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "GameLogic/Items/ItemCategories.h"
#include "GameLogic/Items/ItemLevelVariants.h"

// The rule flags come from the item data; the exceptions below depend on the
// item level.
namespace
{
using Data::Items::ItemAction;

bool IsAllowed(const ITEM* pItem, ItemAction action)
{
    return g_ItemDatabase.IsAllowed(pItem->Type, action);
}
} // namespace

namespace GameLogic::Items
{
    bool IsPersonalShopBan(const ITEM* pItem)
    {
        if (pItem == NULL)
        {
            return false;
        }

        if (pItem->bPeriodItem && !IsAllowed(pItem, ItemAction::SellInPersonalShopWhileRented))
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

        return !IsAllowed(pItem, ItemAction::SellInPersonalShop);
    }

    bool IsSellingBan(const ITEM* pItem)
    {
        const int Level = pItem->Level;

        if (pItem->bPeriodItem && pItem->bExpiredPeriod && IsAllowed(pItem, ItemAction::SellWhenRentalExpired))
        {
            return false;
        }

        // Rena +1 (Stone), Wizard's Ring +1 and +2, Remedy of Love +1 to +5,
        // and a Wizard's Ring +0 while it is rented.
        if ((pItem->Type == ITEM_RENA && Level == 1) ||
            (pItem->Type == ITEM_WIZARDS_RING && (Level == 1 || Level == 2)) ||
            (pItem->Type == ITEM_WIZARDS_RING && Level == 0 && pItem->bPeriodItem && !pItem->bExpiredPeriod) ||
            (pItem->Type == ITEM_REMEDY_OF_LOVE && Level >= 1 && Level <= 5))
        {
            return true;
        }

        return !IsAllowed(pItem, ItemAction::Sell);
    }

    bool IsRepairBan(const ITEM* pItem)
    {
        return !IsAllowed(pItem, ItemAction::Repair);
    }
}
