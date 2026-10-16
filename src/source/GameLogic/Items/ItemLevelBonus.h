#pragma once

namespace GameLogic::Items
{
// The level bonus grows by this much per level up to +9; second tier wings and the Capes of
// Lord and Fighter grow their defense by 2, third tier wings by 4.
inline constexpr int LevelBonusPerLevelUpToNine = 3;
// The first step above +9; the third tier wings' defense starts at 5.
inline constexpr int FirstLevelBonusStepAboveNine = 4;
inline constexpr int ThirdTierWingFirstLevelBonusStepAboveNine = 5;

// What the + level of an item adds to its damage, magic power (before it is halved),
// defense rate, magic defense and defense: perLevelUpToNine per level up to +9, then steps
// of firstStepAboveNine, one more, and so on at +10 to +15, each on top of the ones below.
// With the defaults that is 27 at +9, then 31, 36, 42, 49, 57 and 66 at +10 to +15, as in
// the Season 6 client and OpenMU's level bonus tables.
[[nodiscard]] int GetItemLevelBonus(int level, int perLevelUpToNine = LevelBonusPerLevelUpToNine,
                                    int firstStepAboveNine = FirstLevelBonusStepAboveNine);
} // namespace GameLogic::Items
