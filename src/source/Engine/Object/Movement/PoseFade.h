#pragma once

#include "Render/Models/ZzzBMD.h"

// Pose fades: a player's switch from one action to the next fades over a moment (BMD::Animation blends the pose it
// leaves, PoseBlend), by where it goes.
namespace Engine::Characters
{
// One frame of the fade, before the animation plays (CharacterAnimation, at the speed it plays): a switch between
// standing, walking and running starts one (from walking to running, the new steps start where the old ones were in the
// stride), and a fade under way plays its pose on and wears off.
void UpdatePoseFade(CHARACTER* c, OBJECT* o, float playSpeed);

// The pose a character is fading out of (UpdatePoseFade), or nullptr: `out` is filled in and returned.
const PoseBlend* CharacterPoseFade(const CHARACTER* c, PoseBlend& out);
} // namespace Engine::Characters
