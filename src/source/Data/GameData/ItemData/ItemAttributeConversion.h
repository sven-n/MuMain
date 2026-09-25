#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStructs.h"

#include <string>

// Converts between the legacy ITEM_ATTRIBUTE record (item.bmd layout, and the
// ItemAttribute[] table the game reads) and ItemDefinition. The stats convert
// losslessly in both directions. ITEM_ATTRIBUTE holds one name only: the
// shown name (ItemDefinition::name) goes into it, and names longer than
// MAX_ITEM_NAME - 1 characters are cut.
namespace Data::Items
{
std::wstring ReadItemAttributeName(const ITEM_ATTRIBUTE& attribute);

// Copies everything except the name.
void CopyItemAttributeStats(const ITEM_ATTRIBUTE& attribute, ItemDefinition& definition);

// The attribute's name becomes the neutral (English) name.
ItemDefinition ToItemDefinition(const ITEM_ATTRIBUTE& attribute, int itemType);

void ToItemAttribute(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute);
} // namespace Data::Items
