#pragma once

#include "Data/GameData/MonsterData/MonsterAttacks.h"

#include <cstdint>

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

// The generic pattern's next animation, for a monster without attacks or
// whose attacks say "generic": attack 1 when CHARACTER::SwordCount % 3 is 0,
// else attack 2; the count goes on by one.
int NextGenericAttack(CHARACTER& character);
// The same on a count of its own.
int NextGenericAttack(std::uint8_t& swordCount);

// The animation a draw picks, with rand() and rand_fps_check() as the old
// code's draws.
int DrawAttack(Data::Monsters::MonsterAttackRandom random);

// The animation `choice` plays, as Play plays it, and the counts it moves:
// the draw's pick, the generic pattern's next one (`swordCount` goes on), the
// rotation's turn (`rotation` goes on, CHARACTER::AttackRotation), else its
// animation. Play and the editor's Play next attack share it.
int ResolveAttackAnimation(const Data::Monsters::MonsterAttacks& attacks,
                           const Data::Monsters::MonsterAttackChoice& choice, std::uint8_t& swordCount, int& rotation);

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
