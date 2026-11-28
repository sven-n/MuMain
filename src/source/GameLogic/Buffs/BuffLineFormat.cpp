#include "stdafx.h"

#include "GameLogic/Buffs/BuffLineFormat.h"

#include "GameLogic/Buffs/BuffValues.h"

#include <cmath>
#include <cwchar>

namespace GameLogic::Buffs
{
namespace
{
constexpr size_t NumberSize = 32;
constexpr double PercentPerFraction = 100.0;
// A number this close to a whole one shows without decimals.
constexpr double WholeTolerance = 0.05;
constexpr std::wstring_view ValuePlaceholder = L"{value}";
constexpr std::wstring_view DivisorPlaceholder = L"{divisor}";

// Hands `append` the parts of `text` with {value} and {divisor} replaced, in order.
template <typename Append>
void FillParts(std::wstring_view text, std::wstring_view value, std::wstring_view divisor, Append&& append)
{
    size_t at = 0;
    while (at < text.size())
    {
        if (text.compare(at, ValuePlaceholder.size(), ValuePlaceholder) == 0)
        {
            append(value);
            at += ValuePlaceholder.size();
        }
        else if (text.compare(at, DivisorPlaceholder.size(), DivisorPlaceholder) == 0)
        {
            append(divisor);
            at += DivisorPlaceholder.size();
        }
        else
        {
            append(text.substr(at, 1));
            ++at;
        }
    }
}
} // namespace

void FormatBuffNumber(double value, Data::Buffs::BuffAttributeUnit unit, bool withSign, bool withPercent,
                      std::span<wchar_t> out)
{
    if (out.empty())
    {
        return;
    }
    const bool fraction = unit == Data::Buffs::BuffAttributeUnit::Fraction;
    const double shown = fraction ? value * PercentPerFraction : value;
    const wchar_t* sign = withSign && shown >= 0 ? L"+" : L"";
    const wchar_t* percent = fraction && withPercent ? L" %" : L"";
    if (!fraction)
    {
        std::swprintf(out.data(), out.size(), L"%ls%d", sign, CutToWhole(shown));
    }
    else if (std::abs(shown - std::round(shown)) < WholeTolerance)
    {
        std::swprintf(out.data(), out.size(), L"%ls%d%ls", sign, CutToWhole(std::round(shown)), percent);
    }
    else
    {
        std::swprintf(out.data(), out.size(), L"%ls%.1f%ls", sign, shown, percent);
    }
}

void FormatBuffTotal(double value, Data::Buffs::BuffAttributeUnit unit, std::span<wchar_t> out)
{
    if (out.empty())
    {
        return;
    }
    const bool fraction = unit == Data::Buffs::BuffAttributeUnit::Fraction;
    std::swprintf(out.data(), out.size(), L"%d", CutToWhole(fraction ? value * PercentPerFraction : value));
}

void AppendBuffText(std::span<wchar_t> out, std::wstring_view text, std::wstring_view value, std::wstring_view divisor)
{
    if (out.empty())
    {
        return;
    }
    size_t length = std::wcslen(out.data());
    const auto append = [&](std::wstring_view part)
    {
        for (const wchar_t letter : part)
        {
            if (length + 1 >= out.size())
            {
                return;
            }
            out[length++] = letter;
        }
    };
    FillParts(text, value, divisor, append);
    out[length] = L'\0';
}

std::wstring FormatBuffNumber(double value, Data::Buffs::BuffAttributeUnit unit, bool withSign, bool withPercent)
{
    wchar_t out[NumberSize] = {};
    FormatBuffNumber(value, unit, withSign, withPercent, out);
    return out;
}

std::wstring FormatBuffTotal(double value, Data::Buffs::BuffAttributeUnit unit)
{
    wchar_t out[NumberSize] = {};
    FormatBuffTotal(value, unit, out);
    return out;
}

std::wstring FillBuffText(std::wstring_view text, std::wstring_view value, std::wstring_view divisor)
{
    std::wstring out;
    out.reserve(text.size() + value.size());
    FillParts(text, value, divisor, [&out](std::wstring_view part) { out += part; });
    return out;
}
} // namespace GameLogic::Buffs
