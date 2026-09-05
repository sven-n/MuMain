# UI Kit Target Architecture — Critical Review & Proposal

Companion to [`ui-architecture-assessment.md`](ui-architecture-assessment.md). That document
inventoried the current, fragmented state; this one independently re-verifies its conclusions
against source (not against its own text) and proposes a target architecture, canonical
components, an RmlUi strategy, and a migration plan. No code has been changed as part of either
document. Produced 2026-09-05.

## A. Architecture Verdict

**The prior assessment's diagnosis is correct and, if anything, understated.** Independently
re-verifying:

- **`CObject` having no geometry is confirmed and worse than implied.** It's not just "no
  `Contains()`" — `CManager`'s own dispatch (`WindowManager.cpp`) proves the design intent: every
  `Update()`/`Render()`/`UpdateMouseEvent()`/`UpdateKeyEvent()` call is wrapped in a
  `ScopedActiveTransform` derived purely from `GetLayoutMode()`, with **zero** reference to any
  bounds on the object itself. The manager provides *only* a coordinate space; every window is
  trusted to do its own hit-testing inside it. This is a coherent, deliberate contract, not an
  oversight — but it does mean "add a new window" always includes "reinvent your own rect."
- **The `mu::ui::window::CButton` proposal needs real qualification, not a rubber stamp.** Reading
  `Widgets/Window/Button.cpp` in full changes the verdict from the prior assessment's implicit
  "yes, canonicalize this":
  - It does **not** use `CSprite` at all — it renders via a free function `RenderImage()` against
    a global `Bitmaps[]` table, a *third* rendering primitive alongside `CSprite` and RmlUi,
    re-implementing atlas frame-offset math (`m_CurImgState * m_Size.y`) that `CSprite` already
    solves.
  - It hit-tests via `CheckMouseIn()` — a function that lives in `UIControls.h`, the very "legacy
    `CUIControl` toolkit" the prior assessment treated as a separate, competing generation. The
    "new" tier's own canonical button is not actually independent of the "old" one.
  - It polls input via free functions `IsPress`/`IsRelease`/`IsNone` (`UI/Core/WindowCommon.cpp`),
    which wrap a **third, independent input-sampling system**, `CNewKeyInput`/`g_pNewKeyInput`,
    polling `GetAsyncKeyState()` directly. The legacy `::CButton` instead polls
    `CInput::Instance()`. Neither goes through `CManager`'s own `UpdateMouseEvent()` return-value
    contract at the individual-widget level. **This is a finding the prior assessment missed
    entirely** — it flagged RmlUi-vs-legacy input as "two systems, not three" (quoting
    `STATUS.md`) but didn't catch that a large fraction of `CObject`-tier windows (grep:
    `InventoryCtrl.cpp`, `Trade.cpp`, `MixInventory.cpp`, `GoldBowmanWindow.cpp`, and others)
    bypass both `CInput` and RmlUi's event system for a third, direct `GetAsyncKeyState()` poll.
  - It duplicates `EnsureLocaleObserver()`/`OnLocaleChanged()` nearly verbatim three times
    (`CButton`, `CRadioButton`, `CCheckBox`) instead of once on `CBaseButton`.
  - Its behavior forks under `#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM`/
    `KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE` — meaning "the canonical widget" doesn't have one
    behavior, it has up to four, selected at compile time.
  - It does not consult `UI::Scaling`'s `Transform`/`PositionX()`/`PositionY()` at all — it draws
    and hit-tests at raw `m_Pos`/`m_Size`, assuming the caller already resolved real pixels.
    That's *consistent* with how `CTrade`/`CMyInventory` use it, but it means this "canonical"
    widget has no self-contained scaling story — it inherits whatever discipline its owning
    window happens to apply.

  None of this makes it a bad choice — it's still the most-adopted, most-feature-complete of the
  three C++ button families, and evolving it costs less than inventing a fourth. But **calling it
  canonical as-is would enshrine a widget with a parallel rendering path, a parallel input system,
  and compile-time behavior forks.** Canonicalizing it has to include fixing those things, not
  just picking a name.
