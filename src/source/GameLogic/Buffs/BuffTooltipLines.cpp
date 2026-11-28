#include "stdafx.h"

#include "GameLogic/Buffs/BuffTooltipLines.h"

#include "Data/GameData/BuffData/BuffLineTexts.h"
#include "Data/GameData/SkillData/MasterSkillDatabase.h"
#include "GameLogic/Buffs/BuffLineFormat.h"

#include <algorithm>
#include <optional>

namespace GameLogic::Buffs
{
namespace
{
using Data::Buffs::BuffAttributeUnit;
using Data::Buffs::BuffLineText;

constexpr wchar_t UnknownValue[] = L"?";

BuffAttributeUnit UnitOf(int attribute)
{
    return Data::Buffs::BuffAttributeUnitAt(attribute);
}

bool CutsToZero(double value, BuffAttributeUnit unit)
{
    return FormatBuffTotal(value, unit) == L"0";
}

void AddValueLine(std::vector<BuffTooltipLine>& lines, int attribute, double value)
{
    const std::wstring* text = g_BuffLineTexts.ForAttribute(attribute);
    const BuffAttributeUnit unit = UnitOf(attribute);
    if (text == nullptr || CutsToZero(value, unit))
    {
        return;
    }
    lines.push_back({FillBuffText(*text, FormatBuffTotal(value, unit)), false});
}

// The total of the power-up at `index` from the sent amounts: its own, plus the totals its valueOf names, times the
// total its multipliedBy names (buffs design B21). The loader rejects loops; `depth` stops one anyway.
std::optional<double> ChainedTotal(const Data::Buffs::BuffDefinition& buff, const std::vector<SentValue>& values,
                                   size_t index, int depth = 0)
{
    if (index >= buff.powerUps.size() || depth > static_cast<int>(buff.powerUps.size()))
    {
        return std::nullopt;
    }
    const Data::Buffs::BuffPowerUp& powerUp = buff.powerUps[index];
    const auto own = std::find_if(values.begin(), values.end(), [&powerUp](const SentValue& value)
                                  { return value.attribute == powerUp.attributeIndex; });
    // A power-up made only of others (Berserker's wizardry multiplier) isn't one of the server's effect: it has no own
    // value, only those it adds.
    if (own == values.end() && powerUp.valueOfIndices.empty())
    {
        return std::nullopt;
    }
    double total = own != values.end() ? SentValueAmount(*own) : 0.0;
    // OpenMU applies some maximums outside the effect (Berserker's life decrement stops at -10 % in its class rules),
    // so the sent value can pass the one the entry has. Where a master skill raises the attribute, the sent value is
    // the base stopped at its maximum plus the master part (B18), which passes the maximum rightly.
    if (own != values.end() && powerUp.value.maximum && !g_MasterSkillDatabase.RaisesAttribute(powerUp.attributeIndex))
    {
        total = std::min(total, *powerUp.value.maximum);
    }
    for (const int added : powerUp.valueOfIndices)
    {
        total += ChainedTotal(buff, values, static_cast<size_t>(added), depth + 1).value_or(0.0);
    }
    if (powerUp.multipliedByIndex != Data::Buffs::NoPowerUp)
    {
        const std::optional<double> factor =
            ChainedTotal(buff, values, static_cast<size_t>(powerUp.multipliedByIndex), depth + 1);
        total *= factor.value_or(1.0);
    }
    return total;
}

std::vector<BuffTooltipLine> SentLines(const Data::Buffs::BuffDefinition& buff, const std::vector<SentValue>& values)
{
    std::vector<BuffTooltipLine> lines;
    for (size_t index = 0; index < buff.powerUps.size(); ++index)
    {
        const std::optional<double> total = ChainedTotal(buff, values, index);
        if (total.has_value())
        {
            AddValueLine(lines, buff.powerUps[index].attributeIndex, *total);
        }
    }
    // A value the server sends for an attribute the buff's entry doesn't list: as it came.
    for (const SentValue& value : values)
    {
        const bool listed =
            std::any_of(buff.powerUps.begin(), buff.powerUps.end(), [&value](const Data::Buffs::BuffPowerUp& powerUp)
                        { return powerUp.attributeIndex == value.attribute; });
        if (!listed)
        {
            AddValueLine(lines, value.attribute, SentValueAmount(value));
        }
    }
    return lines;
}

std::vector<BuffTooltipLine> UnknownLines(const Data::Buffs::BuffDefinition& buff)
{
    std::vector<BuffTooltipLine> lines;
    for (const Data::Buffs::BuffPowerUp& powerUp : buff.powerUps)
    {
        // A part only a master skill fills (0 with no terms, Berserker Proficiency's multiplier) has no line of its
        // own.
        const bool masterOnly = powerUp.value.values == 0 && powerUp.value.per.empty() && powerUp.valueOf.empty();
        const std::wstring* text = g_BuffLineTexts.ForAttribute(powerUp.attributeIndex);
        if (text != nullptr && !masterOnly)
        {
            lines.push_back({FillBuffText(*text, UnknownValue), false});
        }
    }
    const std::wstring* note = g_BuffLineTexts.Get(BuffLineText::DependsOnCaster);
    if (!lines.empty() && note != nullptr)
    {
        lines.push_back({*note, true});
    }
    return lines;
}
} // namespace

std::vector<BuffTooltipLine> BuildBuffValueLines(const Data::Buffs::BuffDefinition& buff, const ServerBuffValues& sent,
                                                 const BuffCasterStats& owner)
{
    if (buff.valuesFrom == Data::Buffs::BuffValuesFrom::Caster)
    {
        const std::vector<SentValue>* values = sent.Find(buff.number);
        return values != nullptr ? SentLines(buff, *values) : UnknownLines(buff);
    }
    if (!buff.description.IsEmpty())
    {
        return {};
    }
    std::vector<BuffTooltipLine> lines;
    for (const Data::Buffs::BuffPowerUp& powerUp : buff.powerUps)
    {
        // Uncut: AddValueLine cuts the number in its unit, a fraction after it turns into a percent.
        AddValueLine(lines, powerUp.attributeIndex, ComputeValue(powerUp.value, owner));
    }
    return lines;
}
} // namespace GameLogic::Buffs
