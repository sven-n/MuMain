#pragma once

#include "GameLogic/Character/CommandEffects.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <vector>

// The hint of the character window's command row: what the Dark Lord's command adds (issue #1017).
namespace UI::Character
{
// The command's parts that add something: the PvM attack rate, the Dark Raven's and Dark Horse's damage and attack
// speed, and the guild members the command adds to the level's. Empty, so no hint, when no part adds anything.
std::vector<UI::RmlBridge::Tooltip::Line> BuildCommandHint(const GameLogic::Character::CommandEffects& effects);
} // namespace UI::Character
