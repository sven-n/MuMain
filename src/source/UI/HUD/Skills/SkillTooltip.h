#pragma once

#include "Core/Globals/_enum.h"            // ActionSkillType
#include "Engine/Object/ZzzInventory.h"    // SKILL_TOOLTIP_RENDER_POINT (STRP_NONE et al.)
#include "UI/HUD/Skills/SkillTooltipModel.h"

namespace UI::Skills::Tooltip
{
    // Renders the hover tooltip for a skill in the action bar / skill list.
    // Type is the slot index in CharacterAttribute->Skill[]. SkillNum is kept
    // for signature compatibility with the original RenderSkillInfo (defaults
    // to 0; the body never reads it). iRenderPoint anchors the tooltip
    // relative to (sx, sy); STRP_NONE means "use legacy positioning".
    void Render(int sx, int sy, int Type, int SkillNum = 0, int iRenderPoint = STRP_NONE);

    // 2026-09-02, CNewUIMainFrameWindow pilot Phase 2: the same content resolution Render() itself
    // performs (pet-command dispatch via GIPetManager, then BuildOptions/BuildModel()) with no
    // drawing at all -- lets CNewUIMainFrameWindow::SyncRmlModel() bind the result into RmlUi
    // instead of the legacy TextList/RenderTipTextList path Render() still uses for its other
    // callers (NewUIMuHelper.cpp, NewUISiegeWarBase.cpp -- unrelated, still-legacy windows,
    // unaffected by this addition; Render() itself is unchanged apart from delegating to this).
    // Returns false only in the same defensive case Render() itself already early-outs on
    // (!CharacterAttribute) -- otherwise always true and outModel is filled exactly as Render()
    // would have rendered it (including whatever BuildModel() produces for a degenerate Type/
    // skillType of 0 -- not a new guard added here, the original had none beyond the two checks
    // this function preserves).
    bool BuildModelForSlot(int Type, Model& outModel);
}
