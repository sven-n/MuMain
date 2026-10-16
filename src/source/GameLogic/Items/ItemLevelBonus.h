#pragma once

namespace GameLogic::Items
{
// The first step above +9 of the level bonus; the third tier wings' defense starts at 5.
inline constexpr int FirstLevelBonusStepAboveNine = 4;
inline constexpr int ThirdTierWingFirstLevelBonusStepAboveNine = 5;

// What the + level of an item adds to its damage, magic power (before it is halved),
// defense rate, magic defense and, except for shields and wings, defense: 3 per level up
// to +9, then steps of 4, 5, 6, 7, 8 and 9 at +10 to +15, each on top of the ones below
// (27, 31, 36, 42, 49, 57, 66 at +9 to +15), as in the Season 6 client and OpenMU's
// level bonus tables.
[[nodiscard]] int GetItemLevelBonus(int level);

// The steps above +9 alone (4, 9, 15, 22, 30, 39 at +10 to +15, or 5, 11, 18, 26, 35, 45
// from a first step of 5), for defense, whose slope up to +9 differs for wings (2, 3 or 4
// per level).
[[nodiscard]] int GetItemLevelBonusAboveNine(int level, int firstStep = FirstLevelBonusStepAboveNine);
} // namespace GameLogic::Items
