#include "doctest.h"
#include "GameLogic/Character/CommandEffects.h"

using GameLogic::Character::CommandEffects;
using GameLogic::Character::GetCommandEffects;
using GameLogic::Character::GetDarkLordGuildMembers;

TEST_CASE("the command adds what the server's Dark Lord formulas add")
{
    // OpenMU's ClassDarkLord: attack rate command / 10, raven damage command / 8 ~ command / 4, raven attack speed
    // command / 50, guild size level / 10 + command / 10.
    const CommandEffects effects = GetCommandEffects(250, 1000);
    CHECK(effects.attackRate == 100);
    CHECK(effects.ravenMinimumDamage == 125);
    CHECK(effects.ravenMaximumDamage == 250);
    CHECK(effects.ravenAttackSpeed == 20);
    CHECK(effects.guildMembers == 80);
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

TEST_CASE("a Dark Lord's guild takes at most 80 members")
{
    CHECK(GetDarkLordGuildMembers(400, 390) == 79);
    CHECK(GetDarkLordGuildMembers(400, 400) == 80);
    CHECK(GetDarkLordGuildMembers(400, 32767) == 80);
}

TEST_CASE("no command adds nothing")
{
    const CommandEffects effects = GetCommandEffects(0, 0);
    CHECK(effects == CommandEffects{});
}
