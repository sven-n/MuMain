# UI Kit Target Architecture — Critical Review & Proposal

Companion to [`ui-architecture-assessment.md`](ui-architecture-assessment.md). That document
inventoried the current, fragmented state; this one independently re-verifies its conclusions
against source (not against its own text) and proposes a target architecture, canonical
components, an RmlUi strategy, and a migration plan. No code has been changed as part of either
document. Produced 2026-09-05.

**Correction, 2026-09-05**: this document originally characterized `CInput` and `CNewKeyInput` as
two independent, potentially-disagreeing input samplers. A follow-up trace of the actual call
chain (`Core::Input::IsKeyDown()` → `CNewKeyInput::ScanAsyncKeyState()` → the
`IsPress`/`IsRelease`/`IsNone`/`IsRepeat` free functions) found that's wrong: `CInput` is built
_on top of_ `CNewKeyInput` (its keyboard queries are pure forwards; its own mouse-button state is
derived by calling the same free functions), not a competing root sampler. Sections A, C, D, H,
and Rule 5 below are corrected accordingly.

**Correction, 2026-09-05 (second pass)**: a cross-check against
[`architecture-principles.md`](rmlui-ui-system/architecture-principles.md) found this document's
first pass overstated the native/RmlUi hybrid boundary as permanent for buttons, chrome, and
sprite-atlas icon rendering generally, when only live 3D-camera-viewport content and world-anchored
overlays actually have no RmlUi equivalent. Sections A, D, E, H, and Rule 3 below are corrected to
frame the native button/chrome population as transitional, not a coequal permanent family — see
Section E for the reasoning. As with the first correction: **where this document and
`architecture-principles.md` ever disagree, that governing doc wins** (it says the same of every
other file in its own directory); this document is a proposal built on top of it, not a peer.

**Correction, 2026-09-06 (third pass)**: Section C originally cited `CMyInventory` alongside
`CCharInfoBalloonMng` as an example of a `CObject` subclass with no static 2D rect, projected via
`WorldToScreen()` each frame. Tracing the actual code found that's wrong: `CMyInventory` owns a
plain static `m_Pos` set via `SetPos()`, used with ordinary fixed offsets for its child controls —
an entirely normal, portable window rect. Its `I3DRenderObj` inheritance
(`UI/Core/Window3DRenderMng.h`) has nothing to do with world-space position; it's a pure render-
Z-order registration so its item-slot icons interleave correctly with 3D-camera render passes
mid-frame — the same mechanism already covering `MainFrameWindow`'s still-legacy skill/pet icons
(Section E, item 3's "2D sprite-atlas icon rendering," temporary and portable, not item 2's
permanent world-anchor case). Section C below is corrected to drop `CMyInventory` from that
example list; `CCharInfoBalloonMng` remains a genuine one.

## A. Architecture Verdict

**The prior assessment's diagnosis is correct and, if anything, understated.** Independently
re-verifying:

- **`CObject` having no geometry is confirmed and worse than implied.** It's not just "no
  `Contains()`" — `CManager`'s own dispatch (`WindowManager.cpp`) proves the design intent: every
  `Update()`/`Render()`/`UpdateMouseEvent()`/`UpdateKeyEvent()` call is wrapped in a
  `ScopedActiveTransform` derived purely from `GetLayoutMode()`, with **zero** reference to any
  bounds on the object itself. The manager provides _only_ a coordinate space; every window is
  trusted to do its own hit-testing inside it. This is a coherent, deliberate contract, not an
  oversight — but it does mean "add a new window" always includes "reinvent your own rect."