- **`UI::Scaling::UITransform`/`UILayoutPolicy` is stronger than the prior assessment gave credit
  for.** Reading the full header: seven `LayoutMode` variants (`Hud*`, `Dock*`,
  `FloatingWorkspace`, `Dialog`, `WorldOverlay`, `Legacy`), a real RAII `ScopedActiveTransform`,
  and — confirmed in `WindowManager.cpp` — **every single dispatch call for every window already
  routes through it**. This is not an "existing foundation to build toward"; it is already the
  load-bearing, 100%-adopted coordinate-transform layer. The prior assessment undersold this.
- **RmlUi's bridge layer (`RmlModelBinder`, `RmlTheme`, `RmlDraggable`) independently checks out
  as well-designed**, not just "additive" — `LoadThemedDocument()` is a genuine single required
  entry point, theme selection is name-agnostic with a capability-flag escape hatch (no hardcoded
  theme-name branching), and `RmlModelBinder<T>` is a correctly minimal, non-overreaching wrapper
  (it doesn't try to auto-reflect fields, which would have been over-engineering). The prior
  assessment's characterization of RmlUi as "additive, composed into a Generation-2 window" is
  accurate.
- **One thing the prior assessment got directionally right but didn't push far enough**: it
  treated `CObject`'s lack of geometry as a gap to *fill*. Having now seen `CManager`'s
  transform-per-object dispatch model in full, the more accurate framing is that `CObject` was
  *deliberately* kept thin, and the correct fix is a **composable geometry component**, not a
  base-class field — see Section C.

## B. Definition of the Unified UI Kit

Five layers, derived from what's actually load-bearing today — not a generic 6-layer sketch,
because two illustrative layers (`UI Containers` and `Presentation`) don't cleanly separate in
this codebase and forcing them apart would add abstraction with no current payoff:

```
1. UI Runtime        — CObject / CManager (lifecycle, dispatch, depth/key-order, show/enable)
2. UI Geometry        — UI::Scaling::UITransform / UILayoutPolicy (existing, keep)
                        + a new, OPT-IN geometry/hit-test component (new)
3. UI Components      — canonical native widgets (evolved mu::ui::window::CButton family)
                        for native-only contexts, converging toward RmlUi + base.rcss
                        components for everything else (two-horizon, see Section E)
4. Presentation       — CSprite (native visuals) + RmlUi/RCSS (ported visuals) + CUIRenderText
5. Application UI     — CTrade, CMyInventory, MainFrameWindow, etc. — composes 1-4
```

`UI Containers` isn't a separate layer because nothing in this codebase currently owns
parent/child containment or scroll-clipping generically — `CManager`'s registry is flat. A
container abstraction is a real, named gap (Section D), not an existing layer to describe.

## C. Canonical Architecture

**Keep `CObject` thin — do not add geometry/rendering/input/styling fields to it.** The
investigation confirms this would be wrong for a concrete reason, not just taste: several
`CObject` subclasses (`CMyInventory` via `I3DRenderObj`, `CCharInfoBalloonMng`) have no static 2D
rect at all — their position is a per-frame `WorldToScreen()` projection. A mandatory geometry
field on the base class would be meaningless for them and would recreate exactly the "why does my
window carry fields it never uses" complaint that motivates unifying in the first place.

**Composition, via an opt-in mixin, is the right call — the codebase already validates this
pattern.** `CObject`'s own shown/active split (`IsActive()`/`SetActive()`/`UpdateWhileShown()`/
`UpdateWhileActive()`) is already exactly this shape: additive, default-inert virtuals that do
nothing until a subclass opts in by overriding them. Geometry should follow the identical pattern
rather than a new mechanism:

```cpp
namespace mu::ui::window {

// Opt-in. A window that has a real 2D rect owns one of these (composition,
// not inheritance) and forwards hit-testing to it; a window that doesn't
// (3D-projected, world-anchored) simply never creates one.
class WindowGeometry {
public:
    void SetPosition(int x, int y);
    void SetSize(int w, int h);
    bool Contains(int windowX, int windowY) const;   // uses the ACTIVE transform already
                                                        // pushed by CManager, via UI::Scaling
    RECT Bounds() const;
private:
    POINT m_pos; SIZE m_size;
};

}
```

