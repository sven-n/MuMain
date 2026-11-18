#include "Data/GameData/ItemData/ItemPetStats.h"

namespace Data::Items
{
namespace
{
constexpr int PercentBase = 100;

int DivideOrNone(int value, int divisor)
{
    return divisor > 0 ? value / divisor : 0;
}
} // namespace

int EvaluatePetStat(const PetStatFormula& formula, const PetStatInputs& inputs, int minimumDamage)
{
    return formula.add + DivideOrNone(inputs.petLevel * formula.perPetLevel, formula.petLevelDivisor) +
           DivideOrNone(inputs.strength, formula.strengthDivisor) +
           DivideOrNone(inputs.command, formula.commandDivisor) + minimumDamage * formula.minimumPercent / PercentBase;
}

PetStatValues EvaluatePetStats(const ItemPetStats& stats, const PetStatInputs& inputs)
{
    PetStatValues values;
    values.minimumDamage = EvaluatePetStat(stats.minimumDamage, inputs, 0);
    values.maximumDamage = EvaluatePetStat(stats.maximumDamage, inputs, values.minimumDamage);
    values.attackSpeed = EvaluatePetStat(stats.attackSpeed, inputs, values.minimumDamage);
    return values;
}
} // namespace Data::Items
