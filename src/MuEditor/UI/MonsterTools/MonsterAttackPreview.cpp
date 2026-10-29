#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterAttackPreview.h"

#include "GameLogic/Monsters/MonsterAttackPlay.h"

namespace MuEditor::Monsters::AttackPreview
{
namespace
{
// No entry has this number: the skill numbers are 0 and above.
constexpr int NoSkill = -1;
} // namespace

int NextOtherwise(const Data::Monsters::MonsterAttacks& attacks, Counts& counts)
{
    namespace Attacks = GameLogic::Monsters::Attacks;
    const Data::Monsters::MonsterAttackChoice choice =
        Data::Monsters::ChooseMonsterAttack(attacks, NoSkill, NoSkill, counts.rotation);
    const int animation = Attacks::ResolveAttackAnimation(attacks, choice, counts.swordCount, counts.rotation);
    // SetPlayerAttack counts every attack.
    ++counts.swordCount;
    return animation;
}
} // namespace MuEditor::Monsters::AttackPreview

#endif // _EDITOR
