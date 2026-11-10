// Tests of which skills a right click on a party card casts on that member.

#include "stdafx.h"

#include "doctest.h"

#include "GameLogic/Party/PartyMemberSkill.h"

using GameLogic::Party::IsPartyMemberSkill;

TEST_CASE("support classes cast their party skills through a party card [gamelogic][party]")
{
    CHECK(IsPartyMemberSkill(CLASS_ELF, AT_SKILL_HEALING));
    CHECK(IsPartyMemberSkill(CLASS_ELF, AT_SKILL_DEFENSE_MASTERY));
    CHECK(IsPartyMemberSkill(CLASS_WIZARD, AT_SKILL_SOUL_BARRIER));
    CHECK(IsPartyMemberSkill(CLASS_WIZARD, AT_SKILL_TELEPORT_ALLY));
    CHECK(IsPartyMemberSkill(CLASS_SUMMONER, AT_SKILL_ALICE_THORNS));
    CHECK(IsPartyMemberSkill(CLASS_SUMMONER, AT_SKILL_RECOVER));
}

TEST_CASE("attack skills never target a party card [gamelogic][party]")
{
    CHECK_FALSE(IsPartyMemberSkill(CLASS_ELF, AT_SKILL_TRIPLE_SHOT));
    CHECK_FALSE(IsPartyMemberSkill(CLASS_WIZARD, AT_SKILL_EVIL_SPIRIT));
}

TEST_CASE("other classes never target a party card [gamelogic][party]")
{
    CHECK_FALSE(IsPartyMemberSkill(CLASS_KNIGHT, AT_SKILL_HEALING));
    CHECK_FALSE(IsPartyMemberSkill(CLASS_DARK_LORD, AT_SKILL_DEFENSE));
    CHECK_FALSE(IsPartyMemberSkill(CLASS_RAGEFIGHTER, AT_SKILL_ATTACK));
}
