#pragma once

namespace Rml
{
    class ElementDocument;
}

namespace UI::Scaling
{
    enum class LayoutMode;
}

// Lets native hit-testing (WindowGeometry, CManager::UpdateMouseEvent()) follow a theme's actual
// panel size instead of a hardcoded width/height literal duplicated per window. Two independent
// families use this today: the docked-family windows (character_info/pet_info/party_info/
// my_quest_info/quest_progress/quest_progress_etc/npc_dialogue/npc_quest) whose #panel comes from
// docked_panel_frame.rcss, and the inventory-family windows (my_inventory/trade/storage/
// storage_ext/mix_inventory/npc_shop/my_shop/purchase_shop/inventory_extension) whose #panel is
// each window's own foreground document's frame/chrome root (the paired *_bg.rml's #bg_root is a
// decorative-only duplicate, never the authoritative one). Same function either way -- callers
// just point it at whichever document/id is that window's own real #panel.
namespace UI::RmlBridge
{
    // Looks up `panelId` in `doc` and reads its live resolved border-box size, which is already in
    // the logical/reference-space units WindowGeometry/MouseX/MouseY use: every #panel read here is
    // sized in plain `px` and scaled only at paint time, by `transform: scale(root_scale)` (see
    // SyncRootTransform, RmlRootTransform.h). RmlUi's layout box ignores a render-time transform,
    // so no scale conversion applies -- dividing by the active transform here shrinks the hit box
    // by that scale, which at the usual capped 2.0 leaves only the panel's top-left quarter
    // clickable and walks the character on every click outside it. (RmlTooltip.cpp's own
    // #tooltip_panel read genuinely is in screen pixels, but only because that document carries no
    // root transform and its C++ pre-multiplies the scale into the width it sets -- not a
    // precedent for this family.)
    //
    // Leaves `width`/`height` unchanged and returns false if `doc` is null, `panelId` isn't found,
    // the active transform is degenerate, or the element hasn't been laid out yet (zero size --
    // e.g. the first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next Update()
    // pass resolves layout; see CharMakeWin.cpp's own comment on this same one-frame-stale
    // tradeoff). Callers should pre-seed width/height with a sane fallback (typically the window's
    // own historical hardcoded constant) rather than treating a false return as an error.
    bool RefreshLogicalPanelSize(Rml::ElementDocument* doc, const char* panelId, float& width, float& height);

    // Looks up `anchorId` in `doc`, reads its live resolved screen position (GetAbsoluteOffset()),
    // and converts it into the logical/reference-space units a native renderer/hit-test caller
    // expects (m_Pos, WindowGeometry, ::RenderItemInfo()/::RenderItem3D()'s own x/y contract) --
    // the same conversion CManager::Render()/UpdateMouseEvent() apply via
    // TransformForLayout(layoutMode, ...), computed explicitly here rather than trusting whatever
    // transform happens to be ambient at the call site (safe from any call point, including a
    // separate I3DRenderObj::Render3D() pass that may not share CManager's own ScopedActiveTransform
    // scope).
    //
    // Leaves `x`/`y` unchanged and returns false if `doc` is null, `anchorId` isn't found, or the
    // computed transform is degenerate. Callers should pre-seed x/y with the window's own historical
    // hardcoded offset as a fallback, same convention as RefreshLogicalPanelSize() above.
    bool RefreshLogicalAnchorPosition(Rml::ElementDocument* doc, const char* anchorId,
        UI::Scaling::LayoutMode layoutMode, float& x, float& y);
}
