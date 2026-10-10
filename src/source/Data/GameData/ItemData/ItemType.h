#pragma once

#include "Core/Globals/_define.h"

// An item type is the item's index in the item tables:
// group * MAX_ITEM_INDEX + number. Group and number are the item's identity,
// the same as (Group, Number) on the server.
namespace Data::Items
{
constexpr int MakeItemType(int group, int number)
{
    return group * MAX_ITEM_INDEX + number;
}

constexpr int GetItemGroup(int itemType)
{
    return itemType / MAX_ITEM_INDEX;
}

constexpr int GetItemNumber(int itemType)
{
    return itemType % MAX_ITEM_INDEX;
}

constexpr bool IsValidItemType(int itemType)
{
    return itemType >= 0 && itemType < MAX_ITEM;
}

constexpr bool IsValidItemId(int group, int number)
{
    return group >= 0 && group < MAX_ITEM_TYPE && number >= 0 && number < MAX_ITEM_INDEX;
}
} // namespace Data::Items
