#pragma once

#include "Core/Globals/_enum.h" // ActionSkillType

class CHARACTER;

// Skill execution dispatcher: validates a skill cast (range, mana, conditions)
// and routes it to the per-class executor. Extracted from ZzzInterface.cpp.
namespace GameLogic::Combat
{
    int  ExecuteSkill(CHARACTER* c, ActionSkillType Skill, float Distance);
    bool ExecuteSkillComplete(CHARACTER* c);
    bool CanExecuteSkill(CHARACTER* c, ActionSkillType Skill, float Distance);
    bool CheckMana(CHARACTER* c, int Skill);
}
