#include "GameLogic/Character/CommandEffects.h"

#include "GameLogic/Social/GuildSize.h"

namespace GameLogic::Character
{
int GetCommandAttackRate(int command)
{
    return command / CommandPerAttackRate;
}

Data::Items::PetStatValues GetCommandPetStats(const DarkLordPet& pet, int command)
{
    const Data::Items::PetStatInputs withCommand{pet.petLevel, pet.strength, command};
    const Data::Items::PetStatInputs withoutCommand{pet.petLevel, pet.strength, 0};
    const Data::Items::PetStatValues with = Data::Items::EvaluatePetStats(pet.stats, withCommand);
    const Data::Items::PetStatValues without = Data::Items::EvaluatePetStats(pet.stats, withoutCommand);
    return {with.minimumDamage - without.minimumDamage, with.maximumDamage - without.maximumDamage,
            with.attackSpeed - without.attackSpeed};
}

int GetCommandGuildMembers(int level, int command)
{
    return Social::GetMaxGuildMembers(CLASS_DARK_LORD, level, command) -
           Social::GetMaxGuildMembers(CLASS_DARK_LORD, level, 0);
}

CommandEffects GetCommandEffects(int level, int command, const DarkLordPets& pets)
{
    CommandEffects effects;
    effects.attackRate = GetCommandAttackRate(command);
    effects.raven = GetCommandPetStats(pets.raven, command);
    effects.horse = GetCommandPetStats(pets.horse, command);
    effects.guildMembers = GetCommandGuildMembers(level, command);
    return effects;
}
} // namespace GameLogic::Character
