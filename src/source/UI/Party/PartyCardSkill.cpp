#include "UI/Party/PartyCardSkill.h"

#include <algorithm>
#include <array>

namespace UI::Party
{
namespace
{
constexpr std::array CardSupportSkills{
    AT_SKILL_HEALING,         AT_SKILL_HEALING_STR,  AT_SKILL_DEFENSE,          AT_SKILL_DEFENSE_STR,
    AT_SKILL_DEFENSE_MASTERY, AT_SKILL_ATTACK,       AT_SKILL_ATTACK_STR,       AT_SKILL_ATTACK_MASTERY,
    AT_SKILL_TELEPORT_ALLY,   AT_SKILL_SOUL_BARRIER, AT_SKILL_SOUL_BARRIER_STR, AT_SKILL_SOUL_BARRIER_PROFICIENCY,
    AT_SKILL_ALICE_THORNS,    AT_SKILL_RECOVER,
};

bool CastsSupportSkills(CLASS_TYPE baseClass)
{
    return baseClass == CLASS_ELF || baseClass == CLASS_WIZARD || baseClass == CLASS_SUMMONER;
}
} // namespace

bool CastsOnPartyCard(CLASS_TYPE baseClass, ActionSkillType skill)
{
    if (!CastsSupportSkills(baseClass))
        return false;

    return std::ranges::find(CardSupportSkills, skill) != CardSupportSkills.end();
}
} // namespace UI::Party
