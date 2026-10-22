#pragma once

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"

namespace Data::Items
{
// Whether a new item can take the item type: it has neither an item nor a
// model entry (a model without an item still belongs to that number, e.g. a
// model the game draws for an item of another number).
inline bool IsFreeItemType(int itemType)
{
    return IsValidItemType(itemType) && g_ItemDatabase.Find(itemType) == nullptr &&
           g_ItemModelDatabase.Find(itemType) == nullptr;
}
} // namespace Data::Items
