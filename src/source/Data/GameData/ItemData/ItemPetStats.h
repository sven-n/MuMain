#pragma once

// The combat stats of a Dark Lord's pet (the Dark Raven, the Dark Horse) as formulas in its item data ("petStats";
// docs/item-data.md, "Pet stats"): what the pet window shows and the command row's hint counts.
namespace Data::Items
{
// One stat: add + pet level * perPetLevel / petLevelDivisor + strength / strengthDivisor + command / commandDivisor
// + minimum damage * minimumPercent / 100. Each division is rounded down; a divisor of 0 leaves its term out.
struct PetStatFormula
{
    int add = 0;
    int perPetLevel = 0;
    int petLevelDivisor = 1;
    int strengthDivisor = 0;
    int commandDivisor = 0;
    // The share of the pet's minimum damage, in percent: the Dark Horse's maximum damage is 150 % of its minimum.
    int minimumPercent = 0;

    bool IsEmpty() const
    {
        return *this == PetStatFormula{};
    }

    bool operator==(const PetStatFormula& other) const = default;
};

// An item's pet stats; none for an item that is no such pet.
struct ItemPetStats
{
    PetStatFormula minimumDamage;
    PetStatFormula maximumDamage;
    PetStatFormula attackSpeed;
    PetStatFormula attackSuccess;

    bool IsEmpty() const
    {
        return minimumDamage.IsEmpty() && maximumDamage.IsEmpty() && attackSpeed.IsEmpty() && attackSuccess.IsEmpty();
    }

    bool operator==(const ItemPetStats& other) const = default;
};

// What the formulas count: the pet's level and its owner's strength and command, each with what items and buffs add.
struct PetStatInputs
{
    int petLevel = 0;
    int strength = 0;
    int command = 0;

    bool operator==(const PetStatInputs& other) const = default;
};

struct PetStatValues
{
    int minimumDamage = 0;
    int maximumDamage = 0;
    int attackSpeed = 0;

    bool operator==(const PetStatValues& other) const = default;
};

int EvaluatePetStat(const PetStatFormula& formula, const PetStatInputs& inputs, int minimumDamage);
// The minimum damage first, since the other stats may take a share of it.
PetStatValues EvaluatePetStats(const ItemPetStats& stats, const PetStatInputs& inputs);
} // namespace Data::Items
