#pragma once

class CHARACTER;
class OBJECT;

// The attack animations of monsters from their entries (monsters design,
// D48): SetPlayerAttack's handlers call it where they choose an animation.
namespace GameLogic::Monsters::Attacks
{
// Whether the monster of the character has `attacks` in its entry.
bool HasAttacks(const CHARACTER& character);

// Plays the animation the monster's `attacks` choose for its monster skill and
// player skill, and clears its monster skill as they say. False, with nothing
// done, when its entry has no `attacks`.
bool Play(CHARACTER& character, OBJECT& object);
} // namespace GameLogic::Monsters::Attacks
