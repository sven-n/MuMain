#include <doctest.h>
#include "UI/Party/PartyCardSkill.h"

using UI::Party::CastsOnPartyCard;

TEST_CASE("a right-click on a party card casts the support skills of the wizard, the elf and the summoner")
{
    CHECK(CastsOnPartyCard(CLASS_ELF, AT_SKILL_HEALING));
    CHECK(CastsOnPartyCard(CLASS_ELF, AT_SKILL_DEFENSE_MASTERY));
    CHECK(CastsOnPartyCard(CLASS_ELF, AT_SKILL_ATTACK_STR));
    CHECK(CastsOnPartyCard(CLASS_WIZARD, AT_SKILL_TELEPORT_ALLY));
    CHECK(CastsOnPartyCard(CLASS_WIZARD, AT_SKILL_SOUL_BARRIER_PROFICIENCY));
    CHECK(CastsOnPartyCard(CLASS_SUMMONER, AT_SKILL_ALICE_THORNS));
    CHECK(CastsOnPartyCard(CLASS_ELF, AT_SKILL_RECOVER));
}

TEST_CASE("a right-click on a party card does not cast an attack skill")
{
    CHECK_FALSE(CastsOnPartyCard(CLASS_ELF, AT_SKILL_TRIPLE_SHOT));
    CHECK_FALSE(CastsOnPartyCard(CLASS_WIZARD, AT_SKILL_EVIL_SPIRIT));
    CHECK_FALSE(CastsOnPartyCard(CLASS_ELF, AT_SKILL_UNDEFINED));
}

TEST_CASE("a right-click on a party card casts nothing for the other classes")
{
    CHECK_FALSE(CastsOnPartyCard(CLASS_KNIGHT, AT_SKILL_HEALING));
    CHECK_FALSE(CastsOnPartyCard(CLASS_DARK_LORD, AT_SKILL_DEFENSE));
    CHECK_FALSE(CastsOnPartyCard(CLASS_DARK, AT_SKILL_SOUL_BARRIER));
    CHECK_FALSE(CastsOnPartyCard(CLASS_RAGEFIGHTER, AT_SKILL_ATTACK));
}
