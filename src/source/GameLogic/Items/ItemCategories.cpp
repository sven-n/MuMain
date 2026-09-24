#include "stdafx.h"
#include "GameLogic/Items/ItemCategories.h"

#include "GameLogic/Items/CComGem.h"

namespace
{
    bool IsClothWingType(int itemType)
    {
        return itemType == ITEM_CAPE_OF_LORD
            || itemType == ITEM_WING_OF_RUIN
            || itemType == ITEM_CAPE_OF_EMPEROR
            || itemType == ITEM_CAPE_OF_FIGHTER
            || itemType == ITEM_CAPE_OF_OVERRULE
            || itemType == ITEM_SMALL_CAPE_OF_LORD
            || itemType == ITEM_LITTLE_WARRIORS_CLOAK;
    }

    bool IsRideableMountType(int itemType)
    {
        return itemType == ITEM_HORN_OF_UNIRIA
            || itemType == ITEM_HORN_OF_DINORANT
            || itemType == ITEM_DARK_HORSE_ITEM
            || itemType == ITEM_HORN_OF_FENRIR;
    }

    bool IsHornMountType(int itemType)
    {
        return itemType == ITEM_HORN_OF_UNIRIA
            || itemType == ITEM_HORN_OF_DINORANT
            || itemType == ITEM_HORN_OF_FENRIR;
    }

    bool IsCharacterCardType(int itemType)
    {
        return itemType == ITEM_MAGIC_GLADIATOR_CHARACTER_CARD
            || itemType == ITEM_DARK_LORD_CHARACTER_CARD
            || itemType == ITEM_SUMMONER_CHARACTER_CARD;
    }
}

namespace GameLogic::Items
{
    bool IsWingItem(const ITEM* pItem)
    {
        switch (pItem->Type)
        {
        case ITEM_WING:
        case ITEM_WINGS_OF_HEAVEN:
        case ITEM_WINGS_OF_SATAN:
        case ITEM_WINGS_OF_SPIRITS:
        case ITEM_WINGS_OF_SOUL:
        case ITEM_WINGS_OF_DRAGON:
        case ITEM_WINGS_OF_DARKNESS:
        case ITEM_CAPE_OF_LORD:
        case ITEM_WING_OF_STORM:
        case ITEM_WING_OF_ETERNAL:
        case ITEM_WING_OF_ILLUSION:
        case ITEM_WING_OF_RUIN:
        case ITEM_CAPE_OF_EMPEROR:
        case ITEM_WING_OF_CURSE:
        case ITEM_WINGS_OF_DESPAIR:
        case ITEM_WING_OF_DIMENSION:
        case ITEM_SMALL_CAPE_OF_LORD:
        case ITEM_SMALL_WING_OF_CURSE:
        case ITEM_SMALL_WINGS_OF_ELF:
        case ITEM_SMALL_WINGS_OF_HEAVEN:
        case ITEM_SMALL_WINGS_OF_SATAN:
        case ITEM_CAPE_OF_FIGHTER:
        case ITEM_CAPE_OF_OVERRULE:
        case ITEM_LITTLE_WARRIORS_CLOAK:
            return true;
        }

        return false;
    }

    bool IsSecondTierWingExceptCape(const ITEM* pItem)
    {
        return (pItem->Type >= ITEM_WINGS_OF_SPIRITS && pItem->Type <= ITEM_WINGS_OF_DARKNESS)
            || pItem->Type == ITEM_WINGS_OF_DESPAIR;
    }

    bool IsThirdTierWing(const ITEM* pItem)
    {
        return (pItem->Type >= ITEM_WING_OF_STORM && pItem->Type <= ITEM_CAPE_OF_EMPEROR)
            || pItem->Type == ITEM_WING_OF_DIMENSION
            || pItem->Type == ITEM_CAPE_OF_OVERRULE;
    }

    bool IsSmallWing(const ITEM* pItem)
    {
        return pItem->Type >= ITEM_SMALL_CAPE_OF_LORD && pItem->Type <= ITEM_LITTLE_WARRIORS_CLOAK;
    }

    bool IsClothWing(const ITEM* pItem)
    {
        return IsClothWingType(pItem->Type);
    }

    bool IsClothWingModel(int modelType)
    {
        return IsClothWingType(modelType - MODEL_ITEM);
    }

    bool IsRideableMount(const ITEM* pItem)
    {
        return IsRideableMountType(pItem->Type);
    }

    bool IsRideableMountModel(int modelType)
    {
        return IsRideableMountType(modelType - MODEL_ITEM);
    }

    bool IsHornMountModel(int modelType)
    {
        return IsHornMountType(modelType - MODEL_ITEM);
    }

