#pragma once

#include "Data/GameData/ModelData/ModelFiles.h"

#include <span>

// Which LOD of a model file is drawn: chosen by the projected screen height of the model's bounds as
// a fraction of the window's height, with hysteresis so that a model standing
// near a threshold does not flicker between two levels.
namespace Render::Models
{
// The LOD for `screenHeight` given the LOD drawn before (`previousLod`), for a
// model with `lodCount` levels (1 to 3):
//  - coarser: LOD i + 1 as soon as the height is below screenHeights[i];
//  - finer: back to LOD i only once the height is above
//    screenHeights[i] x (1 + hysteresis).
// Between the two the previous LOD stays. Thresholds beyond the model's
// levels are ignored; a previous LOD outside [0, lodCount) counts as LOD 0.
[[nodiscard]] int SelectLod(float screenHeight, int previousLod, const Data::Models::LodSettings& settings,
                            int lodCount);

// The projected height of a box as a fraction of the window's height, from
// the window rows of its corners. A box with a corner behind the camera
// (`allInFront` false) counts as filling the window, so it gets LOD 0.
[[nodiscard]] float ScreenHeightFraction(std::span<const float> rows, bool allInFront, float windowHeight);
} // namespace Render::Models
