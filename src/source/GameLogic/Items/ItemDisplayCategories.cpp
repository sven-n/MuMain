#include "stdafx.h"
#include "GameLogic/Items/ItemCategories.h"

// Item lists that are only used for drawing items and for tooltips. They stay
// hardcoded until the model data (phase 4) and the tooltip data (phase 8)
// replace them. The categories the game rules use come from the item data
// (ItemCategories.cpp).
namespace
{
bool IsClothWingType(int itemType)
{
    return itemType == ITEM_CAPE_OF_LORD || itemType == ITEM_WING_OF_RUIN || itemType == ITEM_CAPE_OF_EMPEROR ||
           itemType == ITEM_CAPE_OF_FIGHTER || itemType == ITEM_CAPE_OF_OVERRULE ||
           itemType == ITEM_SMALL_CAPE_OF_LORD || itemType == ITEM_LITTLE_WARRIORS_CLOAK;
}

bool IsCharacterCardType(int itemType)
{
    return itemType == ITEM_MAGIC_GLADIATOR_CHARACTER_CARD || itemType == ITEM_DARK_LORD_CHARACTER_CARD ||
           itemType == ITEM_SUMMONER_CHARACTER_CARD;
}

bool IsAccountServiceItemType(int itemType)
{
    return itemType >= ITEM_MASTER_SKILL_RESET && itemType <= ITEM_PREMIUM_PACKAGE;
}

bool IsDayPassType(int itemType)
{
    return itemType >= ITEM_30_DAY_PASS && itemType <= ITEM_90_DAY_PASS_POINTS;
}

bool IsHourPassType(int itemType)
{
    return itemType == ITEM_3_HOUR_PASS || itemType == ITEM_5_HOUR_PASS || itemType == ITEM_10_HOUR_PASS;
}

bool IsPackageBoxType(int itemType)
{
    return itemType >= ITEM_PACKAGE_BOX_A && itemType <= ITEM_PACKAGE_BOX_F;
}

bool IsSilverOrGoldKeyType(int itemType)
{
    return itemType == ITEM_SILVER_KEY || itemType == ITEM_GOLD_KEY;
}

bool IsSecromiconQuestItemType(int itemType)
{
    return itemType >= ITEM_SUSPICIOUS_SCRAP_OF_PAPER && itemType <= ITEM_COMPLETE_SECROMICON;
}

bool IsSummonerStickType(int itemType)
{
    return itemType >= ITEM_MISTERY_STICK && itemType <= ITEM_ETERNAL_WING_STICK;
}

bool IsSummonerSkillParchmentType(int itemType)
{
    return itemType >= ITEM_CHAIN_LIGHTNING_PARCHMENT && itemType <= ITEM_INNOVATION_PARCHMENT;
}

bool IsRageFighterSkillParchmentType(int itemType)
{
    return itemType >= ITEM_CHAIN_DRIVE_PARCHMENT && itemType <= ITEM_INCREASE_BLOCK_PARCHMENT;
}
} // namespace

