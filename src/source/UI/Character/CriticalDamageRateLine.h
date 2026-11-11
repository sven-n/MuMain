#pragma once

#include "GameLogic/Combat/CriticalDamageRate.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <string>
#include <vector>

// The character window's critical damage rate: its line under the strength's lines and the lines shown while the
// pointer is over it.
namespace UI::Character
{
struct CriticalDamageRateLine
{
    // "Critical damage rate: 33%", with "(+9%)" after it while the hero's own buff adds to it.
    std::string text;
    // The window's colour for the line: "buffed" while the buff adds to it, "boosted" when worn items do, "base"
    // otherwise.
    std::string source;
};

CriticalDamageRateLine BuildCriticalDamageRateLine(const GameLogic::Combat::CriticalDamageRate& rate);

// The title, each part above 0 (the luck, the set options, the socket options, the master skill's buff, the Scroll of
// Battle) and the total.
std::vector<UI::RmlBridge::Tooltip::Line>
BuildCriticalDamageRateHint(const GameLogic::Combat::CriticalDamageRate& rate);
} // namespace UI::Character