    bool IsFlyingMount(const ITEM* pItem)
    {
        return pItem->Type == ITEM_HORN_OF_DINORANT
            || pItem->Type == ITEM_DARK_HORSE_ITEM
            || pItem->Type == ITEM_HORN_OF_FENRIR;
    }

    bool HasFlightEquipment(const ITEM* pItemHelper, const ITEM* pItemWing)
    {
        return IsWingItem(pItemWing) || IsFlyingMount(pItemHelper);
    }

    bool IsJewelItem(const ITEM* pItem)
    {
        return pItem->Type == ITEM_JEWEL_OF_BLESS
            || pItem->Type == ITEM_JEWEL_OF_SOUL
            || pItem->Type == ITEM_JEWEL_OF_LIFE
            || pItem->Type == ITEM_JEWEL_OF_CHAOS
            || pItem->Type == ITEM_JEWEL_OF_CREATION
            || pItem->Type == ITEM_JEWEL_OF_GUARDIAN;
    }

    bool IsSocketSeedOrSphereType(int itemType)
    {
        return (itemType >= ITEM_SEED_FIRE && itemType <= ITEM_SEED_EARTH)
            || (itemType >= ITEM_SPHERE_MONO && itemType <= ITEM_SPHERE_5)
            || (itemType >= ITEM_SEED_SPHERE_FIRE_1 && itemType <= ITEM_SEED_SPHERE_EARTH_5);
    }

    bool IsSocketSeedOrSphere(const ITEM* pItem)
    {
        return IsSocketSeedOrSphereType(pItem->Type);
    }

    bool IsSocketSeedOrSphereModel(int modelType)
    {
        return IsSocketSeedOrSphereType(modelType - MODEL_ITEM);
    }

    bool IsSealType(int itemType)
    {
        return itemType == ITEM_SEAL_OF_ASCENSION
            || itemType == ITEM_SEAL_OF_WEALTH
            || itemType == ITEM_SEAL_OF_SUSTENANCE;
    }

    bool IsSeal(const ITEM* pItem)
    {
        return IsSealType(pItem->Type);
    }

    bool IsSealModel(int modelType)
    {
        return IsSealType(modelType - MODEL_ITEM);
    }

    bool IsGambleItemType(int itemType)
    {
        return itemType == ITEM_GAMBLE_SWORD_MACE_SPEAR
            || itemType == ITEM_GAMBLE_STAFF
            || itemType == ITEM_GAMBLE_BOW_CROSSBOW
            || itemType == ITEM_GAMBLE_SCEPTER
            || itemType == ITEM_GAMBLE_STICK;
    }

    bool IsGambleItem(const ITEM* pItem)
    {
        return IsGambleItemType(pItem->Type);
    }

    bool IsGambleItemModel(int modelType)
    {
        return IsGambleItemType(modelType - MODEL_ITEM);
    }

    bool IsCharacterCard(const ITEM* pItem)
    {
        return IsCharacterCardType(pItem->Type);
    }

    bool IsCharacterCardModel(int modelType)
    {
        return IsCharacterCardType(modelType - MODEL_ITEM);
    }

    bool IsDevilSquareItemType(int itemType)
    {
        return itemType == ITEM_DEVILS_EYE
            || itemType == ITEM_DEVILS_KEY
            || itemType == ITEM_DEVILS_INVITATION;
    }

    bool IsDevilSquareItem(const ITEM* pItem)
    {
        return IsDevilSquareItemType(pItem->Type);
    }

    bool IsDivineArchangelWeapon(const ITEM* pItem)
    {
        return IsDivineArchangelWeaponType(pItem->Type);
    }

    bool IsDivineArchangelWeaponType(int itemType)
    {
        return itemType == ITEM_DIVINE_SWORD_OF_ARCHANGEL
            || itemType == ITEM_DIVINE_CB_OF_ARCHANGEL
            || itemType == ITEM_DIVINE_STAFF_OF_ARCHANGEL
            || itemType == ITEM_DIVINE_STICK_OF_ARCHANGEL
            || itemType == ITEM_DIVINE_SCEPTER_OF_ARCHANGEL;
    }

    bool IsDivineArchangelWeaponModel(int modelType)
    {
        return IsDivineArchangelWeaponType(modelType - MODEL_ITEM);
    }

