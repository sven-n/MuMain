#pragma once

#include "Core/Platform/WinCompat.h" // the Win32 scalar types _enum.h uses
#include "Core/Globals/_enum.h"

namespace UI::Party
{
// Whether a right-click on a party card casts the hero's current skill on that member: the support skills (heal,
// defense and attack buffs, Teleport Ally, Soul Barrier, Alice's thorns, recovery) of a Wizard, an Elf or a Summoner
// and their later classes, given by `baseClass`. Any other skill would be cast into the world, at the pointer.
[[nodiscard]] bool CastsOnPartyCard(CLASS_TYPE baseClass, ActionSkillType skill);
} // namespace UI::Party
