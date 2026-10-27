#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStructs.h"

#include <cstdint>
#include <string>

// Converts between the legacy ITEM_ATTRIBUTE record (item.bmd layout, and the
// ItemAttribute[] table the game reads) and ItemDefinition. The stats convert
// losslessly in both directions. ITEM_ATTRIBUTE holds one name only: the
// shown name (ItemDefinition::name) goes into it, and names longer than
// MAX_ITEM_NAME - 1 characters are cut.
namespace Data::Items
{
class ItemStatProfiles;

std::wstring ReadItemAttributeName(const ITEM_ATTRIBUTE& attribute);

// The name as ITEM_ATTRIBUTE holds it: cut to MAX_ITEM_NAME - 1 characters.
std::wstring CutToItemAttributeName(const std::wstring& name);

// Copies everything except the name.
void CopyItemAttributeStats(const ITEM_ATTRIBUTE& attribute, ItemDefinition& definition);

// Copies the fields ITEM_ATTRIBUTE (and so item.bmd) does not have: tags,
// wing tier, weapon kind, stat profile, potion priority, excellent bonus,
// excellentRaisesLevel, the requirements by level and rule flags.
void CopyFieldsNotInItemAttribute(const ItemDefinition& source, ItemDefinition& target);

// The weapon kind the item's group gives, by group and slot as in OpenMU:
// swords in group 0 (gloves when only the Rage Fighter may use the item),
// axes in group 1, maces in group 2 and spears in group 3, all on mainHand; in
// group 4 bows on offHand and crossbows on mainHand, in group 5 staffs on
// mainHand and books on offHand, and shields in group 6 on offHand. An item on
// another slot gets none. The arrows and bolts of group 4 are left out as
// one-handed items (OpenMU tells them apart by their height of 1).
WeaponKind GetGroupWeaponKind(const ItemDefinition& definition);

// The weapon kind of an item after the bmd import or the item editor changed it
// (previous: the item before the change, nullptr for a new item). While the
// item has the kind its group gives, or is new, the kind follows its group and
// slot (in group 0 also the class requirements), so an undone edit undoes it
// too; a kind set in the item file that differs from the group's stays as long
// as it differs.
WeaponKind FollowGroupWeaponKind(const ItemDefinition* previous, const ItemDefinition& updated);

// The stat profile the item's own data gives: the one the first profile rule of
// the item stat file it matches names, "" for the default (also when no rule
// matches). See docs/item-data.md, "Profile rules".
std::string DeriveStatProfile(const ItemStatProfiles& profiles, const ItemDefinition& definition);

// The stat profile of an item after the bmd import or the item editor changed
// it, like FollowGroupWeaponKind: while the item has the profile its data gives,
// has none (as in item files saved before the field) or is new, the profile
// follows its data; a profile set in the item file that differs from it, also
// "default", stays as long as it differs.
std::string FollowStatProfile(const ItemStatProfiles& profiles, const ItemDefinition* previous,
                              const ItemDefinition& updated);

// How storing an item treats the stat profile it names (not "", which follows the data): Follow
// keeps it only while it differs from the one the item had, as a pick of the item tools; AsGiven
// keeps every one as given, for an import that decided each item's profile itself.
enum class StatProfileStore : std::uint8_t
{
    Follow,
    AsGiven,
};

// The attribute's name becomes the neutral (English) name.
ItemDefinition ToItemDefinition(const ITEM_ATTRIBUTE& attribute, int itemType);

void ToItemAttribute(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute);
} // namespace Data::Items
