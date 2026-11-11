#include "doctest.h"
#include "GameLogic/Character/CommandEffects.h"

using GameLogic::Character::CommandEffects;
using GameLogic::Character::GetCommandEffects;
using GameLogic::Character::GetDarkLordGuildMembers;

TEST_CASE("the command adds what the server's Dark Lord formulas add to the attack rate and the Dark Raven")
{
    // OpenMU's ClassDarkLord: attack rate command / 10, raven damage command / 8 ~ command / 4, raven attack speed
    // command / 50.
    const CommandEffects effects = GetCommandEffects(250, 1000);
    CHECK(effects.attackRate == 100);
    CHECK(effects.ravenMinimumDamage == 125);
    CHECK(effects.ravenMaximumDamage == 250);
    CHECK(effects.ravenAttackSpeed == 20);
}

TEST_CASE("each part of the command is rounded down")
{
    const CommandEffects effects = GetCommandEffects(19, 49);
    CHECK(effects.attackRate == 4);
    CHECK(effects.ravenMinimumDamage == 6);
    CHECK(effects.ravenMaximumDamage == 12);
    CHECK(effects.ravenAttackSpeed == 0);
    CHECK(effects.guildMembers == 5);
}

TEST_CASE("by the client's rule, a Dark Lord's guild takes level / 10 plus command / 10 members, at most 80")
{
    // The original client's rule, which the guild window shows; OpenMU does not check the guild size.
    CHECK(GetDarkLordGuildMembers(250, 100) == 35);
    CHECK(GetDarkLordGuildMembers(400, 390) == 79);
    CHECK(GetDarkLordGuildMembers(400, 400) == 80);
    CHECK(GetDarkLordGuildMembers(400, 32767) == 80);
}

TEST_CASE("no command adds nothing")
{
    const CommandEffects effects = GetCommandEffects(0, 0);
    CHECK(effects == CommandEffects{});
}
