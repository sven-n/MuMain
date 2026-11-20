#pragma once

// The drawn sudden turn: a sudden turn of the facing (an attack snapping to its target) drawn as a quick eased turn.
namespace Engine::Characters
{
// One frame (each frame, everyone): the facing itself, which the attack and its effects aim by, still turns at once;
// the body is drawn behind it by CharacterMotion::SnapTurnOffset, catching up.
void UpdateSnapTurn(CHARACTER* c);
} // namespace Engine::Characters
