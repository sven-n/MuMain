#pragma once

#include <functional>

namespace Rml
{
    class Element;
}

// Makes an RmlUi panel draggable-by-mouse, with zero legacy CWin dependency -- see the
// migration plan and docs/rmlui-ui-system/architecture.md for why this exists: CWin's own
// dragging (CWin::Update()'s WS_MOVE state machine, gated by CursorInWin(WA_MOVE)) only works
// for windows still tied to that legacy positioning system, and re-implementing an equivalent
// per migrated window would be exactly the kind of duplicated logic Coding Rule #4 warns against.
// A future migrated panel that wants to be draggable should just call MakeDraggable() once --
// no per-window state machine, no CWin involvement at all.
namespace UI::RmlBridge
{
    // Fired every time the panel's position changes during a drag, with its new absolute
    // top-left in real window pixels. Only needed for a *hybrid* window that still has legacy
    // chrome sharing the same screen position (e.g. a CWin background sprite drawn underneath
    // the RmlUi overlay, positioned independently via CWin::SetPosition()/m_ptPos) -- MakeDraggable
    // itself only ever touches the RmlUi element's own left/top, so without this callback a
    // hybrid window's legacy chrome silently desyncs from the RmlUi overlay as it's dragged
    // (confirmed by a real test: the RmlUi checkboxes/buttons moved, the legacy background
    // sprite underneath them did not). A fully migrated, sprite-free panel has nothing to sync
    // and can omit this entirely.
    using OnPanelMoved = std::function<void(float newLeft, float newTop)>;

    // `handle` is the element the player grabs -- a dedicated drag handle (a title-bar element,
    // or in this pilot's test, a label positioned outside the panel's own box), not usually
    // `panel` itself. Using the whole panel as its own handle only works if the panel's
    // `pointer-events` is `auto` -- a full-window document following the pointer-events fix
    // (docs/rmlui-ui-system/gotchas-and-patterns.md) sets its panel to inherit `none` from `body`
    // by default, specifically so clicks pass through to whatever's underneath, which is exactly
    // why a dedicated handle is the correct pattern here, not a workaround for a limitation.
    //
    // `panel` is the element that actually moves -- must be `position: absolute` with `left`/
    // `top` already resolved to fixed `px` values (true of every window built so far, see
    // docs/rmlui-ui-system/theming-and-modding.md's "Coordinates, scaling, and positioning"
    // section -- a panel positioned in `%`/`vw`/`vh` isn't supported by this helper yet).
    //
    // Sets `drag: drag` and `pointer-events: auto` on `handle` itself (the latter confirmed
    // necessary by a real test -- a handle that inherited `pointer-events: none` never became
    // `hover` at all, so dragstart/drag never fired) so the caller doesn't need to remember
    // either as a matching RCSS rule. Registers a self-owning listener for RmlUi's native
    // `dragstart`/`drag` events (Source/Core/Context.cpp's UpdateHoverChain -- these already
    // carry the current mouse position as event parameters, no manual mousedown/mousemove/
    // mouseup tracking needed) that repositions `panel` by directly setting its `left`/`top`
    // properties as the drag proceeds, then calls `onMove` (if supplied) with the new position.
    void MakeDraggable(Rml::Element* handle, Rml::Element* panel, OnPanelMoved onMove = nullptr);
}
