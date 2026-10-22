#include "stdafx.h"
#include "GameLogic/Automation/HelperPickupFilter.h"

#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemCategories.h"

namespace GameLogic::Automation
{
bool ShouldPickUpHelperItem(const MUHelper::ConfigData& config, ITEM* item)
{
    // Selected also takes precedence for older saves containing both mode bits.
    if (!config.bPickSelectItems)
    {
        return config.bPickAllItems;
    }

    if ((config.bPickZen && IsMoneyItem(item)) || (config.bPickJewel && GameLogic::Items::IsJewelItem(item)) ||
        (config.bPickAncient && IsAncientItem(item)) || (config.bPickExcellent && IsExcellentItem(item)))
    {
        return true;
    }

    if (config.bPickExtraItems)
    {
        const std::wstring displayName = GetItemDisplayName(item);
        for (const auto& keyword : config.aExtraItems)
        {
            if (displayName.find(keyword) != std::wstring::npos)
            {
                return true;
            }
        }
    }

    return false;
}
} // namespace GameLogic::Automation
