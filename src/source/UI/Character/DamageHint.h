#pragma once

#include "GameLogic/Combat/CriticalDamageBonus.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <vector>

// The hint of the character window's damage line: what a hit deals before the target's defense, and where the extra
// damage of a critical hit comes from.
namespace UI::Character
{
// The damage the line shows, from its minimum to its maximum.
struct DamageRange
{
    int minimum = 0;
    int maximum = 0;

    bool operator==(const DamageRange&) const = default;
};

// A normal hit (the range), a critical hit (the maximum plus the critical damage bonus) and an excellent hit (the
// maximum times 1.2 plus the excellent and the critical damage bonus), then, when something adds critical damage,
// each part above 0 and their total.
std::vector<UI::RmlBridge::Tooltip::Line>
BuildDamageHint(DamageRange range, const GameLogic::Combat::CriticalDamageBonus& criticalBonus, int excellentBonus);
} // namespace UI::Character
