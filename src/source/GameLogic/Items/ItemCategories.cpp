#include "stdafx.h"
#include "GameLogic/Items/ItemCategories.h"

#include "Data/GameData/ItemData/ItemDatabase.h"

// The categories come from the item data (tags, slot and wing tier in
// Data/Items/*.json); see docs/item-data.md. The lists that are only used for
// drawing items and for tooltips are still hardcoded, in
// ItemDisplayCategories.cpp.
namespace
{
using Data::Items::ItemSlot;
using Data::Items::ItemTag;
using Data::Items::WingTier;

bool HasTag(int itemType, ItemTag tag)
{
    return g_ItemDatabase.HasTag(itemType, tag);
}

bool HasTag(const ITEM* pItem, ItemTag tag)
{
    return HasTag(pItem->Type, tag);
}

int ToItemType(int modelType)
{
    return modelType - MODEL_ITEM;
}

bool HasWingTier(const ITEM* pItem, WingTier tier)
{
    return g_ItemDatabase.GetWingTier(pItem->Type) == tier;
}

bool IsHornMountType(int itemType)
{
    return HasTag(itemType, ItemTag::Mount) && !HasTag(itemType, ItemTag::DarkLordPet);
}
} // namespace

namespace GameLogic::Items
{
    bool IsWingItem(const ITEM* pItem)
    {
        return g_ItemDatabase.GetSlot(pItem->Type) == ItemSlot::Wings;
    }

    // The capes of the second tier have their own formulas.
    bool IsSecondTierWingExceptCape(const ITEM* pItem)
    {
        return HasWingTier(pItem, WingTier::Second) && pItem->Type != ITEM_CAPE_OF_LORD &&
               pItem->Type != ITEM_CAPE_OF_FIGHTER;
    }

    bool IsThirdTierWing(const ITEM* pItem)
    {
        return HasWingTier(pItem, WingTier::Third);
    }

    bool IsSmallWing(const ITEM* pItem)
    {
        return HasWingTier(pItem, WingTier::Small);
    }

