#pragma once

// Turn lag: the wings and the ponytail trail a little behind the body's turn and settle after it, on a spring.
namespace Engine::Characters
{
// One frame (each frame, everyone): CharacterMotion::TurnLag, which the bone builds and the wings use.
void UpdateTurnLag(CHARACTER* c);
} // namespace Engine::Characters
