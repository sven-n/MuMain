#pragma once

#include "Data/GameData/BuffData/BuffDefinition.h"
#include "GameLogic/Buffs/BuffServerValues.h"
#include "GameLogic/Buffs/BuffValues.h"

#include <string>
#include <vector>

// The value lines of a buff icon's tooltip (buffs design B7, B8, B24, B26), between its name and its time.
namespace GameLogic::Buffs
{
struct BuffTooltipLine
{
    std::wstring text;
    bool note = false; // a gray note under the values ("The value depends on its caster")
};

// The lines of `buff` on the hero:
//   - a buff whose values come from its caster: one line per value the server sent (`sent`), chained as the buff's
//     power-ups are (Berserker's multipliedBy and valueOf), in the order of its power-ups; without sent values, its
//     lines with "?" and the note that the value depends on its caster (B8);
//   - any other buff with a description: none, the description shows instead (B26);
//   - any other buff: one line per power-up, its value from Buffs.json with `owner`'s attributes (B24).
// A line needs its attribute's text in Data/Buffs/BuffTexts.json; a value without one, or one that cuts to 0, has none.
std::vector<BuffTooltipLine> BuildBuffValueLines(const Data::Buffs::BuffDefinition& buff, const ServerBuffValues& sent,
                                                 const BuffCasterStats& owner);
} // namespace GameLogic::Buffs
