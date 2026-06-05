#pragma once

#include "Core/Globals/_enum.h" // ActionSkillType

class CHARACTER;

// Per-class skill dispatch: ExecuteSkill routes to one of these by character
// class, and each runs the chosen skill's client-side cast (action, effects,
// network request). Extracted from ZzzInterface.cpp.
namespace GameLogic::Combat
{
    void AttackElf(CHARACTER* c, int Skill, float Distance);
    void AttackKnight(CHARACTER* c, ActionSkillType Skill, float Distance);
    void AttackWizard(CHARACTER* c, int Skill, float Distance);
    void AttackCommon(CHARACTER* c, int Skill, float Distance);
}
