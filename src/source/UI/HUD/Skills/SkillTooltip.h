#pragma once

#include "Core/Globals/_enum.h"            // ActionSkillType
#include "Engine/Object/ZzzInventory.h"    // SKILL_TOOLTIP_RENDER_POINT (STRP_NONE et al.)
#include "UI/HUD/Skills/SkillTooltipModel.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <vector>

namespace UI::Skills::Tooltip
{
    // Renders the hover tooltip for a skill in the action bar / skill list. Type is the slot
    // index in CharacterAttribute->Skill[]; SkillNum is unused (kept for signature compat).
    // iRenderPoint anchors relative to (sx, sy); STRP_NONE means legacy positioning.
    void Render(int sx, int sy, int Type, int SkillNum = 0, int iRenderPoint = STRP_NONE);

    // Same content resolution as Render() (pet-command dispatch, then BuildModel()) with no
    // drawing -- used by CMainFrameWindow::SyncRmlModel() to feed RmlUi. Returns false only
    // when CharacterAttribute is null.
    bool BuildModelForSlot(int Type, Model& outModel);

    // Converts a resolved Model's lines into UI::RmlBridge::Tooltip's own Line list -- shared by
    // every in-game caller that migrates off Render()'s native draw onto the shared RmlUi tooltip,
    // so each one doesn't hand-roll the same LineColor switch. Deliberately NOT in
    // SkillTooltipModel.h: that header is also shared with the standalone MuEditor (ImGui) tool,
    // which has no RmlUi/UI::RmlBridge dependency to pull in.
    std::vector<UI::RmlBridge::Tooltip::Line> ToRmlBridgeLines(const Model& model);
}
