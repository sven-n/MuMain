#include "GameLogic/Items/ItemLevelBonus.h"

#include <algorithm>

namespace
{
constexpr int LastLevelWithLinearBonus = 9;
constexpr int LinearBonusPerLevel = 3;
// Item levels stop at +15 (a 4-bit field in the Season 6 client, MaximumItemLevel in OpenMU);
// higher values keep the bonus of +15.
constexpr int MaxLevelsAboveNine = 6;
} // namespace

int GameLogic::Items::GetItemLevelBonus(int level)
{
    return LinearBonusPerLevel * std::clamp(level, 0, LastLevelWithLinearBonus) + GetItemLevelBonusAboveNine(level);
}

int GameLogic::Items::GetItemLevelBonusAboveNine(int level, int firstStep)
{
    // The steps are firstStep, firstStep + 1, ... for the levels above +9: their sum.
    const int levelsAboveNine = std::clamp(level - LastLevelWithLinearBonus, 0, MaxLevelsAboveNine);
    return levelsAboveNine * (2 * firstStep + levelsAboveNine - 1) / 2;
}
