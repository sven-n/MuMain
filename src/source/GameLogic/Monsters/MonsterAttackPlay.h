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

// The attack animation of the character ended, played to its end or replaced
// by the shock: its attacks may clear the monster skill now (clearSkill
// afterAttack).
void OnAttackEnded(CHARACTER& character);

// True for the animations an attack plays: attack 1 to 5 and appear.
bool IsAttackAnimation(int animation);
} // namespace GameLogic::Monsters::Attacks
