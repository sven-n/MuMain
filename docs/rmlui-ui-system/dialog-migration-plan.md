# `CommonMessageBox`/`CustomMessageBox` → `CGenericConfirmDialog` migration plan

Tracked, resumable checklist for porting the ~137 remaining native confirm-dialog classes onto the
`CGenericConfirmDialog` primitive (see `component-catalog.md`'s "Dialog" section for what the
primitive supports, and `STATUS.md`'s "What's migrated" for how/why it was built). This file owns
everything about this one migration now — both the per-class worklist below and (2026-09-16,
merged in from `STATUS.md`'s own former "Tracked deferral" entry) the `GenericDialogConfig`
feature-extension history right after this intro — rather than splitting the two across files that
each pointed at the other.

Primitive shape at the time the per-class checklist below was classified, as a reminder for what
"FITS" meant there: a fixed list of plain text lines (each optionally bold), and either an OK-only
or OK+Cancel button set, with callbacks on click. No keypad, no progress bar, no 3D item preview,
no 3+-button menus. **The primitive has since grown well past this baseline** — see the
feature-extension history immediately below for everything added (`title`/`severity`/`input`/
`progress`/`item3D`/`portrait2D`/`tallPanel`, and the `primary`/`secondary`/`cancel` button-role
redesign) — but the checklist's own FITS/DOESNT_FIT calls were made against this original baseline
shape and haven't been retroactively re-run against the grown primitive; a class marked
DOESNT_FIT for, say, needing a 3rd button may fit today and is worth a second look before assuming
the old verdict still holds.

Full inventory pass done 2026-09-13 (`CommonMessageBox.h`/`.cpp` read in full; `CustomMessageBox.h`/
`.cpp` characterized by sampling — see that section's own confidence note). Re-verify a class's
`SetLayout()`/`Create()` at port time rather than trusting this list blindly if its shape looks
even slightly off from the table — this is a starting point, not a guarantee.

## Feature-extension history: `GenericDialogConfig` growth and the full native inventory

A separate native subsystem from `UIControls.h`'s `CUIControl` family (tracked in
`tracked-deferrals.md`) — don't conflate the two. `UI/Dialogs/CommonMessageBox.h`/
`CustomMessageBox.h` together declare 150+ classes, 100% native, confirmed by a full-codebase
inventory (2026-09-13). The primitive to port them onto now exists (`CGenericConfirmDialog`, see
`STATUS.md`'s "What's migrated" for how/why it was built) and is proven on 3 of them. Summary by
category (counts approximate, from the inventory pass, not re-verified per class) — the per-class
checklist below ("Already ported" onward) is the live, resumable worklist itself:

- **`CommonMessageBox.h`** — ~75 classes, ~64 `TMsgBoxLayout<CCommonMessageBox>` (or
  `<C3DItemCommonMsgBox>`/`<CFenrirRepairMsgBox>`) feature-specific confirms spanning guild/quest/
  trade/duel/castle-siege/events/gambling. **This entire file is now fully handled** (2026-09-13):
  63 classes ported-and-deleted or confirmed-dead-and-deleted (3 proof-pass + 7 Guild + 10
  Trade/shop/inventory + 25 Network/server + 14 Siege/castle/CryWolf + 4 dead-code), plus 5 of the 6
  `C3DItemCommonMsgBox`-based classes now ported onto the primitive's `item3D` field
  (`CHighValueItemCheckMsgBoxLayout` — 2 call sites, `CUseFruitMsgBoxLayout`,
  `CUsePartChargeFruitMsgBoxLayout`, `CPersonalShopItemBuyMsgBoxLayout`, `CGambleBuyMsgBoxLayout`).
  `CPersonalShopItemValueCheckMsgBoxLayout` stays native — it needs a numeric price-value field its
  `CTextInputMsgBox` caller sets via `SetItemValue()`, which isn't designed yet. See
  `dialog-migration-plan.md` for the per-class worklist. `CustomMessageBox.h` (~76 more classes,
  separate file) is next.
- **`GenericDialogConfig` extensions** — **done** (2026-09-13): optional `title`, `severity`
  (Normal/Warning/Error, look-and-feel only), `input` (`InputField::Mode::Text`/`NumericKeypad`),
  `progress` (duration-only auto-close), and `item3D` (an `ITEM` snapshot). One struct, not five
  sibling classes.
  - **Correction, 2026-09-13**: the first version rendered `item3D` via a plain `I3DRenderObj`/
    `C3DRenderMng` registration (`Render3D()`), like every other legacy 3D icon. In-game testing
    (5 classes ported onto `item3D`, see `CommonMessageBox.h` entry below) showed the item never
    appeared — that path renders through `CManager::Render()`, which always finishes *before*
    RmlUi's main-context composite, so the dialog's own opaque `#panel` background painted over it
    every frame regardless of z-order.
  - **Reverted, 2026-09-14**: tried moving `item3D` off `I3DRenderObj`/`Render3D()` onto a manually
    invoked `RenderItem3DOnTop()` called from Winmain.cpp's `SetPostRmlUiCallback` instead (the
    established "legacy content after RmlUi" seam `CMsgWin`/`CCharMakeWin`/`CLoginWin` already use
    for native text overlays, just the first time asked to carry 3D content) — **twice**, and both
    attempts crashed on dialog dismiss. The first crash's cause was found and genuinely fixed at the
    renderer level (`MuRendererSDLGpu.cpp`'s post-RmlUi replay pass only re-staged vertex data,
    leaving a skinned draw's bone-matrix buffer unstaged/undersized while `ReplayDrawCommand`'s
    `boneDataReady` guard stayed stale-true — an out-of-bounds GPU read; fixed by reusing
    `StageDeferredGpuData()`, which re-stages vertex/bone/strip-index/texture data together and
    returns a correct `boneDataReady` — this fix is real and stays). That restored visibility and
    (combined with a separate `PanelTranslateCorrection()` fix, below) correct position, but the
    dismiss-time crash persisted regardless — a second, still-unidentified bug in the same seam.
    Enabling SDL_GPU validation (Debug config) to localize it hit a *different* pre-existing
    validation failure at startup, unrelated to this feature, blocking that route. Reverted back to
    `I3DRenderObj`/`Render3D()` (stable; `item3D` renders correctly-positioned but behind the panel)
    rather than ship a crash. `input`'s `Mode::Text` widget keeps its own `RenderTextOnTop()`
    post-RmlUi method (pure 2D, never implicated in the crash, matches the already-proven pattern).
    See `theming-and-modding.md`'s "RmlUi rendering strictly last in the frame" entry for the full
    writeup.
  - **Fixed for real, 2026-09-14**: the fg/bg RmlUi document split `CMainFrameWindow`/the
    inventory-family windows already prove for their own live 3D icons. `generic_confirm_dialog`'s
    own panel background art (the modern gradient/shell-edge/groove/content-well recipe, or
    legacy's `newui_msgbox_*` sprite composite) moved wholesale into a new
    `generic_confirm_dialog_bg.rml`/`.rcss` per theme. The original `generic_confirm_dialog.rml`/
    `.rcss` document is now paint-*less* where the background used to be — same `#panel` id/size/
    centering, just nothing left to cover the item once the background document has already
    painted it earlier the same frame. Needed **no changes at all** to `Render3D()`/
    `PanelTranslateCorrection()`/`I3DRenderObj` registration or the post-RmlUi seam — only where the
    panel's own background art paints from, and (see below) exactly when.
    - **Centering bug, 2026-09-14**: the first cut of `generic_confirm_dialog_bg.rcss` didn't link
      `base.rcss` (per convention, no `*_bg.rml` does), but `base.rcss` is where `body { width:100%;
      height:100%; }` comes from — without it, `#panel`'s `left:50%;top:50%` had a zero-sized
      containing block to resolve against, collapsing the whole background panel to the screen's
      upper-left corner. Fixed by adding that one `body` rule explicitly to both themes' `_bg.rcss`
      (not by linking `base.rcss`, which would pull in a lot more than needed).
    - **Cross-window bleed-through bug, 2026-09-14**: the first cut also loaded the new bg doc into
      the SHARED `RmlUiRuntime::GetBackgroundContext()` (the same one `CNPCShop`/every inventory-
      family window's own bg doc uses), driven by the existing `RenderBackgroundLayer()` hook (fired
      once, globally, before the very first visible window/camera each frame). That fixed the
      standalone case, but broke the moment this dialog opened over another bg-doc window with
      native foreground content: since `RenderBackgroundLayer()` renders ALL currently-visible bg
      docs together, once, strictly before EVERY window's own 2D `Render()` this frame — not just
      this dialog's — `CNPCShop`'s own inventory-slot icons (drawn later, in its own `Render()`)
      always painted over BOTH bg docs regardless of their relative order within that shared
      context, bleeding through the dialog's panel wherever they geometrically overlapped. This is
      exactly the case that matters most for `item3D` (sell-to-shop/gamble-buy confirms are almost
      always shown over an open shop window). Tried reasserting the dialog's own bg doc's stacking
      order every frame via `ElementDocument::PullToFront()` first — insufficient, since
      `PullToFront()` only reorders documents *within* the one shared context; it can't make that
      context's single global render pass happen *after* another window's own `Render()`.
      **Actually fixed** by giving the dialog's own bg doc a dedicated THIRD context
      (`RmlUiRuntime::GetDialogBackgroundContext()`), rendered by a separately-guarded
      `RmlUiRuntime::RenderDialogBackgroundLayer()` that `CManager::Render()` fires at a *different*
      point than `RenderBackgroundLayer()`: right before the first visible object whose
      `GetLayerDepth()` reaches the shared 3D camera's own z-order (`INFORMATION_CAMERA_Z_ORDER`,
      `Window3DRenderMng.h` — the same z-order `item3D` itself renders through). Since
      `CManager::Render()` sorts every registered object by `GetLayerDepth()` (`CNPCShop` = 2.5f,
      the shared 3D camera = 10.9f, this dialog's own 2D `Render()` = 60.0f) and calls each one's
      `Render()` in that order, "right before the object at/past 10.9f" is guaranteed to be
      strictly after every ordinary window's own `Render()` this frame and strictly before
      `item3D` draws. Deliberately triggered from `CManager::Render()`'s own loop, NOT from inside
      `Render3D()` itself — `C3DCamera::Render()` pushes a legacy GL matrix stack and enables depth
      test/mask before looping over every registered object's `Render3D()`, and recording an RmlUi
      render pass from inside that block is exactly the kind of mid-frame GPU-state interleaving
      that crashed the `SetPostRmlUiCallback` attempts above; `CManager::Render()`'s own loop,
      before any `(*vi)->Render()` call, is the same safe, pre-matrix-stack position
      `RenderBackgroundLayer()` itself already uses.
    - Simpler than both reference implementations either way: this dialog's `#panel` centers via
      plain CSS (`.center-both`), never a per-frame C++-computed position, so its background
      document needs no `RmlModelBinder` and no position-sync code at all (`CMainFrameWindow`/
      `CNPCShop`'s own bg docs both need a small one, since their content is anchored to an
      adjustable HUD-band/inventory-window position).
  - **`PanelTranslateCorrection()`, 2026-09-14**: separately, `item3D`/`input`'s anchor position was
    found to be wrong (rendering outside the panel) because `#panel`'s `.center-both` centering uses
    `transform: translate(-50%,-50%)`, and RmlUi's `GetAbsoluteOffset()` doesn't apply CSS
    `transform` at any level — every descendant of `#panel` reported its position as if the panel
    were still sitting at its untranslated `left:50%;top:50%` spot. Fixed by subtracting half of
    `#panel`'s own box size from the raw offset before converting to reference space; this fix is
    independent of the `Render3D()`/`RenderItem3DOnTop()` back-and-forth above and stays either way.
  - `title` and `item3D` gained their first real consumers via the `GameShop/MsgBoxIGS*.h` batch
    (2026-09-14, see that entry below): `title` is now used by all 7 ported GameShop classes (its
    first consumer of any kind), `item3D` grew from 5 (`CommonMessageBox.h`'s own
    `C3DItemCommonMsgBox` family) to 7 (+`CMsgBoxIGSStorageItemInfo`/`CMsgBoxIGSGiftStorageItemInfo`).
    `progress` still has zero consumers.
  - **Layout bug found in-game (2026-09-14, via `$igs1`/`$igs2` test commands), fixed**: item3D
    visually overlapped the body text, and always had (this predates the GameShop batch — same bug
    on the already-shipped `CHighValueItemCheckMsgBoxLayout`/sell-expensive-item-to-NPC-shop
    confirm). Root cause: native `C3DItemCommonMsgBox::Render3D()`/`RenderTexts()`
    (`CommonMessageBox.cpp`) place the 40x40 icon at a fixed top-left offset with body text
    starting to its *right* (`MSGBOX_TEXT_LEFT_BLANK_3DITEM`/`MSGBOX_TEXT_MAXWIDTH_3DITEM`,
    `CommonMessageBox.h`) — icon and text side-by-side — but the RmlUi port put `#gcd_item3d_anchor`
    in the same single centered flex *column* as `lines`, stacking them instead. Fixed by adding a
    `has_item3d`-bound model field, moving `title`/`lines`/`input-row`/`progress-track` into a new
    `.gcd-text-col` wrapper, and making `.gcd-body` a flex *row*: the anchor (hidden via
    `data-class-hidden="!has_item3d"`, collapsing to nothing when absent) sits to the left,
    `.gcd-text-col` fills the remaining width to its right — reproducing native's side-by-side
    layout exactly. Also fixed a smaller pre-existing issue this exposed: the anchor was never
    gated at all before, so every non-item3D dialog silently reserved an empty 40dp flex slot.
  - **Long-text clipping, also found in-game (2026-09-14), fixed**: very long body text got clipped
    with no visible way to read the rest — `.gcd-body`'s `overflow: auto hidden` was technically
    already there, but this theme system has zero pre-existing scrollbar-decorator CSS anywhere
    (RmlUi generates real `scrollbarvertical`/`slidertrack`/`sliderbar` elements for this, not
    pseudo-elements, and without styling they're effectively invisible). Added minimal flat-color
    scrollbar CSS to `.gcd-text-col` (the same wrapper from the item3D fix above, since overflow
    now lives there instead of on `.gcd-body`) in both themes — no decorator images, matching this
    dialog's own progress-bar/keypad styling. Considered dynamic panel resizing instead (more
    native-like — `CMsgBoxIGSCommon` itself grows `m_iMsgBoxHeight` with content) but rejected: it
    would require syncing height between this document and its own background document (currently
    100% static, no data-model, deliberately kept that way — see the fg/bg split's own comment)
    every time content changes, a bigger and riskier change to shared dialog infrastructure than a
    self-contained scrollbar.
  - **`title` upgraded from plain colored text to a real header banner (2026-09-14)**, matching
    Login/`CSysMenuWin`'s own header-rail/hero-banner look instead of just a bold colored line —
    requested after seeing the plain-text version in-game, then refined again after the user
    supplied a full reference HTML/CSS mockup of this theme's "blackened iron/carved frame/crimson
    title-strip/gold rivet" visual language and flagged two remaining gaps against it (see below).
    `.gcd-title` (a flex child of `.gcd-text-col`) removed entirely, replaced with
    `.gcd-header-rail`/`.gcd-header-title` — but the two themes now diverge structurally, not just
    in paint, because the reference exposed a real per-theme asset difference (below).

    **Modern**: kept the in-flow flex design from the first pass — `.gcd-body` became a flex
    *column* of `[.gcd-header-rail, .gcd-content-row]` (the latter holding
    `#gcd_item3d_anchor`+`.gcd-text-col`, i.e. everything `.gcd-body` used to flex-center directly);
    the banner collapses to zero height when `!has_title`, so `.gcd-content-row`'s own
    `flex:1 1 auto` claims the full box and untitled dialogs stay pixel-identical. `.gcd-header-rail`
    copies `base.rcss`'s `.modern-header-rail` paint recipe (already matched the reference's own
    `.window__header`/`.hero-banner` almost to the rgba value — both are clearly built from the same
    tokens) but as an in-flow ~20dp element instead of an absolutely-positioned 38dp one;
    `severity_error` recolors it toward `.modern-title-plate`'s crimson hero-banner palette,
    `severity_warning` is an amber recolor of the same rail. The reference comparison caught one
    real omission the first pass missed entirely: the rivet/stud hardware. This engine has no
    `::before`/`::after` support (confirmed: zero real usages anywhere in `themes/*/`), so every
    existing stud/rivet in this codebase (`.modern-joint`, `.login-hero-cap-left/right`) is a
    literal sibling `<div>` — added the same way here: `.gcd-header-joint` (copy of `.modern-joint`,
    13×13dp/-8dp scaled to 10×10dp/-6dp) + `.gcd-header-accent` (copy of `.modern-header-accent`)
    for the plain/warning look, `.gcd-header-cap-left/right` (copy of `.login-hero-cap`, 11×11dp
    scaled to 8×8dp) for the twin-stud crimson look, mutually exclusive via
    `data-class-hidden="severity_error"` / `"!severity_error"` — mirrors `.modern-header-rail`'s own
    documented "one rivet OR two side studs, never both" rule. Needed `position:relative` added to
    `.gcd-header-rail` so these new absolutely-positioned children anchor to it, not `#panel`.

    **Legacy**: the first pass built a flat-color CSS band here too, reasoning "no header-rail asset
    exists to reuse" — wrong: the user pointed out a purpose-made sprite exists, and it does —
    `newui_Message_03.tga` (`src/bin/Data/Interface/newui_Message_03.OZT`, 230×67, pixel-identical
    size to the plain top-cap `newui_msgbox_top.tga` already used by `.gcd-bg-sprite-top`), loaded
    natively as `CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR` (`MessageBox.cpp:461`) and used by 12
    `CustomMessageBox.cpp`/`CursedTempleEnter.cpp`/`CursedTempleResult.cpp` classes as a drop-in
    top-cap swap whenever the box has a title, with text drawn over it — exactly the pattern
    needed. Reused it directly: new `@spritesheet` in `generic_confirm_dialog.rcss` (legacy) sources
    it as `msgbox-titlebar-image` (`0px 0px 230px 67px`); `.gcd-header-rail` (legacy) is now
    `position:absolute; left:0;right:0;top:0;height:67dp;` with `decorator:image(...)`, a **direct
    `#panel` child sibling of `.gcd-body`, not nested inside it** — lives in the *foreground*
    document (not `generic_confirm_dialog_bg.rml`), since unlike item3D a title banner has no
    compositing-order reason to sit behind anything, and an ordinary opaque foreground overlay
    avoids adding a second data-model to the (deliberately static) background document.
    `.gcd-header-title` sits over it at `top:10dp`, matching native's own
    `(GetPos().x+10,GetPos().y+10)` title inset — well above `.gcd-body`'s own unrelated `top:35dp`
    text offset, so no geometry conflict, no change to `.gcd-body` needed. No severity color
    variants of the *art* (native has no colored-banner concept and this field has zero real
    severity consumers today) — severity stays a text-color-only distinction, matching what
    `.gcd-title.warning/.error` already did. This also meant reverting legacy's own
    `.gcd-content-row` wrapper from the first pass (`.gcd-body` goes back to being the
    item3D-anchor+`.gcd-text-col` row directly) — that indirection was only needed to share space
    with an in-flow *painted* rail; a fixed-size sprite overlay outside `.gcd-body` entirely doesn't
    need it. Modern's own `.gcd-content-row` structure is unaffected and still correct (matches the
    reference's own header+body DOM shape). Build clean, both RmlUi verification scripts pass after
    each pass. **Not yet in-game-tested** — same caution as `title` itself above.

    **Follow-up pass (2026-09-14)**, after the user confirmed legacy's own banner looked right and
    asked for a closer paddings/frames/shadows audit of modern against the reference: two Explore
    passes over every relevant file turned up two separate, unrelated issues.

    First, **the scrollbar from the fix above has never actually been able to render, in either
    theme** — `.gcd-text-col`'s `overflow: auto hidden;` has the shorthand's `overflow-x, overflow-y`
    argument order backwards (confirmed against `StyleSheetSpecification.cpp:337` and 7+ real usages
    elsewhere in the vendored RmlUi tree, all correctly `hidden auto`). This engine only ever
    instantiates a `scrollbarvertical` element when `overflow-y` is `Auto`/`Scroll`
    (`Layout/ContainerBox.cpp:126-176`), never `Hidden` — so with the axes backwards, no scrollbar
    was ever created regardless of text length; this was a real, always-reproducible bug, not a "not
    tested with long enough text yet" gap. Fixed by swapping to `overflow: hidden auto;` in both
    themes. Considered switching to dynamic dialog height instead (the user asked) but decided
    against it once the actual bug was found — no bg/fg document sync risk this way, consistent with
    why dynamic resizing was rejected for this dialog earlier.

    Fixing the overflow order alone wasn't enough, though — once it built, the user reported the
    scrollbar still didn't respond to clicks/drags at all. Root cause: `base.rcss`'s `body {
    pointer-events: none; }` is inherited (`StyleSheetSpecification.cpp:386`,
    `RegisterProperty(PropertyId::PointerEvents, "pointer-events", "auto", true, false)` — third arg
    is `inherited`), and every genuinely-interactive element in this theme opts back in explicitly
    (`.btn`, `.checkbox-row`, `#backdrop`, etc. all set their own `pointer-events: auto`) —
    `.gcd-text-col` never did, so it (and, since RmlUi's generated `scrollbarvertical`/`slidertrack`/
    `sliderbar` elements are real DOM children of it, the scrollbar itself) silently inherited
    `none` all the way down and could never receive hover/mousedown/drag events, independent of the
    overflow-order bug. Fixed by adding `pointer-events: auto;` directly to `.gcd-text-col` in both
    themes. Confirmed via a separate investigation that no C++/application-side input plumbing was
    ever needed here — `WidgetScroll`/`ElementScroll` (vendored, unmodified) handle scrollbar
    generation and drag capture entirely internally once mouse events reach the element at all, and
    this dialog's own document already proves that pipeline works (its OK/Cancel/keypad buttons use
    the exact same `GetContext()`/`ProcessSdlEvent` path) — the blocker was purely this one missing
    CSS property, not a missing hook.

    Second, the paddings/shadows audit (scoped to this dialog's own files only — shared `base.rcss`
    classes like `.btn` were checked and are already clean, zero changes) found real, confirmed
    numeric mismatches against the reference, all in `themes/modern/`:
    - `generic_confirm_dialog_bg.rcss`: three box-shadow rgba entries (`.gcd-bg-shell-edge` x2,
      `.gcd-bg-groove` x1, `#panel` x1) had colored (non-black) highlight-line alphas rendering
      ~3–4.5x more opaque than the reference intends (every pure-black entry in the same rules was
      already a correct `round(alpha×255)` conversion — this wasn't a wholesale re-derivation, just
      the colored entries); `#panel`'s own entry also had the wrong RGB triplet, not just alpha.
      Fixed all four to the correct converted values. Also added the reference's subtle diagonal
      noise texture (`repeating-linear-gradient`, present in the mockup's `.window`/`.window__body`
      but missing here entirely) to `#panel` and `.gcd-bg-content-fill`.
    - `generic_confirm_dialog.rcss`: added padding to `.gcd-body` (safe here specifically because
      it's sized via anchored left/right/top/bottom edges, not an explicit width/height — this
      engine has no box-sizing support, so padding on a `width:100%`/`height:100%` child would have
      overflowed it instead, which is why the padding went on `.gcd-body` and not
      `.gcd-content-row`/`.gcd-text-col` directly) — reference's confirm-dialog-specific
      `.window__body{padding:20px 18px}` breathing room had no counterpart before this. Also gave
      `.gcd-header-rail.error`'s title its own brighter gold (`#e1b94e`, matching the reference's
      distinct `.hero-banner__title` color instead of reusing the plain rail's `text-warm` token),
      and gave `.gcd-line` its own smaller `font-size`/`line-height` (previously just inherited
      `#panel`'s 12dp default) matching the reference's own body-text-subtler-than-title hierarchy.
      Left `.gcd-header-rail`'s own side margin/padding alone despite the reference having some
      (`margin:7px 8px 0`/`padding:0 11px`) — same box-sizing hazard, not worth the risk for a
      cosmetic-only gap. Left `.btn` sizing, the `.gcd-bg-groove` ring's existence, and the header
      title's second (glow) text-shadow layer alone too — respectively: already provably correct,
      a visual-identity call without a clear reference mandate, and an apparent single-shadow-only
      engine limitation. Build clean, both RmlUi verification scripts pass. **Not yet in-game-tested.**

    **In-game test results (2026-09-14)**: legacy's banner confirmed good, no changes needed there
    again. The scrollbar still didn't respond to clicks/drags even after the overflow-order fix —
    root cause was a second, unrelated bug: `base.rcss`'s `body { pointer-events: none; }` is
    inherited (`StyleSheetSpecification.cpp:386`), and every real interactive element in this theme
    opts back in explicitly (`.btn`, `.checkbox-row`, `#backdrop`, ...) — `.gcd-text-col` never did,
    so it and its generated scrollbar children silently inherited `none` and couldn't receive any
    mouse events, independent of the overflow fix. Fixed with `pointer-events: auto;` on
    `.gcd-text-col` in both themes. Confirmed separately that no C++/application-side input
    plumbing was ever needed — `WidgetScroll`/`ElementScroll` (vendored, unmodified) handle
    scrollbar generation and drag capture entirely internally once mouse events reach the element,
    and this dialog's own OK/Cancel/keypad buttons already prove that pipeline works for this exact
    document/context. Also recolored both themes' scrollbar thumbs on request: legacy from gold
    (`#ffd23c`) to a dark neutral gray (`#4a4a45`, hover/active `#6e6e66`); modern from the gold
    accent (`token(accent-gold)`/`token(text-warm)`) to the same steel-gray palette as the header
    rail (`token(metal-edge)`/`token(metal-highlight)`, track `token(metal-rail-dark)`) instead of
    gold, tying it visually to the header hardware rather than the progress-bar/accent language.

    **Follow-up frame/padding pass (2026-09-14)**, focused specifically on frame geometry (borders,
    corner radius, inset rings) and content-to-container padding, per the user's request. A third
    Explore pass found corner radius, the inner groove ring inset (4dp), and the outer shell edge
    inset (-3dp) all already exactly matched the reference — no changes there. The one real,
    confirmed gap: `.gcd-body`'s own comment already documented the target
    (`.confirm-dialog .window__body{padding:20px 18px}`) but the shipped padding was still the
    original conservative `4dp 6dp`. Fixed to `6dp 18dp` — horizontal matches the reference
    directly, but vertical deliberately stays far short of 20dp: this box's height is a fixed 96dp
    (not auto-sized to content like the reference's own window), and the item3D+title combination
    (7 ported classes use both together) needs header-rail (20dp+4dp margin) + item3D's own fixed
    40dp icon height just to fit — 20dp vertical padding would leave only 12dp for everything else,
    too tight for that combination; 6dp leaves 32dp, still workable. `.gcd-header-rail`'s own
    margin/padding (reference: `margin:7px 8px 0`/`padding:0 11px`) stays unset, same `width:100%`-
    plus-no-box-sizing hazard as before — it already gets an equivalent inset for free now via
    `.gcd-body`'s own larger padding. Build clean, both RmlUi verification scripts pass.

    **In-game test (2026-09-14) found this `6dp 18dp` padding wrong — corrected to `2dp 3dp`.**
    Screenshot showed the "Gift Confirmation" dialog's content floating with clearly too much
    margin from the panel edges, clipping/scrolling harder than before. Root cause: `.gcd-body`'s
    pre-existing `left/right:16dp` already plays the exact role of the reference's own
    `.window__body{margin:6px 8px 8px}` (outer inset from the panel edge to the content well — kept
    in sync with `.gcd-bg-content-fill`, per that file's own comment) — and at 16/230≈7% of this
    panel's width, it was *already* more generous than the reference's own 8/320≈2.5%. Copying the
    reference's `padding:20px 18px` on top of that double-counted the inset: combined ≈15% per
    side vs. the reference's own combined (margin+padding) ≈8% — and because this panel is a fixed
    160dp (not auto-height like the reference's own window, which just grows to absorb its
    padding), that excess came directly out of an already-scarce fixed content budget instead of
    being absorbed by a taller window. Corrected to a much smaller `2dp 3dp` — sized to close the
    gap to the reference's ~8% *combined* target given `.gcd-body`'s own inset already covers most
    of it, not to re-add the reference's own padding figure a second time. Build clean, both RmlUi
    verification scripts pass.

    **Second in-game test (2026-09-14) — 3 more corrections**, from a screenshot of the
    "Gift Confirmation" dialog: (1) `.gcd-body`'s padding was *still* too much even at `2dp 3dp` —
    tightened further to a flat `1dp`, relying almost entirely on the pre-existing `left/right:16dp`
    outer inset rather than adding a second, independent one. (2) The header rivet
    (`.gcd-header-joint`) "not accurately positioned" — root cause: it was a scaled-down guess
    (10×10dp/`top:-6dp`/`margin-left:-5dp`) instead of `.modern-joint`'s (`base.rcss`) own proven,
    already-shipped values; restored to the exact original 13×13dp/`top:-8dp`/`margin-left:-6.5dp`
    (same fix applied to `.gcd-header-accent` and the twin `.gcd-header-cap` studs — both restored
    to their own source classes' exact sizes instead of scaled-down guesses). (3) The header banner
    itself was too small next to the reference — `.gcd-header-rail`'s height grew from a scaled-down
    20dp to `.modern-header-rail`'s own full 38dp (`.gcd-header-title`'s `font-size` likewise grew
    from 11dp to `.modern-header-title`'s own 13dp), and — per the user's explicit go-ahead — rather
    than let that shrink the content area, `#panel` itself grew from 160dp to 178dp (+18dp, exactly
    matching the rail's own +18dp growth) in *both* `generic_confirm_dialog.rcss` and
    `_bg.rcss` (kept in sync, per that pair's own standing requirement) — `.gcd-body`'s `top`/
    `bottom` stayed the same 10dp/54dp, so the entire +18dp becomes new content-area height, not a
    wash. `.gcd-footer`/the button row's own `top` values shifted +18dp to match (both are
    `top`-anchored, not `bottom`-anchored, so they'd otherwise have stayed put while the panel's
    bottom edge moved away from them). Legacy theme is unaffected by any of this — its panel is
    sprite-asset-sized (230×160, tied to real `newui_msgbox_*`/`newui_Message_03` art) and was
    already confirmed correct; only modern's fully CSS-painted panel is free to resize. Confirmed
    `GenericConfirmDialog.cpp`'s own `PanelTranslateCorrection()` (used for the item3D/anchor
    position pipeline) reads `#panel`'s live computed box size at runtime rather than a hardcoded
    constant, so this resize needed no C++ changes. Build clean, both RmlUi verification scripts
    pass.

    **Third in-game test (2026-09-14) — one more correction**, from a screenshot of the
    "Buff Item Use Confirmation" dialog: a large empty gap appeared between the header and the
    first line of body text, with the text floating roughly centered in the leftover space rather
    than sitting right below the header. Root cause: `.gcd-body { justify-content: center; }`
    centers the `[.gcd-header-rail, .gcd-content-row]` group as a whole within the box — fine when
    there's no header (matches the reference's own `.confirm-dialog` single-message-centering
    intent) but wrong once a real 38dp header is competing for space, since `.gcd-content-row`'s
    own `flex:1 1 auto` doesn't reliably claim exactly 100% of the remaining space in every case
    this engine's flex implementation has been tested against — with the group's rendered size
    smaller than the box, centering left the large gap under the header. Fixed with a
    `data-class-has-title="has_title"` binding on `.gcd-body` (same pattern as every other
    conditional class in this dialog) and a new `.gcd-body.has-title { justify-content: flex-start;
    }` override — content now starts right after the header's own `margin-bottom` regardless of
    how much space `.gcd-content-row`'s flex-grow actually claims, robust either way. Untitled
    dialogs are unaffected (still centered, `has-title` never applies). Legacy is unaffected too —
    its header lives outside `.gcd-body` entirely as a `#panel`-level sprite overlay, never part of
    this flex group. Build clean, both RmlUi verification scripts pass.

    **Fourth in-game test (2026-09-14) — the flex-start fix wasn't it either.** User: "text
    contents [are] too far down. They still don't look centered." The `flex-start` change fixed the
    big gap but as a side effect gave up centering entirely (content now hugs the top of whatever
    space is left instead of centering within it) — not what was wanted; the actual ask was for
    text to center within the space *below* the header, not within the whole box including the
    header, and not hug the top either. Real fix: took `.gcd-header-rail` out of the flex flow
    entirely (`position:absolute; left:0;right:0;top:0;` in `.gcd-body`'s own box, no longer a flex
    sibling of `.gcd-content-row`) and reverted `.gcd-body.has-title` from `justify-content:
    flex-start` to `padding-top: 42dp` (38dp header height + 4dp gap) instead. With the header out
    of the flex flow, `.gcd-content-row` goes back to being `.gcd-body`'s *only* flex child, so its
    own `justify-content: center` (unconditional again, no `has-title` override needed for it)
    centers content within whatever space `padding-top` leaves below the header — not a
    `[header, content-row]` group centered as a unit (the original bug: put the group's midpoint
    below the true midpoint of the remaining space, since the header's height counted toward what
    was being centered), and not hugging the top either (the `flex-start` attempt's own overshoot).
    This also stops depending on `.gcd-content-row`'s `flex-grow` reliably claiming exactly 100% of
    remaining space at all — with only one flex child, centering it is a much simpler, safer flex
    scenario than balancing two. Untitled dialogs, whose `.gcd-body` never gets the `has-title`
    class, are unaffected; legacy is unaffected for the same reason as the previous entry. Build
    clean, both RmlUi verification scripts pass. **Not yet in-game-tested against this
    correction.**
  - `input.Mode::Text` has 9 and `input.Mode::NumericKeypad` has 3 (as of 2026-09-14 --
    see `dialog-migration-plan.md`'s "Text input"/"Numeric keypad" entries) -- every unconsumed field
    still defaults to unset, so pre-existing call sites are unaffected. `Mode::NumericKeypad`
    is in-game-tested and confirmed working (2026-09-15). See `dialog-migration-plan.md`'s
    own entry for what's deliberately out of scope (the older `g_iChatInputType == 0` input path;
    input-row/keypad/progress-bar layout geometry not yet visually verified against a real
    consumer). Porting `input.Mode::Text` also surfaced a real primitive gap, now closed:
    `CGenericConfirmDialog::KeepOpen()`, letting `onPrimary`/`onSecondary` veto a click's
    `Resolve()` (native's own `CALLBACK_CONTINUE` convention for invalid input) -- see that entry
    for the full writeup. In-game-tested and confirmed working (2026-09-14) after fixing one bug
    found only by that testing: the field was invisible at first (opaque-black default text color
    against this dialog's own dark panel, the same gotcha `CharMakeWin.cpp` already hit and
    documented) -- fixed with the same light-cream text color `LoginWin.cpp`/`CharMakeWin.cpp` use,
    plus a visible dark recessed background fill since this dialog's anchor has no native sprite
    frame of its own to give the field a visible affordance.
- **`CustomMessageBox.h`** — ~76 classes on the same pattern. **2 done** (2026-09-13):
  `CDialogMsgBoxLayout`/`CDialogMsgBox` (the one near-miss that already fit as-is) and
  `CreateOkMessageBox()` (a third, previously-untracked OK-only helper, ~90 call sites migrated via
  one function-body change). The rest: keypad/numeric-entry boxes and text-input boxes (both now
  unblocked by the extensions above, not yet ported), fruit/gem-integration confirms, the in-game
  system-menu box (distinct from the already-ported `CSysMenuWin`), event result screens (Blood
  Castle/Devil Square/Chaos Castle), duel challenge/result, progress-bar modals (also now unblocked),
  and ~46 `T*MsgBoxLayout<...>` wrappers.
- **`CUIPopup`** (`UI/Dialogs/UIPopup.h`, `g_pUIPopup`) — **done** (2026-09-13). Every real
  `POPUP_OK`/`POPUP_YESNO` call site (9 live across `Guild/UIGuildInfo.cpp`,
  `Guild/UIGuildMaster.cpp`, `Network/Server/WSclient.cpp`) ported to `CGenericConfirmDialog`; one
  dead `POPUP_YESNO` site (`CUIGuildMaster::ReceiveGuildRelationShip`, superseded by
  `CGuildInfoWindow`'s own earlier port) deleted outright. `CUIPopup` itself is **not** deleted —
  one live `POPUP_CUSTOM` site (`UIGuildInfo.cpp`'s "Appoint" picker) is a bespoke multi-option menu
  out of scope, same as the multi-option `CustomMessageBox.h` classes below. See
  `dialog-migration-plan.md` for the full per-call-site breakdown.
- **`GameShop/MsgBoxIGS*.h`** — **done** (2026-09-14). Re-inventoried all 11 files (1 base class used
  directly + 10 "subclasses", actually independent copy-paste siblings, not real inheritance):
  7 ported (`CMsgBoxIGSCommon` — factored into a shared `CreateOkMessageBoxWithTitle()` helper next
  to `CreateOkMessageBox()`, ~50 call sites, mostly `WSclient.cpp`'s cash-shop response handlers;
  `CMsgBoxIGSBuyConfirm`, `CMsgBoxIGSUseBuffConfirm`, `CMsgBoxIGSUseItemConfirm`,
  `CMsgBoxIGSSendGiftConfirm` — plain `title`+`lines`+`OkCancel`; `CMsgBoxIGSStorageItemInfo`/
  `CMsgBoxIGSGiftStorageItemInfo` — `title`+`item3D`, the first non-`C3DItemCommonMsgBox` consumers
  of `item3D`, built from a minimal `ITEM{.Type=wItemCode}` snapshot since these are virtual
  cash-shop items with no real level/excellent/ancient state), 1 confirmed dead and deleted
  (`CMsgBoxIGSDeleteItemConfirm` — zero call sites anywhere), 2 staying native
  (`CMsgBoxIGSBuyPackageItem`/`CMsgBoxIGSBuySelectItem` — genuine Buy/Present/Cancel 3-button shape
  plus a scrollable/selectable list box, same DOESNT_FIT category as the multi-option menus
  elsewhere) plus `CMsgBoxIGSSendGift` staying native for a different reason (needs two simultaneous
  text-entry fields — recipient name + separate multiline message — a real, documented
  `GenericDialogConfig` gap, not designed yet). The 3 still-native classes' own Buy/Present/error
  button handlers were updated to call the newly-ported free functions where the class they used to
  construct was deleted. Build clean (zero new warnings) and both RmlUi verification scripts pass.
  One incidental bug found and fixed while porting: `WSclient.cpp` relied on an accidental file-scope
  `using namespace mu::ui::window;` that leaked in via the now-deleted `MsgBoxIGSCommon.h`'s own
  (unwrapped) using-directive — replaced with an explicit `using namespace mu::ui::window;` in
  `WSclient.cpp` itself rather than re-relying on a transitive leak. **In-game-tested and confirmed
  working (2026-09-15)** — this was `title`'s first real exercise (see the extensions entry above)
  and `item3D`'s first non-`C3DItemCommonMsgBox` use.
- **Duel dialogs** (`CDuelMsgBoxLayout`/`CDuelResultMsgBoxLayout`) — **done** (2026-09-15). Both
  render a fixed native sprite (`newui_DuelWindow.tga`, 148x138) with a caption drawn on top of it,
  then a few lines of body text — a shape only these two classes use anywhere, distinct from
  `item3D`'s live-rendered 3D icon. Added `GenericDialogConfig::Portrait2D` for it (`Overlay`:
  caption on the sprite, matching native; `Beside`: icon-left/text-right like `item3D`, no consumer
  yet), named generically rather than duel-specific since the primitive itself isn't. Also added
  `GenericDialogConfig::tallPanel`: grows `#panel` (both fg/bg documents, both themes) via a "tall"
  CSS class instead of relying on `.gcd-text-col`'s scrollbar, for content that doesn't comfortably
  fit the default height (a `Portrait2D`, or `Mode::NumericKeypad`'s digit pad) where scrolling
  mid-interaction is bad UX — applied to both Duel dialogs and the 3 vault-PIN keypad dialogs. The
  bg document has no data model of its own, so its `#panel` picks up the "tall" class imperatively
  from `Show()`/`ShowNext()` instead of a binding. In-game-tested and confirmed working, both themes.
- **Misc**: `CHelpWindow`, `CWindowMenu`, `CChatCommandWindow` (`UI/Dialogs/`) are dialog-shaped but
  don't fit the confirm-box mold at all (help overlay, per-window popup menu, command picker) —
  out of `CGenericConfirmDialog`'s scope entirely, would need their own primitives if ported.

Not blocked on anything — each of the ~137 remaining classes is an independent, same-shape port
(config data, not new code) for the plain-text OK/OK-Cancel ones; the keypad/progress/3D-preview
variants need their own scoped extension to `GenericDialogConfig` first, proven the same way this
pass proved the plain-text shape before being applied broadly.

## Already ported (proof pass, done)

- [x] `CGuildOutPerson` (`Guild/GuildInfoWindow.cpp`) — OK-only.
- [x] `CQuestGiveUpMsgBoxLayout` (`UI/Quests/MyQuestInfoWindow.cpp`'s `RmlClickGiveUp()`) — OK/Cancel.
- [x] `CGuildRequestMsgBoxLayout` (`Network/Server/WSclient.cpp`'s `ReceiveGuild()`) — OK/Cancel.

## `CommonMessageBox.h` — FITS (plain OK/OkCancel + `AddMsg` text, ~62 classes)

Batch these first — same shape as the 3 already proven, pure config-data ports. Grouped roughly by
call-site area so a batch can be done (and built/verified) per group rather than all 62 at once.

Zero-call-site classes are flagged `DEAD?` — confirm still-dead at port time (grep again) and
prefer deleting outright over porting if so; don't spend primitive-config effort on unreachable code.

### Guild — done (2026-09-13)

- [x] `CGuildBreakMsgBoxLayout` — OkCancel — `Guild/GuildInfoWindow.cpp` (guild-disband confirm)
- [x] `CGuildPerson_Get_Out` — OkCancel — `Guild/GuildInfoWindow.cpp` (kick-member confirm)
- [x] `CGuildPerson_Cancel_Position_MsgBoxLayout` — OkCancel (no `AddMsg`) — `Guild/GuildInfoWindow.cpp` (cancel-rank confirm)
- [x] `CUnionGuild_Break_MsgBoxLayout` — OkCancel — `Guild/GuildInfoWindow.cpp` (alliance-remove confirm)
- [x] `CUnionGuild_Out_MsgBoxLayout` — OK — `Guild/GuildInfoWindow.cpp` (alliance-master-can't-withdraw notice)
- [x] `CGuildRelationShipMsgBoxLayout` — OkCancel (no `AddMsg`) — `Guild/GuildInfoWindow.cpp`'s
  `ReceiveGuildRelationShip()` (alliance/hostility request approve-decline). Preserved the original
  "already busy → auto-decline instead of showing" branch (was `!g_MessageBox->IsEmpty()`, now
  `g_pGenericConfirmDialog->IsVisible()`) rather than letting it fall into the queue — this one
  specific call site needed that carried over, not a change to the primitive itself.
- [x] `CCanNotUseWordMsgBoxLayout` — OK (no `AddCallbackFunc`, default close only) — `Guild/UIGuildMaster.cpp`
- [x] `CGuildFireMsgBoxLayout` — deleted outright as dead code (confirmed zero call sites;
  `UIGuildInfo.cpp`'s `DoFireAction()` already sets `DeleteGuildIndex` directly without going
  through this class) — not ported, per the plan's own dead-code-candidate guidance. Its
  now-unused `DeleteGuildIndex`/`s_nTargetFireMemberIndex` externs were also removed from
  `CommonMessageBox.cpp`.

All 8 declarations + implementations removed from `CommonMessageBox.h`/`.cpp`; grep-confirmed zero
remaining references. Build + both RmlUi verification scripts still to run for this batch (see
"How to use this file" step 5) before considering it fully done.

### Trade / shop / inventory actions — done (2026-09-13)

- [x] `CTradeMsgBoxLayout` — OkCancel — `UI/Inventory/Trade.cpp` (incoming trade request)
- [x] `CTradeAlertMsgBoxLayout` — OkCancel — `UI/Inventory/Trade.cpp` (scam-alert confirm before
  trading). Original colored each of its 4 lines individually (3x orange warning, 1x red) via
  `TMsgBoxLayout`'s per-`AddMsg` color param; `GenericDialogConfig::Line` only has bold/not-bold,
  no arbitrary color, so all 4 collapsed to bold as a deliberate simplification (documented inline
  at the call site too). If this loses too much visual urgency in practice, the real fix is a
  scoped color/severity extension to `GenericDialogConfig::Line`, not a one-off hack here.
- [x] `CPersonalshopCreateMsgBoxLayout` — OkCancel — 2 call sites, `Engine/Object/ZzzInventory.cpp`
  and `UI/Inventory/MyShopInventory.cpp` (open-personal-shop confirm)
- [x] `CFenrirRepairMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`. Its
  `CFenrirRepairMsgBox` helper subclass (existed only to stash 2 caller-side int indices for the
  callback) was deleted entirely too — the indices now live in the `onPrimary` lambda's capture
  instead.
- [x] `CLuckyItemMsgBoxLayout` — OkCancel — `UI/Inventory/LuckyItemWnd.cpp` (variable 2-5 line
  body built from a runtime lookup table, first line bold — ported as a loop over `cfg.lines`)
- [x] `CMixCheckMsgBoxLayout` — OkCancel — `UI/Inventory/MixInventory.cpp` (combine-items confirm)
- [x] `CUseReviveCharmMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`
- [x] `CUsePortalCharmMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`
- [x] `CReturnPortalCharmMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`
- [x] `CUseSantaInvitationMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`

All 10 declarations + implementations removed from `CommonMessageBox.h`/`.cpp` (11 including the
`CFenrirRepairMsgBox` helper); grep-confirmed zero remaining references. Build + both RmlUi
verification scripts passed for this batch.

### Network/server-triggered (`WSclient.cpp`) — done (2026-09-13)

- [x] `CPartyMsgBoxLayout` — OkCancel — `Network/Server/WSclient.cpp` (party invite)
- [x] `CGuildWarMsgBoxLayout` / `CBattleSoccerMsgBoxLayout` — OkCancel — merged into one call site
  (`ReceiveDeclareWar`, same two `SendGuildWarResponse` callbacks for both, only body text differs)
- [x] `CServerImmigrationErrorMsgBoxLayout` — OK — `Network/Server/WSclient.cpp`
- [x] `CGemIntegrationUnityResultMsgBoxLayout` / `CGemIntegrationDisjointResultMsgBoxLayout` — OK —
  `Network/Server/WSclient.cpp`
- [x] `CGemIntegrationUnityCheckMsgBoxLayout` (4 call sites) / `CGemIntegrationDisjointCheckMsgBoxLayout`
  (1 call site) — OkCancel — actually called from `UI/Dialogs/CustomMessageBox.cpp` (native
  `CGemIntegrationUnityMsgBox`/`CGemIntegrationDisjointMsgBox` menu button handlers, which stay
  native — only the check-dialog-they-open was ported); `CustomMessageBox.cpp` needed the
  `GenericConfirmDialog.h` include added
- [x] `CChaosCastleTimeCheckMsgBoxLayout` — OkCancel (2 call sites, 1-2 dynamic lines) —
  `Network/Server/WSclient.cpp`
- [x] `CHarvestEventLayout` / `CWhiteAngelEventLayout` — OkCancel — `Network/Server/WSclient.cpp`
- [x] `CDuelCreateErrorMsgBoxLayout` (2 call sites) / `CDuelWatchErrorMsgBoxLayout` — OK —
  `Network/Server/WSclient.cpp`
- [x] `CDoppelGangerMsgBoxLayout` (4 call sites, each with its own body lines incl. blank-line
  spacers) — OK — `Network/Server/WSclient.cpp`
- [x] `CGatemanFailMsgBoxLayout` (2 call sites) — OK — `Network/Server/WSclient.cpp`
- [x] `COsbourneMsgBoxLayout` — OK — `Network/Server/WSclient.cpp`. Original colored its 2 lines
  differently (red bold, gold bold); collapsed to bold=true for both, same simplification as
  `CTradeAlertMsgBoxLayout` above.
- [x] `CCry_Wolf_Dont_Set_Temple1` / `CCry_Wolf_Wat_Set_Temple1` — OK — `Network/Server/WSclient.cpp`
- [x] `CSantaTownLeaveMsgBoxLayout` — OkCancel — `CSantaTownSantaMsgBoxLayout` (2 call sites) —
  OkCancel (no `AddMsg` in `SetLayout`, body supplied per call site) — `Network/Server/WSclient.cpp`
- [x] `CUseRegistLuckyCoinMsgBoxLayout` / `CRegistOverLuckyCoinMsgBoxLayout` /
  `CExchangeLuckyCoinMsgBoxLayout` / `CExchangeLuckyCoinInvenErrMsgBoxLayout` — OK —
  `Network/Server/WSclient.cpp`
- [x] `CEmpireGuardianMsgBoxLayout` (8 call sites across 2 functions, all OK-only with per-site
  dynamic text) — `Network/Server/WSclient.cpp`

`CQuestCountLimitMsgBoxLayout` intentionally **not** touched — still compiled out
(`#ifdef ASG_ADD_TIME_LIMIT_QUEST`, never defined); leave as dead code unless that macro is ever
enabled. All 25 real classes' declarations + implementations removed from `CommonMessageBox.h`/
`.cpp`; grep-confirmed zero remaining references. Build + both RmlUi verification scripts passed
for this batch.

(Gem integration check dialogs `CGemIntegrationUnityCheckMsgBoxLayout`/
`CGemIntegrationDisjointCheckMsgBoxLayout` — done, see the Network group above; the menu variants
they're opened from live in `CustomMessageBox` and stay native.)

### Siege / castle / CryWolf / combat events — done (2026-09-13)

- [x] `CMapEnterWerwolfMsgBoxLayout` / `CMapEnterGateKeeperMsgBoxLayout` — OkCancel (only OK wired;
  Cancel defaults to close) — `Engine/Object/ZzzInterface.cpp`. **Known regression**: both
  originally called `pMsgBox->LockOkButton()` to visually disable OK while a quest-state gate
  wasn't met — `GenericDialogConfig` has no disabled-button concept yet, so `onPrimary` now
  re-checks the same quest state and silently no-ops instead (functionally safe, cosmetically a
  downgrade: the button no longer looks disabled). Revisit if a locked/disabled-button field gets
  added to the primitive.
- [x] `CCastleMsgBoxLayout` (10 call sites across `UpdateGateManagingTab`/`UpdateStatueManagingTab`/
  `UpdateTaxManagingTab`) — OkCancel — `UI/Combat/CastleWindow.cpp`. Added one file-local
  `ExecuteCastleMsgBoxRequest()` helper (mirrors the original single shared `OkBtnDown`'s switch
  over `GetCurrMsgBoxRequest()`) reused by all 10 `onPrimary` lambdas, instead of duplicating the
  10-case switch at each site.
- [x] `CSiegeLevelMsgBoxLayout` / `CSiegeGiveUpMsgBoxLayout` — `UI/Combat/GuardWindow.cpp`
- [x] `CGatemanMoneyMsgBoxLayout` — `UI/NPCs/GatemanWindow.cpp`
- [x] `CMaster_Level_Interface` — OkCancel — `UI/HUD/MasterLevel.cpp`. Preserved the mouse-button
  state reset (`MouseLButton`/`MouseLButtonPop`/`MouseLButtonPush = false`) in both `onPrimary` and
  `onSecondary` — original did this on both OK and Cancel, presumably to stop the triggering click
  from falling through to the skill tree underneath.
- [x] `CCry_Wolf_Get_Temple` / `CCry_Wolf_Set_Temple` / `CCry_Wolf_Set_Temple1` /
  `CCry_Wolf_Dont_Set_Temple` / `CCry_Wolf_Destroy_Set_Temple` / `CCry_Wolf_Ing_Set_Temple` /
  `CCry_Wolf_Result_Set_Temple` — `Engine/Object/ZzzInterface.cpp`, `Network/Server/WSclient.cpp`,
  `UI/Events/CryWolf.cpp`. `CCry_Wolf_Set_Temple1`/`CCry_Wolf_Dont_Set_Temple` had no external call
  site (only chained from `CCry_Wolf_Get_Temple`'s own Cancel/OK) — ported as `Show()` calls made
  directly inside `CCry_Wolf_Get_Temple`'s `onSecondary`/`onPrimary` lambdas rather than as
  separate classes/call sites. Preserved one original quirk verbatim: `CCry_Wolf_Set_Temple`'s OK
  button was wired to `CCry_Wolf_Get_Temple::OkBtnDown` (not its own handler) in the native code —
  the ported call site duplicates that same handler logic with a comment explaining why.
  `CCry_Wolf_Result_Set_Temple`'s columnar scoreboard (header + up to 5 rank rows + blank spacer
  lines + summary) ported as plain text lines, colors collapsed (see `CTradeAlertMsgBoxLayout`
  note above for the same simplification) — readability not yet verified in-game, flagged for the
  general in-game verification pass. Porting this also exposed and fixed a pre-existing, unrelated
  type bug in `UI/Events/CryWolf.cpp`: it declared `extern BYTE HeroClass[5]` (should have been
  `CLASS_TYPE`, matching the real definition in `GMCrywolf1st.cpp` and `CommonMessageBox.cpp`'s own
  now-deleted `extern` decl) — harmless before since the file never actually used `HeroClass`, but
  a link error the moment it did. Fixed in place.

All 14 declarations + implementations removed from `CommonMessageBox.h`/`.cpp`; grep-confirmed zero
remaining references (one explanatory code comment mentions a deleted class name in prose, not a
real reference). Build + both RmlUi verification scripts passed for this batch.

### Dead-code candidates — confirmed dead and deleted (2026-09-13)

- [x] `CServerLostMsgBoxLayout` — its `OkBtnDown` did `SendMessage(g_hWnd, WM_DESTROY, 0, 0)`
  (quit-on-disconnect) — real-looking logic, but re-confirmed zero call sites anywhere; the actual
  disconnect flow must go through a different, already-existing mechanism. Deleted.
- [x] `CInfinityArrowCancelMsgBoxLayout` — re-confirmed zero call sites (its own header/cpp were
  the only references) despite non-trivial `g_iCancelSkillTarget` logic. Deleted.
- [x] `CBuffSwellOfMPCancelMsgBoxLayOut` — re-confirmed zero call sites. Deleted.
- [x] `CUnitedMarketPlaceMsgBoxLayout` — re-confirmed zero call sites. Deleted.

(`CGuildFireMsgBoxLayout` was the same story, already handled in the Guild batch above.)

All 4 declarations + implementations removed from `CommonMessageBox.h`/`.cpp`. This closes out
every `CommonMessageBox.h` class this plan originally classified as FITS — nothing left to port
or delete in that file. `CustomMessageBox.h`'s ~76 classes (see below) remain; `CUIPopup` and
`GameShop/MsgBoxIGS*.h` (the other tracked subsystems) are both done now too.

## `GenericDialogConfig` extensions — done (2026-09-13); `item3D` (5) and `input.Mode::Text` (9) now have consumers, `title`/`severity`/`input.Mode::NumericKeypad`/`progress` still don't

`GenericDialogConfig`/`CGenericConfirmDialog` (`UI/Dialogs/GenericConfirmDialog.h/.cpp`) grew
optional `title`, `severity` (Normal/Warning/Error, look-and-feel only), `input`
(`InputField::Mode::Text`/`NumericKeypad`), `progress` (duration-only, matching every real native
progress-bar call site being fire-and-forget), and `item3D` (an `ITEM` snapshot) — one struct, not
five sibling classes, per the design session that preceded this. `CGenericConfirmDialog` itself now
also implements `I3DRenderObj` (registers with `g_pNewUI3DRenderMng` in `Create()`) so `item3D` can
actually render via a `Render3D()` that reads an invisible `#gcd_item3d_anchor`'s live screen
position and converts it back to reference space (`UI::Scaling::LogicalX/Y`) before calling
`RenderItem3D()` — the same overall native-3D-inside-RmlUi split `CItemHotKey` proves, without that
class's extra delta-correction math (not needed for a brand-new anchor with no legacy hardcoded
coordinates to reconcile). `Mode::Text` hosts the shared `g_pSingleTextInputBox` portable widget,
positioned from a second anchor (`#gcd_input_anchor`) the same way CharMakeWin.cpp's own name/
password fields are — only the `g_iChatInputType == 1` path (the actual runtime default) is wired;
the older raw-global-buffer `== 0` path is a deliberate, documented gap. `Mode::NumericKeypad` is
pure RmlUi (10 shuffled-digit buttons + delete, reproducing `CKeyPadMsgBox`'s own anti-shoulder-
surfing shuffle). Layout geometry (input row size, keypad grid, progress bar) is a first-pass
placement inside the existing fixed 230x160dp panel, not yet visually verified in-game against a
real consumer — expect to need tuning once the first class using each field is actually ported.

**`item3D` is now proven end-to-end (2026-09-14)**, both the primitive and its rendering: the 5
`C3DItemCommonMsgBox`-derived classes below all consume it, in-game-tested (both themes) after
fixing 4 real bugs surfaced only by that testing — see `engine-findings.md` for the full
writeup, summarized: a CSS cascade-tie hiding bug, an anchor `position:absolute` flex-flow bug, a
`GetAbsoluteOffset()`-ignores-`transform` positioning bug (`PanelTranslateCorrection()`), and —the
big one— item3D rendering invisibly *behind* the dialog's own opaque panel background, since
RmlUi's main context always composites after the legacy 3D pass that draws it. That last one took
three real attempts to close out: a post-RmlUi callback approach crashed twice and was abandoned;
the fg/bg RmlUi document split that replaced it then surfaced a *second*-order bug of its own once
tested with another window open behind the dialog (NPC Shop) — the panel bled through beneath the
shop's own foreground content, because the shared background context every other window's bg doc
uses renders once, globally, before any window's own 2D content, not just this dialog's. Fixed for
real with a third, dedicated `Rml::Context` for this dialog's own panel, rendered at a precise point
in `CManager::Render()`'s own z-sorted loop (right before the shared 3D camera's turn) rather than
the generic background-layer hook. `title`/`severity`/`progress` remain **infrastructure only** —
no consuming class ported onto either yet, every field still defaults to unset/empty, so every
already-ported call site is unaffected by that.

**`input.Mode::Text` is now also proven end-to-end (2026-09-14)**: all 9 `CTextInputMsgBox`-derived
classes (see "Text input" below) consume it. This surfaced one real primitive gap:
`CGenericConfirmDialog::KeepOpen()`, letting `onPrimary`/`onSecondary` veto a click's `Resolve()`
for input validation failures, matching every native `OkBtnDown`'s own `CALLBACK_CONTINUE`
convention — see that section's own writeup. `Mode::NumericKeypad` still has zero consumers (the
"Numeric keypad" batch below is separate, unstarted work).

## `CommonMessageBox.h` — 3D-item-preview classes (6 classes)

All `TMsgBoxLayout<C3DItemCommonMsgBox>` — call `Set3DItem(pItem)`. The primitive's own `item3D`
support unblocked these — done 2026-09-13:

- [x] `CHighValueItemCheckMsgBoxLayout` — OkCancel, two live call sites (`UI/Inventory/
  InventoryActionController.cpp`'s double-click sell path, and `UI/NPCs/NPCShop.cpp`'s
  drag-into-shop `InventoryProcess()` path — both found and ported; the plan's original per-class
  grep had only caught the first one).
- [x] `CUseFruitMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`. `onPrimary`
  still creates the native `CUseFruitCheckMsgBoxLayout` chain dialog (a quantity stepper, different
  class, out of scope here) exactly as the old `OkBtnDown` did.
- [x] `CUsePartChargeFruitMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`.
- [x] `CPersonalShopItemBuyMsgBoxLayout` — OkCancel — `UI/Inventory/PurchaseShopInventory.cpp`.
- [x] `CGambleBuyMsgBoxLayout` — OkCancel — `UI/NPCs/NPCShop.cpp`.

All 5 declarations + implementations removed from `CommonMessageBox.h`/`.cpp`; grep-confirmed zero
remaining references. Per-line custom `RGBA()` colors from the original `AddMsg()` calls are not
preserved (`GenericDialogConfig::Line` only has `bold`, matching every earlier FITS batch) — only
the bold/non-bold distinction carries over, styled via the theme's existing warm/secondary text
tokens. Build (162/162, zero new warnings) + both RmlUi verification scripts passed.

- [x] `CPersonalShopItemValueCheckMsgBoxLayout` — OkCancel + `item3D` —
  `UI/Inventory/MyShopInventory.cpp`'s `ShowPersonalShopItemValueDialog()` (chained from its own
  `onPrimary`, once the entered price fails the "at least item value" check). This note previously
  said it needed a numeric price-value field on `GenericDialogConfig` because its native `OkBtnDown`
  reads `pMsgBox->GetItemValue()` — but that value is only ever set once via `SetItemValue()` and
  read back once, never rendered as a widget inside this dialog itself (the actual numeric entry
  happens one dialog earlier, in the already-ported `CPersonalShopItemValueMsgBoxLayout` chain, an
  `input`-mode confirm). So it needed no new primitive capability at all — just the entered price
  (`iInputZen`) captured by value into `onPrimary`, same shape as `NPCShop.cpp`'s `IsHighValueItem()`
  sell-to-NPC confirm. Native class + its `CommonMessageBox.h`/`.cpp` declaration/implementation
  removed; grep-confirmed zero remaining references.

## `CustomMessageBox.h` (~76 classes: 46 layout wrappers + ~30 underlying box classes)

Characterized by sampling (medium-high confidence — see caveats below), not a full per-class read
like `CommonMessageBox.h` got. **Estimated FITS: 0** (one near-miss, see below) — this entire family
needs its own extensions before any meaningful batch can be ported. Re-survey with a full read
before starting real work here; don't trust the exact class list below as final.

- [x] `CDialogMsgBoxLayout`/`CDialogMsgBox` — done (2026-09-13). One live call site
  (`Network/Server/WSclient.cpp`'s `ReceiveServerCommand`, `Cmd1 == 5` — an NPC/server dialog-line
  popup, "shell + caller fills in body via `AddMsg` after construction" pattern, same as
  `CGuildRequestMsgBoxLayout` from the original proof batch). Ported straight to `ButtonSet::Ok`
  with `primaryLabel = I18N::Game::ConversationIsOver` (the real native button caption — not
  literally "End" as this row's earlier note guessed) and one dynamic line
  (`I18N::Dialog::Lookup(Data->Cmd2)`). No primitive change needed; `primaryLabel` already covered
  a non-"OK" label. Declarations + implementations removed from `CustomMessageBox.h`/`.cpp`;
  grep-confirmed zero remaining references. Build + both RmlUi verification scripts passed.
  - **`mu::ui::window::CreateOkMessageBox()`** (`UI/Core/WindowCommon.h/.cpp`) — done (2026-09-13).
    A *third*, previously-untracked pattern: a standalone helper that directly `new`s a base
    `CCommonMessageBox` (not a `TMsgBoxLayout<>` subclass) for a one-off OK-only message. Unlike
    every other row in this file, this needed **zero call-site edits** — it's a single centralized
    function called from ~90 sites across ~19 files (`WSclient.cpp` alone accounts for most of
    them), and every call site just calls `CreateOkMessageBox(text)` with the same signature, so
    redirecting the one function body to `g_pGenericConfirmDialog->Show(...)` migrates all of them
    at once. `dwColor` (only ever overridden by 2 identical call sites using a warning-red
    highlight — `InventoryActionController.cpp`/`MainFrameWindow.cpp`, same message both times)
    collapses to bold when non-default, same simplification as `CTradeAlertMsgBoxLayout`/
    `COsbourneMsgBoxLayout`. `fPriority` (never overridden by any caller) is now unused — kept in
    the signature so no caller needed touching. Confirmed via grep that no caller anywhere uses
    the `bool` return value, so it now unconditionally returns `true`. Build + both RmlUi
    verification scripts passed.
- **Text input** (`CTextInputMsgBox`-based) — **done (2026-09-14)**: all 9 classes ported onto
  `GenericDialogConfig::InputField::Mode::Text` (`CTradeZenMsgBoxLayout` — `Trade.cpp`;
  `CZenReceiptMsgBoxLayout`/`CZenPaymentMsgBoxLayout`/`CStorageUnlockMsgBoxLayout` —
  `StorageInventory.cpp`; `CPersonalShopItemValueMsgBoxLayout` — factored into a shared
  `ShowPersonalShopItemValueDialog()` free function (`MyShopInventory.h`/`.cpp`), since 3 of its 4
  call sites already live there (the 4th, `ZzzInventory.cpp`'s `OpenPersonalShopMsgWnd(2)`, is
  unreachable in practice — its only caller always passes 1 — but was still updated to keep the
  function compiling); `CPersonalShopNameMsgBoxLayout` — `ZzzInventory.cpp`;
  `CCastleWithdrawMsgBoxLayout` — `CastleWindow.cpp` (reads its own typed amount directly, not via
  `ExecuteCastleMsgBoxRequest()`'s generic switch, since none of that switch's 10 existing cases
  need input); `CStorageLockMsgBoxLayout` — chained from the still-native
  `CStorageLockCheckKeyPadMsgBoxLayout::OkBtnDown` (`CustomMessageBox.cpp`, unchanged — a PIN-entry
  keypad dialog, separate "Numeric keypad" batch below), which now captures the just-entered 4-digit
  PIN directly in the closure instead of round-tripping it through
  `CTextInputMsgBox::SetPassword()`/`GetPassword()`; `CGuildBreakPasswordMsgBoxLayout` — factored
  into a shared `ShowGuildBreakPasswordDialog()` file-local helper (`GuildInfoWindow.cpp`) for its 3
  call sites. `CTextInputMsgBox` itself (the now-unused native base class) and its own
  `INPUTBOX_TYPE_NUMBER`/`_TEXT`/`INPUTBOX_WIDTH`/`_HEIGHT`/`_TEXTLIMIT` constants were deleted too
  (grep-confirmed dead once all 9 subclasses were gone). Build + both RmlUi verification scripts
  passed.
  - **Primitive extension needed, `CGenericConfirmDialog::KeepOpen()`**: every native
    `CTextInputMsgBox`-derived `OkBtnDown`/`ReturnDown` validates its typed input (empty field, a
    zero/unparsed amount) and returns `CALLBACK_CONTINUE` — leaving its own MsgBox open for the
    user to retry — rather than closing unconditionally like every plain-text confirm dialog does.
    `GenericConfirmDialog` had no equivalent, so `onPrimary`/`onSecondary` can now call
    `g_pGenericConfirmDialog->KeepOpen()` to veto that click's `Resolve()`: nothing gets
    hidden/reset/advanced, `m_Active` is restored exactly as it was, as if the click never
    happened. Used by 7 of the 9 classes in this batch (checked per-class against its own native
    `ProcessOk` — not assumed uniform: `CGuildBreakPasswordMsgBoxLayout` specifically does NOT veto
    on empty input, it always closes and just logs an error message, so it deliberately never calls
    `KeepOpen()`). One known, accepted simplification: native only plays `SOUND_CLICK01` on the
    success path, never on a `CALLBACK_CONTINUE` retry, while `CGenericConfirmDialog::Update()`
    plays it unconditionally before `Resolve()` even knows whether `onPrimary` will veto —
    cosmetic-only (an extra click sound on a failed retry), not worth plumbing sound timing through
    the veto path for.
  - Per-class WEBZEN.COM password fields (`CStorageLockMsgBoxLayout`/`CStorageUnlockMsgBoxLayout`/
    `CGuildBreakPasswordMsgBoxLayout`) are `masked = true` but deliberately NOT `numericOnly` —
    confirmed against native: despite `Create()`'s `dwInputType` parameter being
    `INPUTBOX_TYPE_NUMBER` for all three, none of them ever called
    `SetInputBoxOption(UIOPTION_NUMBERONLY | ...)` the way every zen-amount dialog does, so real
    (alphanumeric) WEBZEN passwords were never actually digit-restricted natively either.
  - Every dialog in this batch reuses `GenericConfirmDialog.cpp`'s existing fixed real-pixel input
    widget size (`kInputFieldWidth`/`kInputFieldHeight` = 150x18) regardless of native's own
    per-class width (native varies 50-130 reference px, plus a `g_iLengthAuthorityCode`-scaled width
    for the 3 password fields) — same "one struct, not per-dialog geometry" simplification as
    `progress`/`title`. In-game-tested (2026-09-14) and confirmed working, position included.
  - **In-game-testing bug, 2026-09-14**: the field was completely invisible on first test (typed
    text unreadable, no visible box at all) — `UpdateTextInputWidget()`'s `InputBoxConfig` never
    overrode the text color, which defaults to opaque black, invisible against this dialog's own
    dark panel fill. The exact same gotcha `CharMakeWin.cpp`'s own `#input_text_anchor` field
    already hit and documented in its own code, just not one this new call site had inherited.
    Fixed by setting the same light-cream text color `LoginWin.cpp`/`CharMakeWin.cpp` already use
    (255,230,210), plus a visible dark recessed background fill (`.gcd-progress-track`'s own
    `#100c06`, reused via the widget's own `backR/G/B/Alpha` rather than CSS) since -- unlike
    those two windows, whose native sprite chrome frames the input row -- this dialog's anchor has
    no visual frame of its own, so the field needs to paint its own affordance to read as a
    clickable box at all.
- **Numeric keypad** (`CKeyPadMsgBox`-based) — **done (2026-09-14)**: re-inventoried all 5 classes
  (grep, not trusted from this row's own old list) and found only 3 with live call sites, all in
  `UI/Inventory/StorageInventory.cpp`. `CPasswordKeyPadMsgBoxLayout` (vault PIN verify, 2 call
  sites — a locked-vault item-move guard and `CZenPaymentMsgBoxLayout`'s own onPrimary from last
  batch) → `ShowVaultPinVerifyDialog()`. `CStorageLockKeyPadMsgBoxLayout` (vault-lock flow, step 1:
  choose new PIN, 1 call site) → `ShowStorageLockPinDialog()`. `CStorageLockCheckKeyPadMsgBoxLayout`
  (step 2: PIN re-entry confirm, 0 external call sites — only ever chained from step 1) →
  `ShowStorageLockPinConfirmDialog(firstPin)`, called from `ShowStorageLockPinDialog()`'s own
  `onPrimary`; on match, this now directly builds the already-ported WEBZEN-password `Mode::Text`
  `Show()` config inline (moved verbatim from where it used to live inside the native
  `CStorageLockCheckKeyPadMsgBoxLayout::OkBtnDown`) rather than a third separate call site. All 3
  file-local helpers reuse `KeepOpen()` for incomplete input and an inline adjacent-char check for
  `IsAllSameNumber()` (native: 4-in-a-row rejection, `CreateOkMessageBox(...)`, closes normally —
  not a `KeepOpen()` case).
  - **`CStorageLockFinalKeyPadMsgBoxLayout`/`CStorageUnlockKeyPadMsgBoxLayout` — confirmed dead,
    deleted, not ported**: zero call sites anywhere (grep-confirmed) — both superseded by the
    `Mode::Text`-based WEBZEN password dialogs already ported last batch
    (`CStorageLockMsgBoxLayout`/`CStorageUnlockMsgBoxLayout`), left behind as unreachable leftovers.
  - `CKeyPadMsgBox` itself (the native base class, plus its own `CKeyPadButton`/
    `CDeleteKeyPadButton` helper classes and the `KEYPAD_TYPE_*` enum) deleted too once all 5
    subclasses were gone — grep-confirmed no other consumer, same full-cleanup discipline
    `CTextInputMsgBox` got last batch. `MAX_KEYPADINPUT`/`MAX_PASSWORD_SIZE` (shared `_define.h`
    constants `CKeyPadMsgBox` also used) were left alone — still used by unrelated code
    (`g_lpszKeyPadInput` in `ZzzInventory.cpp`).
  - Build (zero new warnings) + both RmlUi verification scripts passed. **In-game-tested and
    confirmed working (2026-09-15)**, both live flows (locked-vault item/zen withdrawal PIN verify,
    and the full vault-lock chain: PIN choose → confirm match/mismatch/all-same-digit → WEBZEN
    password).
- **3D item preview**: `CUseFruitCheckMsgBoxLayout` — `item3D` now exists, see above.
- **Gem-selection menu** (bespoke, embedded live list widget): `CGemIntegrationDisjointMsgBoxLayout`
  -- see the `CGenericMenuDialog` entry below for why its two siblings (`CGemIntegrationMsgBoxLayout`/
  `CGemIntegrationUnityMsgBoxLayout`) *were* portable and this one isn't.
- **Fixed-format result/ranking tables** (custom `RenderTexts`, no `AddMsg`):
  `CBloodCastleResultMsgBoxLayout`, `CDevilSquareRankMsgBoxLayout`, `CChaosCastleResultMsgBoxLayout`.
- **Multi-option menus** (3+ named action buttons) -- **new primitive built (2026-09-15)**:
  `CGenericMenuDialog`/`GenericMenuConfig` (`UI/Dialogs/GenericMenuDialog.h/.cpp`), a sibling to
  `CGenericConfirmDialog` for an arbitrary N-button list instead of two fixed OK/Cancel slots. No
  fg/bg document split (no consumer needs item3D); modern theme reuses base.rcss's shared
  `.modern-frame`/`.modern-shell-edge`/`.modern-groove`/`.modern-header-rail`/`.modern-content-well`
  directly (the same pieces `sys_menu.rcss` already proves), so it needed no bespoke paint of its
  own, unlike `CGenericConfirmDialog`. Buttons are a real `data-for`-bound array using RmlUi's own
  `it_index` loop variable in the click handler (`char_make.rml`'s job-select buttons are the
  proven precedent for this) -- not a fixed-slot hack like the numeric keypad. Per-button
  `tooltip` (plain CSS `:hover` reveal) and `enabled` (reuses `.btn.disabled`) are supported but
  have no real consumer yet. No explicit column count: `.gmd-button-row` is plain `flex-wrap`,
  so how many buttons fit per row falls out of button width vs. available width on its own --
  every current consumer's fixed 128dp buttons only ever fit one per row, reproducing their own
  single-column stack for free.

  [x] `CSystemMenuMsgBoxLayout` -- proof-of-concept port, done. `ShowSystemMenuDialog()`
  (`UI/Core/WindowCommon.h/.cpp`), 2 call sites (`HotKey.cpp`'s Esc handler, `WindowMenu.cpp`'s
  menu item 0). All 5 buttons keep native's own uniform 108x29 size (this is the one class in
  this bucket where Cancel is NOT the smaller size). Native class removed; grep-confirmed zero
  remaining references. Build clean, both RmlUi verification scripts pass. **In-game tested and
  signed off (2026-09-15)**, alongside the other 9 below.

  [x] Remaining 9 -- done (2026-09-15), same primitive, all free functions in `WindowCommon.h`/
  `CustomMessageBox.cpp`: `ShowChaosMixMenuDialog()` (`MixInventory.cpp`'s `OpeningProcess`),
  `ShowTrainerMenuDialog()`/`ShowTrainerRecoverDialog()` (the Recover button on the first opens
  the second, same nesting native had; `WSclient.cpp` opcode 7), `ShowSeedMasterMenuDialog()`/
  `ShowSeedInvestigatorMenuDialog()`/`ShowResetCharacterPointDialog()`/
  `ShowDelgardoMainMenuDialog()`/`ShowLuckyTradeMenuDialog()` (`WSclient.cpp` opcodes 0x17/0x18/
  0x19/0x20/0x26), `ShowCherryBlossomMenuDialog()` (no live caller, see below). All native classes
  and their `*Layout::SetLayout()` removed; grep-confirmed zero remaining references. `cfg.onCancel`
  is set to the same lambda as the trailing Cancel/Exit button wherever that button has a real side
  effect (e.g. ChaosMixMenu's Cancel also hides/clears the mix inventory behind it) -- native itself
  never wired Esc for any of these 9 (no `MSGBOX_EVENT_PRESSKEY_ESC` handler), so without this an
  Esc-close newly made possible by this primitive could leave a caller window stuck half-configured.
  `CTrainerRecoverMsgBox`'s native per-pet repair-cost sentence (`npcBreeder::CalcRecoveryZen`,
  computed once at `Show()` time) was folded into `cfg.lines` (prefixed with the pet's own name)
  rather than adding a per-button caption field to the primitive for this one consumer.
  `ShowCherryBlossomMenuDialog()` faithfully ports `CCherryBlossomMsgBox` as-is including its 3 color
  buttons being no-ops beyond closing (native bug/unfinished feature, not introduced here) -- and
  grep-confirmed there was never a live `CreateMessageBox()` call site for this class even before
  the port, so it has none now either. Build clean (378/379, no new warnings), both RmlUi
  verification scripts pass. **In-game tested and signed off (2026-09-15)**. Testing surfaced and
  fixed several primitive-level bugs along the way: Esc not closing dialogs at all, per-button
  description text (`MenuButton::lines`, a nested `data-for`) rendering as one bunched block
  instead of interleaved with its own button, insufficient button-row/bottom padding, a missing
  legacy-theme back-fill sprite, a content-vs-title-banner layout gap in both themes, and (the
  significant one) `CGenericMenuDialog`/`CGenericConfirmDialog` losing the Esc keypress to
  whatever plain window happened to sort ahead of them under `CManager::CompareKeyEventOrder`'s
  real descending sort -- see `engine-findings.md` for that last one. All 10
  dialogs (this proof-of-concept plus the 9 below) now close correctly via Esc even with other
  windows open behind them, and Esc on Cancel/Exit runs the same side-effecting lambda as clicking
  that button (verified via `ShowChaosMixMenuDialog()`'s `cfg.onCancel`).

  [x] `CGemIntegrationMsgBox` + `CGemIntegrationUnityMsgBox` -- done (2026-09-15), ported as 3
  chained free functions instead of a 1:1 class replacement: `ShowGemIntegrationMenuDialog()`
  (native's Unity/Disjoint/Cancel entry selector), `ShowGemIntegrationJewelDialog()` (Unity's
  10-button jewel-type grid), `ShowGemIntegrationMixDialog()` (Unity's 3-button mix-amount grid).
  Native's single `CGemIntegrationUnityMsgBox` swapped its own button set in place
  (`ResetWndSize()`) between those last two grids without closing; since `CGenericMenuDialog`
  buttons always close on click, that in-place swap became "close this menu, open a different
  one," reusing the exact reentrant-`Show()`-during-click chaining `ShowTrainerMenuDialog()`/
  `ShowTrainerRecoverDialog()` already prove (`Resolve()` invokes a button's `onClick` while
  `m_bActive` is still `true`, so a `Show()` called from inside it queues instead of clobbering,
  and `Resolve()`'s own trailing `ShowNext()` drains it immediately) -- no primitive change
  needed. The 10-button jewel grid originally reused `MenuButton::compact` (64dp) to get 2 columns
  for free from `.gmd-button-row`'s existing `flex-wrap`, but several jewel names ("Higher Refining
  Stone") don't fit a 64dp button even wrapped -- **superseded 2026-09-15** by a proper general
  mechanism, `GenericMenuConfig::columns` (see `GenericMenuDialog.h`'s own comment and STATUS.md's
  `CGenericMenuDialog` entry for the full design), which decouples "N-per-row grid" from
  `compact`'s own "small Close/Cancel button" meaning and supports wrapped 2-line labels. The
  mix-amount grid's confirm step reuses the existing, already-ported
  `CGenericConfirmDialog` call verbatim (native already used it here). One deliberate,
  non-literal behavior choice: native's `SelectMixBtnDown` left the mix-amount grid visually open
  on a failed `COMGEM::CheckInv()`, even though `CheckInv()` itself already calls
  `COMGEM::GetBack()` (resetting the jewel-type state) -- the port instead reopens the jewel-type
  grid on that failure, matching what `COMGEM`'s own state now says rather than native's stale
  literal behavior. `TenBtnDown`/`TwentyBtnDown`/`ThirtyBtnDown` and both classes'
  `BlessingBtnDown`/`SoulBtnDown` were dropped, not ported -- grep-confirmed unreachable from real
  input even natively (their buttons are declared but never positioned/updated/rendered/hit-tested
  anywhere). Native classes and their `*Layout::SetLayout()` removed; grep-confirmed zero
  remaining references. Build clean (379/379).

  [x] `CElpisMsgBox` -- done (2026-09-15), ported to `ShowElpisMenuDialog(int iMessageType = 0)`.
  Unlike every consumer above, native's button set here never changed -- the "About Refinery"/
  "About Jewel of Harmony" buttons just set `m_iMessageType`, which `RenderTexts()`'s own `switch`
  used to pick a different info blurb above the same unchanged 4 buttons; `RefineBtnDown` opens
  `INTERFACE_MIXINVENTORY` and closes, `ExitBtnDown`/Esc send `SendCraftingDialogCloseRequest()`
  and close. The port is one `GenericMenuConfig` re-`Show()`n with a different `cfg.lines` entry --
  the same reentrant-`Show()`-during-click chaining every other consumer above uses, but swapping
  *text* instead of *buttons*, making this the simplest consumer of the mechanism so far (no
  second phase/dialog needed). Native's own `CutText3`-based manual word-wrap was dropped as
  unnecessary -- `.gmd-line`'s existing `white-space: normal` already wraps at the panel's CSS
  width. No RML/RCSS changes needed (reuses the primitive's existing 4-button markup as-is). Native
  class and its `*Layout::SetLayout()` removed; grep-confirmed zero remaining references. Build
  clean (378/379).

  `CGuild_ToPerson_PositionLayout` and `CGemIntegrationDisjointMsgBoxLayout` stay native -- bespoke
  shapes this primitive's plain "click closes" button model doesn't fit (Guild-to-person: two
  buttons are a simultaneous radio-select pair, not navigation -- both stay clickable/visible at
  once and a separate OK button reads whichever was last selected, nothing here is a sequential
  swap chaining could replace; Gem Integration Disjoint: an embedded, live, continuously-updating
  inventory list-selection widget (`COMGEM::m_UnmixTarList`), a genuinely different UI need
  chaining doesn't solve -- `component-catalog.md`'s "List / repeated rows" section already names
  the proven pattern for a future dedicated port (`CMyQuestInfoWindow`'s quest list: `data-for`
  over `{text, selected, index}` entries, `index` the real stable slot not the positional
  `it_index`)).
- **Progress bar / timed auto-close** (`CProgressMsgBox`/`CCursedTempleProgressMsgBox`-based, needs
  a progress-bar concept — not built): `CCrownSwitchPopLayout`, `CCrownSwitchPushLayout`,
  `CCrownSwitchOtherPushLayout`, `CSealRegisterStartLayout`, `CSealRegisterSuccessLayout`,
  `CSealRegisterFailLayout`, `CSealRegisterOtherLayout`, `CSealRegisterOtherCampLayout`,
  `CCrownDefenseRemoveLayout`, `CCrownDefenseCreateLayout`, `CCursedTempleHolicItemGetLayout`,
  `CCursedTempleHolicItemSaveLayout`.
- [x] **Duel dialogs** — `CDuelMsgBoxLayout` (invite, OkCancel) + `CDuelResultMsgBoxLayout` (result,
  Ok) — `Network/Server/WSclient.cpp`'s `ReceiveDuelRequest`/`ReceiveDuelResult`. Not actually
  countdown rendering (that guess was wrong) — both classes render the exact same fixed native
  sprite (`newui_DuelWindow.tga`, 148x138) with a single bold name-line drawn directly on top of it,
  then 1-3 plain lines below/overlapping its bottom edge. This is a shape only these two classes use
  anywhere in the codebase (a flat 2D image with text overlaid, unlike `item3D`'s live-rendered 3D
  icon), so `GenericDialogConfig` gained one new optional field for it, `portrait2D` (named
  generically, not `duelPortrait` -- this primitive is meant to be reusable, not duel-specific, even
  though Duel is the only real consumer so far; a fixed 100x93dp-ish sprite + one bold overlaid
  caption in its default `Overlay` layout, or icon-left/text-right like `item3D` in its `Beside`
  layout, rendered as the first child of the text column, above `lines`) — worth adding since it
  matches a genuinely distinct native rendering shape for exactly two real consumers today,
  following the same "one field per recurring native shape" pattern
  as `title`/`input`/`progress`/`item3D`. Both native classes + their `CustomMessageBox.h`/`.cpp`
  declarations/implementations removed; grep-confirmed zero remaining references. Also gained
  `GenericDialogConfig::tallPanel` alongside this (grows `#panel` via a CSS class instead of relying
  on `.gcd-text-col`'s scrollbar, for content that doesn't comfortably fit the default height) —
  applied to both Duel dialogs and the 3 `Mode::NumericKeypad` vault-PIN dialogs. **In-game-tested
  and confirmed working (2026-09-15)**, both themes.

## Other native dialog subsystems (out of `CommonMessageBox`/`CustomMessageBox`, tracked here too)

- [x] `CUIPopup` (`UI/Dialogs/UIPopup.h`, `g_pUIPopup`) — done (2026-09-13). Every real call site
  (grepped, not just the header's declared capability) used only `POPUP_OK` (5 sites, `ResultFunc`
  always `NULL`) or `POPUP_YESNO` (4 live sites with a callback + 1 dead one) — no live
  `POPUP_OKCANCEL`/`POPUP_INPUT`/`POPUP_TIMEOUT` anywhere. All 9 live sites ported:
  `Guild/UIGuildInfo.cpp` (6: alliance-master-can't-disband/withdraw notices, guild-disband,
  member-disband-rank, member-fire, alliance-ban confirms) and `Guild/UIGuildMaster.cpp` (1:
  guild-mark-edit-not-activated notice), `Network/Server/WSclient.cpp` (2: hunt-zone-enter
  fail/no-authorization notices). `POPUP_YESNO` → `ButtonSet::OkCancel` with
  `primaryLabel/secondaryLabel = I18N::Game::Yes/No` (first live use of a non-OK/Cancel label —
  `GenericDialogConfig` already supported it, no primitive change needed). Each old free-function
  callback's body was folded directly into `onPrimary`/`onSecondary`, same convention as the
  `CommonMessageBox.h` batches.
  - Also deleted as confirmed-dead (not ported): `CUIGuildMaster::ReceiveGuildRelationShip` +
    its `DoGuildRelationReplyAction` callback + the `m_dwEditGuildMarkConfirmPopup`/
    `m_dwGuildRelationShipReplyPopup` members + `CUIGuildMaster::CloseMyPopup()` — grep confirmed
    `WSclient.cpp`'s `ReceiveGuildRelationShip` packet handler actually calls
    `g_pGuildInfoWindow->ReceiveGuildRelationShip(...)` (the already-ported `CGuildInfoWindow`,
    from the earlier Guild batch), never this one; it was dead leftover from before that port.
  - Two now-pointless `g_pUIPopup->CancelPopup()` defensive calls removed from `WSclient.cpp`
    (`ReceiveHuntZoneEnter`'s two branches, `ReceiveCrownState`'s entry) — nothing left for them to
    cancel once their neighboring `SetPopup(...POPUP_OK...)` calls moved off `CUIPopup`.
  - **`CUIPopup` itself is *not* deleted** — `UIGuildInfo.cpp`'s `POPUP_CUSTOM` "Appoint"
    sub-guild-master/battle-master picker (`DoAppointAction`/`RenderAppoint`, a bespoke 2-option
    picker plus its own OK/Cancel) stays fully native, same DOESNT_FIT category as
    `CChaosMixMenuMsgBoxLayout` etc. `CUIGuildInfo::CloseMyPopup()`/`m_dwPopupID` also stay
    unchanged — they now exclusively (and correctly) govern just that surviving popup.
  - **Follow-up done (2026-09-15)**: `UIPopup.cpp`/`.h` trimmed to what the one surviving caller
    (`UIGuildInfo.cpp`'s Appoint picker) actually exercises. Confirmed via grep that
    `SetPopup()`/`SetPopupExtraFunc()` have exactly one call site, always `POPUP_CUSTOM` with a
    `NULL` `ResultFunc`, always immediately followed by `SetPopupExtraFunc()` — so every built-in
    OK/OK-Cancel/Yes-No/timeout/text-input branch in `SetPopup()`/`CancelPopup()`/`PressKey()`/
    `UpdateInput()`/`Render()`, plus their backing members (`m_OkButton`/`m_CancelButton`/
    `m_YesButton`/`m_NoButton`, the text array, timeout/input state) and macros
    (`POPUP_OK`/`POPUP_OKCANCEL`/`POPUP_YESNO`/`POPUP_TIMEOUT`/`POPUP_INPUT`,
    `POPUP_RESULT_OK`/`_CANCEL`/`_YES`/`_NO`/`_TIMEOUT`, `POPUP_ALIGN`), were dead. `SetPopup()`'s
    signature dropped to just `(ResultFunc)`; `CUIManager::IsInputEnable()`'s now-always-false
    `g_pUIPopup->IsInputEnable()` clause removed too. Also deleted two fully-dead
    `POPUP_RESULT`-shaped free functions this cleanup exposed as unreferenced from anywhere:
    `DenyCrownRegistPopupClose` (`WSclient.cpp`) and `DoEditGuildMarkConfirmAction`
    (`UIGuildMaster.cpp`). Build clean.
  - Also removed one unrelated pre-existing dead `extern int DoBreakUpGuildAction_New(POPUP_RESULT)`
    declaration found in `CommonMessageBox.cpp` while working in this area (no definition, no
    caller, anywhere — stale leftover, unrelated to this session's own `DoBreakUpGuildAction`).
- [x] `GameShop/MsgBoxIGS*.h` — **done (2026-09-14)**. Re-inventoried all 11 files (headers read in
  full, not trusted from this row's own old description) — 1 base class used directly
  (`CMsgBoxIGSCommon`) + 10 "subclasses" that turned out to be independent copy-paste siblings, no
  shared C++ base beyond native `CMessageBoxBase`.

  **This is the one family in the whole inventory with a genuine title**: `CMsgBoxIGSCommon::
  Initialize(pszTitle, pszText)` (deleted along with the rest, see below) rendered `m_szTitle` in
  bold at a fixed top offset, separate from the body text below it — a real title/body split, not
  just a bold first line the way `CCommonMessageBox`'s bold `AddMsg` lines are.
  `GenericDialogConfig.title`/`.gcd-title` (both themes) already existed from the earlier extensions
  session but had zero real consumers until this batch — first exercise for both.

  **`title` upgraded from plain colored text to a real header banner (2026-09-14, after this batch's
  own in-game testing)** — `.gcd-title` replaced by `.gcd-header-rail`/`.gcd-header-title`, styled
  like Login/`CSysMenuWin`'s own header-rail/hero-banner rather than a bold text line, colored per
  `severity`. See the feature-extension history above (next to the item3D/scrollbar fixes) for the
  full layout writeup — no `GenericDialogConfig`/C++ changes, purely an `.rml`/`.rcss` restructure.

  Every button in this family renders as literal `I18N::Game::OK`/`Cancel` text (grep-checked) — no
  `primaryLabel`/`secondaryLabel` overrides needed anywhere. Every native `Initialize()` builds its
  body as one `mu_swprintf`-formatted string, pixel-wrapped via `DivideStringByPixel()` — ported as
  a single `cfg.lines` entry each, letting `.gcd-line`'s own `white-space: normal` reflow it (same
  simplification every earlier FITS batch used).

  - **Plain `title`+`lines`+`OkCancel`/`Ok` (5 classes)**:
    - `CMsgBoxIGSCommon` (~50 call sites — `InGameShopSystem.cpp` x2, `InGameShop.cpp` x5,
      `StorageInventory.cpp` x1, `WSclient.cpp` ~39) → new shared free function
      `mu::ui::window::CreateOkMessageBoxWithTitle(title, text)` (`UI/Core/WindowCommon.h/.cpp`),
      modeled directly on the existing `CreateOkMessageBox()`. Every call site's 3-line
      construct-then-`Initialize()` pattern collapsed to one line via a scripted regex replace
      (47 sites across 4 files) plus 2 manual conversions in `MsgBoxIGSSendGift.cpp`'s error
      branches (still-native, see below).
    - `CMsgBoxIGSBuyConfirm` (2 call sites, both in the still-native `BuyPackageItem`/`BuySelectItem`)
      → `ShowIGSBuyConfirmDialog(...)` (kept in its own file as a free function, since those 2
      native classes still need to open it from their own Buy button).
    - `CMsgBoxIGSUseBuffConfirm` (0 external call sites, only ever chained) → `ShowIGSUseBuffConfirmDialog(...)`.
    - `CMsgBoxIGSUseItemConfirm` (2 call sites, both ported below) → `ShowIGSUseItemConfirmDialog(...)`
      — `onPrimary` replicates the native buff-conflict branch verbatim (`TheBuffInfo().GetBuffType()`
      + `Hero->Object.m_BuffMap.IsEqualBuffType()`, respecting the existing
      `#ifdef LEM_FIX_WARNINNGMSG_DELETE` override) and chains to `ShowIGSUseBuffConfirmDialog()` on
      conflict, else sends the consume/point-info requests directly.
    - `CMsgBoxIGSSendGiftConfirm` (1 call site, in the still-native `CMsgBoxIGSSendGift`) →
      `ShowIGSSendGiftConfirmDialog(...)`, `id`/`message` captured by value (`std::wstring`) in the
      `onPrimary` closure.
  - **`item3D` + `title` (2 classes, first non-`C3DItemCommonMsgBox` consumers of `item3D`)**: both
    native classes called `RenderItem3D(x,y,w,h, wItemCode, 0,0,0, true)` directly (a bare item code,
    not a real `ITEM*`) since these are virtual cash-shop items with no level/excellent/ancient
    state — ported by building a minimal `ITEM{} ; item.Type = wItemCode;` snapshot (everything else
    stays zero, matching native's own zero args) and setting `cfg.item3D` to it.
    - `CMsgBoxIGSStorageItemInfo` (1 call site) → `ShowIGSStorageItemInfoDialog(...)`, `onPrimary`
      chains to `ShowIGSUseItemConfirmDialog(...)`.
    - `CMsgBoxIGSGiftStorageItemInfo` (1 call site) → `ShowIGSGiftStorageItemInfoDialog(...)`. Its
      native `CUITextInputBox m_MessageInputBox` is read-only display here (`SetText()` in
      `Initialize`, never read back) — ported as one more plain `cfg.lines` entry, no `input` field
      needed, no primitive gap. `onPrimary` chains to the same `ShowIGSUseItemConfirmDialog(...)`.
  - **Confirmed dead, deleted (1 class)**: `CMsgBoxIGSDeleteItemConfirm` — zero call sites anywhere
    (grep-confirmed), not ported.
  - **Stay native, out of scope (3 classes)**: `CMsgBoxIGSBuyPackageItem`/`CMsgBoxIGSBuySelectItem`
    — genuine `Buy`/`Present`(Gift)/`Cancel` 3-button shape (`ButtonSet` only had `Ok`/`OkCancel` at
    the time; the button-role redesign below removed this specific blocker, see `secondaryLabel`/
    `showCancel`) plus a scrollable/selectable description or price-tier list box (`lines` is static
    text, no interactive-list concept) — the list-box gap is the same DOESNT_FIT category as the
    multi-option menus elsewhere, and still keeps these 2 classes native.
    `CMsgBoxIGSSendGift` — needs two simultaneous text-entry fields (single-line recipient ID +
    separate multiline message) at once; `GenericDialogConfig.input` is a single
    `std::optional<InputField>` — a genuine, documented primitive gap, not improvised around. All 3
    classes' own Buy/Present/error button handlers were still updated (mechanical call-site edits
    only) to call the newly-ported free functions in place of the classes that got deleted.

  All 8 deleted classes' declarations + implementations removed (`MsgBoxIGSCommon.h`/`.cpp` deleted
  outright — no remaining consumer needed the file; the other 6 ported classes' files were kept
  alive, gutted down to just the new free function, since the 3 still-native classes still
  `#include` them). Grep-confirmed zero remaining references. Build clean (zero new warnings) + both
  RmlUi verification scripts pass.

  **Incidental bug found while porting**: removing `MsgBoxIGSCommon.h`'s include from `WSclient.cpp`
  broke 2 unrelated unqualified `CSystem::GetInstance()` calls later in that same file (line
  1547/8396) — turned out `WSclient.cpp` never had its own `using namespace mu::ui::window;`, it was
  relying the entire time on `MsgBoxIGSCommon.h`'s own file-scope (unwrapped) using-directive leaking
  in via `#include` and silently applying to the rest of the translation unit. Fixed by adding an
  explicit `using namespace mu::ui::window;` directly to `WSclient.cpp` instead of re-relying on a
  transitive accident.

  **In-game-tested and confirmed working (2026-09-15)** — this was `title`'s first real exercise
  (previously infrastructure-only) and `item3D`'s first non-`C3DItemCommonMsgBox` use.
- **Explicitly out of scope for `CGenericConfirmDialog`** (would need their own primitives if ever
  ported): `CHelpWindow`, `CWindowMenu`, `CChatCommandWindow` (`UI/Dialogs/`) — help overlay,
  per-window popup menu, command picker; none are confirm-dialog shaped.

## Button model redesign: `primary`/`secondary`/`cancel` roles — done (2026-09-16)

`GenericDialogConfig`'s old `ButtonSet::Ok`/`OkCancel` enum only supported one action plus a fixed
"Cancel" second slot, blocking any dialog with a genuine third named button (`CUseFruitCheckMsgBox`'s
`item3D` + `[Create] [Decrease] [Cancel]`; also the reason `CMsgBoxIGSBuyPackageItem`/
`CMsgBoxIGSBuySelectItem` above stayed native). Replaced with three fixed, role-named slots instead
of two positional ones: `primaryLabel`/`onPrimary` (always shown, free-form label), optional
`secondaryLabel`/`onSecondary` (a real second action — Decrease/Gift/Discard, never Esc-bound), and
`showCancel`/`cancelLabel`/`onCancel` (always dismiss semantics, fires on Esc, text overridable).
Still exactly 3 fixed slots, not an arbitrary list — stays out of `CGenericMenuDialog`'s territory.

Mechanically swept all ~70 existing `ButtonSet::OkCancel` call sites to `showCancel = true` (zero
behavior change — `ButtonSet::Ok` needed no change at all, it's the default) plus the one explicit
`ButtonSet::Ok` assignment (`WSclient.cpp`, deleted outright as redundant). `UIGuildInfo.cpp`'s 4
Yes/No confirms, which repurposed the old "secondary" slot's text as `No`, were renamed to
`cancelLabel = I18N::Game::No` specifically — leaving them as `secondaryLabel` under the new meaning
would have silently added a real (unwanted) second button alongside Cancel.

Button-row RML/RCSS (both themes) grew a third `#gcd_btn_cancel` element (today's original Cancel
slot, renamed) alongside the new `#gcd_btn_secondary`, with `solo`/`triple` layout classes computed
from `has_secondary && show_cancel`; the existing 2-button `paired` position values are untouched
(`.gcd-btn-secondary`/`.gcd-btn-cancel` share the same left offset when only one of them is visible),
so every pre-existing consumer renders pixel-identical to before. `.triple` (all 3 shown) uses new
position values derived from native's own `triwidth = panelWidth / 3` centering
(`CUseFruitCheckMsgBox::SetButtonInfo()`) — not yet visually tuned in-game.

**First (and so far only) `.triple` consumer: `CUseFruitCheckMsgBox` migrated and deleted.**
`InventoryActionController.cpp`'s existing "Do you want to use the fruit?" confirm (`item3D` +
Ok/Cancel, already on `CGenericConfirmDialog`) now chains a second `Show()` from its own `onPrimary`
instead of opening the native box: `primaryLabel = Create`/`onPrimary` sends the add-points request,
`secondaryLabel = Decrease`/`onSecondary` sends the remove-points request, `showCancel = true`
(default "Cancel", no-op). `byIndex` is looked up fresh inside each callback via
`g_pMyInventory->GetStandbyItemIndex()`, matching native's own `AddBtnDown`/`MinusBtnDown` exactly;
`item3D` is the same `ITEM` snapshot already captured by the first dialog's own `cfg.item3D`, copied
into the closure rather than re-read from the original `ITEM*` (which doesn't outlive the callback).
`CUseFruitCheckMsgBox`/`CUseFruitCheckMsgBoxLayout` deleted from `CustomMessageBox.h/.cpp` outright,
zero remaining references. Not yet in-game-tested.

## How to use this file

1. Pick a batch (a grouping above, or a handful of related `[ ]` rows).
2. Re-check each class's `SetLayout()`/`Create()` still matches the noted shape (things drift).
3. Swap the call site(s) to `g_pGenericConfirmDialog->Show({...})`, following the pattern the 3
   already-ported call sites established.
4. Grep to confirm zero remaining references, delete the native class's declaration + implementation
   from `CommonMessageBox.h`/`.cpp` (or `CustomMessageBox.h`/`.cpp`).
5. Check the row off here, and run both `Tools/check_rml_rcss_syntax.py` /
   `check_rml_rcss_drift.py` plus a full build before considering the batch done.
6. Flag in-game verification for whoever can launch the client — same as the original 3.
