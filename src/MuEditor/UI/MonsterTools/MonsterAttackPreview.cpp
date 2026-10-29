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
    int animation = choice.animation;
    if (choice.random)
    {
        animation = Attacks::DrawAttack(*choice.random);
    }
    else if (choice.generic)
    {
        animation = Attacks::NextGenericAttack(counts.swordCount);
    }
    if (choice.advancesRotation)
    {
        counts.rotation = (counts.rotation + 1) % static_cast<int>(attacks.otherwise.size());
    }
    ++counts.swordCount;
    return animation;
}
} // namespace MuEditor::Monsters::AttackPreview

#endif // _EDITOR
