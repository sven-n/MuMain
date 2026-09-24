#include "stdafx.h"
#include "GameLogic/Items/ItemCategories.h"

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

    bool IsSocketSeedOrSphereType(int itemType)
    {
        return (itemType >= ITEM_SEED_FIRE && itemType <= ITEM_SEED_EARTH)
            || (itemType >= ITEM_SPHERE_MONO && itemType <= ITEM_SPHERE_5)
            || (itemType >= ITEM_SEED_SPHERE_FIRE_1 && itemType <= ITEM_SEED_SPHERE_EARTH_5);
    }

    bool IsSealType(int itemType)
    {
        return itemType == ITEM_SEAL_OF_ASCENSION
            || itemType == ITEM_SEAL_OF_WEALTH
            || itemType == ITEM_SEAL_OF_SUSTENANCE;
    }

    bool IsGambleItemType(int itemType)
    {
        return itemType == ITEM_GAMBLE_SWORD_MACE_SPEAR
            || itemType == ITEM_GAMBLE_STAFF
            || itemType == ITEM_GAMBLE_BOW_CROSSBOW
            || itemType == ITEM_GAMBLE_SCEPTER
            || itemType == ITEM_GAMBLE_STICK;
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

    bool IsSocketSeedOrSphere(const ITEM* pItem)
    {
        return IsSocketSeedOrSphereType(pItem->Type);
    }

    bool IsSocketSeedOrSphereModel(int modelType)
    {
        return IsSocketSeedOrSphereType(modelType - MODEL_ITEM);
    }

    bool IsSeal(const ITEM* pItem)
    {
        return IsSealType(pItem->Type);
    }

    bool IsSealModel(int modelType)
    {
        return IsSealType(modelType - MODEL_ITEM);
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

    bool IsDevilSquareItem(const ITEM* pItem)
    {
        return pItem->Type == ITEM_DEVILS_EYE
            || pItem->Type == ITEM_DEVILS_KEY
            || pItem->Type == ITEM_DEVILS_INVITATION;
    }
}
