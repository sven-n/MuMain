#pragma once

// The tunables of how a walking character sets off and turns (Enhanced Movement), together so they can be retuned in
// one place. The tests derive from them.
namespace Engine::Characters
{
// How long a walker setting off from rest takes to reach its full speed (seconds; StepMomentum, eased). Raising it
// makes the start heavier and loses more travel at every start (about half of it at full speed: 0.03 s here);
// lowering it towards 0 sets off at full speed at once, as without Enhanced Movement.
constexpr float MomentumStartSeconds = 0.06f;

// How many rendered frames without a step count as standing still, so the next step sets off from rest (frames, not
// time, so a slow frame rate doesn't make every step a start). Raising it lets a longer pause keep the speed;
// lowering it to 1 restarts after any frame the walker didn't step in.
constexpr unsigned int MomentumRestFrames = 2;

// A walking turn this sharp or sharper (a reversal) faces and travels the new way at once, as without Enhanced
// Movement, so a quick change of direction stays as quick (degrees; MovePath, UpdateSnapTurn draws the body catching
// up). Raising it eases more turns (smoother, slower to face the new way); lowering it snaps more of them.
constexpr float SharpTurnDegrees = 45.f;
} // namespace Engine::Characters
