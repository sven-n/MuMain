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
} // namespace Render::Items
