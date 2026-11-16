#include "doctest.h"
#include "GameLogic/Character/CommandEffects.h"
#include "GameLogic/Social/GuildSize.h"

using GameLogic::Character::CommandEffects;
using GameLogic::Character::GetCommandEffects;
using GameLogic::Character::GetCommandGuildMembers;
using GameLogic::Social::GetMaxGuildMembers;

TEST_CASE("the command adds what the server's Dark Lord formulas add to the attack rate and the pets")
{
    // OpenMU's ClassDarkLord: attack rate command / 10, raven damage command / 8 ~ command / 4, raven attack speed
    // command / 50, horse damage command / 10 and attack speed command / 50.
    const CommandEffects effects = GetCommandEffects(250, 1000);
    CHECK(effects.attackRate == 100);
    CHECK(effects.ravenMinimumDamage == 125);
    CHECK(effects.ravenMaximumDamage == 250);
    CHECK(effects.ravenAttackSpeed == 20);
    CHECK(effects.horseDamage == 100);
    CHECK(effects.horseAttackSpeed == 20);
}

TEST_CASE("each part of the command is rounded down")
{
    const CommandEffects effects = GetCommandEffects(19, 49);
    CHECK(effects.attackRate == 4);
    CHECK(effects.ravenMinimumDamage == 6);
    CHECK(effects.ravenMaximumDamage == 12);
    CHECK(effects.ravenAttackSpeed == 0);
    CHECK(effects.horseDamage == 4);
    CHECK(effects.horseAttackSpeed == 0);
    CHECK(effects.guildMembers == 4);
}

TEST_CASE("by the client's rule, a guild master's guild takes level / 10 members, a Dark Lord's plus command / 10, "
          "at most 80")
{
    // The original client's rule, which the guild window shows; OpenMU does not check the guild size.
    CHECK(GetMaxGuildMembers(false, 250, 1000) == 25);
    CHECK(GetMaxGuildMembers(false, 1000, 0) == 100);
    CHECK(GetMaxGuildMembers(true, 250, 100) == 35);
    CHECK(GetMaxGuildMembers(true, 400, 390) == 79);
    CHECK(GetMaxGuildMembers(true, 400, 400) == 80);
    CHECK(GetMaxGuildMembers(true, 400, 32767) == 80);
}

TEST_CASE("the command adds the guild members above the level's, up to the cap")
{
    CHECK(GetCommandGuildMembers(250, 100) == 10);
    CHECK(GetCommandGuildMembers(400, 390) == 39);
    CHECK(GetCommandGuildMembers(400, 1000) == 40);
    CHECK(GetCommandGuildMembers(800, 1000) == 0);
}

TEST_CASE("no command adds nothing")
{
    const CommandEffects effects = GetCommandEffects(0, 0);
    CHECK(effects == CommandEffects{});
}
