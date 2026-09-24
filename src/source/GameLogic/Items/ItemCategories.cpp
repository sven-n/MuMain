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
        if ((pItem->Type >= ITEM_HELPER + 46 && pItem->Type <= ITEM_HELPER + 48)
            || (pItem->Type == ITEM_POTION + 54)
            || (pItem->Type >= ITEM_POTION + 58 && pItem->Type <= ITEM_POTION + 62)
            || (pItem->Type >= ITEM_POTION + 145 && pItem->Type <= ITEM_POTION + 150)
            || (pItem->Type >= ITEM_HELPER + 125 && pItem->Type <= ITEM_HELPER + 127)
            || pItem->Type == ITEM_POTION + 53
            || IsSeal(pItem)
            || (pItem->Type >= ITEM_POTION + 70 && pItem->Type <= ITEM_POTION + 71)
            || (pItem->Type >= ITEM_POTION + 72 && pItem->Type <= ITEM_POTION + 77)
            || (pItem->Type == ITEM_HELPER + 59)
            || (pItem->Type >= ITEM_HELPER + 54 && pItem->Type <= ITEM_HELPER + 58)
            || (pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82)
            || (pItem->Type == ITEM_HELPER + 60)
            || (pItem->Type == ITEM_HELPER + 61)
            || (pItem->Type == ITEM_POTION + 91)
            || (pItem->Type >= ITEM_POTION + 92 && pItem->Type <= ITEM_POTION + 93)
            || (pItem->Type == ITEM_POTION + 95)
            || (pItem->Type == ITEM_POTION + 94)
            || (pItem->Type >= ITEM_HELPER + 62 && pItem->Type <= ITEM_HELPER + 63)
            || (pItem->Type >= ITEM_POTION + 97 && pItem->Type <= ITEM_POTION + 98)
            || (pItem->Type == ITEM_POTION + 96)
            || (pItem->Type == ITEM_DEMON || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN)
            || (pItem->Type == ITEM_HELPER + 69)
            || (pItem->Type == ITEM_HELPER + 70)
            || pItem->Type == ITEM_HELPER + 81
            || pItem->Type == ITEM_HELPER + 82
            || pItem->Type == ITEM_HELPER + 93
            || pItem->Type == ITEM_HELPER + 94
            || pItem->Type == ITEM_HELPER + 107
            || pItem->Type == ITEM_HELPER + 104
            || pItem->Type == ITEM_HELPER + 105
            || pItem->Type == ITEM_HELPER + 103
            || pItem->Type == ITEM_POTION + 133
            || pItem->Type == ITEM_HELPER + 109
            || pItem->Type == ITEM_HELPER + 110
            || pItem->Type == ITEM_HELPER + 111
            || pItem->Type == ITEM_HELPER + 112
            || pItem->Type == ITEM_HELPER + 113
            || pItem->Type == ITEM_HELPER + 114
            || pItem->Type == ITEM_HELPER + 115
            || pItem->Type == ITEM_POTION + 112
            || pItem->Type == ITEM_POTION + 113
            || pItem->Type == ITEM_POTION + 120
            || pItem->Type == ITEM_POTION + 123
            || pItem->Type == ITEM_POTION + 124
            || pItem->Type == ITEM_POTION + 134
            || pItem->Type == ITEM_POTION + 135
            || pItem->Type == ITEM_POTION + 136
            || pItem->Type == ITEM_POTION + 137
            || pItem->Type == ITEM_POTION + 138
            || pItem->Type == ITEM_POTION + 139
            || IsSmallWing(pItem)
            || pItem->Type == ITEM_HELPER + 116
            || pItem->Type == ITEM_PET_UNICORN
            || pItem->Type == ITEM_HELPER + 124
            || pItem->Type == ITEM_POTION + 114
            || pItem->Type == ITEM_POTION + 115
            || pItem->Type == ITEM_POTION + 116
            || pItem->Type == ITEM_POTION + 117
            || pItem->Type == ITEM_POTION + 118
            || pItem->Type == ITEM_POTION + 119
            || pItem->Type == ITEM_POTION + 126
            || pItem->Type == ITEM_POTION + 127
            || pItem->Type == ITEM_POTION + 128
            || pItem->Type == ITEM_POTION + 129
            || pItem->Type == ITEM_POTION + 130
            || pItem->Type == ITEM_POTION + 131
            || pItem->Type == ITEM_POTION + 132
            || pItem->Type == ITEM_HELPER + 121
            || pItem->Type == ITEM_POTION + 140
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
            || pItem->Type == ITEM_POTION + 121
            || pItem->Type == ITEM_POTION + 122
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
            || pItem->Type == ITEM_POTION + 112
            || pItem->Type == ITEM_POTION + 113
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
