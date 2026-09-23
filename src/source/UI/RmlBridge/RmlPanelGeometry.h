#pragma once

namespace Rml
{
    class ElementDocument;
}

// Lets native hit-testing (WindowGeometry, CManager::UpdateMouseEvent()) follow a theme's actual
// panel size instead of a hardcoded width/height literal duplicated per window -- see the docked-
// family windows (character_info/pet_info/party_info/my_quest_info/quest_progress/
// quest_progress_etc/npc_dialogue/npc_quest) whose #panel comes from docked_panel_frame.rcss.
namespace UI::RmlBridge
{
    // Looks up `panelId` in `doc`, reads its live resolved border-box size (RmlUi's own real
    // screen-pixel space -- confirmed via RmlTooltip.cpp's own #tooltip_panel read), and converts
    // it into the logical/reference-space units WindowGeometry/MouseX/MouseY use, via whichever
    // transform is active at the call site (UI::Scaling::GetActiveTransform() -- CManager pushes
    // the exact same TransformForLayout(GetLayoutMode(), ...) result before UpdateMouseEvent() as
    // it does before Update(), where these windows compute root_x/root_y with the identical
    // transform, so the two always agree).
    //
    // Leaves `width`/`height` unchanged and returns false if `doc` is null, `panelId` isn't found,
    // the active transform is degenerate, or the element hasn't been laid out yet (zero size --
    // e.g. the first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next Update()
    // pass resolves layout; see CharMakeWin.cpp's own comment on this same one-frame-stale
    // tradeoff). Callers should pre-seed width/height with a sane fallback (typically the window's
    // own historical hardcoded constant) rather than treating a false return as an error.
    bool RefreshLogicalPanelSize(Rml::ElementDocument* doc, const char* panelId, float& width, float& height);
}
