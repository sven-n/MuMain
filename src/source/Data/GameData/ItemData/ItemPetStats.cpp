#include "Data/GameData/ItemData/ItemPetStats.h"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace Data::Items
{
namespace
{
constexpr int PercentBase = 100;

std::int64_t DivideOrNone(std::int64_t value, int divisor)
{
    return divisor > 0 ? value / divisor : 0;
}
} // namespace

int EvaluatePetStat(const PetStatFormula& formula, const PetStatInputs& inputs, int minimumDamage)
{
    const auto levelTerm =
        DivideOrNone(static_cast<std::int64_t>(inputs.petLevel) * formula.perPetLevel, formula.petLevelDivisor);
    const auto minimumShare = static_cast<std::int64_t>(minimumDamage) * formula.minimumPercent / PercentBase;
    const auto value = formula.add + levelTerm + DivideOrNone(inputs.strength, formula.strengthDivisor) +
                       DivideOrNone(inputs.command, formula.commandDivisor) + minimumShare;
    // The pet window stores each stat in an unsigned 16-bit field.
    return static_cast<int>(std::clamp<std::int64_t>(value, 0, std::numeric_limits<std::uint16_t>::max()));
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
