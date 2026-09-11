#pragma once

#include <functional>

namespace Rml
{
    class Element;
}

// Makes an RmlUi panel draggable-by-mouse with no legacy CWin dependency -- call MakeDraggable()
// once per migrated panel instead of a per-window drag state machine.
//
// Gotchas:
//   - Always write the dragged position in `dp` (divide by GetDensityIndependentPixelRatio()),
//     never raw `px` -- `px` doesn't scale with UIScalePercent and drifts from dp-authored
//     siblings. Route onMove/onDragEnd back through the panel's own position-binding rather than
//     leaving RmlUi's property write as the only source of truth, so a `px`-authored panel still
//     round-trips correctly.
//   - Persist position in `onDragEnd` via GameConfig::Get/SetWindowPosition(), as an
//     anchor-relative logical position (not a raw device-pixel pair), so it survives
//     resolution/UI-scale changes.
//   - A window with a real Type-2 companion object (e.g. CUITextInputBox) must rescale it in
//     OnPanelMoved by the same ratio RmlUi's dp uses (GameConfig::GetUIScalePercent() x
//     UI::Scaling::ViewportFitScale(), see LoginWin.cpp's LoginUIScaleRatio()), or it drifts from
//     the dragged element at any non-reference UI scale.
namespace UI::RmlBridge
{
    // Fired on every drag move with the panel's new absolute top-left in real window pixels.
    // Needed only for a hybrid window whose legacy CWin chrome (e.g. a background sprite) shares
    // screen space but isn't part of the RmlUi element tree, so it must be repositioned manually
    // or it desyncs from the dragged overlay. A fully migrated panel can omit this.
    using OnPanelMoved = std::function<void(float newLeft, float newTop)>;

    // Fired once when the drag completes -- use this to persist the position instead of writing
    // to disk on every OnPanelMoved tick.
    using OnDragEnd = std::function<void()>;

    // `handle` is the element the player grabs (usually a title bar, not `panel` itself) --
    // a panel typically inherits `pointer-events: none` from its document, so it can't be its own
    // handle without a matching RCSS override.
    //
    // `panel` is the element that moves -- must be `position: absolute` with `left`/`top` already
    // resolved to fixed `px` (a panel positioned in `%`/`vw`/`vh` isn't supported).
    //
    // Sets `drag: drag` and `pointer-events: auto` on `handle` so the caller doesn't need a
    // matching RCSS rule -- without `pointer-events: auto` the handle never enters the hover
    // chain and dragstart/drag never fire. Listens for RmlUi's native dragstart/drag/dragend
    // events, moves `panel` by setting left/top directly, and calls onMove/onDragEnd as the drag
    // proceeds and finishes.
    void MakeDraggable(Rml::Element* handle, Rml::Element* panel, OnPanelMoved onMove = nullptr,
        OnDragEnd onDragEnd = nullptr);
}
