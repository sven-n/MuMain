#include "TradeChangeWarning.h"

#include "Core/Platform/WinCompat.h" // the Win32 scalar types _enum.h and _define.h use
#include "Core/Globals/_enum.h"

namespace UI::Inventory
{
bool IsTrackedForChangeWarning(int itemType, int level, int excellentFlags, bool compiledGem)
{
    return (itemType >= ITEM_HELPER && itemType <= ITEM_DARK_HORSE_ITEM)
           || itemType == ITEM_JEWEL_OF_BLESS || itemType == ITEM_JEWEL_OF_SOUL || itemType == ITEM_JEWEL_OF_LIFE
           || itemType == ITEM_JEWEL_OF_GUARDIAN
           || compiledGem
           || (itemType >= ITEM_WING && itemType <= ITEM_WINGS_OF_DARKNESS)
           || itemType == ITEM_CAPE_OF_LORD
           || itemType == ITEM_HORN_OF_FENRIR // a lower durability is a real downgrade, as for the Uniria and Dinorant
           || (itemType >= ITEM_WING_OF_STORM && itemType <= ITEM_WING_OF_DIMENSION)
           || itemType == ITEM_JEWEL_OF_CHAOS
           || (itemType >= ITEM_CAPE_OF_FIGHTER && itemType <= ITEM_CAPE_OF_OVERRULE)
           || (level > 4 && itemType < ITEM_WING)
           || excellentFlags > 0;
}
} // namespace UI::Inventory
