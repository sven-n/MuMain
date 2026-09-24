#pragma once

typedef struct tagITEM ITEM;

namespace GameLogic::Items
{
    bool IsWingItem(const ITEM* pItem);
    bool IsSecondTierWingExceptCape(const ITEM* pItem);
    bool IsThirdTierWing(const ITEM* pItem);
    bool IsSmallWing(const ITEM* pItem);
    bool IsClothWing(const ITEM* pItem);
    bool IsClothWingModel(int modelType);

    bool IsRideableMount(const ITEM* pItem);
    bool IsRideableMountModel(int modelType);
    bool IsHornMountModel(int modelType);
    bool IsFlyingMount(const ITEM* pItem);
    bool HasFlightEquipment(const ITEM* pItemHelper, const ITEM* pItemWing);

    bool IsJewelItem(const ITEM* pItem);
    bool IsSocketSeedOrSphere(const ITEM* pItem);
    bool IsSocketSeedOrSphereType(int itemType);
    bool IsSocketSeedOrSphereModel(int modelType);

    bool IsSeal(const ITEM* pItem);
    bool IsSealType(int itemType);
    bool IsSealModel(int modelType);
    bool IsGambleItem(const ITEM* pItem);
    bool IsGambleItemType(int itemType);
    bool IsGambleItemModel(int modelType);
    bool IsCharacterCard(const ITEM* pItem);
    bool IsCharacterCardModel(int modelType);
    bool IsDevilSquareItem(const ITEM* pItem);
    bool IsDevilSquareItemType(int itemType);
    bool IsDivineArchangelWeapon(const ITEM* pItem);
    bool IsDivineArchangelWeaponType(int itemType);
    bool IsDivineArchangelWeaponModel(int modelType);

    bool IsPartChargeItem(const ITEM* pItem);
    bool IsHighValueItem(const ITEM* pItem);
}
