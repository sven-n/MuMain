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
            || IsPandaOrSkeletonItem(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0)
            )
        {
            return false;
        }

        if (pItem->Type == ITEM_MOONSTONE_PENDANT
            || pItem->Type == ITEM_ELITE_TRANSFER_SKELETON_RING
            || (pItem->Type == ITEM_RENA && pItem->Level != 3)
            || IsSecondClassQuestItem(pItem)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
            || IsThirdClassQuestItem(pItem)
            || IsPartChargeItem(pItem)
            || IsCharacterCard(pItem)
            || pItem->Type == ITEM_HELPER + 99
            || IsPandaOrSkeletonItem(pItem)
            || (pItem->Type == ITEM_TALISMAN_OF_CHAOS_ASSEMBLY)
            || IsGemJewelry(pItem)
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

        if (IsSilverOrGoldKey(pItem)
            || pItem->Type == ITEM_SEALED_GOLDEN_BOX
            || pItem->Type == ITEM_SEALED_SILVER_BOX
            || pItem->Type == ITEM_GOLDEN_BOX
            || pItem->Type == ITEM_SILVER_BOX
            || IsSmallWing(pItem)
            || IsPandaOrSkeletonItem(pItem)
            || IsDemonOrSpiritOfGuardian(pItem)
            || IsGemJewelry(pItem)
            || ((pItem->Type == ITEM_WIZARDS_RING) && (Level == 0))
            || (pItem->Type == ITEM_PET_UNICORN)
            || (pItem->Type == ITEM_LETHAL_WIZARDS_RING)
            )
        {
            if (pItem->bPeriodItem && pItem->bExpiredPeriod)
            {
                return false;
            }
        }

        if (pItem->Type == ITEM_BOX_OF_LUCK
            || (pItem->Type == ITEM_RENA && Level == 1)
            || (pItem->bPeriodItem && !pItem->bExpiredPeriod && pItem->Type == ITEM_WIZARDS_RING && Level == 0)
            || (pItem->Type == ITEM_WIZARDS_RING && (Level == 1 || Level == 2))
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_REMEDY_OF_LOVE && Level >= 1 && Level <= 5)
            || IsPartChargeItem(pItem)
            || IsWingMixCharm(pItem)
            || IsPandaOrSkeletonItem(pItem)
            || (pItem->Type == ITEM_PET_UNICORN)
            || (pItem->Type == ITEM_LETHAL_WIZARDS_RING)
            )
        {
            return true;
        }

        if (Check_ItemAction(pItem, eITEM_SELL))	return true;

        return false;
    }

    bool IsRepairBan(const ITEM* pItem)
    {
        if (g_ChangeRingMgr->CheckRepair(pItem->Type))
        {
            return true;
        }
        if (IsPartChargeItem(pItem) || IsWingMixCharm(pItem))
        {
            return true;
        }

        if ((pItem->Type >= ITEM_GREEN_CHAOS_BOX && pItem->Type <= ITEM_PURPLE_CHAOS_BOX)
            || IsSeal(pItem)
            || (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HORN_OF_DINORANT)
            || IsAmmunition(pItem)
            || pItem->Type >= ITEM_POTION
            || (pItem->Type >= ITEM_ORB_OF_TWISTING_SLASH && pItem->Type <= ITEM_ORB_OF_DEATH_STAB)
            || (pItem->Type >= ITEM_LOCHS_FEATHER && pItem->Type <= ITEM_WEAPON_OF_ARCHANGEL)
            || pItem->Type == ITEM_RENA
            || IsDarkLordPet(pItem)
            || pItem->Type == ITEM_MOONSTONE_PENDANT
            || pItem->Type == ITEM_PET_RUDOLF
            || IsPandaOrSkeletonItem(pItem)
            || pItem->Type == ITEM_PET_UNICORN
            || pItem->Type == ITEM_CHERRY_BLOSSOM_PLAYBOX
            || pItem->Type == ITEM_CHERRY_BLOSSOM_WINE
            || pItem->Type == ITEM_CHERRY_BLOSSOM_RICE_CAKE
            || pItem->Type == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL
            || pItem->Type == ITEM_WHITE_CHERRY_BLOSSOM_BRANCH
            || pItem->Type == ITEM_RED_CHERRY_BLOSSOM_BRANCH
            || pItem->Type == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH
            || pItem->Type == ITEM_CONTRACT_SUMMON
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
            || pItem->Type == ITEM_INVITATION_TO_SANTA_VILLAGE
            || IsGambleItem(pItem)
            )
        {
            return true;
        }

        if (Check_ItemAction(pItem, eITEM_REPAIR))	return true;

        return false;
    }
}