    bool IsPartChargeItem(const ITEM* pItem)
    {
        if ((pItem->Type >= ITEM_DEVIL_SQUARE_TICKET && pItem->Type <= ITEM_KALIMA_TICKET)
            || (pItem->Type == ITEM_CHAOS_CARD)
            || (pItem->Type >= ITEM_RARE_ITEM_TICKET_1 && pItem->Type <= ITEM_RARE_ITEM_TICKET_5)
            || (pItem->Type >= ITEM_RARE_ITEM_TICKET_7 && pItem->Type <= ITEM_RARE_ITEM_TICKET_12)
            || (pItem->Type >= ITEM_OPEN_ACCESS_TICKET_TO_DOPPELGANGER && pItem->Type <= ITEM_OPEN_ACCESS_TICKET_TO_VARKA_7)
            || pItem->Type == ITEM_TALISMAN_OF_LUCK
            || IsSeal(pItem)
            || (pItem->Type >= ITEM_ELITE_HEALING_POTION && pItem->Type <= ITEM_ELITE_MANA_POTION)
            || (pItem->Type >= ITEM_SCROLL_OF_QUICKNESS && pItem->Type <= ITEM_SCROLL_OF_MANA)
            || (pItem->Type == ITEM_SEAL_OF_MOBILITY)
            || (pItem->Type >= ITEM_RESET_FRUIT_STRENGTH && pItem->Type <= ITEM_RESET_FRUIT_CONTROL)
            || (pItem->Type >= ITEM_ELIXIR_OF_STRENGTH && pItem->Type <= ITEM_ELIXIR_OF_CONTROL)
            || (pItem->Type == ITEM_INDULGENCE)
            || (pItem->Type == ITEM_ILLUSION_TEMPLE_TICKET)
            || (pItem->Type == ITEM_POTION + 91)
            || (pItem->Type >= ITEM_CHAOS_CARD_GOLD && pItem->Type <= ITEM_CHAOS_CARD_RARE)
            || (pItem->Type == ITEM_CHAOS_CARD_MINI)
            || (pItem->Type == ITEM_MEDIUM_ELITE_HEALING_POTION)
            || (pItem->Type >= ITEM_SEAL_OF_HEALING && pItem->Type <= ITEM_SEAL_OF_DIVINITY)
            || (pItem->Type >= ITEM_SCROLL_OF_BATTLE && pItem->Type <= ITEM_SCROLL_OF_STRENGTH)
            || (pItem->Type == ITEM_TALISMAN_OF_CHAOS_ASSEMBLY)
            || (pItem->Type == ITEM_DEMON || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN)
            || (pItem->Type == ITEM_TALISMAN_OF_RESURRECTION)
            || (pItem->Type == ITEM_TALISMAN_OF_MOBILITY)
            || pItem->Type == ITEM_TALISMAN_OF_GUARDIAN
            || pItem->Type == ITEM_TALISMAN_OF_ITEM_PROTECTION
            || pItem->Type == ITEM_MASTER_SEAL_OF_ASCENSION
            || pItem->Type == ITEM_MASTER_SEAL_OF_WEALTH
            || pItem->Type == ITEM_LETHAL_WIZARDS_RING
            || pItem->Type == ITEM_MAX_AG_BOOST_AURA
            || pItem->Type == ITEM_MAX_SD_BOOST_AURA
            || pItem->Type == ITEM_PARTY_EXP_SCROLL
            || pItem->Type == ITEM_ELITE_SD_POTION
            || pItem->Type == ITEM_SAPPHIRE_RING
            || pItem->Type == ITEM_RUBY_RING
            || pItem->Type == ITEM_TOPAZ_RING
            || pItem->Type == ITEM_AMETHYST_RING
            || pItem->Type == ITEM_RUBY_NECKLACE
            || pItem->Type == ITEM_EMERALD_NECKLACE
            || pItem->Type == ITEM_SAPPHIRE_NECKLACE
            || pItem->Type == ITEM_SILVER_KEY
            || pItem->Type == ITEM_GOLD_KEY
            || pItem->Type == ITEM_GOBLIN_GOLD_COIN
            || pItem->Type == ITEM_GOLDEN_BOX
            || pItem->Type == ITEM_SILVER_BOX
            || pItem->Type == ITEM_PACKAGE_BOX_A
            || pItem->Type == ITEM_PACKAGE_BOX_B
            || pItem->Type == ITEM_PACKAGE_BOX_C
            || pItem->Type == ITEM_PACKAGE_BOX_D
            || pItem->Type == ITEM_PACKAGE_BOX_E
            || pItem->Type == ITEM_PACKAGE_BOX_F
            || IsSmallWing(pItem)
            || pItem->Type == ITEM_HELPER + 116
            || pItem->Type == ITEM_PET_UNICORN
            || pItem->Type == ITEM_PAID_CHANNEL_ACCESS_TICKET
            || pItem->Type == ITEM_MASTER_SKILL_RESET
            || pItem->Type == ITEM_STAT_ADJUSTMENT
            || pItem->Type == ITEM_CHARACTER_RELOCATION_SERVICE
            || pItem->Type == ITEM_CHARACTER_RENAME_SERVICE
            || pItem->Type == ITEM_SERVER_RELOCATION_SERVICE
            || pItem->Type == ITEM_PREMIUM_PACKAGE
            || pItem->Type == ITEM_30_DAY_PASS
            || pItem->Type == ITEM_90_DAY_PASS
            || pItem->Type == ITEM_30_DAY_PASS_POINTS
            || pItem->Type == ITEM_90_DAY_PASS_POINTS
            || pItem->Type == ITEM_3_HOUR_PASS
            || pItem->Type == ITEM_5_HOUR_PASS
            || pItem->Type == ITEM_10_HOUR_PASS
            || pItem->Type == ITEM_OPEN_ACCESS_TICKET_TO_CHAOS_CASTLE
            || pItem->Type == ITEM_SCROLL_OF_HEALING
            )
        {
            return true;
        }

        return false;
    }

