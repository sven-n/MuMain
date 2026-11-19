#pragma once

// Momentum: a walking character setting off from rest speeds up over a moment instead of moving at full speed at once.
// Turns keep the speed it has, a reversal too, so a change of direction is as quick as without it.
namespace Engine::Characters
{
// One frame of a walking character's momentum: the share of its speed it moves at this frame (0 to 1), eased. From
// rest (it didn't walk in the last MomentumRestFrames rendered frames) it speeds up over MomentumStartSeconds
// (MovementTuning.h); then it stays at full speed. Also kept in CharacterMotion::MomentumSpeed, for the steps to play
// at.
float StepMomentum(CHARACTER* c);
} // namespace Engine::Characters
