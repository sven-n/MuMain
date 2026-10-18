#include "GameLogic/Items/ItemLevelBonus.h"

#include <algorithm>

namespace
{
constexpr int LastLevelWithLinearBonus = 9;
// Item levels stop at +15 (a 4-bit field in the Season 6 client, MaximumItemLevel in OpenMU);
// higher values keep the bonus of +15.
constexpr int MaxLevelsAboveNine = 6;
} // namespace

int GameLogic::Items::GetItemLevelBonus(int level, int perLevelUpToNine, int firstStepAboveNine)
{
    int bonus = perLevelUpToNine * std::clamp(level, 0, LastLevelWithLinearBonus);

    // Each level above +9 adds a step one larger than the step of the level before.
    const int levelsAboveNine = std::clamp(level - LastLevelWithLinearBonus, 0, MaxLevelsAboveNine);
    for (int step = 0; step < levelsAboveNine; ++step)
    {
        bonus += firstStepAboveNine + step;
    }
    return bonus;
}
