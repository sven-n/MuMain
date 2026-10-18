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

// The name as ITEM_ATTRIBUTE holds it: cut to MAX_ITEM_NAME - 1 characters.
std::wstring CutToItemAttributeName(const std::wstring& name);

// Copies everything except the name.
void CopyItemAttributeStats(const ITEM_ATTRIBUTE& attribute, ItemDefinition& definition);

// Copies the fields ITEM_ATTRIBUTE (and so item.bmd) does not have: tags,
// wing tier, weapon kind and rule flags.
void CopyFieldsNotInItemAttribute(const ItemDefinition& source, ItemDefinition& target);

// The weapon kind the item's group gives, by group and slot as in OpenMU:
// spears in group 3, shields in group 6, and in group 4 bows on offHand and
// crossbows on mainHand. The arrows and bolts of group 4 are left out as
// one-handed items (OpenMU tells them apart by their height of 1).
WeaponKind GetGroupWeaponKind(const ItemDefinition& definition);

// The weapon kind of an item after the bmd import or the item editor changed it
// (previous: the item before the change, nullptr for a new item). While the
// item has the kind its group gives, or is new, the kind follows its group and
// slot, so an undone edit undoes it too; a different kind set in the item file
// stays.
WeaponKind FollowGroupWeaponKind(const ItemDefinition* previous, const ItemDefinition& updated);

// The attribute's name becomes the neutral (English) name.
ItemDefinition ToItemDefinition(const ITEM_ATTRIBUTE& attribute, int itemType);

void ToItemAttribute(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute);
} // namespace Data::Items
