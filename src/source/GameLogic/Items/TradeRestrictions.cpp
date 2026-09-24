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
            || (pItem->Type == ITEM_POTION + 21 && pItem->Level != 3)
            || (pItem->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && pItem->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
            || pItem->Type == ITEM_POTION + 64
            || pItem->Type == ITEM_FLAME_OF_DEATH_BEAM_KNIGHT
            || pItem->Type == ITEM_HORN_OF_HELL_MAINE
            || pItem->Type == ITEM_FEATHER_OF_DARK_PHOENIX
            || pItem->Type == ITEM_EYE_OF_ABYSSAL
            || IsPartChargeItem(pItem)
            || IsCharacterCard(pItem)
            || pItem->Type == ITEM_HELPER + 99
            || pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
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
            (pItem->Type == ITEM_POTION + 96
                || pItem->Type == ITEM_POTION + 54
                || pItem->Type == ITEM_DEMON
                || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
                || pItem->Type == ITEM_PET_PANDA
                || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
                || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
                || pItem->Type == ITEM_PET_SKELETON
                || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0)
                ))
        {
            return false;
        }

        if (true == false || pItem->Type == ITEM_POTION + 123 || pItem->Type == ITEM_POTION + 124)
        {
            return false;
        }

        if ((pItem->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && pItem->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
            || (pItem->Type >= ITEM_FLAME_OF_DEATH_BEAM_KNIGHT && pItem->Type <= ITEM_EYE_OF_ABYSSAL)
            || IsPartChargeItem(pItem)
            || ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN)
                && (pItem->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END))
            || IsCharacterCard(pItem)
            || pItem->Type == ITEM_HELPER + 99
            || pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
            || pItem->Type == ITEM_POTION + 121
            || pItem->Type == ITEM_POTION + 122
            )
        {
            return true;
        }

        if (Check_ItemAction(pItem, eITEM_DROP))	return true;

        return false;
    }

    bool IsStoreBan(const ITEM* pItem)
    {
        if ((pItem->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && pItem->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
            || (pItem->Type == ITEM_POTION + 21 && pItem->Level != 3)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || pItem->Type == ITEM_HELPER + 93
            || pItem->Type == ITEM_HELPER + 94
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
            || pItem->Type == ITEM_FLAME_OF_DEATH_BEAM_KNIGHT
            || pItem->Type == ITEM_HORN_OF_HELL_MAINE
            || pItem->Type == ITEM_FEATHER_OF_DARK_PHOENIX
            || pItem->Type == ITEM_EYE_OF_ABYSSAL
            || (pItem->Type == ITEM_HELPER + 70 && pItem->Durability == 1)
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
            || (pItem->bPeriodItem == true)
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
            )
        {
            return true;
        }

        if (Check_ItemAction(pItem, eITEM_STORE))	return true;

        return false;
    }
}
