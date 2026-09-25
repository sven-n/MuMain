#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStructs.h"

// Converts between the legacy ITEM_ATTRIBUTE record (item.bmd layout) and
// ItemDefinition. The conversion is lossless in both directions, except that
// names longer than MAX_ITEM_NAME - 1 characters are cut when converting back.
// ITEM_ATTRIBUTE has no English name, so englishName stays empty.
namespace Data::Items
{
ItemDefinition ToItemDefinition(const ITEM_ATTRIBUTE& attribute, int itemType);
void ToItemAttribute(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute);
} // namespace Data::Items
