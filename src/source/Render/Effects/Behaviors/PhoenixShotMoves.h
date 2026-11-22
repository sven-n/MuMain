#pragma once

#include "Render/Effects/EffectDef.h"

// Phoenix Shot (AT_SKILL_PHOENIX_SHOT), as the Season 6 client draws it: the rig
// (MODEL_PHOENIX_SHOT) lays its arms' trails and lets go three blades (MODEL_WINDSPIN03)
// that fly at the target and strike it. The 5.2 client loaded both and drew nothing.
namespace Render::Effects::Behaviors
{
bool Move_MODEL_PHOENIX_SHOT(OBJECT* o, int index, float Luminosity);
bool Move_MODEL_WINDSPIN03(OBJECT* o, int index, float Luminosity);
// One of the spirals a blade lays on the ground where it strikes (BITMAP_TWLIGHT subtype 4).
void MoveStrikeSpiral(OBJECT* o);
} // namespace Render::Effects::Behaviors
