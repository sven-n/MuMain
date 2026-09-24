#include "stdafx.h"
#include "GameLogic/Items/ShopRestrictions.h"

#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemCategories.h"
#include "GameLogic/Items/ChangeRingManager.h"

namespace GameLogic::Items
{
    bool IsPersonalShopBan(const ITEM* pItem)
    {
        if (pItem == NULL)
        {
            return false;
        }

        if ((!pItem->bPeriodItem) &&
            pItem->Type == ITEM_DEMON
            || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
            || pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0)
            )
        {
            return false;
        }

        if (pItem->Type == ITEM_MOONSTONE_PENDANT
            || pItem->Type == ITEM_ELITE_TRANSFER_SKELETON_RING
            || (pItem->Type == ITEM_POTION + 21 && pItem->Level != 3)
            || (pItem->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && pItem->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
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
            || (pItem->Type == ITEM_POTION + 96)
            || pItem->Type == ITEM_HELPER + 109
            || pItem->Type == ITEM_HELPER + 110
            || pItem->Type == ITEM_HELPER + 111
            || pItem->Type == ITEM_HELPER + 112
            || pItem->Type == ITEM_HELPER + 113
            || pItem->Type == ITEM_HELPER + 114
            || pItem->Type == ITEM_HELPER + 115
            )
        {
            return true;
        }
        if (Check_ItemAction(pItem, eITEM_PERSONALSHOP))	return true;

        return false;
    }

    bool IsSellingBan(const ITEM* pItem)
    {
        int Level = pItem->Level;

        if (true == false
            || pItem->Type == ITEM_POTION + 112
            || pItem->Type == ITEM_POTION + 113
            || pItem->Type == ITEM_POTION + 121
            || pItem->Type == ITEM_POTION + 122
            || pItem->Type == ITEM_POTION + 123
            || pItem->Type == ITEM_POTION + 124
            || IsSmallWing(pItem)
            || pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
            || pItem->Type == ITEM_DEMON
            || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
            || pItem->Type == ITEM_HELPER + 109
            || pItem->Type == ITEM_HELPER + 110
            || pItem->Type == ITEM_HELPER + 111
            || pItem->Type == ITEM_HELPER + 112
            || pItem->Type == ITEM_HELPER + 113
            || pItem->Type == ITEM_HELPER + 114
            || pItem->Type == ITEM_HELPER + 115
            || ((pItem->Type == ITEM_WIZARDS_RING) && (Level == 0))
            || (pItem->Type == ITEM_PET_UNICORN)
            || (pItem->Type == ITEM_HELPER + 107)
            )
        {
            if (true == pItem->bPeriodItem && true == pItem->bExpiredPeriod)
            {
                return false;
            }
        }

        if (pItem->Type == ITEM_BOX_OF_LUCK
            || (pItem->Type == ITEM_POTION + 21 && Level == 1)
            || ((pItem->bPeriodItem == true) && (pItem->bExpiredPeriod == false) && (pItem->Type == ITEM_WIZARDS_RING) && (Level == 0))
            || (pItem->Type == ITEM_WIZARDS_RING && (Level == 1 || Level == 2))
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_POTION + 20 && Level >= 1 && Level <= 5)
            || IsPartChargeItem(pItem)
            || ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN)
                && (pItem->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END))
            || pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
            || (pItem->Type == ITEM_PET_UNICORN)
            || (pItem->Type == ITEM_HELPER + 107)
            )
        {
            return true;
        }

        if (Check_ItemAction(pItem, eITEM_SELL))	return true;

        return false;
    }

    bool IsRepairBan(const ITEM* pItem)
    {
        if (g_ChangeRingMgr->CheckRepair(pItem->Type) == true)
        {
            return true;
        }
        if (IsPartChargeItem(pItem) == true || ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN) && (pItem->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END)))
        {
            return true;
        }

        if ((pItem->Type >= ITEM_POTION + 55 && pItem->Type <= ITEM_POTION + 57)
            || IsSeal(pItem)
            || (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HORN_OF_DINORANT)
            || pItem->Type == ITEM_BOLT
            || pItem->Type == ITEM_ARROWS
            || pItem->Type >= ITEM_POTION
            || (pItem->Type >= ITEM_ORB_OF_TWISTING_SLASH && pItem->Type <= ITEM_ORB_OF_DEATH_STAB)
            || (pItem->Type >= ITEM_LOCHS_FEATHER && pItem->Type <= ITEM_WEAPON_OF_ARCHANGEL)
            || pItem->Type == ITEM_POTION + 21
            || pItem->Type == ITEM_DARK_HORSE_ITEM
            || pItem->Type == ITEM_DARK_RAVEN_ITEM
            || pItem->Type == ITEM_MOONSTONE_PENDANT
            || pItem->Type == ITEM_PET_RUDOLF
            || pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
            || pItem->Type == ITEM_PET_UNICORN
            || pItem->Type == ITEM_CHERRY_BLOSSOM_PLAYBOX
            || pItem->Type == ITEM_CHERRY_BLOSSOM_WINE
            || pItem->Type == ITEM_CHERRY_BLOSSOM_RICE_CAKE
            || pItem->Type == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL
            || pItem->Type == ITEM_POTION + 88
            || pItem->Type == ITEM_POTION + 89
            || pItem->Type == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH
            || pItem->Type == ITEM_HELPER + 7
            || pItem->Type == ITEM_TRANSFORMATION_RING
            || pItem->Type == ITEM_LIFE_STONE_ITEM
            || pItem->Type == ITEM_WIZARDS_RING
            || pItem->Type == ITEM_ARMOR_OF_GUARDSMAN
            || pItem->Type == ITEM_SPLINTER_OF_ARMOR
            || pItem->Type == ITEM_BLESS_OF_GUARDIAN
            || pItem->Type == ITEM_CLAW_OF_BEAST
            || pItem->Type == ITEM_FRAGMENT_OF_HORN
            || pItem->Type == ITEM_BROKEN_HORN
            || pItem->Type == ITEM_HORN_OF_FENRIR
            || pItem->Type == ITEM_OLD_SCROLL
            || pItem->Type == ITEM_ILLUSION_SORCERER_COVENANT
            || pItem->Type == ITEM_SCROLL_OF_BLOOD
            || pItem->Type == ITEM_HELPER + 66
            || IsGambleItem(pItem)
            )
        {
            return true;
        }

        if (Check_ItemAction(pItem, eITEM_REPAIR))	return true;

        return false;
    }
}
