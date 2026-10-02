#pragma once

#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemType.h"

// The model slot of each item: the entry of the Models array its model is
// opened into, which objects (equipment, dropped items) keep in their Type.
// Every item has a slot of its own; items that share a model share its
// loaded data (SharedModels.json), not the slot.
//
// All conversions between item types and model slots go through these
// functions, so where the slots lie can change (e.g. for new item groups)
// without changing the callers. The named slots of single items
// (MODEL_KRIS, ...) stay valid as long as an item keeps its slot.
namespace Data::Items
{
// The model slot of an item type.
constexpr int ToModelSlot(int itemType)
{
    return MODEL_ITEM + itemType;
}

// The item type of an item's model slot. Other slots (-1 for none, the class
// models such as MODEL_BODY_* or MODEL_HELM2) give no item type; callers that
// can get them check IsItemModelSlot where it matters.
constexpr int ToItemType(int modelSlot)
{
    return modelSlot - MODEL_ITEM;
}

// Whether the model slot belongs to an item.
constexpr bool IsItemModelSlot(int modelSlot)
{
    return IsValidItemType(ToItemType(modelSlot));
}
} // namespace Data::Items