    bool IsRideableMount(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::Mount);
    }

    bool IsRideableMountModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::Mount);
    }

    bool IsHornMountModel(int modelType)
    {
        return IsHornMountType(ToItemType(modelType));
    }

    bool IsFlyingMount(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::Flying);
    }

    bool HasFlightEquipment(const ITEM* pItemHelper, const ITEM* pItemWing)
    {
        return IsWingItem(pItemWing) || IsFlyingMount(pItemHelper);
    }

    bool IsDarkLordPet(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::DarkLordPet);
    }

    bool IsDarkLordPetType(int itemType)
    {
        return HasTag(itemType, ItemTag::DarkLordPet);
    }

    bool IsDemonOrSpiritOfGuardian(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::GuardianPet);
    }

    bool IsDemonOrSpiritOfGuardianType(int itemType)
    {
        return HasTag(itemType, ItemTag::GuardianPet);
    }

    bool IsDemonOrSpiritOfGuardianModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::GuardianPet);
    }

    bool IsPandaOrSkeletonItem(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::PandaOrSkeleton);
    }

    bool IsJewelItem(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::Jewel);
    }

    bool IsRefineStone(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::RefineStone);
    }

    bool IsRefineStoneModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::RefineStone);
    }

    bool IsSocketSeedOrSphereType(int itemType)
    {
        return HasTag(itemType, ItemTag::SocketSeed) || HasTag(itemType, ItemTag::SocketSphere) ||
               HasTag(itemType, ItemTag::SocketSeedSphere);
    }

    bool IsSocketSeedOrSphere(const ITEM* pItem)
    {
        return IsSocketSeedOrSphereType(pItem->Type);
    }

    bool IsSocketSeedOrSphereModel(int modelType)
    {
        return IsSocketSeedOrSphereType(ToItemType(modelType));
    }

    bool IsSocketSeed(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::SocketSeed);
    }

    bool IsSocketSeedModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::SocketSeed);
    }

    bool IsSocketSphere(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::SocketSphere);
    }

    bool IsSocketSphereModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::SocketSphere);
    }

    bool IsSocketSeedSphere(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::SocketSeedSphere);
    }

    bool IsSocketSeedSphereType(int itemType)
    {
        return HasTag(itemType, ItemTag::SocketSeedSphere);
    }

    bool IsSocketSeedSphereModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::SocketSeedSphere);
    }

    bool IsHealingPotion(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::HealingPotion);
    }

    bool IsHealingPotionType(int itemType)
    {
        return HasTag(itemType, ItemTag::HealingPotion);
    }

    bool IsManaPotionType(int itemType)
    {
        return HasTag(itemType, ItemTag::ManaPotion);
    }

    bool IsComplexPotion(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::ComplexPotion);
    }

    bool IsComplexPotionType(int itemType)
    {
        return HasTag(itemType, ItemTag::ComplexPotion);
    }

    bool IsElitePotion(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::ElitePotion);
    }

    bool IsElitePotionType(int itemType)
    {
        return HasTag(itemType, ItemTag::ElitePotion);
    }

    bool IsElitePotionModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::ElitePotion);
    }

    bool IsElixir(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::Elixir);
    }

    bool IsElixirType(int itemType)
    {
        return HasTag(itemType, ItemTag::Elixir);
    }

    bool IsElixirModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::Elixir);
    }

    bool IsBuffScroll(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::BuffScroll);
    }

    bool IsBuffScrollType(int itemType)
    {
        return HasTag(itemType, ItemTag::BuffScroll);
    }

    bool IsBuffScrollModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::BuffScroll);
    }

    bool IsBattleOrStrengthScroll(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::BattleOrStrengthScroll);
    }

    bool IsBattleOrStrengthScrollType(int itemType)
    {
        return HasTag(itemType, ItemTag::BattleOrStrengthScroll);
    }

    bool IsBattleOrStrengthScrollModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::BattleOrStrengthScroll);
    }

    bool IsAmmunition(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::Ammunition);
    }

    bool IsAmmunitionType(int itemType)
    {
        return HasTag(itemType, ItemTag::Ammunition);
    }

    bool IsAmmunitionModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::Ammunition);
    }

    bool IsLuckyItemTicket(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::LuckyItemTicket);
    }

    bool IsLuckyItemTicketModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::LuckyItemTicket);
    }

    bool IsCashShopItem(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::CashShop);
    }

    bool IsGemJewelry(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::GemJewelry);
    }

    bool IsGambleItem(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::GambleItem);
    }

    bool IsGambleItemType(int itemType)
    {
        return HasTag(itemType, ItemTag::GambleItem);
    }

    bool IsGambleItemModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::GambleItem);
    }

    bool IsBloodCastleTicketPart(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::BloodCastleTicketPart);
    }

    bool IsBloodCastleTicketPartType(int itemType)
    {
        return HasTag(itemType, ItemTag::BloodCastleTicketPart);
    }

    bool IsBloodCastleTicketPartModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::BloodCastleTicketPart);
    }

    bool IsSecondClassQuestItem(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::SecondClassQuestItem);
    }

    bool IsSecondClassQuestItemType(int itemType)
    {
        return HasTag(itemType, ItemTag::SecondClassQuestItem);
    }

    bool IsThirdClassQuestItem(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::ThirdClassQuestItem);
    }

    bool IsDivineArchangelWeapon(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::DivineArchangelWeapon);
    }

    bool IsDivineArchangelWeaponType(int itemType)
    {
        return HasTag(itemType, ItemTag::DivineArchangelWeapon);
    }

    bool IsDivineArchangelWeaponModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::DivineArchangelWeapon);
    }

    bool IsSummonerBook(const ITEM* pItem)
    {
        return HasTag(pItem, ItemTag::SummonerBook);
    }

    bool IsSummonerBookType(int itemType)
    {
        return HasTag(itemType, ItemTag::SummonerBook);
    }

    bool IsSummonerBookModel(int modelType)
    {
        return HasTag(ToItemType(modelType), ItemTag::SummonerBook);
    }

    bool IsHighValueItem(const ITEM* pItem)
    {
        const bool isValuable = HasTag(pItem, ItemTag::Valuable) || pItem->AncientDiscriminator > 0 ||
                                pItem->ExcellentFlags > 0 || (pItem->Level > 6 && pItem->Type < ITEM_WING) ||
                                (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0);
        if (!isValuable)
        {
            return false;
        }

        // Rented items are not valuable while the rental time runs; rented
        // pets only once it ran out.
        if (pItem->bPeriodItem && !pItem->bExpiredPeriod)
        {
            return false;
        }

        if (IsPandaOrSkeletonItem(pItem) || IsDemonOrSpiritOfGuardian(pItem))
        {
            return pItem->bPeriodItem && pItem->bExpiredPeriod;
        }

        return true;
    }
}
