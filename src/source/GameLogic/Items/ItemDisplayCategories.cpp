#include "stdafx.h"
#include "GameLogic/Items/ItemCategories.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"

// Item lists that are only used for drawing items and for tooltips. They stay
// hardcoded until the model data (phase 4) and the tooltip data (phase 8)
// replace them. The categories the game rules use come from the item data
// (ItemCategories.cpp).
namespace
{
// Capes worn as cloth are marked in the item model files ("cloth": true); the
// cloth of the character is deleted when one is put on or taken off.
bool IsClothWingType(int itemType)
{
    const Data::Items::ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType);
    return model != nullptr && model->cloth;
}

bool IsSecromiconQuestItemType(int itemType)
{
    return itemType >= ITEM_SUSPICIOUS_SCRAP_OF_PAPER && itemType <= ITEM_COMPLETE_SECROMICON;
}

bool IsSummonerStickType(int itemType)
{
    return itemType >= ITEM_MISTERY_STICK && itemType <= ITEM_ETERNAL_WING_STICK;
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
    return IsClothWingType(Data::Items::ToItemType(modelType));
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
    return IsWingMixCharmType(Data::Items::ToItemType(modelType));
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

bool IsChocolateBoxType(int itemType)
{
    return itemType == ITEM_PINK_CHOCOLATE_BOX || itemType == ITEM_RED_CHOCOLATE_BOX ||
           itemType == ITEM_BLUE_CHOCOLATE_BOX;
}

bool IsSecromiconQuestItem(const ITEM* pItem)
{
    return IsSecromiconQuestItemType(pItem->Type);
}

bool IsSummonerStickModel(int modelType)
{
    return IsSummonerStickType(Data::Items::ToItemType(modelType));
}
} // namespace GameLogic::Items