- **The `mu::ui::window::CButton` proposal needs real qualification, not a rubber stamp.** Reading
  `Widgets/Window/Button.cpp` in full changes the verdict from the prior assessment's implicit
  "yes, canonicalize this":
  - It does **not** use `CSprite` at all — it renders via a free function `RenderImage()` against
    a global `Bitmaps[]` table, a _third_ rendering primitive alongside `CSprite` and RmlUi,
    re-implementing atlas frame-offset math (`m_CurImgState * m_Size.y`) that `CSprite` already
    solves.
  - It hit-tests via `CheckMouseIn(m_Pos.x, m_Pos.y, m_Size.x, m_Size.y)` — hand-rolled per call
    site rather than an owned rect object. **Correction, 2026-09-06**: this document originally
    also claimed that function "lives in `UIControls.h`, the very legacy `CUIControl` toolkit,"
    making the "new" tier's own button not actually independent of the "old" one. Traced precisely:
    it doesn't — `mu::ui::window::CheckMouseIn` is `WindowCommon.h`'s own free function, in the same
    namespace as `CButton`, visible everywhere via the app's precompiled header; C++'s member-
    function lookup rules find it there and never reach the unrelated global `::CheckMouseIn`
    that actually does live in `UIControls.h`. `Button.cpp`'s real (and legitimate) dependency on
    `UIControls.h` is `g_pRenderText`, the shared text renderer — not this.
  - It polls input via free functions `IsPress`/`IsRelease`/`IsNone` (`UI/Core/WindowCommon.cpp`),
    which wrap `CNewKeyInput`/`g_pNewKeyInput` — confirmed, on tracing the full chain, to be the
    single root input sampler for both keyboard and mouse-button-as-VK state (`Core::Input::
IsKeyDown()`, SDL3-backed — `ScanAsyncKeyState()`'s name is stale, it hasn't called
    `GetAsyncKeyState()` since the SDL3 port). The legacy `::CButton` instead polls
    `CInput::Instance()` — which turns out to be a thin façade _built on the same free functions_
    for its keyboard queries, adding only its own scene-scoped (`LOG_IN_SCENE`/`CHARACTER_SCENE`
    only) mouse-button/double-click/left-hand-mode/cursor-position bookkeeping on top. Not two
    competing samplers, in other words — one root sampler plus one narrow, correctly-scoped
    convenience layer. Neither goes through `CManager`'s own `UpdateMouseEvent()` return-value
    contract at the individual-widget level, though — consumption is purely a dispatch-order
    convention (Section D), not a property either input path enforces itself.
  - It duplicates `EnsureLocaleObserver()`/`OnLocaleChanged()` nearly verbatim three times
    (`CButton`, `CRadioButton`, `CCheckBox`) instead of once on `CBaseButton`.
  - Its behavior forks under `#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM`/
    `KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE` — meaning "the canonical widget" doesn't have one
    behavior, it has up to four, selected at compile time.
  - It does not consult `UI::Scaling`'s `Transform`/`PositionX()`/`PositionY()` at all — it draws
    and hit-tests at raw `m_Pos`/`m_Size`, assuming the caller already resolved real pixels.
    That's _consistent_ with how `CTrade`/`CMyInventory` use it, but it means this "canonical"
    widget has no self-contained scaling story — it inherits whatever discipline its owning
    window happens to apply.

  None of this makes it a bad choice for what it should actually be used for — it's still the
  most-adopted, most-feature-complete of the three C++ button families, and evolving it costs less
  than inventing a fourth. But **calling it canonical as-is would enshrine a widget with a parallel
  rendering path, a parallel input system, and compile-time behavior forks.** Canonicalizing it has
  to include fixing those things — and, per the second-pass correction above, framing it as a
  transitional bridge for the still-native population, not a permanent peer to RmlUi's own button
  convention (see Section D).

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
  treated `CObject`'s lack of geometry as a gap to _fill_. Having now seen `CManager`'s
  transform-per-object dispatch model in full, the more accurate framing is that `CObject` was
  _deliberately_ kept thin, and the correct fix is a **composable geometry component**, not a
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
investigation confirms this would be wrong for a concrete reason, not just taste: at least one
`CObject` subclass (`CCharInfoBalloonMng`) has no static 2D rect at all — its position is a
per-frame `WorldToScreen()` projection. A mandatory geometry field on the base class would be
meaningless for it and would recreate exactly the "why does my window carry fields it never uses"
complaint that motivates unifying in the first place. (**Correction, 2026-09-06**: `CMyInventory`
was previously miscited here too — it has a normal static rect; see the correction note above.)

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

This is deliberately a _value object a window owns_, not a mixin base `CObject` derives from —
matching the instinct to prefer composition where it minimizes coupling. A window that wants it
declares `WindowGeometry m_geometry;` and calls `m_geometry.Contains(...)` from its own
`UpdateMouseEvent()` instead of hand-rolling the same rect math ~88 times. Windows with no
meaningful rect (world-projected overlays) never touch it. `CManager` itself never needs to know
it exists — dispatch stays exactly as it is today.