This is deliberately a *value object a window owns*, not a mixin base `CObject` derives from —
matching the instinct to prefer composition where it minimizes coupling. A window that wants it
declares `WindowGeometry m_geometry;` and calls `m_geometry.Contains(...)` from its own
`UpdateMouseEvent()` instead of hand-rolling the same rect math ~88 times. Windows with no
meaningful rect (world-projected overlays) never touch it. `CManager` itself never needs to know
it exists — dispatch stays exactly as it is today.

**Rendering and input stay explicitly out of `CObject` and out of `WindowGeometry`, for the same
reason.** A window's rendering is either `CSprite`-based, RmlUi-based, or both simultaneously
during migration — there is no single "Render" contract narrower than the `bool Render()`
`CObject` already declares that would fit all three without becoming a leaky abstraction. Input
is already unified at the *dispatch* level (`CManager`'s topmost-first, consume-and-stop loop,
confirmed correct and coherent in `WindowManager.cpp`); the actual defect is one level down —
three different widget-level input-polling APIs (`CInput`, `CNewKeyInput`, RmlUi events) — which
Section D addresses directly rather than by adding a fourth abstraction on top.

## D. Canonical Components

| Concern | Current owners (verified) | Canonical owner | Reason |
|---|---|---|---|
| Window lifecycle | `CObject`/`CManager` | **`CObject`/`CManager`, unchanged** | Already ~88/97 windows; zero live top-level `CWin` subclasses remain |
| Dispatch / input propagation | `CManager`'s depth-sorted, consume-and-stop loop | **`CManager`, unchanged** | Verified correct and coherent (topmost-first, `ScopedActiveTransform` per object) — this was never actually broken |
| Widget-level input polling | `CInput` (legacy `::CButton`), `CNewKeyInput`/`g_pNewKeyInput` (new-tier widgets, ~15+ files), RmlUi's own event system | **`CInput`, consolidate the other two onto it** | `CNewKeyInput` independently polls `GetAsyncKeyState()` — a real, previously-uncaught duplicate input sampler, not just a naming difference |
| Geometry / hit-testing | Hand-rolled per window (`CWin`'s own, `CUIControl`'s own, ~88 independent rect checks) | **New opt-in `WindowGeometry` (Section C)** | No existing implementation is reusable as-is; all are per-family, not per-concern |
| Coordinate transforms | `UI::Scaling::UITransform`/`UILayoutPolicy` | **Unchanged — already canonical** | 100% dispatch coverage confirmed in `WindowManager.cpp`; the strongest existing primitive in the codebase |
| Buttons (native-only contexts) | 4 implementations | **Evolved `mu::ui::window::CButton`, after removing its `CUIControl`/`CNewKeyInput` dependencies and macro-forked behavior** | Most adopted, richest feature set; not clean enough to canonicalize unmodified |
| Buttons (RmlUi-reachable contexts) | ad hoc per-window `.btn` CSS | **`base.rcss`'s `.btn` + a documented variant convention** | RmlUi is the long-term presentation layer; a native button abstraction is a transitional concern there, not a permanent one |
| Text input | `CUITextInputBox` | **Unchanged — already canonical** | Genuinely shared across every generation including RmlUi-hybrid windows; don't touch what already works |
| Tooltips | 4 mechanisms | **New standalone `UI::Tooltip` primitive** (native) + **`base.rcss`'s `.tooltip` convention** (RmlUi) | Same two-horizon split as buttons; extract from `mu::ui::window::CButton`'s existing anchor/position logic rather than inventing new math |
| Window chrome | None — every window reimplements | **Not unified natively.** For RmlUi-reachable windows, chrome is a theme/RCSS concern (already true). For native-only windows, not worth building — see Section F | Building a generic native chrome abstraction for a shrinking population (windows not yet, and possibly never, ported) is negative-value work |
| Layout/scaling | `UILayoutPolicy` | **Unchanged — already canonical** | Same as above |
| Rendering | `CSprite`, raw `RenderImage()`, RmlUi | **`CSprite` for native visuals, RmlUi for ported visuals — retire the second `RenderImage()` path** | `RenderImage()` re-derives what `CSprite`'s frame/atlas system already does; no reason for a second implementation |
| Theme | `UI::RmlBridge::LoadThemedDocument()`/`GetActiveThemeName()`/capability flags | **Unchanged — already canonical** | Independently verified well-designed: name-agnostic, capability-flag-gated, single required entry point |
| Window-repositioning drag | `CWin::SetMovable()` (legacy, inert now) / `MakeDraggable()` (RmlUi, unused) | **`MakeDraggable()`, once it gets a first real caller** | Well-designed, narrowly scoped, honestly documents its own two open gaps |
| Item-slot drag/drop | `CInventoryActionController` | **Unchanged, explicitly not part of the UI kit** | Domain logic wearing UI clothing — see Section F |
| Open/close mutual exclusion (inventory family) | `CUIManager` | **Unchanged, rename its `INTERFACE_*` enum values only** | Legitimate, narrow domain policy; the only real defect is the name collision with the tier's own registry key |

