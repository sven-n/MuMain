#pragma once

#include "Render/Models/ZzzBMD.h"

#include <array>

// The one place a character's (or a mount's) bones are built with what the movement features change in how it's
// drawn, so the update's poses (Engine::Object::Pose::Build) and the draws (DrawPose) can't drift apart.
namespace Engine::Characters
{
// Builds `o`'s bones into `bones` at `animationFrame` (BMD::Animation, whose `translate` and `currentAction` these
// are): a character's body a little behind a sudden turn (UpdateSnapTurn), its ponytail trailing a turn
// (UpdateTurnLag) and its fade from the last action (CharacterPoseFade); a mount's head turned where its rider looks
// and rolled when it shakes (MountHeadTurn, MountHeadRoll); then, built in model space, a player's feet on the ground
// under them (PlaceFeet).
void AnimateBones(const OBJECT* o, BMD& model, float (*bones)[3][4], float animationFrame, bool translate,
                  short currentAction = -1);

// What AnimateBones adds to `o`'s pose beyond the animation state Engine::Object::PoseInputs records (the option; a
// character's sudden turn, ponytail, fade and where its feet stand; a mount's head): a pose built with other values is
// out of date.
using MovementPoseKey = std::array<float, 9>;
MovementPoseKey CurrentMovementPoseKey(const OBJECT* o);
} // namespace Engine::Characters
