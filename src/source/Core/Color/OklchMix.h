#pragma once

#include "Core/Color/Rgb.h"

namespace Core::Color
{
// `from` moved `share` (0 to 1) of the way to `to` in OKLCH, a color model in which equal steps
// look equally far apart: the lightness and chroma move evenly and the hue the shorter way
// around the color wheel, so green goes to red through yellow and orange without getting
// brighter than both. A gray end takes the hue of the other end.
Rgb MixOklch(Rgb from, Rgb to, float share);
} // namespace Core::Color
