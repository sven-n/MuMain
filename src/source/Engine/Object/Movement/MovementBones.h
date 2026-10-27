#pragma once

#include "Render/Models/ZzzBMD.h"

// The one place a character's (or a mount's) bones are built with what the movement features change in how it's
// drawn, so the move phase's pre-pass (AnimateCharacterBones) and the draws (Calc_ObjectAnimation, Calc_RenderObject)
// can't drift apart.
namespace Engine::Characters
{
// Builds `o`'s bones into `bones` (BMD::Animation, whose `translate` and `currentAction` these are): a character's
// body a little behind a sudden turn (UpdateSnapTurn), its ponytail trailing a turn (UpdateTurnLag) and its fade
// from the last action (CharacterPoseFade); a mount's head turned where its rider looks and rolled when it shakes
// (MountHeadTurn, MountHeadRoll); then, built in model space, a player's feet on the ground under them (PlaceFeet).
void AnimateBones(OBJECT* o, BMD& model, float (*bones)[3][4], bool translate, short currentAction = -1);
} // namespace Engine::Characters
