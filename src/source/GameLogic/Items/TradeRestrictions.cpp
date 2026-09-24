#include "stdafx.h"
#include "GameLogic/Items/TradeRestrictions.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemCategories.h"

namespace GameLogic::Items
{
    bool IsTradeBan(const ITEM* pItem)
    {
        if (pItem->Type == ITEM_MOONSTONE_PENDANT
            || pItem->Type == ITEM_ELITE_TRANSFER_SKELETON_RING
            || (pItem->Type == ITEM_RENA && pItem->Level != 3)
            || IsSecondClassQuestItem(pItem)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
            || pItem->Type == ITEM_CURSED_CASTLE_WATER
            || IsThirdClassQuestItem(pItem)
            || IsPartChargeItem(pItem)
            || IsCharacterCard(pItem)
            || pItem->Type == ITEM_HELPER + 99
            || IsPandaOrSkeletonItem(pItem)
            )
        {
            return true;
        }

        if (pItem->Type == ITEM_GM_GIFT)
        {
            if (g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) ||
                (Hero->CtlCode == CTLCODE_20OPERATOR) || (Hero->CtlCode == CTLCODE_08OPERATOR))
                return false;
            else
                return true;
        }
        if (Check_ItemAction(pItem, eITEM_TRADE))	return true;

        return false;
    }

    bool IsDropBan(const ITEM* pItem)
    {
        if ((!pItem->bPeriodItem) &&
            (pItem->Type == ITEM_TALISMAN_OF_CHAOS_ASSEMBLY
                || pItem->Type == ITEM_CHAOS_CARD
                || IsDemonOrSpiritOfGuardian(pItem)
                || IsPandaOrSkeletonItem(pItem)
                || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0)
                ))
        {
            return false;
        }

        if (pItem->Type == ITEM_GOLDEN_BOX || pItem->Type == ITEM_SILVER_BOX)
        {
            return false;
        }

        if (IsSecondClassQuestItem(pItem)
            || IsThirdClassQuestItem(pItem)
            || IsPartChargeItem(pItem)
            || IsWingMixCharm(pItem)
            || IsCharacterCard(pItem)
            || pItem->Type == ITEM_HELPER + 99
            || IsPandaOrSkeletonItem(pItem)
            || pItem->Type == ITEM_SEALED_GOLDEN_BOX
            || pItem->Type == ITEM_SEALED_SILVER_BOX
            )
        {
            return true;
        }

        if (Check_ItemAction(pItem, eITEM_DROP))	return true;

        return false;
    }

    bool IsStoreBan(const ITEM* pItem)
    {
        if (IsSecondClassQuestItem(pItem)
            || (pItem->Type == ITEM_RENA && pItem->Level != 3)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || pItem->Type == ITEM_MASTER_SEAL_OF_ASCENSION
            || pItem->Type == ITEM_MASTER_SEAL_OF_WEALTH
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
            || IsThirdClassQuestItem(pItem)
            || (pItem->Type == ITEM_TALISMAN_OF_MOBILITY && pItem->Durability == 1)
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
            || pItem->bPeriodItem
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
            )
        {
            return true;
        }

        if (Check_ItemAction(pItem, eITEM_STORE))	return true;

        return false;
    }
}
