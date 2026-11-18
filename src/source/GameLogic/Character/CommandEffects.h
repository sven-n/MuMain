#pragma once

#include "Data/GameData/ItemData/ItemPetStats.h"

// What the Dark Lord's command adds: the attack rate by OpenMU's ClassDarkLord formula; the Dark Raven's and Dark
// Horse's damage and attack speed by their pet stats in the item data ("petStats": the raven's follow OpenMU's
// ClassDarkLord, the horse's the original client, since OpenMU's command reaches the horse through Earthshake's
// damage); and the guild size by the original client's rule (GameLogic/Social/GuildSize.h; OpenMU does not check
// it). Each part is what the stat is with the command less what it is without it, as the client counts it.
namespace GameLogic::Character
{
// Command per point of the PvM attack rate.
inline constexpr int CommandPerAttackRate = 10;

// A pet as the command hint counts it: its item's pet stats, and its level and its owner's strength (the command is
// the hint's own).
struct DarkLordPet
{
    Data::Items::ItemPetStats stats;
    int petLevel = 0;
    int strength = 0;
};

struct DarkLordPets
{
    DarkLordPet raven;
    DarkLordPet horse;
};

struct CommandEffects
{
    int attackRate = 0;
    Data::Items::PetStatValues raven;
    Data::Items::PetStatValues horse;
    // The guild members the command adds to what the Dark Lord's level gives, by the client's rule.
    int guildMembers = 0;

    bool operator==(const CommandEffects&) const = default;
};

int GetCommandAttackRate(int command);
// What the command adds to each of the pet's stats: the stat with the command less the stat without it.
Data::Items::PetStatValues GetCommandPetStats(const DarkLordPet& pet, int command);
// The guild members the command adds at this level: less near the cap of 80, none at it.
int GetCommandGuildMembers(int level, int command);

CommandEffects GetCommandEffects(int level, int command, const DarkLordPets& pets);
} // namespace GameLogic::Character
