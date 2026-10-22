#include "stdafx.h"

#ifdef _EDITOR

#include "ItemLevelModels.h"

#include "Data/GameData/ItemData/ItemType.h"
#include "Render/Items/ItemDisplay.h"

namespace MuEditor::Items
{
std::vector<LevelModelRun> FindLevelModelRuns(int itemType)
{
    std::vector<LevelModelRun> runs;
    for (int level = 0; level < Data::Items::ItemLevelCount; ++level)
    {
        const std::optional<Render::Items::Display::InventoryModel> drawn =
            Render::Items::Display::FindInventoryModel(itemType, level);
        LevelModelRun run{level, level, std::nullopt, false};
        if (drawn)
        {
            // Also the event models two levels of (14,12) are drawn with.
            run.modelType = Render::Items::Display::GetDrawnModel(drawn->modelType, drawn->drawLevel);
            run.small =
                Render::Items::Display::GetSmallArchangelWeaponScale(drawn->modelType, drawn->drawLevel).has_value();
        }
        if (!runs.empty() && runs.back().modelType == run.modelType && runs.back().small == run.small)
        {
            runs.back().lastLevel = level;
        }
        else
        {
            runs.push_back(run);
        }
    }
    return runs;
}

bool IsDrawnByCodeAtSomeLevel(int itemType)
{
    const int own = Render::Items::Display::GetInventoryModel(itemType);
    for (const LevelModelRun& run : FindLevelModelRuns(itemType))
    {
        if (run.modelType != own)
        {
            return true;
        }
    }
    return false;
}
} // namespace MuEditor::Items

#endif // _EDITOR
