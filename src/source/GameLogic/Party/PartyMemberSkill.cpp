#include "stdafx.h"

#include "GameLogic/Party/PartyMemberSkill.h"

namespace GameLogic::Party
{
bool IsPartyMemberSkill(CLASS_TYPE heroBaseClass, ActionSkillType skill)
{
    if (heroBaseClass != CLASS_ELF && heroBaseClass != CLASS_WIZARD && heroBaseClass != CLASS_SUMMONER)
    {
        return false;
    }

    switch (skill)
    {
    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
    case AT_SKILL_TELEPORT_ALLY:
    case AT_SKILL_SOUL_BARRIER:
    case AT_SKILL_SOUL_BARRIER_STR:
    case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
    case AT_SKILL_ALICE_THORNS:
    case AT_SKILL_RECOVER:
        return true;
    default:
        return false;
    }
}
} // namespace GameLogic::Party
