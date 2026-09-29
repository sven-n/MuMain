#pragma once

#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"

namespace Render::Items
{
// The item model entry of a model slot (MODEL_ITEM + item type), or nullptr
// for slots that are not item models.
inline const Data::Items::ItemModelDefinition* FindItemModel(int modelType)
{
    return g_ItemModelDatabase.Find(modelType - MODEL_ITEM);
}

// The item a model slot is drawn like: its own item type, or for the second
// models of the Rage Fighter gloves (one per hand, drawn on the character)
// their glove. -1 for models that are not items.
inline int GetItemTypeOfModel(int modelType)
{
    static_assert(MODEL_SWORD_35_RIGHT - MODEL_SWORD_32_LEFT == 7 && ITEM_PHOENIX_SOUL_STAR - ITEM_SACRED_GLOVE == 3,
                  "a left and a right model for each of the four gloves");
    if (modelType >= MODEL_SWORD_32_LEFT && modelType <= MODEL_SWORD_35_RIGHT)
    {
        return ITEM_SACRED_GLOVE + (modelType - MODEL_SWORD_32_LEFT) / 2;
    }
    const int itemType = modelType - MODEL_ITEM;
    return Data::Items::IsValidItemType(itemType) ? itemType : -1;
}
} // namespace Render::Items
