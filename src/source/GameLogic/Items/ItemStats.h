#pragma once

#include "Data/GameData/ItemData/ItemOptionDatabase.h"
#include "Data/GameData/ItemData/ItemStructs.h"

#include <span>

namespace Data::Items
{
struct ItemDefinition;
struct ItemStatProfile;
} // namespace Data::Items

namespace GameLogic::Items
{
// What an item has of the stats and requirements the item stat file
// (Data/Items/Stats/ItemStats.json) covers: the kind's base value with what the item's
// + level, excellent options, ancient set and option level add. Each stops at 65535, what
// the item's fields hold.
struct ItemStatValues
{
    int damageMin = 0;
    int damageMax = 0;
    int magicPower = 0;
    // Defense rate is a BYTE in the item and stops at 255.
    int blockRate = 0;
    int defense = 0;
    int magicDefense = 0;

    int requireLevel = 0;
    int requireStrength = 0;
    int requireDexterity = 0;
    int requireEnergy = 0;
    int requireVitality = 0;
    int requireLeadership = 0;
};

struct ItemStatInput
{
    int itemType = 0;
    // The + level, a byte in the item data.
    int level = 0;
    // The excellent byte. With the item's option links it says whether the item has active excellent options
    // (GameLogic::Items::HasExcellentOptions): the excellent stat bonus and the excellent level requirement. The drop
    // level's excellent bonus reads the byte alone, which wings with wing options and the Horn of Fenrir have too.
    BYTE excellentFlags = 0;
    BYTE ancientDiscriminator = 0;
    // The level the additional option counts at (GameLogic::Items::GetAdditionalOptionLevel): the strength
    // requirement grows by strengthPerOptionLevel per level.
    BYTE optionLevel = 0;
};

// The base values and the drop level come from `attribute`, the kind's ItemAttribute
// entry, so the item editor's changes count; the stat profile, the fixed excellent
// bonus, the requirements by level, excellentRaisesLevel and the slot come from the
// item database. A stat or requirement with a base value of 0 stays 0, except a requirement
// given by level and the strength the additional option's counted level adds. For the stats,
// active excellent options count (the excellent byte with the item's option links), and
// ancient items get the excellent part too. For the requirements the excellent byte gives the excellent drop level
// bonus (before the ancient one), and active excellent options raise the level requirement.
[[nodiscard]] ItemStatValues ComputeItemStats(const ITEM_ATTRIBUTE& attribute, const ItemStatInput& input);

// What ComputeItemStats takes from the item database, given instead: for an item as the
// item editor has it before it is stored. `definition` may be null (no requirements by
// level, excellent options raise the level requirement, no fixed excellent bonus);
// `optionLinks` are the item's resolved option links, which say whether its excellent byte
// turns an excellent option on (an item without links is never excellent).
struct ItemStatSources
{
    const ITEM_ATTRIBUTE& attribute;
    const Data::Items::ItemDefinition* definition;
    const Data::Items::ItemStatProfile& profile;
    std::span<const Data::Items::ItemOptionIndex> optionLinks;
};

// ComputeItemStats with the item's data from `sources`; the item stat file's drop level
// bonuses still come from the item database. `input.itemType` is not read.
[[nodiscard]] ItemStatValues ComputeItemStats(const ItemStatSources& sources, const ItemStatInput& input);
} // namespace GameLogic::Items
