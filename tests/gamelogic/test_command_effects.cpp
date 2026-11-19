#include "doctest.h"
#include "GameLogic/Character/CommandEffects.h"
#include "GameLogic/Social/GuildSize.h"

#include <cstdint>
#include <limits>

using Data::Items::ItemPetStats;
using Data::Items::PetStatValues;
using GameLogic::Character::CommandEffects;
using GameLogic::Character::DarkLordPet;
using GameLogic::Character::DarkLordPets;
using GameLogic::Character::GetCommandEffects;
using GameLogic::Character::GetCommandGuildMembers;
using GameLogic::Character::GetCommandPetStats;
using GameLogic::Social::GetMaxGuildMembers;

namespace
{
// Example Dark Raven stats using OpenMU's ClassDarkLord formulas.
ItemPetStats RavenStats()
{
    ItemPetStats stats;
    stats.minimumDamage = {180, 15, 1, 0, 8, 0};
    stats.maximumDamage = {200, 15, 1, 0, 4, 0};
    stats.attackSpeed = {20, 4, 5, 0, 50, 0};
    return stats;
}

// Example Dark Horse stats using the original client's formulas.
ItemPetStats HorseStats()
{
    ItemPetStats stats;
    stats.minimumDamage = {0, 5, 1, 10, 10, 0};
    stats.maximumDamage = {0, 0, 1, 0, 0, 150};
    stats.attackSpeed = {20, 4, 5, 0, 50, 0};
    return stats;
}

DarkLordPets Pets(int horseLevel = 0, int strength = 0)
{
    return {DarkLordPet{RavenStats(), 0, strength}, DarkLordPet{HorseStats(), horseLevel, strength}};
}
} // namespace

TEST_CASE("the command adds to the attack rate and to what the pets' stats count of it")
{
    const CommandEffects effects = GetCommandEffects(250, 1000, Pets());
    CHECK(effects.attackRate == 100);
    CHECK(effects.raven == PetStatValues{125, 250, 20});
    CHECK(effects.horse == PetStatValues{100, 150, 20});
}

TEST_CASE("each part of the command is rounded down")
{
    const CommandEffects effects = GetCommandEffects(19, 49, Pets());
    CHECK(effects.attackRate == 4);
    CHECK(effects.raven == PetStatValues{6, 12, 0});
    CHECK(effects.horse == PetStatValues{4, 6, 0});
    CHECK(effects.guildMembers == 4);
}

TEST_CASE("the command adds to the Dark Horse's maximum what the whole minimum's share gains")
{
    // The maximum is 150 % of the whole minimum: strength 100 and horse level 1 give 15 (maximum 22), 310 command makes
    // it 46 (maximum 69), so the command adds 31 to the minimum and 47, not 46, to the maximum.
    CHECK(GetCommandPetStats(DarkLordPet{HorseStats(), 1, 100}, 310) == PetStatValues{31, 47, 6});
}

TEST_CASE("by the client's rule, a guild master's guild takes level / 10 members, a Dark Lord's plus command / 10, "
          "at most 80")
{
    // The original client's rule, which the guild window shows; OpenMU does not check the guild size.
    CHECK(GetMaxGuildMembers(CLASS_KNIGHT, 250, 1000) == 25);
    CHECK(GetMaxGuildMembers(CLASS_WIZARD, 1000, 0) == 100);
    CHECK(GetMaxGuildMembers(CLASS_DARK_LORD, 250, 100) == 35);
    CHECK(GetMaxGuildMembers(CLASS_DARK_LORD, 400, 390) == 79);
    CHECK(GetMaxGuildMembers(CLASS_DARK_LORD, 400, 400) == 80);
    CHECK(GetMaxGuildMembers(CLASS_DARK_LORD, 400, 32767) == 80);
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
    CHECK(GetCommandEffects(0, 0, Pets(5, 300)) == CommandEffects{});
}

TEST_CASE("a pet without pet stats gets nothing from the command")
{
    CHECK(GetCommandPetStats(DarkLordPet{}, 1000) == PetStatValues{});
}

TEST_CASE("Pet stats use wide arithmetic before limiting the result to the pet window's range")
{
    ItemPetStats stats;
    constexpr int largeCoefficient = 100'000'000;
    stats.minimumDamage.perPetLevel = largeCoefficient;
    stats.minimumDamage.petLevelDivisor = largeCoefficient;
    stats.maximumDamage.minimumPercent = largeCoefficient;
    stats.attackSpeed.add = -1;

    CHECK(Data::Items::EvaluatePetStats(stats, {50, 0, 0}) ==
          PetStatValues{50, std::numeric_limits<std::uint16_t>::max(), 0});
}