## E. RmlUi Strategy

**Answer: C (hybrid) — but as the deliberate, permanent target shape for one category of window,
not a temporary state on the way to "B."**

The evidence for this, not just an assumption:

1. The background-layer flush mechanism (`IMuRenderer::FlushRenderCommands()`,
   `RmlUiRuntime::GetBackgroundContext()`/`RenderBackgroundLayer()`) was purpose-built to let
   RmlUi content interleave with mid-frame 3D rendering — this is real engineering investment in
   making hybrid work *well*, not a stopgap being tolerated until everything moves to RmlUi.
2. `component-catalog.md`'s own documented gap list still has no plan for item-slot
   3D-composited icon rendering inside RmlUi — because RmlUi has no primitive for "a real-time 3D
   camera view composited into a 2D layout," and building one is a materially different problem
   than porting static chrome.
3. World-anchored content (`CCharInfoBalloonMng`, name tags) is fundamentally a
   `WorldToScreen()` projection problem, not a layout problem — RmlUi's box model doesn't help
   there regardless of migration completeness.

So the boundary is not "how much is ported yet" but **what kind of content it is**:

```
RmlUi owns:                              Native C++ (CSprite/CObject) owns:
  - all static 2D chrome/layout/text       - anything camera-composited (item/skill
    for any window that has been            icons rendered against the live 3D scene)
    or will be ported                     - world-anchored overlays (name tags,
  - buttons/checkboxes/text once             character-info balloons) -- WorldOverlay
    a window is ported                       transform, not a layout concern
  - tooltips once a window is ported       - anything not yet ported (transitional,
                                              shrinks over time)
```

A ported window keeps exactly one native companion pattern, not native widgets generally: a
**Type-2 companion object** (a real interactive widget like `CUITextInputBox` that RmlUi can't
yet host) or a **Type-1 redundant click-detector** (a `CButton` doing pure bounding-box
click-consumption behind RmlUi's real click handling, kept only because the legacy activation
gate was unreliable — already documented and already being retired as windows get their own
direct `RmlClickX()` handlers). Neither is "native UI persisting because RmlUi migration
stalled" — both are documented, bounded exceptions. New windows should not add a third kind of
native companion without a specific, new reason as strong as those two.

## F. Non-Unified Areas

Explicitly kept separate, with the concrete reason each earns it:

- **`CInventoryActionController`/item drag-drop** — this is inventory *business logic*
  (move/split/stack rules, server round-trips) that happens to render through UI, not a UI
  mechanism. Folding it into a generic "UI drag" abstraction would force domain rules through a
  presentation-layer interface for no reuse benefit — nothing else in the game needs "split a
  stack across two 2D grids."
