#pragma once

typedef struct tagITEM ITEM;

// Item categories. The ones in the first part come from the item data
// (tags, slot, wing tier and weapon kind in Data/Items); see docs/item-data.md.
namespace GameLogic::Items
{
    // Wings
    bool IsWingItem(const ITEM* pItem);
    bool IsSecondTierWingExceptCape(const ITEM* pItem);
    // Every second tier wing, the Capes of Lord and Fighter included.
    bool IsSecondTierWing(const ITEM* pItem);
    bool IsThirdTierWing(const ITEM* pItem);
    bool IsSecondOrThirdTierWing(const ITEM* pItem);
    // The Capes of Lord and Fighter.
    bool IsSecondTierCape(const ITEM* pItem);
    bool IsSmallWing(const ITEM* pItem);

    // Mounts and pets
    // The model of an item that goes into the pet slot (helpers, mounts, pets).
    bool IsPetSlotModel(int modelType);
    bool IsRideableMount(const ITEM* pItem);
    bool IsRideableMountModel(int modelType);
    bool IsHornMountModel(int modelType);
    bool IsFlyingMount(const ITEM* pItem);
    // Wings or a flying mount that are not broken (durability above 0).
    bool HasFlightEquipment(const ITEM* pItemHelper, const ITEM* pItemWing);
    bool IsDarkLordPet(const ITEM* pItem);
    bool IsDarkLordPetType(int itemType);
    bool IsDemonOrSpiritOfGuardian(const ITEM* pItem);
    bool IsDemonOrSpiritOfGuardianType(int itemType);
    bool IsDemonOrSpiritOfGuardianModel(int modelType);
    bool IsPandaOrSkeletonItem(const ITEM* pItem);
    bool IsPandaOrSkeletonType(int itemType);
    bool IsTransformationRingType(int itemType);

    // Crafting materials
    bool IsJewelItem(const ITEM* pItem);
    bool IsRefineStone(const ITEM* pItem);
    bool IsSocketSeedOrSphere(const ITEM* pItem);
    bool IsSocketSeedOrSphereType(int itemType);
    bool IsSocketSeedOrSphereModel(int modelType);
    bool IsSocketSeed(const ITEM* pItem);
    bool IsSocketSphere(const ITEM* pItem);
    bool IsSocketSphereModel(int modelType);
    bool IsSocketSeedSphere(const ITEM* pItem);
    bool IsSocketSeedSphereType(int itemType);

    // Consumables
    bool IsHealingPotion(const ITEM* pItem);
    bool IsHealingPotionType(int itemType);
    bool IsManaPotionType(int itemType);
    bool IsComplexPotion(const ITEM* pItem);
    bool IsComplexPotionType(int itemType);
    bool IsShieldPotionType(int itemType);
    bool IsElitePotion(const ITEM* pItem);
    bool IsElitePotionType(int itemType);
    bool IsElixir(const ITEM* pItem);
    bool IsElixirType(int itemType);
    bool IsBuffScroll(const ITEM* pItem);
    bool IsBuffScrollType(int itemType);
    bool IsBattleOrStrengthScroll(const ITEM* pItem);
    bool IsBattleOrStrengthScrollType(int itemType);
    bool IsAmmunition(const ITEM* pItem);
    bool IsAmmunitionType(int itemType);
    bool IsAmmunitionModel(int modelType);

    // Tickets and cash shop items
    bool IsLuckyItemTicket(const ITEM* pItem);
    bool IsLuckyItemTicketModel(int modelType);
    // The lucky set armour the lucky item tickets give.
    bool IsLuckyItemType(int itemType);
    bool IsGemJewelry(const ITEM* pItem);
    bool IsGambleItem(const ITEM* pItem);
    bool IsGambleItemType(int itemType);
    bool IsGambleItemModel(int modelType);

    // Event and quest items
    bool IsBloodCastleTicketPart(const ITEM* pItem);
    bool IsBloodCastleTicketPartType(int itemType);
    bool IsBloodCastleTicketPartModel(int modelType);
    bool IsSecondClassQuestItem(const ITEM* pItem);
    bool IsSecondClassQuestItemType(int itemType);
    bool IsThirdClassQuestItem(const ITEM* pItem);

    // Class equipment
    bool IsDivineArchangelWeapon(const ITEM* pItem);
    bool IsDivineArchangelWeaponType(int itemType);
    bool IsDivineArchangelWeaponModel(int modelType);
    bool IsSummonerBook(const ITEM* pItem);
    bool IsSummonerBookType(int itemType);
    bool IsSummonerBookModel(int modelType);
    bool IsSceptreType(int itemType);
    // The Magic Gladiator's swords with magic power.
    bool IsMagicSwordType(int itemType);

    // Weapons
    bool IsBowType(int itemType);
    bool IsBowModel(int modelType);
    bool IsCrossbowType(int itemType);
    bool IsCrossbowModel(int modelType);
    bool IsSpearType(int itemType);
    bool IsSpearModel(int modelType);
    bool IsShieldType(int itemType);
    // A bow, a crossbow, arrows or bolts.
    bool IsBowCrossbowOrAmmunition(const ITEM* pItem);
    bool IsSwordOrGloveType(int itemType);
    bool IsSwordOrGloveModel(int modelType);
    bool IsSwordAxeMaceOrGloveType(int itemType);
    // A staff or a stick.
    bool IsStaffType(int itemType);
    // A staff, a stick or a book.
    bool IsStaffOrBookType(int itemType);
    bool IsStaffOrBookModel(int modelType);
    // A sword, glove, axe, mace, spear, bow, crossbow, staff or book, or arrows or bolts.
    bool IsWeaponOrAmmunition(const ITEM* pItem);

    // Item value
    bool IsHighValueItem(const ITEM* pItem);

    // Only used for drawing items and for tooltips; still hardcoded until
    // phase 4 (model data) and phase 8 (tooltip data).
    bool IsClothWing(const ITEM* pItem);
    bool IsClothWingModel(int modelType);
    bool IsWingMixCharm(const ITEM* pItem);
    bool IsWingMixCharmType(int itemType);
    bool IsWingMixCharmModel(int modelType);
    bool IsResetFruit(const ITEM* pItem);
    bool IsResetFruitType(int itemType);
    bool IsSealType(int itemType);
    bool IsHealingOrDivinitySealType(int itemType);
    bool IsEventTicket(const ITEM* pItem);
    bool IsEventTicketType(int itemType);
    bool IsDoppelgangerOrVarkaTicket(const ITEM* pItem);
    bool IsDoppelgangerOrVarkaTicketType(int itemType);
    bool IsDevilSquareItem(const ITEM* pItem);
    bool IsDevilSquareItemType(int itemType);
    bool IsChocolateBox(const ITEM* pItem);
    bool IsChocolateBoxType(int itemType);
    bool IsRibbonBox(const ITEM* pItem);
    bool IsRibbonBoxType(int itemType);
    bool IsSecromiconQuestItem(const ITEM* pItem);
    bool IsSummonerStickModel(int modelType);
}
