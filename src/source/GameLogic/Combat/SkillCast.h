#pragma once

class CHARACTER;
class OBJECT;
struct tagITEM;
typedef tagITEM ITEM;

// Client-side skill casting per class family: picks the action, plays effects,
// and sends the cast request. Driven by the per-class executors and the skill
// dispatcher. Extracted from ZzzInterface.cpp.
namespace GameLogic::Combat
{
    bool SkillWarrior(CHARACTER* c, ITEM* p);
    bool SkillElf(CHARACTER* c, ITEM* p);
    void UseSkillWarrior(CHARACTER* c, OBJECT* o);
    void UseSkillWizard(CHARACTER* c, OBJECT* o);
    void UseSkillElf(CHARACTER* c, OBJECT* o);
    void UseSkillSummon(CHARACTER* pCha, OBJECT* pObj);
    void UseSkillRagefighter(CHARACTER* pCha, OBJECT* pObj);
    void AttackRagefighter(CHARACTER* pCha, int nSkill, float fDistance);
    bool UseSkillRagePosition(CHARACTER* pCha);
}