- **`CUIManager`'s open/close mutual exclusion** — genuinely domain policy ("opening Inventory
  closes Personal-Shop-Purchase"), not a generic UI concern. The only actionable defect is its
  enum name collision (Section D); the mechanism itself should not be merged into `CManager`.
- **3D-camera-composited rendering** (`Window3DRenderMng`/`I3DRenderObj`) — see Section E.
  Forcing this through the 2D component model would be unification for its own sake.
- **World-space overlays** (`WorldOverlay` layout mode content) — same reasoning; a
  `WorldToScreen()`-driven position isn't a layout bug to fix, it's a different problem than
  panel UI.
- **`CWinEx` as a composed widget** (e.g. `CServerSelWin::m_winDescription`) — not worth ripping
  out on its own. It's inert as a base class already; touching it buys nothing unless that
  specific window is being changed for another reason anyway.
- **Skill-specific tooltip content logic** (`UI::Skills::Tooltip::BuildModelForSlot`) — the
  *rendering* should converge on the one canonical Tooltip primitive (Section D), but the domain
  logic that decides *what a skill tooltip says* is correctly separate and should stay that way,
  same reasoning as item drag-drop.

## G. Developer Usage Model

Concretely, for *"a settings window with a title, three tabs, labels, checkboxes, a text field, a
slider, and a tooltip"* — the answer genuinely forks on one question first, because that's what
the evidence in Section E demands:

**Does this window need any 3D-camera-composited or world-anchored content?** No, for a settings
window. So the path is:

```
1. Author the window as an RmlUi document + RCSS (both themes) —
   title, tabs, labels, checkboxes, text field, slider are all pure
   2D layout/chrome. Use base.rcss's existing .btn/.checkbox-box/.tooltip
   classes; do not invent new markup patterns for controls that already
   have one.
        ↓
2. Create a mu::ui::window::CObject subclass as the state/lifecycle owner
   (settings values, tab-selection state, event callbacks to GameConfig).
   It does NOT get a WindowGeometry — RmlUi owns its own box model.
        ↓
3. Load it via UI::RmlBridge::LoadThemedDocument() — never Context::LoadDocument
   directly.
        ↓
4. Bind state via RmlModelBinder<SettingsModel> — one struct, one
   registration function, per the existing pattern.
        ↓
5. Wire the slider's live value / tab switch via data-model bindings and
   RmlUi event listeners (the AddEventListener idiom already used by
   CLoginMainWin) -- no CButton/CUIControl involved at all.
        ↓
6. Register with mu::ui::window::CManager via AddUIObj(), give it an
   INTERFACE_* key, add its LayoutMode::Dialog (or whatever fits) case
   to UILayoutPolicy::ForInterface().
        ↓
7. Done. No CWin, no CUIControl, no legacy CButton, no CNewKeyInput poll.
```

**If** the window had a 3D-composited element (it doesn't, but for contrast — e.g. an equipment
preview panel), only *that* element stays native: a `CSprite`/3D-camera-rendered sub-view
composited via the background-layer flush mechanism (Section E), while every other control on the
same window still goes through the RmlUi path above. The developer never has to choose between
"the CWin way" and "the CUIControl way" and "the mu::ui::window::CButton way" — those three only
remain relevant for windows that predate this model and haven't been touched yet, and the rule
for touching them is in Section H.

## H. Migration Plan

Ordered by leverage-per-risk, using what's actually true today (not a generic template):

**Needed immediately (cheap, zero behavior risk, unblocks clear thinking):**
1. Rename `CUIManager`'s `INTERFACE_*` enum values (e.g. `INTERFACE_INVENTORY` →
   `MUTEX_INVENTORY`) so no two enums share names. Pure mechanical rename.
2. Document, in the repo's dev instructions (Section I), that `mu::ui::window::CButton`/
   `CCheckBox`/`CRadioButton` are the native-only canonical family and RmlUi + `base.rcss` is
   canonical for anything with a presentation layer — so no new window picks a fourth path by
   default.