    bool IsHighValueItem(const ITEM* pItem)
    {
        int iLevel = pItem->Level;

        if (
            pItem->Type == ITEM_HORN_OF_DINORANT ||
            pItem->Type == ITEM_JEWEL_OF_BLESS ||
            pItem->Type == ITEM_JEWEL_OF_SOUL ||
            pItem->Type == ITEM_JEWEL_OF_LIFE ||
            pItem->Type == ITEM_JEWEL_OF_CREATION ||
            pItem->Type == ITEM_JEWEL_OF_CHAOS ||
            pItem->Type == ITEM_JEWEL_OF_GUARDIAN ||
            pItem->Type == ITEM_PACKED_JEWEL_OF_BLESS ||
            pItem->Type == ITEM_PACKED_JEWEL_OF_SOUL ||
            (pItem->Type >= ITEM_WING && pItem->Type <= ITEM_WINGS_OF_DARKNESS) ||
            pItem->Type == ITEM_DARK_HORSE_ITEM ||
            pItem->Type == ITEM_DARK_RAVEN_ITEM ||
            pItem->Type == ITEM_CAPE_OF_LORD ||
            (pItem->Type >= ITEM_WING_OF_STORM && pItem->Type <= ITEM_WING_OF_DIMENSION) ||
            pItem->AncientDiscriminator > 0 ||
            IsDivineArchangelWeapon(pItem) ||
            pItem->Type == ITEM_LOCHS_FEATHER ||
            pItem->Type == ITEM_FRUITS ||
            pItem->Type == ITEM_WEAPON_OF_ARCHANGEL ||
            pItem->Type == ITEM_SPIRIT ||
            (pItem->Type >= ITEM_GEMSTONE && pItem->Type <= ITEM_HIGHER_REFINE_STONE) ||
            (iLevel > 6 && pItem->Type < ITEM_WING) ||
            pItem->ExcellentFlags > 0 ||
            (pItem->Type >= ITEM_CLAW_OF_BEAST && pItem->Type <= ITEM_HORN_OF_FENRIR)
            || pItem->Type == ITEM_FLAME_OF_CONDOR
            || pItem->Type == ITEM_FEATHER_OF_CONDOR
            || pItem->Type == ITEM_SEALED_GOLDEN_BOX
            || pItem->Type == ITEM_SEALED_SILVER_BOX
            || IsSmallWing(pItem)
            || pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
            || pItem->Type == ITEM_DEMON
            || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
            || pItem->Type == ITEM_SAPPHIRE_RING
            || pItem->Type == ITEM_RUBY_RING
            || pItem->Type == ITEM_TOPAZ_RING
            || pItem->Type == ITEM_AMETHYST_RING
            || pItem->Type == ITEM_RUBY_NECKLACE
            || pItem->Type == ITEM_EMERALD_NECKLACE
            || pItem->Type == ITEM_SAPPHIRE_NECKLACE
            || pItem->Type == ITEM_SILVER_KEY
            || pItem->Type == ITEM_GOLD_KEY
            || (pItem->Type == ITEM_WIZARDS_RING && iLevel == 0)
            || (pItem->Type >= ITEM_CAPE_OF_FIGHTER && pItem->Type <= ITEM_CAPE_OF_OVERRULE)
            || (COMGEM::isCompiledGem(pItem))
            )
        {
            if (true == pItem->bPeriodItem && false == pItem->bExpiredPeriod)
            {
                return false;
            }
            else if (pItem->Type == ITEM_PET_PANDA
                || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
                || pItem->Type == ITEM_DEMON
                || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
                || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
                || pItem->Type == ITEM_PET_SKELETON
                )
            {
                if (true == pItem->bPeriodItem && true == pItem->bExpiredPeriod)
                {
                    return true;
                }
                else
                    return false;
            }
            return true;
        }

        return false;
    }
}
