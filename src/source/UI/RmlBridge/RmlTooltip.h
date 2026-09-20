#pragma once

#include "stdafx.h"

#include <string>
#include <vector>

// The one tooltip mechanism every window should use from now on -- a single, always-on-top RmlUi
// document shared by every caller, callable from native code (an inline hover check in a legacy
// CButton::Render()) exactly as easily as an RmlUi data-event-mouseover callback. This replaces
// five previously separate mechanisms (see docs/rmlui-ui-system/component-catalog.md's "Tooltip"
// entry): the skill-hotkey tooltip, the item Set/Socket option tooltip, CBuffStrip/CMuHelperBar's
// CSS-only hover tooltip, the generic native CTooltip/CButton mechanism, and the item tooltip
// (RenderItemInfo()/RenderTipTextList(), the one with the z-order bug this exists to fix).
//
// Lives in RmlUiRuntime's main context -- the same one every ordinary window and
// CGenericConfirmDialog use -- and its document root carries an explicit z-index (see
// tooltip.rcss), which is what actually fixes the z-order bug: a document with the default
// z-index:auto (every other document in this codebase) paints in plain DOM/show order among
// itself, so a native tooltip queued through the legacy 3D-camera effect system could always be
// painted over by RmlUi's own "renders once, last, every frame" main-context pass. An explicit
// z-index sidesteps that entirely -- see RmlTooltip.cpp's own comment for the full reasoning.
namespace UI::RmlBridge::Tooltip
{
    // Superset of every color capability across the five prior mechanisms -- the item tooltip's
    // own palette (TEXT_COLOR_*, Core/Globals/_define.h) was already the richest, so nothing loses
    // capability by unifying onto it. The four *Highlight values keep their original meaning: text
    // renders in the color RenderTipTextList() used for it (white, except GreenBlue's own green),
    // on top of a colored highlight bar behind the line.
    enum class LineColor
    {
        White,
        Blue,
        Gray,
        Red,
        Yellow,
        Green,
        Purple,
        RedPurple,
        Violet,
        Orange,
        DarkRedHighlight,
        DarkBlueHighlight,
        DarkYellowHighlight,
        GreenBlueHighlight,
    };

    struct Line
    {
        // A spacer line renders no text at all -- `text` is ignored for these two kinds. Replaces
        // RenderItemInfo()'s old convention of sniffing the first character of a native text
        // buffer ('\n' for a half-height spacer, a lone ' ' for a full-height one) with an
        // explicit field.
        enum class Kind
        {
            Text,
            HalfSpacer,
            FullSpacer,
        };

        std::string text;
        LineColor color = LineColor::White;
        bool bold = false;
        Kind kind = Kind::Text;
    };

    // Where the tooltip is anchored before edge-clamping: BelowLeft grows down-right from
    // (anchorX, anchorY) (the common case -- a slot/button's own top-left or bottom edge);
    // AboveLeft grows up-right instead, matching RenderTipTextList()'s STRP_BOTTOMCENTER callers
    // (mostly hotkey/menu rows near the bottom of the screen, where growing downward would run
    // off-screen far more often than not). Show() still fully clamps either way -- this only picks
    // which direction the tooltip grows from the anchor before that clamp applies.
    enum class AnchorPoint
    {
        BelowLeft,
        AboveLeft,
    };

    struct Config
    {
        std::vector<Line> lines;

        // Real screen-pixel anchor -- already converted by the CALLER using whichever transform
        // actually applies to its own coordinate's origin (usually
        // UI::Scaling::PositionX/Y(UI::Scaling::GetActiveTransform(), refX/refY), the same
        // conversion CharacterInfoWindow/PartyInfoWindow/PetInfoWindow/MyQuestInfoWindow already do
        // for their own root_x/root_y). Show() does NOT apply any transform of its own -- it used
        // to, via the ambient UI::Scaling::GetActiveTransform(), but that's wrong for a caller whose
        // anchor is meaningful only through a DIFFERENT transform than whatever's ambient in its own
        // scope (e.g. the skill-hotkey tooltip: its anchor is relative to MainFrameWindow's own
        // hand-rolled BottomHudCenterTransform, matching #bars's CSS scale/offset, but
        // MainFrameWindow's own ambient GetLayoutMode() resolves to a completely different formula --
        // see MainFrameWindow.cpp's own comment). Pushing the conversion out to each caller means
        // every caller picks the transform that's actually correct for its own anchor, instead of
        // this shared primitive guessing at "whatever's active right now." Not a live cursor
        // position and not `dp`.
        float anchorX = 0.0f;
        float anchorY = 0.0f;
        AnchorPoint anchor = AnchorPoint::BelowLeft;

