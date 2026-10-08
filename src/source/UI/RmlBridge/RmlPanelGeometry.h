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
// each window's own frame. Same function either way.
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

    // Gives `anchorId`'s position in the logical/reference-space units a native renderer expects
    // (m_Pos, ::RenderItemInfo()/::RenderItem3D()'s own x/y contract), as `panelPos` plus the
    // anchor's own offset inside `panelId`. Both the theme's declared offset and `panelPos` are
    // already reference-space, so nothing is converted -- see RefreshLogicalPanelSize() above for
    // why a scale conversion is wrong for this document family.
    //
    // The delta against `#panel` is what makes that true: an anchor's raw GetAbsoluteOffset() is
    // mixed-space (the panel's own left/top were pre-multiplied by the scale in SyncRootTransform,
    // the anchor's offset inside it was not), so un-mapping the whole sum through the transform
    // divides the child half and drags the result toward the panel's top-left. Take the delta, add
    // the caller's own position; don't reintroduce a transform here.
    //
    // Leaves `x`/`y` unchanged and returns false if `doc` is null or either id isn't found. Callers
    // should pre-seed x/y with the window's own historical hardcoded offset as a fallback, same
    // convention as RefreshLogicalPanelSize() above.
    bool RefreshLogicalAnchorPosition(Rml::ElementDocument* doc, const char* panelId,
        const char* anchorId, const POINT& panelPos, float& x, float& y);

    // RefreshLogicalAnchorPosition() plus the anchor's own size: a native rectangle (a hit area,
    // a slot) wherever the theme draws `anchorId`. Leaves the outputs unchanged and returns false
    // until the anchor exists and has a size.
    bool RefreshLogicalAnchorRect(Rml::ElementDocument* doc, const char* panelId, const char* anchorId,
        const POINT& panelPos, float& x, float& y, float& width, float& height);

    // The size a theme's data-fit="fill" slot gives a window (CObject::SetFillPlacementSize()), in
    // the panel's reference units; zero while the window is content-sized.
    struct FillPlacementSize
    {
        float width = 0.f;
        float height = 0.f;

        // Returns whether the size changed.
        bool Set(float newWidth, float newHeight);
        // Sizes `panelId` in `doc` and sets its "fill-placement" class, or clears both while zero,
        // so the theme's fill rules apply only to a filled panel.
        void Apply(Rml::ElementDocument* doc, const char* panelId) const;
        // Applies a non-zero size again to a document that lost it (rebuilt by a theme switch).
        void Sync(Rml::ElementDocument* doc, const char* panelId) const;
    };

    // Where a workspace slot puts a window's panel (CObject::GetPlacedDocument()): the slot's
    // top-left in screen pixels and its region's scale. Set inline on the panel, with the scale also
    // as --root-scale for the counter-scaled layers; a zero scale means not placed.
    struct SlotPlacement
    {
        float left = 0.f;
        float top = 0.f;
        float scale = 0.f;

        // Returns whether the placement changed.
        bool Set(float newLeft, float newTop, float newScale);
        // Places `panelId` in `doc` and sets its "slot-placed" class, or clears both while unplaced.
        void Apply(Rml::ElementDocument* doc, const char* panelId) const;
        // Applies the placement again to a document that lost it (rebuilt by a theme switch).
        void Sync(Rml::ElementDocument* doc, const char* panelId) const;
    };
}
