#pragma once

class CHARACTER;
class OBJECT;

// The attack animations of monsters from their entries (monsters design,
// D48): SetPlayerAttack's handlers call it where they choose an animation.
namespace GameLogic::Monsters::Attacks
{
// Plays the animation the monster's `attacks` choose for its monster skill and
// player skill and their sound, and clears its monster skill as they say.
// False, with nothing done, when its entry has no `attacks`.
bool Play(CHARACTER& character, OBJECT& object);

// The attack animation of the character ended: its attacks may clear the
// monster skill now (clearSkill afterAttack).
void OnAttackEnded(CHARACTER& character);
} // namespace GameLogic::Monsters::Attacks
