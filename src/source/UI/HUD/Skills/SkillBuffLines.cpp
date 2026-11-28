#include "stdafx.h"

#include "UI/HUD/Skills/SkillBuffLines.h"

#include "Data/GameData/BuffData/BuffLineTexts.h"
#include "GameLogic/Buffs/BuffLineFormat.h"
#include "Engine/Object/ZzzInfomation.h"

#include <cmath>
#include <cwchar>
#include <string_view>

namespace UI::Skills::Tooltip
{
namespace
{
using Data::Buffs::BuffAttributeUnit;
using Data::Buffs::BuffLineText;
using GameLogic::Buffs::SkillBuffValue;

constexpr size_t NumberSize = 32;
constexpr double PercentPerFraction = 100.0;
constexpr wchar_t PartIndent[] = L"  ";

using Number = wchar_t[NumberSize];

// The shared formats (GameLogic/Buffs/BuffLineFormat.h) straight into the tooltip's fixed buffers: the tooltip builds
// its lines every frame while it shows, so nothing allocates.
void FormatNumber(double value, BuffAttributeUnit unit, bool withSign, bool withPercent, Number& out)
{
    GameLogic::Buffs::FormatBuffNumber(value, unit, withSign, withPercent, out);
}

void FormatTotal(double value, BuffAttributeUnit unit, Number& out)
{
    GameLogic::Buffs::FormatBuffTotal(value, unit, out);
}

// `text` with {value} and {divisor} replaced, appended to `out` (a line of MAX_TOOLTIP_LINE_TEXT letters).
void AppendFilled(wchar_t* out, std::wstring_view text, const wchar_t* value, const wchar_t* divisor = L"")
{
    GameLogic::Buffs::AppendBuffText(std::span<wchar_t>(out, MAX_TOOLTIP_LINE_TEXT), text, value, divisor);
}

Line& StartLine(Model& model, LineColor color)
{
    Line& line = model.lines[model.count < MAX_TOOLTIP_LINES ? model.count++ : MAX_TOOLTIP_LINES - 1];
    line.text[0] = L'\0';
    line.color = color;
    line.isBold = false;
    line.isBlank = false;
    return line;
}

// How a value's numbers read: a power-up's in its attribute's unit, the duration's in seconds.
struct ValueStyle
{
    BuffAttributeUnit unit = BuffAttributeUnit::Flat;
    bool seconds = false;
};

// A part's amount, in seconds for the duration.
void FormatPart(double value, const ValueStyle& style, Number& out)
{
    Number number;
    FormatNumber(value, style.unit, true, true, number);
    const std::wstring* seconds = style.seconds ? g_BuffLineTexts.Get(BuffLineText::Seconds) : nullptr;
    out[0] = L'\0';
    if (seconds != nullptr)
    {
        wchar_t line[MAX_TOOLTIP_LINE_TEXT] = L"";
        AppendFilled(line, *seconds, number);
        std::wcsncpy(out, line, NumberSize - 1);
        out[NumberSize - 1] = L'\0';
    }
    else
    {
        std::wcsncpy(out, number, NumberSize);
    }
}

void AddBaseLine(Model& model, double constant, const ValueStyle& style)
{
    const std::wstring* base = g_BuffLineTexts.Get(BuffLineText::Base);
    if (base == nullptr || constant == 0)
    {
        return;
    }
    Number amount;
    FormatPart(constant, style, amount);
    Line& line = StartLine(model, LineColor::Gray);
    AppendFilled(line.text, PartIndent, L"");
    AppendFilled(line.text, *base, amount);
}

void AddTermLines(Model& model, const Data::Buffs::BuffValue& data, const GameLogic::Buffs::BuffCasterStats& stats,
                  const ValueStyle& style)
{
    for (const Data::Buffs::BuffTerm& term : data.per)
    {
        const std::wstring* label = g_BuffLineTexts.ForTerm(term.heroAttribute);
        const double amount = GameLogic::Buffs::ComputeTerm(term, stats);
        Number shown;
        FormatNumber(amount, style.unit, false, false, shown);
        if (label == nullptr || std::wcscmp(shown, L"0") == 0)
        {
            continue;
        }
        Number stat;
        FormatNumber(stats[static_cast<size_t>(term.heroAttribute)], BuffAttributeUnit::Flat, false, false, stat);
        Number part;
        FormatPart(amount, style, part);
        Line& line = StartLine(model, LineColor::Gray);
        AppendFilled(line.text, PartIndent, L"");
        AppendFilled(line.text, *label, stat);
        AppendFilled(line.text, L": ", L"");
        AppendFilled(line.text, part, L"");
        const std::wstring* rule =
            g_BuffLineTexts.Get(term.divisor > 0 ? BuffLineText::RuleDivisor : BuffLineText::RuleTimes);
        if (rule != nullptr)
        {
            // A rule's number in the shown unit, so 1 % per 200 energy reads "1 per 200" where the data divides the
            // fraction by 20000; else as the data writes it (x0.02 of the maximum mana).
            const double scale = style.unit == BuffAttributeUnit::Fraction ? PercentPerFraction : 1;
            Number number;
            std::swprintf(number, NumberSize, L"%g", term.divisor > 0 ? term.divisor / scale : term.times * scale);
            AppendFilled(line.text, L" (", L"");
            AppendFilled(line.text, *rule, number, number);
            AppendFilled(line.text, L")", L"");
        }
    }
}

void AddMaximumLine(Model& model, const Data::Buffs::BuffValue& data, const SkillBuffValue& value,
                    const ValueStyle& style)
{
    const std::wstring* maximum = g_BuffLineTexts.Get(BuffLineText::Maximum);
    if (maximum == nullptr || !value.stoppedAtMaximum || !data.maximum)
    {
        return;
    }
    Number amount;
    FormatNumber(*data.maximum, style.unit, false, true, amount);
    Number shown;
    if (style.seconds)
    {
        FormatPart(*data.maximum, style, shown);
        std::wcsncpy(amount, shown[0] == L'+' ? shown + 1 : shown, NumberSize);
    }
    Line& line = StartLine(model, LineColor::Gray);
    AppendFilled(line.text, PartIndent, L"");
    AppendFilled(line.text, *maximum, amount);
}

void AddMasterLines(Model& model, const SkillBuffValue& value, const ValueStyle& style)
{
    for (size_t index = 0; index < value.masterCount; ++index)
    {
        const GameLogic::Buffs::MasterPart& master = value.masters[index];
        Number amount;
        if (master.how == Data::Skills::MasterSkillHow::Multiply)
        {
            FormatNumber(master.value / PercentPerFraction, BuffAttributeUnit::Fraction, true, true, amount);
        }
        else if (style.seconds || style.unit == BuffAttributeUnit::Flat)
        {
            FormatPart(master.value, style, amount);
        }
        else
        {
            FormatNumber(master.value / PercentPerFraction, BuffAttributeUnit::Fraction, true, true, amount);
        }
        Line& line = StartLine(model, LineColor::Gray);
        AppendFilled(line.text, PartIndent, L"");
        if (master.skill >= 0 && master.skill < MAX_SKILLS && SkillAttribute != nullptr)
        {
            AppendFilled(line.text, SkillAttribute[master.skill].Name, L"");
        }
        AppendFilled(line.text, L": ", L"");
        AppendFilled(line.text, amount, L"");
    }
}

// The line of another power-up of the buff this one adds or is multiplied by, with that one's total.
void AddChainLine(Model& model, const GameLogic::Buffs::SkillBuffValues& values, int other, bool multiplied)
{
    if (other < 0 || static_cast<size_t>(other) >= values.powerUpCount)
    {
        return;
    }
    const int attribute = values.buff->powerUps[other].attributeIndex;
    const std::wstring* text = g_BuffLineTexts.ForAttribute(attribute);
    if (text == nullptr)
    {
        return;
    }
    Number total;
    FormatTotal(values.powerUps[other].total, Data::Buffs::BuffAttributeUnitAt(attribute), total);
    Line& line = StartLine(model, LineColor::Gray);
    AppendFilled(line.text, PartIndent, L"");
    if (multiplied)
    {
        AppendFilled(line.text, L"x ", L"");
    }
    AppendFilled(line.text, *text, total);
}
} // namespace

void AddSkillBuffLines(Model& model, const GameLogic::Buffs::SkillBuffValues& values,
                       const GameLogic::Buffs::BuffCasterStats& stats)
{
    if (values.buff == nullptr)
    {
        return;
    }
    for (size_t index = 0; index < values.powerUpCount; ++index)
    {
        const Data::Buffs::BuffPowerUp& powerUp = values.buff->powerUps[index];
        const SkillBuffValue& value = values.powerUps[index];
        const std::wstring* text = g_BuffLineTexts.ForAttribute(powerUp.attributeIndex);
        const bool chained = value.valueOfCount > 0 || value.multipliedBy != Data::Buffs::NoPowerUp;
        // A placeholder value (Infinity Arrow's damage without its master skill) has nothing to show.
        if (text == nullptr || (value.total == 0 && value.masterCount == 0 && !chained))
        {
            continue;
        }
        const ValueStyle style{Data::Buffs::BuffAttributeUnitAt(powerUp.attributeIndex), false};
        Number total;
        FormatTotal(value.total, style.unit, total);
        AppendFilled(StartLine(model, LineColor::White).text, *text, total);
        AddBaseLine(model, powerUp.value.values, style);
        AddTermLines(model, powerUp.value, stats, style);
        AddMaximumLine(model, powerUp.value, value, style);
        for (size_t added = 0; added < value.valueOfCount; ++added)
        {
            AddChainLine(model, values, value.valueOf[added], false);
        }
        AddMasterLines(model, value, style);
        AddChainLine(model, values, value.multipliedBy, true);
    }
    const std::wstring* duration = g_BuffLineTexts.Get(BuffLineText::Duration);
    if (values.hasDuration && duration != nullptr && values.duration.total > 0)
    {
        const ValueStyle style{BuffAttributeUnit::Flat, true};
        Number total;
        FormatTotal(values.duration.total, BuffAttributeUnit::Flat, total);
        AppendFilled(StartLine(model, LineColor::White).text, *duration, total);
        AddBaseLine(model, values.buff->duration->values, style);
        AddTermLines(model, *values.buff->duration, stats, style);
        AddMaximumLine(model, *values.buff->duration, values.duration, style);
        AddMasterLines(model, values.duration, style);
    }
}
} // namespace UI::Skills::Tooltip
