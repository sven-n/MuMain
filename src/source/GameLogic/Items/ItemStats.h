#pragma once

#include "Data/GameData/ItemData/ItemStructs.h"

namespace GameLogic::Items
{
// What an item has of the stats the item stat file (Data/Items/Stats/ItemStats.json)
// covers: the kind's base value with what the item's + level, excellent options and
// ancient set add.
struct ItemStatValues
{
    int damageMin = 0;
    int damageMax = 0;
    // Magic power and defense rate are BYTEs in the item and stop at 255.
    int magicPower = 0;
    int blockRate = 0;
    int defense = 0;
    int magicDefense = 0;
};

struct ItemStatInput
{
    int itemType = 0;
    int level = 0;
    BYTE excellentFlags = 0;
    BYTE ancientDiscriminator = 0;
};

// The base values and the drop level come from `attribute`, the kind's ItemAttribute
// entry, so the item editor's changes count; the stat profile, the fixed excellent
// bonus and the slot come from the item database. A stat the kind does not have stays
// 0. Excellent options count on items that are not wings (a wing's are wing options),
// and ancient items get the excellent part too.
[[nodiscard]] ItemStatValues ComputeItemStats(const ITEM_ATTRIBUTE& attribute, const ItemStatInput& input);
} // namespace GameLogic::Items
