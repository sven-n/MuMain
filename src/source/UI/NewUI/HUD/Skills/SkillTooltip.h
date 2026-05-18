#pragma once

#include "Core/Globals/_enum.h"            // ActionSkillType
#include "Engine/Object/ZzzInventory.h"    // SKILL_TOOLTIP_RENDER_POINT (STRP_NONE et al.)

namespace UI::Skills::Tooltip
{
    // Renders the hover tooltip for a skill in the action bar / skill list.
    // Type is the slot index in CharacterAttribute->Skill[]. SkillNum is kept
    // for signature compatibility with the original RenderSkillInfo (defaults
    // to 0; the body never reads it). iRenderPoint anchors the tooltip
    // relative to (sx, sy); STRP_NONE means "use legacy positioning".
    void Render(int sx, int sy, int Type, int SkillNum = 0, int iRenderPoint = STRP_NONE);
}
