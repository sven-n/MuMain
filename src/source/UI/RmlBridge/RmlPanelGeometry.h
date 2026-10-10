#pragma once

namespace Rml
{
    class ElementDocument;
}

// Where a window's panel goes: the size a fill slot gives it and the place a workspace slot or the
// stage gives it, applied to its document as properties the theme reads.
namespace UI::RmlBridge
{
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
    // top-left in screen pixels and its region's scale, given to the panel as --slot-left,
    // --slot-top and --root-scale with the "slot-placed" class. base.rcss's #panel.slot-placed
    // applies them, so a theme can place the panel otherwise. A zero scale means not placed.
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
        // Returns a panel the player dragged (MakeDraggable()) to where its slot puts it.
        static void ResetDrag(Rml::ElementDocument* doc, const char* panelId);
    };
}
