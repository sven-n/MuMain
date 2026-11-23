#pragma once

#include "Data/GameData/ItemData/ItemPetStats.h"

namespace GameLogic::Pets
{
// The pet stats of the item (its "petStats" in the item data); none when the item is missing or has none.
const Data::Items::ItemPetStats& GetItemPetStats(int itemType);
} // namespace GameLogic::Pets
