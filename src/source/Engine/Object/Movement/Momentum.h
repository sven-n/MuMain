#pragma once

// Momentum: a walking character slows down and speeds up instead of flipping its ground velocity at once.
namespace Engine::Characters
{
// A walking character's momentum, one frame: the way it really moves this frame (in `heading`, MoveHeading coming in)
// and the share of its speed it has. Turning back (more than a right angle) it slows to a stop the way it was going
// before it takes the new way; from rest, and after that stop, it speeds up. Gentler turns follow at once.
float StepMomentum(CHARACTER* c, float& heading);
} // namespace Engine::Characters