**Rendering and input stay explicitly out of `CObject` and out of `WindowGeometry`, for the same
reason.** A window's rendering is either `CSprite`-based, RmlUi-based, or both simultaneously
during migration — there is no single "Render" contract narrower than the `bool Render()`
`CObject` already declares that would fit all three without becoming a leaky abstraction. Input
is already unified at the _dispatch_ level (`CManager`'s topmost-first, consume-and-stop loop,
confirmed correct and coherent in `WindowManager.cpp`); the widget-level input-polling API is
already effectively unified too, once `CInput`'s real relationship to `CNewKeyInput` is understood
(Section D) — the remaining question is RmlUi's separate event system, which is fine as its own
thing since it only ever drives RmlUi-owned elements.

## D. Canonical Components

| Concern                                        | Current owners (verified)                                                                                                                                                                                                                             | Canonical owner                                                                                                                                                                                                                                                                                                                                                                                        | Reason                                                                                                                                                                                                                                                                      |
| ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Window lifecycle                               | `CObject`/`CManager`                                                                                                                                                                                                                                  | **`CObject`/`CManager`, unchanged**                                                                                                                                                                                                                                                                                                                                                                    | Already ~88/97 windows; zero live top-level `CWin` subclasses remain                                                                                                                                                                                                        |
| Dispatch / input propagation                   | `CManager`'s depth-sorted, consume-and-stop loop                                                                                                                                                                                                      | **`CManager`, unchanged**                                                                                                                                                                                                                                                                                                                                                                              | Design verified correct and coherent (topmost-first, `ScopedActiveTransform` per object) — **but a real caller-side bug was found and fixed 2026-09-13** (`CMyInventory`'s ground-drop guard returned `false` meaning "not mine, let someone else handle it," when the actual contract is `false` = consumed, stop dispatching entirely; see `STATUS.md`). The contract itself isn't written down anywhere but `WindowManager.cpp`'s own loop: `UpdateMouseEvent()` must return `true` to let dispatch continue to lower-z windows, `false` only to genuinely consume the event and halt it there.                                                                                                                                                          |
| Widget-level input polling                     | `CNewKeyInput`/`g_pNewKeyInput` via the `IsPress`/`IsRelease`/`IsNone`/`IsRepeat` free functions (new-tier widgets, ~15+ files, and `CInput`'s own keyboard queries), `CInput` (legacy `::CButton`, login/char-select only), RmlUi's own event system | **Already `CNewKeyInput`/the free functions — no change needed.** `CInput` stays, scoped to the login/char-select window family only                                                                                                                                                                                                                                                                   | Tracing the chain found `CInput` is built on the free functions, not a competing root sampler — the "duplication" is one root plus one correctly-scoped façade, not two disagreeing systems                                                                                 |
| Geometry / hit-testing                         | Hand-rolled per window (`CWin`'s own, `CUIControl`'s own, ~88 independent rect checks)                                                                                                                                                                | **New opt-in `WindowGeometry` (Section C)**                                                                                                                                                                                                                                                                                                                                                            | No existing implementation is reusable as-is; all are per-family, not per-concern                                                                                                                                                                                           |
| Coordinate transforms                          | `UI::Scaling::UITransform`/`UILayoutPolicy`                                                                                                                                                                                                           | **Unchanged — already canonical**                                                                                                                                                                                                                                                                                                                                                                      | 100% dispatch coverage confirmed in `WindowManager.cpp`; the strongest existing primitive in the codebase                                                                                                                                                                   |
| Buttons                                        | 4 implementations, split further into native/RmlUi contexts                                                                                                                                                                                           | **`base.rcss`'s `.btn` + a documented variant convention, going forward.** The evolved `mu::ui::window::CButton` (after replacing its `RenderImage()` rendering path and hand-rolled `CheckMouseIn(m_Pos.x, ...)` hit-tests with `CSprite`/`WindowGeometry`, and removing its macro-forked behavior) is a **transitional bridge for the still-native population only**, not a coequal permanent family | No hybrid window found actually needs a _primary_ native button — every documented case (`CMsgWin`, `CSysMenuWin`) is a redundant click-detector already being retired; the native family should shrink toward zero, not stand alongside RmlUi as a second permanent target |
| Text input                                     | `CUITextInputBox`                                                                                                                                                                                                                                     | **Unchanged — already canonical**                                                                                                                                                                                                                                                                                                                                                                      | Genuinely shared across every generation including RmlUi-hybrid windows; don't touch what already works                                                                                                                                                                     |
| Tooltips                                       | 4 mechanisms                                                                                                                                                                                                                                          | **New standalone `mu::ui::window::CTooltip` primitive** (native, transitional — namespaced with the rest of that tier, not `UI::Tooltip` as this document originally said) + **`base.rcss`'s `.tooltip` convention** (RmlUi, the long-term target)                                                                                                                                                     | Same transitional-vs-permanent split as buttons; extract from `mu::ui::window::CButton`'s existing anchor/position logic rather than inventing new math                                                                                                                     |
| Window chrome                                  | None — every window reimplements                                                                                                                                                                                                                      | **Not unified natively.** For RmlUi-reachable windows, chrome is a theme/RCSS concern (already true). For native-only windows, not worth building — see Section F                                                                                                                                                                                                                                      | Building a generic native chrome abstraction for a shrinking population (windows not yet, and possibly never, ported) is negative-value work                                                                                                                                |
| Layout/scaling                                 | `UILayoutPolicy`                                                                                                                                                                                                                                      | **Unchanged — already canonical**                                                                                                                                                                                                                                                                                                                                                                      | Same as above                                                                                                                                                                                                                                                               |
| Rendering                                      | `CSprite`, raw `RenderImage()`, RmlUi                                                                                                                                                                                                                 | **`CSprite` for native visuals, RmlUi for ported visuals — retire the second `RenderImage()` path**                                                                                                                                                                                                                                                                                                    | `RenderImage()` re-derives what `CSprite`'s frame/atlas system already does; no reason for a second implementation                                                                                                                                                          |
| Theme                                          | `UI::RmlBridge::LoadThemedDocument()`/`GetActiveThemeName()`/capability flags                                                                                                                                                                         | **Unchanged — already canonical**                                                                                                                                                                                                                                                                                                                                                                      | Independently verified well-designed: name-agnostic, capability-flag-gated, single required entry point                                                                                                                                                                     |
| Window-repositioning drag                      | `CWin::SetMovable()` (legacy, inert now) / `MakeDraggable()` (RmlUi, unused)                                                                                                                                                                          | **`MakeDraggable()`, once it gets a first real caller**                                                                                                                                                                                                                                                                                                                                                | Well-designed, narrowly scoped, honestly documents its own two open gaps                                                                                                                                                                                                    |
| Item-slot drag/drop                            | `CInventoryActionController`                                                                                                                                                                                                                          | **Unchanged, explicitly not part of the UI kit**                                                                                                                                                                                                                                                                                                                                                       | Domain logic wearing UI clothing — see Section F                                                                                                                                                                                                                            |
| Open/close mutual exclusion (inventory family) | `CUIManager`                                                                                                                                                                                                                                          | **Unchanged, rename its `INTERFACE_*` enum values only**                                                                                                                                                                                                                                                                                                                                               | Legitimate, narrow domain policy; the only real defect is the name collision with the tier's own registry key                                                                                                                                                               |

`architecture-principles.md` §20 also names `Panel`/`Dialog`/`ScrollContainer`/`HUDContainer` as
target reusable components — deliberately not a row above. RmlUi's own DOM composition already
gives any ported window real containment for free; the only population that would need a _native_
container abstraction is the same transitional, shrinking native-only population the Buttons row
above describes. Building one now would be investing in a population this plan is actively
retiring, not filling a permanent gap.

## E. RmlUi Strategy

**Answer: C (hybrid) — but the permanent slice of that hybrid is narrower than this document
originally claimed.** Correcting an overreach from the first pass: "camera-composited" was used
to cover two genuinely different things, only one of which is actually permanent.

**Permanent, not a migration debt** — content with no RmlUi equivalent at all:

1. **Live 3D-camera-viewport content** (e.g. `CCharMakeWin`'s real-time character-preview panel,
   driven by direct `BeginOpengl()` calls into the 3D pipeline) — RmlUi has no primitive for
   compositing a live camera render into its layout, and nothing in `architecture-principles.md`
   anticipates this case (a real, honest gap in the governing doc, not something this proposal can
   resolve). The background-layer flush mechanism (`IMuRenderer::FlushRenderCommands()`,
   `RmlUiRuntime::GetBackgroundContext()`/`RenderBackgroundLayer()`) makes RmlUi content interleave
   _around_ this kind of content mid-frame; it doesn't make the 3D content itself portable.
2. **World-anchored content** (`CCharInfoBalloonMng`, name tags) — fundamentally a
   `WorldToScreen()` projection problem, not a layout problem; RmlUi's box model doesn't help
   there regardless of migration completeness.

**Temporary, sequenced — not permanent, despite looking similar at a glance:** 3. **2D sprite-atlas icon rendering** (the skill-grid/pet-row icons `MainFrameWindow.cpp` still
draws as legacy 2D sprites). `STATUS.md` itself calls this "a deliberate Phase 2 scope cut,"
blocked on `RenderSkillIcon()`'s irregular atlas addressing, not on any RmlUi limitation — this
is a decoded-texture-atlas problem, not a live-3D-compositing one. It's expected to retire into
RmlUi (most likely via generated `@spritesheet` rects, the same mechanism `STATUS.md` already
documents solving an equivalent atlas case elsewhere) once that port lands. Treating it as
permanent, as this document's first pass did, would wrongly justify never doing that port.

So the durable boundary is **"can this content be represented as a live 3D render target or a
world-space projection at all,"** not "is this currently hard to port":

```
RmlUi owns (eventually all of this):     Native C++ owns permanently:
  - all static 2D chrome/layout/text       - live 3D-camera-viewport content
  - buttons/checkboxes/text                  (e.g. character-preview panels)
  - tooltips                               - world-anchored overlays (name tags,
  - icon/sprite-atlas art (once the           character-info balloons) -- WorldOverlay
    atlas port lands)                         transform, not a layout concern
```

A ported window keeps exactly one native companion pattern, not native widgets generally: a
**Type-2 companion object** (a real interactive widget like `CUITextInputBox` that RmlUi can't
yet host) or a **Type-1 redundant click-detector** (a `CButton` doing pure bounding-box
click-consumption behind RmlUi's real click handling, kept only because the legacy activation
gate was unreliable — already documented and already being retired as windows get their own
direct `RmlClickX()` handlers). Neither is "native UI persisting because RmlUi migration
stalled" — both are documented, bounded exceptions. New windows should not add a third kind of
native companion without a specific, new reason as strong as those two. Neither pattern requires
the evolved native `CButton` family to be a permanent fixture — a Type-1 click-detector is exactly
the kind of use that disappears as windows get direct `RmlClickX()` handlers, consistent with
Section D's "transitional bridge, not a coequal family" framing.

## F. Non-Unified Areas

Explicitly kept separate, with the concrete reason each earns it:

- **`CInventoryActionController`/item drag-drop** — this is inventory _business logic_
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
  _rendering_ should converge on the one canonical Tooltip primitive (Section D), but the domain
  logic that decides _what a skill tooltip says_ is correctly separate and should stay that way,
  same reasoning as item drag-drop.

## G. Developer Usage Model

Concretely, for _"a settings window with a title, three tabs, labels, checkboxes, a text field, a
slider, and a tooltip"_ — the answer genuinely forks on one question first, because that's what
the evidence in Section E demands:

**Does this window need any live 3D-camera-viewport or world-anchored content?** No, for a settings
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

**If** the window had a live 3D-camera-viewport element (it doesn't, but for contrast — e.g. an equipment
preview panel), only _that_ element stays native: a `CSprite`/3D-camera-rendered sub-view
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
2. Document, in the repo's dev instructions (Section I), that RmlUi + `base.rcss` is canonical for
   anything with a presentation layer, `mu::ui::window::CButton`/`CCheckBox`/`CRadioButton` is the
   transitional bridge for the still-native population only, and nothing else — so no new window
   picks a fourth path by default.

**Needed before further native UI development continues at any real pace:**

3. ~~Consolidate `CNewKeyInput` and `CInput`~~ — **resolved by investigation, no code change
   needed.** `CInput` is already built on `CNewKeyInput`'s free functions; the only action item is
   documenting `CInput`'s scope (Rule 5) so no future `MAIN_SCENE` or non-UI code reaches for its
   stale-outside-login/char-select mouse/cursor state.
4. ~~Extract the Tooltip primitive out of `mu::ui::window::CButton::ChangeToolTipText`'s existing
   logic into a standalone, attachable component.~~ — **done, commit `b9b667b9`.** Landed as
   `mu::ui::window::CTooltip` (`UI/Widgets/Window/Tooltip.h`/`.cpp`): owns no position of its own,
   takes the anchor rect fresh on each `Render()` call the same way `CButton`'s inline version did.
   `CButton` owns one as a member and forwards `ChangeToolTipText()`/etc. into it; every pre-existing
   call site is unchanged. `CRadioButton`/`CCheckBox` never had tooltip logic to begin with, so
   there was nothing to migrate there for this item. Adoption elsewhere (the other ~3 tooltip
   mechanisms this document's Tooltips row counts — `RenderTipTextList()`'s many multi-line call
   sites, `CItemEnduranceInfo::RenderTooltip()`'s one-off hand-rolled version, and the RmlUi
   `.tooltip` convention currently duplicated per-theme-file rather than centralized in
   `base.rcss`) is real follow-up value but a separate, unscoped effort — not part of this item.
5. ~~Build the opt-in `WindowGeometry` component (Section C) and start using it in any window
   touched for other reasons.~~ — **done.** The component landed first (commit `e7b91353`), then
   adopted by `CBaseButton::IsMouseIn()` (H6). Broadened to every other new-tier own-top-level-rect
   `mu::ui::window::CheckMouseIn()` call site — 73 call sites across 57 files (commit `cecd25eb`).
   Deliberately left untouched: the legacy `CUIControl`/`CWin` family's differently-named
   `::CheckMouseIn(x, y, w, h, CoordType)` (a different, older component tier with a
   bottom-anchored mode `WindowGeometry` has no equivalent for), and inline sub-rect checks inside
   a window body (per-tab/per-row/per-icon/scrollbar-thumb regions) — `WindowGeometry`'s own scope
   is a widget's own top-level rect, not every ad hoc hit-test a window performs internally.

**Can wait (real value, not blocking):**

6. ~~Retire `mu::ui::window::CButton`'s `RenderImage()` path in favor of `CSprite`, and its
   hand-rolled `CheckMouseIn(m_Pos.x, m_Pos.y, m_Size.x, m_Size.y)` call sites in favor of the new
   `WindowGeometry`~~ — **done, broadened to the whole transitional family**
   (`CButton`/`CRadioButton`/`CCheckBox`, per Rule 3's own grouping, not deferred piecemeal).
   Part 1 (`CheckMouseIn` → `WindowGeometry`) and part 2 (`RenderImage()` → `CSprite`) both landed;
   `CButton::Render(true)`'s single-caller `MiniMap.cpp` UV-crop special case deliberately keeps
   `RenderImage()`, since `CSprite` has no "clip to an explicit UV fraction" primitive. `CSprite`
   turned out to need real care to drive correctly from here: it takes LOGICAL/reference-resolution
   coordinates at `SetPosition()`/`SetSize()` and applies the active transform's scale *and* the
   live screen offset itself, once, inside `Render()` — pre-scaling inputs while keeping it at
   identity scale (an early attempt at this) double-applies the offset. It also bakes both that
   scale and its internal Y-flip's `WindowHeight` basis in at `Create()` time with no live updater,
   so a widget must rebuild the sprite whenever `(imgIndex, frameCount, size, WindowHeight,
   scaleX, scaleY)` change, not just the first three.
7. Implement new RmlUi-representable UI screens on the RmlUi path (Section G) by default.
   Migrate existing native-only screens to RmlUi opportunistically when they are being substantially modified for another reason, rather than performing a blanket rewrite. Every such migration must follow the per-UI migration requirements in [`architecture-principles.md`](rmlui-ui-system/architecture-principles.md), especially §§2–6 and §27: first establish the existing UI's layout intent, distinguish that intent from legacy implementation artifacts, identify its component/layout hierarchy and responsive behavior, then express the result declaratively through RML/RCSS. Do not mechanically translate legacy coordinates or rendering calls into RmlUi.
8. Deprecate and eventually remove `::CButton : CSprite`, `CUIButton : CUIControl`, and the
   global `::CRadioButton` once grep shows zero remaining callers. The evolved
   `mu::ui::window::CButton` family itself is next once its own remaining callers (the
   not-yet-ported window population) reach zero — it is not exempt from this same trajectory.
8b. **Newly found, 2026-09-13: port `CUITextListBox<T>` consumers to RmlUi `data-for`.** Unlike
    `CUIButton`, no rule anywhere named this class before now — and it shows: `CGuildInfoWindow`
    (`CUINewGuildMemberListBox`), `CMixInventory` (`CUISocketListBox`/`CUIUnmixgemList`), and
    `CInGameShop` (`CUIInGameShopListBox`/`CUIBuyingListBox`/`CUIPackCheckBuyingListBox`) are all
    already on `mu::ui::window::CObject` yet still reach into this legacy family for their list
    content, because nothing told them not to. `CMyQuestInfoWindow`'s `data-for` port off
    `CUICurQuestListBox`/`CUIQuestContentsListBox` is the proven reference (same pattern
    `CBuffStrip` established for a simpler array). ~18 `CUITextListBox<T>` subclasses remain
    (`UI/Widgets/UIControls.h`) spanning guild/chat/letter/socket/in-game-shop/move-command lists —
    see `STATUS.md`'s tracked-deferral entry for the full list. Also found while investigating this:
    `CUIPopup`/`CUIButton`'s remaining live path (`WSclient.cpp`'s generic server-error popups)
    duplicates `CCommonMessageBox`'s job — moving those call sites there retires `CUIPopup`, and
    with it `CUIButton`'s only other confirmed-live consumer besides the suspected-dead
    `CUIGuildInfo`/`CUIGuildMaster` (see `building-new-ui.md`).

**Should never be done:**

9. Do not add geometry/rendering/input/styling fields to `CObject` itself (Section C) — the thin
   base class is correct, not a gap.
10. Do not fold `CInventoryActionController`, `CUIManager`'s domain policy, or 3D-camera/
    world-overlay rendering into a generic UI abstraction (Section F).
11. Do not attempt a full RmlUi replacement of native rendering for **live 3D-camera-viewport
    content or world-anchored overlays** (Section E) — that narrower boundary is permanent. Do
    _not_ extend this exemption to sprite-atlas icon rendering or to native buttons/chrome
    generally — those are transitional and should keep shrinking (items 6–8 above).
12. Do not build a generic native "window chrome" base class for the shrinking population of
    not-yet-ported windows — that population is meant to shrink toward RmlUi, not grow a parallel
    framework of its own.

**Needed to complete UI-kit coherence:**

13. Normalize the canonical UI surface.

    Once the canonical primitives are stable, reorganize or facade them so
    developers encounter one obvious component family rather than historical
    header/layout boundaries.

    Do not mass-rename solely for aesthetics. Prefer compatibility aliases or
    forwarding headers where migration risk is high.

14. Publish the canonical component catalog.

    Document the supported primitive for each reusable UI responsibility and
    explicitly mark superseded implementations.

15. ~~Establish reference screens.~~ — **done, 2026-09-07.** One canonical example named per
    shape, each already built, verified against a real server, and cited elsewhere in this doc set
    — not new code, a naming/pointing exercise:

    - **RmlUi-only 2D UI** — `CMsgWin` (`UI/Windows/MsgWin.h`), an ordinary screen-anchored modal
      with no `CWin`/`CSceneUICoordinator` involvement at all (`component-catalog.md`'s "Pure
      RmlUi" bucket). `RememberPasswordPrompt` (`UI/Windows/RememberPasswordPrompt.h`) is the
      variant to copy instead for a window with no reusable state worth a class — a free-function
      module in `namespace UI::Login`.
    - **Native-only UI** — `CFriendWindow` (`UI/Party/FriendWindow.h`), already called out in
      `building-new-ui.md` as the template for wrapping a live legacy subsystem
      (`CUIWindowMgr`/friend/mail/chat-room) behind a thin `mu::ui::window::CObject` adapter whose
      methods forward one-to-one. Picked over a plain not-yet-touched `CObject` window because it's
      already been audited as correct, not because native windows without an RmlUi presentation are
      themselves a pattern to keep building — Rule 3 still applies: this shape is transitional and
      shrinking, not a peer to the other three.
    - **Hybrid RmlUi/native 3D UI** — `CItemHotKey` (welded into `UI/HUD/MainFrameWindow.h/.cpp`,
      Phase 3 of that file's 3-phase port): RmlUi owns the potion-slot hover-highlight border,
      stack-count text, and right-click-to-use; the potion icon itself stays a genuine live 3D
      model render (`RenderItem3D()`/`RenderObjectScreen()`), the permanent Section E boundary this
      whole document is built around. The same split `CSkillList` (Phase 2) already proved for
      skill icons — not a one-off.
    - **World-overlay UI** — `CCharInfoBalloonMng` (`Character/CharInfoBalloonMng.h`), already
      this document's own Section C example of a window with no static 2D rect at all: its position
      is a per-frame `WorldToScreen()` projection, RmlUi-presented, no legacy widget members.

    New UI should follow these reference implementations rather than copying arbitrary neighboring
    legacy windows — cross-referenced from `building-new-ui.md` and `README.md` so a new session
    finds this list from either entry point.

16. Enforce deprecation boundaries.

    Deprecated widget families may remain while old screens migrate, but:
    - no new call sites
    - no new features added to them
    - no new components derived from them

17. Define and track migration exit criteria.

    The migration is complete when every common UI concern has one canonical
    implementation or an explicitly documented presentation-specific split,
    and developers can discover the correct component without historical
    knowledge of the codebase.

    **Concrete instance, 2026-09-13**: `UIControls.h`'s `CUIControl` family is not a permanent
    third toolkit — it's a fully enumerable, closeable checklist. It reaches zero consumers and
    can be deleted outright (the same treatment `CWin`/`::CButton`/`CGaugeBar`/`CSlider` already
    got) once: (a) `CUITextInputBox`'s callers move to RmlUi's own native `<input>`/`<textarea>`
    (a separate, unscheduled design effort — IME composition through RmlUi's DOM is the open
    question, per `building-new-ui.md`); (b) every `CUITextListBox<T>` subclass ports to `data-for`
    (item 8b above); (c) `CUIButton`'s remaining consumers (`CUIPopup`, and the suspected-dead
    `CUIGuildInfo`/`CUIGuildMaster`) are retired or deleted; (d) `CUIWindowMgr`/`CUIBaseWindow`
    (friend/mail/chat-room) either gets a real RmlUi port or is explicitly re-affirmed as staying
    native indefinitely. None of these are blocked on anything else — they're independent, and
    (a) has no target date by design. "Is `CUIControl` necessary" should be answered against this
    list, not treated as an open architectural question.

## I. Architectural Rules (for repo dev instructions)

1. **New UI screens are RmlUi documents by default.** A native-only window requires a specific
   reason (live 3D-camera-viewport content, world-space anchoring, or a documented Type-1/Type-2
   RmlUi-companion pattern) — not "it was faster to hand-roll."
2. **`mu::ui::window::CObject`/`CManager` is the only window base/registry.** Never derive a new
   window from `CWin`/`CWinEx`, and never add a new `INTERFACE_*`-keyed registry alongside
   `CManager`'s.
3. **For the still-native population, one transitional button/checkbox/radio family:
   `mu::ui::window::CButton`/`CCheckBox`/`CRadioButton`.** Do not add a new native button
   implementation, and do not reach for `::CButton : CSprite` or `CUIButton : CUIControl` in new
   code. Treat this family itself as something to migrate away from as its windows port to RmlUi
   (Rule 1), not as a permanent second component system.
4. **For anything with an RmlUi presentation, controls are RmlUi + `base.rcss`'s shared classes
   (`.btn`, `.checkbox-box`, `.tooltip`).** Do not hand-roll a new native widget for a window that
   already has a document.
5. **All widget-level mouse/key polling goes through `mu::ui::window::IsPress`/`IsRelease`/
   `IsNone`/`IsRepeat`** (the `CNewKeyInput`-backed free functions, `UI/Core/WindowCommon.h`) —
   the single, always-updated, all-scene input source, already SDL3-backed (not
   `GetAsyncKeyState()`, despite `ScanAsyncKeyState()`'s stale name). **`CInput::Instance()` is
   reserved for the login/character-select window family's own real-pixel needs** (double-click
   detection, left-hand-mode swap, raw untransformed cursor position) — its mouse/cursor state is
   only updated during `LOG_IN_SCENE`/`CHARACTER_SCENE` and is stale everywhere else, so do not
   reach for it from `MAIN_SCENE` or non-UI code; its `IsKeyDown()`/`IsKeyHeldDown()` forwards are
   fine to use if already in scope, but new code should call the free functions directly.
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
11. **For scrollable list/row content, use RmlUi's `data-for` binding, not `CUITextListBox<T>`.**
    `CBuffStrip`'s buff-icon strip and `CMyQuestInfoWindow`'s quest list are the proven references.
    Do not derive a new `CUITextListBox<T>` subclass and do not add a new consumer to an existing
    one, even from a window already on `mu::ui::window::CObject` — that base class alone doesn't
    make a window RmlUi-native, and reaching into this family for list content is exactly how
    `CGuildInfoWindow`/`CMixInventory`/`CInGameShop` ended up depending on it despite being on the
    modern base class otherwise. See `STATUS.md`'s tracked-deferral entry for the full remaining
    consumer list.