**Needed before further native UI development continues at any real pace:**
3. Consolidate `CNewKeyInput`'s `GetAsyncKeyState()` polling onto `CInput` (or vice versa —
   whichever is the real, intended source of truth; that's a one-question investigation, not a
   redesign) so there's one input sampler, not two disagreeing ones.
4. Extract the Tooltip primitive out of `mu::ui::window::CButton::ChangeToolTipText`'s existing
   logic into a standalone, attachable component.
5. Build the opt-in `WindowGeometry` component (Section C) and start using it in any window
   touched for other reasons.

**Can wait (real value, not blocking):**
6. Retire `mu::ui::window::CButton`'s `RenderImage()` path in favor of `CSprite`, and its
   `CheckMouseIn()` dependency on `UIControls.h` in favor of the new `WindowGeometry`. Do this
   opportunistically per-window, not as a dedicated sweep.
7. Migrate new UI screens onto the RmlUi path (Section G) by default; migrate existing
   native-only screens to RmlUi only when they're being touched for another reason anyway
   (matches the branch's own established practice).
8. Deprecate and eventually remove `::CButton : CSprite`, `CUIButton : CUIControl`, and the
   global `::CRadioButton` once grep shows zero remaining callers.

**Should never be done:**
9. Do not add geometry/rendering/input/styling fields to `CObject` itself (Section C) — the thin
   base class is correct, not a gap.
10. Do not fold `CInventoryActionController`, `CUIManager`'s domain policy, or 3D-camera/
    world-overlay rendering into a generic UI abstraction (Section F).
11. Do not attempt a full RmlUi replacement of native rendering for camera-composited/
    world-anchored content (Section E) — the hybrid boundary there is permanent, not a migration
    debt.
12. Do not build a generic native "window chrome" base class for the shrinking population of
    not-yet-ported windows — that population is meant to shrink toward RmlUi, not grow a parallel
    framework of its own.

## I. Architectural Rules (for repo dev instructions)

1. **New UI screens are RmlUi documents by default.** A native-only window requires a specific
   reason (3D-camera-composited content, world-space anchoring, or a documented Type-1/Type-2
   RmlUi-companion pattern) — not "it was faster to hand-roll."
2. **`mu::ui::window::CObject`/`CManager` is the only window base/registry.** Never derive a new
   window from `CWin`/`CWinEx`, and never add a new `INTERFACE_*`-keyed registry alongside
   `CManager`'s.
3. **One button/checkbox/radio family for native contexts: `mu::ui::window::CButton`/
   `CCheckBox`/`CRadioButton`.** Do not add a new native button implementation, and do not reach
   for `::CButton : CSprite` or `CUIButton : CUIControl` in new code.
4. **For anything with an RmlUi presentation, controls are RmlUi + `base.rcss`'s shared classes
   (`.btn`, `.checkbox-box`, `.tooltip`).** Do not hand-roll a new native widget for a window that
   already has a document.
5. **All widget-level mouse/key polling goes through `CInput`.** Do not call
   `GetAsyncKeyState()` directly or add a call site to `CNewKeyInput`'s free functions in new
   code.
6. **All coordinate math goes through `UI::Scaling`.** Never introduce a second
   `g_fScreenRate_x`-style global or a hand-rolled reference-resolution scale; add a `LayoutMode`
   case if none of the existing seven fit.
7. **A window with a real 2D rect uses `WindowGeometry` (once built) instead of hand-rolling its
   own bounds/hit-test.**
8. **Tooltips use the one canonical Tooltip primitive (once extracted) or the `.tooltip` RCSS
   convention** — never a new per-window `RenderTooltip()` method.
9. **Theme loading always goes through `UI::RmlBridge::LoadThemedDocument()`**, never
   `Context::LoadDocument` directly; theme-specific behavior is a declared `theme.ini` capability,
   never a hardcoded `GetActiveThemeName() == "..."` branch.
10. **Domain logic (item drag-drop, inventory-panel mutual exclusion, skill-tooltip content
    rules) stays out of the UI kit's own types.** A UI component may call into it; it must never
    be reimplemented inside a generic UI abstraction.
