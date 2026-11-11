#pragma once

#include "Core/Globals/_enum.h"

namespace GameLogic::Party
{
// Whether a hero of this base class casts this skill on a party member chosen through the member's party card.
[[nodiscard]] bool IsPartyMemberSkill(CLASS_TYPE heroBaseClass, ActionSkillType skill);
} // namespace GameLogic::Party
