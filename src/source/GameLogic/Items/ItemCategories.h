#pragma once

typedef struct tagITEM ITEM;

namespace GameLogic::Items
{
    // Wings
    bool IsWingItem(const ITEM* pItem);
    bool IsSecondTierWingExceptCape(const ITEM* pItem);
    bool IsThirdTierWing(const ITEM* pItem);
    bool IsSmallWing(const ITEM* pItem);
    bool IsClothWing(const ITEM* pItem);
    bool IsClothWingModel(int modelType);

    // Mounts and pets
    bool IsRideableMount(const ITEM* pItem);
    bool IsRideableMountModel(int modelType);
    bool IsHornMountModel(int modelType);
    bool IsFlyingMount(const ITEM* pItem);
    bool HasFlightEquipment(const ITEM* pItemHelper, const ITEM* pItemWing);
    bool IsDarkLordPet(const ITEM* pItem);
    bool IsDarkLordPetType(int itemType);
    bool IsDemonOrSpiritOfGuardian(const ITEM* pItem);
    bool IsDemonOrSpiritOfGuardianType(int itemType);
    bool IsDemonOrSpiritOfGuardianModel(int modelType);
    bool IsPandaOrSkeletonItem(const ITEM* pItem);

    // Crafting materials
    bool IsJewelItem(const ITEM* pItem);
    bool IsRefineStone(const ITEM* pItem);
    bool IsRefineStoneModel(int modelType);
    bool IsWingMixCharm(const ITEM* pItem);
    bool IsWingMixCharmType(int itemType);
    bool IsWingMixCharmModel(int modelType);
    bool IsSocketSeedOrSphere(const ITEM* pItem);
    bool IsSocketSeedOrSphereType(int itemType);
    bool IsSocketSeedOrSphereModel(int modelType);
    bool IsSocketSeed(const ITEM* pItem);
    bool IsSocketSeedModel(int modelType);
    bool IsSocketSphere(const ITEM* pItem);
    bool IsSocketSphereModel(int modelType);
    bool IsSocketSeedSphere(const ITEM* pItem);
    bool IsSocketSeedSphereType(int itemType);
    bool IsSocketSeedSphereModel(int modelType);

    // Consumables
    bool IsHealingPotion(const ITEM* pItem);
    bool IsHealingPotionType(int itemType);
    bool IsManaPotionType(int itemType);
    bool IsComplexPotion(const ITEM* pItem);
    bool IsComplexPotionType(int itemType);
    bool IsElitePotion(const ITEM* pItem);
    bool IsElitePotionType(int itemType);
    bool IsElitePotionModel(int modelType);
    bool IsElixir(const ITEM* pItem);
    bool IsElixirType(int itemType);
    bool IsElixirModel(int modelType);
    bool IsBuffScroll(const ITEM* pItem);
    bool IsBuffScrollType(int itemType);
    bool IsBuffScrollModel(int modelType);
    bool IsBattleOrStrengthScroll(const ITEM* pItem);
    bool IsBattleOrStrengthScrollType(int itemType);
    bool IsBattleOrStrengthScrollModel(int modelType);
    bool IsResetFruit(const ITEM* pItem);
    bool IsResetFruitType(int itemType);
    bool IsResetFruitModel(int modelType);
    bool IsSeal(const ITEM* pItem);
    bool IsSealType(int itemType);
    bool IsSealModel(int modelType);
    bool IsHealingOrDivinitySeal(const ITEM* pItem);
    bool IsHealingOrDivinitySealType(int itemType);
    bool IsHealingOrDivinitySealModel(int modelType);
    bool IsAmmunition(const ITEM* pItem);
    bool IsAmmunitionType(int itemType);
    bool IsAmmunitionModel(int modelType);

    // Tickets and cash shop items
    bool IsEventTicket(const ITEM* pItem);
    bool IsEventTicketType(int itemType);
    bool IsEventTicketModel(int modelType);
    bool IsDoppelgangerOrVarkaTicket(const ITEM* pItem);
    bool IsDoppelgangerOrVarkaTicketType(int itemType);
    bool IsDoppelgangerOrVarkaTicketModel(int modelType);
    bool IsRareItemTicket(const ITEM* pItem);
    bool IsLuckyItemTicket(const ITEM* pItem);
    bool IsLuckyItemTicketModel(int modelType);
    bool IsAccountServiceItem(const ITEM* pItem);
    bool IsAccountServiceItemModel(int modelType);
    bool IsDayPass(const ITEM* pItem);
    bool IsDayPassModel(int modelType);
    bool IsHourPass(const ITEM* pItem);
    bool IsHourPassModel(int modelType);
    bool IsPackageBox(const ITEM* pItem);
    bool IsPackageBoxModel(int modelType);
    bool IsSilverOrGoldKey(const ITEM* pItem);
    bool IsSilverOrGoldKeyModel(int modelType);
    bool IsGemJewelry(const ITEM* pItem);
    bool IsGambleItem(const ITEM* pItem);
    bool IsGambleItemType(int itemType);
    bool IsGambleItemModel(int modelType);
    bool IsCharacterCard(const ITEM* pItem);
    bool IsCharacterCardModel(int modelType);
    bool IsPartChargeItem(const ITEM* pItem);

    // Event and quest items
    bool IsDevilSquareItem(const ITEM* pItem);
    bool IsDevilSquareItemType(int itemType);
    bool IsBloodCastleTicketPart(const ITEM* pItem);
    bool IsBloodCastleTicketPartType(int itemType);
    bool IsBloodCastleTicketPartModel(int modelType);
    bool IsChocolateBox(const ITEM* pItem);
    bool IsChocolateBoxType(int itemType);
    bool IsChocolateBoxModel(int modelType);
    bool IsRibbonBox(const ITEM* pItem);
    bool IsRibbonBoxType(int itemType);
    bool IsRibbonBoxModel(int modelType);
    bool IsSecondClassQuestItem(const ITEM* pItem);
    bool IsSecondClassQuestItemType(int itemType);
    bool IsThirdClassQuestItem(const ITEM* pItem);
    bool IsSecromiconQuestItem(const ITEM* pItem);
    bool IsSecromiconQuestItemModel(int modelType);

    // Class equipment
    bool IsDivineArchangelWeapon(const ITEM* pItem);
    bool IsDivineArchangelWeaponType(int itemType);
    bool IsDivineArchangelWeaponModel(int modelType);
    bool IsSummonerBook(const ITEM* pItem);
    bool IsSummonerBookType(int itemType);
    bool IsSummonerBookModel(int modelType);
    bool IsSummonerStickModel(int modelType);
    bool IsSummonerSkillParchmentModel(int modelType);
    bool IsRageFighterSkillParchmentModel(int modelType);

    // Item value
    bool IsHighValueItem(const ITEM* pItem);
}
