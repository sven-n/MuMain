#pragma once

namespace Render::PostProcessing
{

// Gives the renderer the colour grading table of map `mapNumber`, or none when
// the map has no usable table; a missing or invalid table is logged. Only a
// change of map loads anything, so this may be called every frame.
void SelectMapColorGrading(int mapNumber);

} // namespace Render::PostProcessing
