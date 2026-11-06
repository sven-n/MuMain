#include "stdafx.h"
#include "GameLogic/Items/ItemOptionSlots.h"

#include "Core/Utilities/Log/MuLogger.h"

#include <bitset>

namespace
{
// The item types that dropped an option, so the log names each type once.
std::bitset<MAX_ITEM> g_typesWithDroppedOptions;

void ReportDroppedOption(const ITEM& item, std::uint16_t option)
{
    if (item.Type < 0 || item.Type >= MAX_ITEM || g_typesWithDroppedOptions.test(item.Type))
    {
        return;
    }
    g_typesWithDroppedOptions.set(item.Type);
    MU_LOG_WARN(mu::log::Get("gameplay"), "Item {} has more than {} options; option {} and those after it are dropped",
                item.Type, MAX_ITEM_SPECIAL, option);
}
} // namespace

namespace GameLogic::Items
{
void AddOptionSlot(ITEM& item, std::uint16_t option, int value)
{
    if (item.SpecialNum >= MAX_ITEM_SPECIAL)
    {
        ReportDroppedOption(item, option);
        return;
    }
    item.Special[item.SpecialNum] = option;
    item.SpecialValue[item.SpecialNum] = static_cast<BYTE>(value);
    ++item.SpecialNum;
}
} // namespace GameLogic::Items
