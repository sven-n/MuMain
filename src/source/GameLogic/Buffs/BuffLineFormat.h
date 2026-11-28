#pragma once

#include "Data/GameData/BuffData/BuffAttributes.h"

#include <span>
#include <string>
#include <string_view>

// How a buff's numbers read in a tooltip line (buffs design B7, B9): the skill tooltip and the buff tooltip fill the
// texts of Data/Buffs/BuffTexts.json the same way.
namespace GameLogic::Buffs
{
// `value` in its unit, with a "+" before it when `withSign` and it is not negative: a flat number cut to a whole one,
// as the game counts it (so a value's cut parts can add up to less than its total, which is cut once); a fraction in
// percent with " %" when `withPercent`, whole when it is close to whole, else with one decimal (a master skill's 32.4
// %).
std::wstring FormatBuffNumber(double value, Data::Buffs::BuffAttributeUnit unit, bool withSign, bool withPercent);

// A total as the game counts it: cut to a whole number in its unit (a fraction's percent).
std::wstring FormatBuffTotal(double value, Data::Buffs::BuffAttributeUnit unit);

// `text` with {value} and {divisor} replaced.
std::wstring FillBuffText(std::wstring_view text, std::wstring_view value, std::wstring_view divisor = {});

// The same into a fixed buffer, ended by a 0 and cut where it is full: the skill tooltip builds its lines every frame
// while it shows, so it formats without allocating (coding rules, section 12).
void FormatBuffNumber(double value, Data::Buffs::BuffAttributeUnit unit, bool withSign, bool withPercent,
                      std::span<wchar_t> out);
void FormatBuffTotal(double value, Data::Buffs::BuffAttributeUnit unit, std::span<wchar_t> out);
// Appends `text` with its placeholders replaced to the 0-ended text in `out`.
void AppendBuffText(std::span<wchar_t> out, std::wstring_view text, std::wstring_view value,
                    std::wstring_view divisor = {});
} // namespace GameLogic::Buffs
