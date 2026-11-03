#pragma once

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"

namespace Data::Items
{
// Whether a new item can take the item type: the item files have no entry for
// it (an item or an entry without a name) and it has no model entry (a model
// needs an entry, but a file may still lack it).
inline bool IsFreeItemType(int itemType)
{
    return IsValidItemType(itemType) && g_ItemDatabase.FindEntry(itemType) == nullptr &&
           g_ItemModelDatabase.Find(itemType) == nullptr;
}
} // namespace Data::Items