        // True: anchorX is the tooltip's horizontal CENTER (matches RenderTipTextList()'s own
        // `sx - fWidth/2` centering -- the item tooltip's existing behavior, anchored under/over
        // the hovered slot's center). False: anchorX is the tooltip's left edge (matches the
        // skill-hotkey tooltip's existing simpler convention). Either way this is resolved using
        // the tooltip's real measured width (Show() doesn't know it up front, unlike the old native
        // path's synchronous GDI measurement), not a caller-side estimate.
        bool centerHorizontally = false;

        // Per-line TEXT alignment WITHIN the panel's own width -- independent of centerHorizontally
        // above (that only controls where the whole panel sits relative to anchorX). The old native
        // RenderTipTextList()'s default `iSort` was RT3_SORT_CENTER, and CTooltip::Render()'s own
        // RenderTextWithColors() call always passed RT3_SORT_CENTER too -- both item/pet and generic
        // button tooltips need Center to match; the skill-hotkey tooltip's own pre-existing RmlUi
        // CSS never set text-align (so it was already effectively Left), hence the differing default.
        enum class TextAlign { Left, Center };
        TextAlign textAlign = TextAlign::Left;
    };

    // Show()'s edge-clamping always wins over `anchor`/`centerHorizontally`'s preferred direction:
    // if growing the requested way would run off the top/bottom/left/right of the real viewport,
    // Show() flips/shifts it back on screen instead. A caller never needs its own clamping math --
    // this is exactly the gap the four prior mechanisms with partial (horizontal-only, or none)
    // clamping left open.

    // `owner` is an opaque identity token (typically `this`) distinguishing which caller currently
    // owns the shared tooltip -- needed because several independent callers (every visible
    // CButton's own CTooltip, every open inventory-family window's own hover tracking) each run
    // their own hover check every frame, and a naive ownerless Hide() from a NOT-hovered caller
    // could otherwise clobber a DIFFERENT caller's legitimate Show() from earlier the same frame,
    // before that frame ever renders. Show() always takes over regardless of the previous owner
    // (a new hover should always win); Hide() only actually hides if `owner` matches whoever most
    // recently called Show() -- a stale/irrelevant Hide() from a caller that never owned the
    // tooltip (or no longer does) is a safe no-op instead of hiding someone else's tooltip.
    // Default `nullptr` preserves the simpler "always wins" behavior for a caller that has no
    // realistic simultaneous competitor (e.g. a single inventory grid's own hover tracking).
    using Owner = const void*;

    // Builds (on first use) or updates the shared tooltip document with `config`, then shows it at
    // a position derived from anchorX/anchorY and clamped to stay fully within the real viewport on
    // all four sides (see .cpp for how the real, measured size is obtained before the clamp is
    // applied). Safe to call every frame while a hover persists (e.g. from a native Render() hover
    // check) -- rebuilding identical content is cheap and idempotent. No-op if RmlUiRuntime isn't
    // created yet or `config.lines` is empty.
    void Show(const Config& config, Owner owner = nullptr);

    // Hides the shared tooltip document -- but only if `owner` matches the current owner (or
    // either is nullptr). No-op if it was never built, already hidden, or owned by someone else.
    void Hide(Owner owner = nullptr);

    // Rebuilds the shared document against the now-active theme. Not a CObject/IObject, so
    // CManager::ReloadAllRmlThemes()'s sweep can't reach this -- call sites that switch the active
    // theme (the `$theme` chat command, the Options window's theme picker) must call this
    // explicitly, the same way they already call UI::Login::ReloadRmlTheme() for
    // RememberPasswordPrompt (also not a CObject).
    void ReloadRmlTheme();
}
