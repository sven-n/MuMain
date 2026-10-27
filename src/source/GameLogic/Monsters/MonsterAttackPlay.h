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

// The attack that a monster skill packet (0x69, ReceiveMonsterSkill) starts,
// for the life of the scope. Attacks that clear the skill after the attack
// play the skill only for this attack: any other attack of the monster is a
// normal one, also while the skill's attack still plays, and clears the skill
// (P4).
class SkillAttackScope
{
public:
    explicit SkillAttackScope(const CHARACTER& character);
    ~SkillAttackScope();

    SkillAttackScope(const SkillAttackScope&) = delete;
    SkillAttackScope& operator=(const SkillAttackScope&) = delete;

private:
    const CHARACTER* m_previous;
};
} // namespace GameLogic::Monsters::Attacks