namespace GameLogic::Items
{
bool IsClothWing(const ITEM* pItem)
{
    return IsClothWingType(pItem->Type);
}

bool IsClothWingModel(int modelType)
{
    return IsClothWingType(modelType - MODEL_ITEM);
}

bool IsWingMixCharmType(int itemType)
{
    return itemType >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN && itemType <= ITEM_TYPE_CHARM_MIXWING + EWS_END;
}

bool IsWingMixCharm(const ITEM* pItem)
{
    return IsWingMixCharmType(pItem->Type);
}

bool IsWingMixCharmModel(int modelType)
{
    return IsWingMixCharmType(modelType - MODEL_ITEM);
}

bool IsSealType(int itemType)
{
    return itemType == ITEM_SEAL_OF_ASCENSION || itemType == ITEM_SEAL_OF_WEALTH || itemType == ITEM_SEAL_OF_SUSTENANCE;
}

bool IsSeal(const ITEM* pItem)
{
    return IsSealType(pItem->Type);
}

bool IsSealModel(int modelType)
{
    return IsSealType(modelType - MODEL_ITEM);
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
    return itemType == ITEM_DEVILS_EYE || itemType == ITEM_DEVILS_KEY || itemType == ITEM_DEVILS_INVITATION;
}

bool IsDevilSquareItem(const ITEM* pItem)
{
    return IsDevilSquareItemType(pItem->Type);
}

bool IsResetFruitType(int itemType)
{
    return itemType >= ITEM_RESET_FRUIT_STRENGTH && itemType <= ITEM_RESET_FRUIT_CONTROL;
}

bool IsResetFruit(const ITEM* pItem)
{
    return IsResetFruitType(pItem->Type);
}

bool IsResetFruitModel(int modelType)
{
    return IsResetFruitType(modelType - MODEL_ITEM);
}

bool IsHealingOrDivinitySealType(int itemType)
{
    return itemType == ITEM_SEAL_OF_HEALING || itemType == ITEM_SEAL_OF_DIVINITY;
}

bool IsHealingOrDivinitySeal(const ITEM* pItem)
{
    return IsHealingOrDivinitySealType(pItem->Type);
}

bool IsHealingOrDivinitySealModel(int modelType)
{
    return IsHealingOrDivinitySealType(modelType - MODEL_ITEM);
}

bool IsEventTicketType(int itemType)
{
    return itemType == ITEM_DEVIL_SQUARE_TICKET || itemType == ITEM_BLOOD_CASTLE_TICKET ||
           itemType == ITEM_KALIMA_TICKET;
}

bool IsEventTicket(const ITEM* pItem)
{
    return IsEventTicketType(pItem->Type);
}

bool IsEventTicketModel(int modelType)
{
    return IsEventTicketType(modelType - MODEL_ITEM);
}

bool IsDoppelgangerOrVarkaTicketType(int itemType)
{
    return itemType == ITEM_OPEN_ACCESS_TICKET_TO_DOPPELGANGER || itemType == ITEM_OPEN_ACCESS_TICKET_TO_VARKA ||
           itemType == ITEM_OPEN_ACCESS_TICKET_TO_VARKA_7;
}

bool IsDoppelgangerOrVarkaTicket(const ITEM* pItem)
{
    return IsDoppelgangerOrVarkaTicketType(pItem->Type);
}

bool IsDoppelgangerOrVarkaTicketModel(int modelType)
{
    return IsDoppelgangerOrVarkaTicketType(modelType - MODEL_ITEM);
}

bool IsAccountServiceItem(const ITEM* pItem)
{
    return IsAccountServiceItemType(pItem->Type);
}

bool IsAccountServiceItemModel(int modelType)
{
    return IsAccountServiceItemType(modelType - MODEL_ITEM);
}

bool IsDayPass(const ITEM* pItem)
{
    return IsDayPassType(pItem->Type);
}

bool IsDayPassModel(int modelType)
{
    return IsDayPassType(modelType - MODEL_ITEM);
}

bool IsHourPass(const ITEM* pItem)
{
    return IsHourPassType(pItem->Type);
}

bool IsHourPassModel(int modelType)
{
    return IsHourPassType(modelType - MODEL_ITEM);
}

bool IsPackageBox(const ITEM* pItem)
{
    return IsPackageBoxType(pItem->Type);
}

bool IsPackageBoxModel(int modelType)
{
    return IsPackageBoxType(modelType - MODEL_ITEM);
}

bool IsSilverOrGoldKey(const ITEM* pItem)
{
    return IsSilverOrGoldKeyType(pItem->Type);
}

bool IsSilverOrGoldKeyModel(int modelType)
{
    return IsSilverOrGoldKeyType(modelType - MODEL_ITEM);
}

bool IsChocolateBoxType(int itemType)
{
    return itemType == ITEM_PINK_CHOCOLATE_BOX || itemType == ITEM_RED_CHOCOLATE_BOX ||
           itemType == ITEM_BLUE_CHOCOLATE_BOX;
}

bool IsChocolateBox(const ITEM* pItem)
{
    return IsChocolateBoxType(pItem->Type);
}

bool IsChocolateBoxModel(int modelType)
{
    return IsChocolateBoxType(modelType - MODEL_ITEM);
}

bool IsRibbonBoxType(int itemType)
{
    return itemType == ITEM_RED_RIBBON_BOX || itemType == ITEM_GREEN_RIBBON_BOX || itemType == ITEM_BLUE_RIBBON_BOX;
}

bool IsRibbonBox(const ITEM* pItem)
{
    return IsRibbonBoxType(pItem->Type);
}

bool IsRibbonBoxModel(int modelType)
{
    return IsRibbonBoxType(modelType - MODEL_ITEM);
}

bool IsSecromiconQuestItem(const ITEM* pItem)
{
    return IsSecromiconQuestItemType(pItem->Type);
}

bool IsSecromiconQuestItemModel(int modelType)
{
    return IsSecromiconQuestItemType(modelType - MODEL_ITEM);
}

bool IsSummonerStickModel(int modelType)
{
    return IsSummonerStickType(modelType - MODEL_ITEM);
}

bool IsSummonerSkillParchmentModel(int modelType)
{
    return IsSummonerSkillParchmentType(modelType - MODEL_ITEM);
}

bool IsRageFighterSkillParchmentModel(int modelType)
{
    return IsRageFighterSkillParchmentType(modelType - MODEL_ITEM);
}
} // namespace GameLogic::Items
