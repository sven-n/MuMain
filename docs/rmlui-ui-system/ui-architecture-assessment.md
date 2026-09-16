# UI Architecture Assessment — MU Online Client

> **Point-in-time snapshot, 2026-09-05 — not kept current.** Three sections were later found to
> need correction; see [`ui-target-architecture.md`](ui-target-architecture.md)'s "Correction,
> 2026-09-05/06" notes in Sections A and C before trusting this document's claims about `CInput`/
> `CNewKeyInput`, `CMyInventory`'s geometry, or the native/RmlUi permanent-boundary framing. Other
> facts here have also since changed without an inline correction (e.g. `RmlDraggable`/
> `MakeDraggable()`'s "zero live call sites" claim below — false since 2026-09-07, see
> [`STATUS.md`](STATUS.md)). Treat this file as historical evidence for how the target architecture
> in `ui-target-architecture.md` was derived, not as a source of current facts — check `STATUS.md`
> for those.

A point-in-time architectural assessment of `src/source/UI/` and its dependencies, done by direct
code inspection (base classes, `#include` graphs, live call sites via grep) rather than by reading
this project's own status docs, which were deliberately treated as unverified claims and checked
independently. Produced 2026-09-05, ahead of any decision to unify the UI system further — see
[`newui-legacy-merger.md`](newui-legacy-merger.md) (window-ownership unification, already done) and
this directory's own [`README.md`](README.md) (the RmlUi presentation-layer migration, in progress)
for the initiatives this assessment evaluates the current state against.

## Executive Summary

**Fragmented, with one axis of real, load-bearing unification and several axes of unresolved
duplication.**

Window *ownership and lifecycle* has one real answer today: `mu::ui::window::CObject`/`CManager`
(`UI/Core/WindowObject.h`/`WindowManager.h`). ~88 of the ~97 windows found already derive from it,
and the historical alternative (`CWin`, `UI/Widgets/Win.h`) has zero remaining top-level
subclasses — it survives only as a composed widget inside a few migrated windows. That axis is
genuinely, not superficially, unified.

Everything *below* that base class — buttons, text fields, radio buttons, checkboxes, tooltips,
progress bars, geometry/hit-testing, rendering — is not. This assessment found **four independent
button implementations**, **two independent radio-button implementations** (one of which needs
explicit `::` qualification in source to disambiguate from the other, confirmed in
`UIControls.h`), **at least four distinct tooltip mechanisms**, **zero shared base class for
window chrome** (background/bounds/close-button — every window reimplements this itself), and
**three parallel window-content-rendering technologies** (legacy `CSprite`/GDI-descended drawing, a
`CUIControl`-composition toolkit, and RmlUi markup) coexisting per-window with no adapter layer
unifying them except a manually-maintained per-`INTERFACE_*` case table.

A new UI developer today has to *know*, not infer, which of at least three widget families and
three tooltip patterns to copy for a new window — nothing in the type system nudges them toward
one.

## Current UI Architecture

Three structurally distinct systems currently coexist, each with its own base class, geometry
model, and widget family:

**1. `mu::ui::window` tier** (`UI/Core/WindowObject.h`, `WindowManager.h`) — the de facto standard.
`CObject`/`IObject` is a pure lifecycle interface: `Render()`/`Update()`/`UpdateMouseEvent()`/
`UpdateKeyEvent()`, `IsVisible()`/`IsEnabled()`, a shown-vs-active split, `GetLayerDepth()`/
`GetKeyEventOrder()` for `CManager`'s depth-sorted dispatch. **Critically, it has no geometry at
all** — no position, no size, no `Contains()`/hit-test. Every subclass (`CTrade`, `CMyInventory`,
`CMainFrameWindow`, ~85 others) reimplements its own bounding-box math and click detection
independently. `CManager` is a flat `DWORD`-keyed registry (`AddUIObj`/`FindUIObj`/
`ShowInterface`), not a scene graph — no parent/child containment.

**2. `CWin`/`CWinEx` tier** (`UI/Widgets/Win.h`/`WinEx.h`) — the historical base class. Unlike
`CObject`, it *does* own real geometry (`m_ptPos`/`m_Size`/`CursorInWin()`), a button list
(`CPList m_BtnList`), and a background sprite. No live window derives from it as a top-level base
anymore (verified via grep — the only subclass is `CWinEx`→`RememberPasswordPrompt`, itself
composed inside a `CObject` window, not used for dispatch). Still alive purely as an optional
composed member (e.g. `CServerSelWin::m_winDescription`).

**3. `CUIControl` composition toolkit** (`UI/Widgets/UIControls.h`) — a third, independent
geometry/message system (`SetPosition`/`SetSize`/`SendUIMessage`/`HandleMessage`, its own
`UISTATE_*`/`UI_MESSAGE_*` enums), unrelated to both of the above. Home to `CUITextInputBox` (the
real text-input widget, still used everywhere including RmlUi-hybrid windows), ~20
`CUITextListBox<T>` specializations, and a global (non-namespaced) `CRadioButton` that renders via
raw `GLuint` texture calls, bypassing `CSprite` entirely.

**4. RmlUi layer** (`UI/RmlBridge/`, `Render/RmlUi/`) — a fourth, additive rendering/interaction
system. `RmlModelBinder<T>` binds a C++ struct to a `data-model`; `RmlTheme.cpp` does `token(...)`
substitution and per-theme RML forking; `RmlDraggable.h` is a **built but never-called** generic
drag helper (zero live call sites, confirmed). This layer sits *on top of* a `CObject` window (a
window owns both an `Rml::ElementDocument` and its legacy `CButton`/`CSprite` members
simultaneously during migration), not as a replacement for the widget layer below it.

**5. `CUIManager`** (`UI/Core/UIManager.h`/`.cpp`, `g_pUIManager`) — a fifth, orthogonal system
nothing in the folder name suggests. It owns no windows and no lifecycle; it's a mutual-exclusion
policy tracker (`Open()`/`Close()`/`IsCanOpen()`) for the inventory-family panels, keyed on its
**own private `INTERFACE_*` enum** that duplicates names (`INTERFACE_INVENTORY`,
`INTERFACE_TRADE`) from the unrelated, namespaced `mu::ui::window::INTERFACE_LIST` enum in
`Core/Globals/_enum.h` used by `CManager`'s registry. Two enums, same names, same rough subject, no
relationship — distinguished only by namespace qualification.

**Cross-cutting services**, each reasonably coherent in isolation: `UI::Scaling::UITransform` (a
real, generalized `LayoutMode` enum — `Legacy`/`WorldOverlay`/`Hud`/`Dialog`/`DockLeft`/
`FloatingWorkspace` — resolved centrally in `UILayoutPolicy.cpp`'s one `ForInterface()` switch);
`CUIRenderText`/`g_pRenderText` (one singleton text renderer, actually shared); `CSprite` (one
sprite-rendering base, though it does **not** consult the active transform's scale, only its
offset — a documented, load-bearing asymmetry between text and sprite rendering that every window
author has to know about, not something the type system enforces).

## UI Architecture Map

```
                         ┌────────────────────┐
                         │  UI::Scaling        │  (shared, coherent)
                         │  UITransform /      │
                         │  UILayoutPolicy     │
                         └─────────┬──────────┘
                    consulted by (inconsistently) ↓
┌───────────────┐   ┌──────────────────────┐   ┌───────────────────┐   ┌──────────────┐
│ CWin / CWinEx │   │ mu::ui::window        │   │ CUIControl          │   │ RmlUi layer  │
│ (Widgets/     │   │ ::CObject / CManager  │   │ toolkit             │   │ (RmlBridge / │
│ Win.h)        │   │ (Core/WindowObject.h) │   │ (Widgets/           │   │ Render/RmlUi)│
│ - own geometry│   │ - NO geometry         │   │  UIControls.h)      │   │ - documents  │
│ - CButton     │   │ - CButton (2nd impl,  │   │ - own geometry      │   │   composed   │
│   (sprite)    │   │   Widgets/Window/)    │   │ - CUITextInputBox   │   │   INTO a     │
│ - only alive  │   │ - CRadioButton (2nd   │   │   (shared, reused   │   │   CObject    │
│   as composed │   │   impl, needs ::      │   │   by everything     │   │   window     │
│   member now  │   │   qualification vs    │   │   incl. RmlUi rows) │   │ - RmlDraggable│
│               │   │   the other one)      │   │ - CRadioButton (1st │   │   unused     │
│               │   │ - CCheckBox, tooltips │   │   impl, global,     │   │              │
│               │   │   baked into CButton  │   │   raw GL calls)     │   │              │
└───────────────┘   └───────────┬───────────┘   └─────────────────────┘   └──────┬───────┘
                                 │ ~88 concrete windows                          │ composed into
                                 │ (Character/Combat/Dialogs/Events/HUD/          │ ~9 windows so far
                                 │  Inventory/NPCs/Options/Party/Quests/          │ (login/char-select
                                 │  Windows/)                                     │  + MainFrameWindow
                                 ▼                                                ▼  partial)
                    ┌─────────────────────────┐
                    │ CUIManager (5th system)  │  open/close mutual-exclusion
                    │ own INTERFACE_* enum,    │  for inventory-family panels
                    │ collides by name with    │  only — not a window owner
                    │ mu::ui::window's own     │
                    └─────────────────────────┘
```

No arrow above represents composition through a shared abstract "Widget" or "Container" base —
every horizontal relationship is "happens to be included by," not "derives from a common
interface."

## Competing Implementations

| Concept | Implementations found | Representative classes |
|---|---|---|
| Button | **4 distinct implementations** | `::CButton : CSprite` (legacy sprite button, `Widgets/Button.h`); `mu::ui::window::CButton : CBaseButton` (event-state button with built-in i18n slots + tooltip text, `Widgets/Window/Button.h`); `CUIButton : CUIControl` (`UIControls.h`); ad hoc inline button-like sprites drawn directly in some windows without any button class at all |
| Radio button | **2 distinct implementations**, name-colliding | global `::CRadioButton` (`UIControls.h`, raw `GLuint` rendering) vs `mu::ui::window::CRadioButton : CBaseButton` (`Widgets/Window/Button.h`) — `IGS_SelectBuyItem::m_RadioBtn` in `UIControls.h` needs explicit `::CRadioButton` because both are reachable in the same translation unit |
| Checkbox | 1 in the new tier (`mu::ui::window::CCheckBox`), plus ad hoc RCSS `.checkbox-box` styling in RmlUi with no shared C++ counterpart | `Widgets/Window/Button.h`; `themes/modern/base.rcss` |
| Text input | Actually unified — `CUITextInputBox` (`UIControls.h`) is the one real text-field implementation, reused by both legacy and RmlUi-hybrid windows | `CUITextInputBox` |
| Progress/gauge bar | 1 implementation, but window-geometry-model-independent (its own private `Rect` struct, not `CWin`'s or `CObject`'s) | `CGaugeBar` (`Widgets/GaugeBar.h`) |
| Tooltip | **≥4 mechanisms, no shared primitive** | `UI::Skills::Tooltip::Render()` (free-function namespace, skill-specific); `mu::ui::window::CButton::ChangeToolTipText()` (baked into the button widget); per-window `RenderTooltip()` methods (`ChatInputBox.cpp`, `ItemEnduranceInfo.cpp`, each independently drawn); RmlUi `.tooltip` class + `:hover ~ .tooltip` CSS pattern, copy-pasted near-identically per themed `.rcss` file (`mu_helper_bar.rcss`, `main_frame.rcss`) with the same shrink-to-fit workaround re-derived in each |
| Window chrome (background/bounds/close) | No shared abstraction at either tier; `CWin` at least centralized `RegisterButton`/`RenderButtons`/a background sprite for its own subclasses, but `CObject` provides none of this — every one of the ~88 windows reimplements background+bounds+click-consumption itself | `WindowObject.h` (nothing); contrast with `CWin`'s own (retired) `m_psprBg`/`m_BtnList` |
| Drag/drop | 2 unrelated mechanisms under one word | window-repositioning: legacy `CWin::SetMovable()`/`CursorInWin(WA_MOVE)` vs RmlUi's `MakeDraggable()` (built, zero live callers); item-slot drag-drop: `CInventoryActionController::TryDropItem`/`IInventoryActionContext` — a third, domain-specific system unrelated to either window-drag mechanism |
| Interface-open bookkeeping | 2 unrelated `INTERFACE_*` enums, same names | `mu::ui::window::INTERFACE_LIST` (`_enum.h`, window-registry key) vs `CUIManager::INTERFACE_*` (`UIManager.h`, open/close mutual-exclusion flag) |
| Layout/scaling | Actually unified — one real policy table | `UI::Scaling::UITransform`, `UILayoutPolicy::ForInterface()` |

## Inconsistencies

- **A developer adding a new button has no canonical answer.** All four implementations are live
  and reachable from currently-compiling code; nothing marks one deprecated. The choice today is
  whichever family the surrounding file already happens to use, discovered by reading neighboring
  code, not by any documented convention.
- **Geometry ownership is inconsistent by tier, not by need.** `CWin` and `CUIControl` both carry
  position/size/hit-testing on the base class; `CObject` — the tier essentially every current
  window is on — carries none, pushing that responsibility down into every individual window. This
  isn't a deliberate simplification for a stateless tier; `CObject` windows are exactly as
  rect-based as the others, they just each reinvent the rect.
- **Two same-named, unrelated `INTERFACE_*` enums** is a genuine correctness hazard, not just an
  aesthetic one — an unqualified `INTERFACE_INVENTORY` compiles successfully against whichever one
  is in scope via `using namespace`, silently picking the wrong system if a future edit changes
  what's open in a file.
- **Tooltip logic is duplicated by shape, not by name** — each of the four mechanisms
  independently solved "show text near the cursor/anchor after a hover delay," including
  independently rediscovering the same RmlUi shrink-to-fit/transform hazard in at least two
  separate `.rcss` files (confirmed: `mu_helper_bar.rcss` and `main_frame.rcss` both carry the
  identical hand-written workaround).
- **`CSprite` vs text-render's transform contract is asymmetric and undocumented in the type
  system** — `CSprite::Render()` silently ignores the active transform's scale (only applies
  offset), while text rendering fully applies it. A window author moving a value from a
  sprite-based to a text-based widget (or vice versa) can get a silently wrong result with no
  compiler signal.

## Duplicate / Missing Abstractions

- **Missing: a `CObject`-tier geometry/hit-test mixin.** Every one of ~88 windows reimplements its
  own bounding-rect Contains-check. This is a real, present-tense duplication, not a legacy one,
  since it's happening in the *current* standard tier, not just old code.
- **Missing: a shared window-chrome base** (background fill, standard close-button wiring, title
  text) — `CWin` had a partial version of this (`RegisterButton`/`RenderButtons`/`m_psprBg`) that
  was never carried forward into `CObject`, so the newer, more-used tier is actually *less*
  equipped for this than the one it replaced.
- **Missing: a unified Tooltip primitive** — four independent, working implementations is strong
  evidence the abstraction is wanted repeatedly, not that it's unneeded.
- **Legitimate specialization, not duplication:** `CUITextInputBox` being reused as-is by both
  legacy and RmlUi windows is the one case in this codebase actually behaving like a shared toolkit
  primitive should — worth naming as the positive counter-example.
- **Accidental duplication, not architecturally motivated:** the two `INTERFACE_*` enums and the
  two `CRadioButton`s look like independent, unaware-of-each-other authorship rather than any
  considered design split.

## Architectural Generations

Three generations are identifiable by dependency direction, not by naming:

1. **Generation 1 — `CWin`/`CWinEx`/`CUIControl` (MFC-era).** Own geometry, own message pump
   (`UI_MESSAGE_*`), GDI-flavored (`HFONT`, `HDC` in `CUIRenderText::Create(HDC)`). Confirmed
   retired as a *base class* for dispatch — zero live top-level subclasses — but its *widgets*
   (`CUITextInputBox`, `CWinEx` as a composed member, the global `CRadioButton`) are still
   load-bearing dependencies of current-generation code. This is a case of "legacy architecture
   partially replaced": the shell is gone, several organs are still transplanted into the new
   body.
2. **Generation 2 — `mu::ui::window::CObject`/`CManager` (the "NewUI"/`SEASON3B` tier, predates
   the RmlUi work and predates the CUIMng merger).** This is the actual current standard — ~88
   windows, including every window the recent merger migrated. It depends on Generation 1 for
   widgets it never replaced (`CUITextInputBox`) and coexists with a still-uncoordinated second
   button/radio-button family of its own.
3. **Generation 3 — RmlUi (`UI/RmlBridge/`, markup+CSS).** Additive, not a replacement — it
   composes *into* a Generation-2 `CObject` window rather than being its own base class. Only ~9 of
   ~97 windows have any RmlUi content, and even those keep their Generation-1/2 widget members
   alive for click-detection redundancy (documented, deliberate, per the merger's own status
   notes — not second-guessed here, just noted as a fact about current dependency shape).

Newer code *does* still depend on older mechanisms in a load-bearing way: every RmlUi-hybrid
window still owns and updates legacy `CButton`/`CSprite` members, and `CUITextInputBox`
(Generation 1) remains the only real text-input widget anyone uses, RmlUi included.

## Existing Reusable Core

The strongest genuine, already-working foundations, in order of how cleanly they already
generalize:

1. **`UI::Scaling::UITransform` / `UILayoutPolicy::ForInterface()`** — a real, centralized,
   table-driven policy (`LayoutMode::Legacy/WorldOverlay/Hud/Dialog/DockLeft/FloatingWorkspace`)
   that every window's coordinate behavior routes through in one place. This is the one part of
   the system that already looks like "a framework," not a per-window reinvention.
2. **`CUITextInputBox`** — one implementation, genuinely shared across every generation, including
   new RmlUi-hybrid windows. Proof this codebase *can* converge on a single widget when there's
   enough pressure to.
3. **`mu::ui::window::CObject`/`CManager`** — the right base for lifecycle/dispatch/depth-ordering;
   its glaring gap (no geometry) is a well-scoped, addable extension, not a redesign.
4. **`RmlModelBinder<T>`** — a real, reusable, correctly-generic pattern for binding C++ state into
   a `data-model`, already proven across several windows.
5. **`CGaugeBar`** and **`CSprite`** — narrow but genuinely single-implementation primitives,
   usable as-is.

None of the four button/tooltip implementations qualifies as a "strongest existing foundation" —
pick one to promote rather than inventing a fifth.

## Proposed Unified UI Model

Convergence-oriented, built from what's already strongest above, not a new hierarchy invented from
scratch:

```
mu::ui::window::CObject        (existing — keep as the lifecycle root)
    + add: optional geometry mixin (position/size/Contains()) — opt-in,
      so windows with real bounds (the majority) stop hand-rolling it,
      while 3D-camera-space windows that don't need it aren't forced to carry it

UI::Scaling::UITransform / UILayoutPolicy   (existing — keep as the one coordinate authority,
                                              already correctly separated from CObject itself)

Widget family (pick ONE, canonicalize, retire the others):
    mu::ui::window::CButton / CCheckBox / CRadioButton   (Widgets/Window/*.h)
        — closest to "already the standard": already used by every
          currently-migrated CObject window (CTrade, CMyInventory, etc.)
    CUITextInputBox                                       — already correct, no change needed
    A new, single Tooltip primitive, extracted from the strongest of the
    four existing mechanisms (mu::ui::window::CButton's own ChangeToolTipText
    logic looks closest to reusable — it already has anchor/position handling)

RmlUi layer                     (existing — keep as the presentation layer for
                                  windows that have been ported; not a replacement
                                  for the widget layer above, a renderer for it)
```

Composition over inheritance where possible: a window is a `CObject` that *owns* widgets from the
one canonical family, the same shape `CTrade`/`CMyInventory` already use today — this model asks
for consolidation of what already exists in that shape, not a new composition mechanism.

## Migration Strategy

Incremental, in the order that removes ambiguity fastest per unit of effort:

1. **Resolve the two `INTERFACE_*` enums first** — cheapest, highest-risk-reduction step. Rename
   `CUIManager`'s local enum values (e.g. `INTERFACE_INVENTORY` → `MUTEX_INVENTORY`) so no two
   enums share names anywhere in the codebase. Pure rename, no behavior change, fully mechanical.
2. **Declare `mu::ui::window::CButton`/`CCheckBox`/`CRadioButton` (`Widgets/Window/*.h`)
   canonical** — it's already what every currently-migrated `CObject` window uses. Document this
   once; don't touch working call sites yet.
3. **Add the geometry mixin to `CObject`** as purely additive (default-inert, like the existing
   shown/active split was) — no existing subclass is forced to change, but new windows get real
   position/size/hit-test for free instead of reinventing it.
4. **Extract one Tooltip primitive** from `mu::ui::window::CButton`'s existing `ChangeToolTipText`
   logic, make it independently attachable (not button-only), and require new windows to use it.
5. **Migrate new UI development only** onto steps 2–4 — no forced rewrite of existing windows yet.
6. **Consolidate the legacy widget family opportunistically** — when a window already on
   `CWin`-composed or `CUIControl`-based widgets needs *any* other change, swap its
   buttons/radio-buttons to the canonical family in the same pass; don't schedule a dedicated
   sweep.
7. **Deprecate, then remove, the losing implementations** (`::CButton : CSprite`,
   `CUIButton : CUIControl`, the global `::CRadioButton`) once grep shows zero remaining call
   sites — likely a multi-month tail given ~88 windows' worth of surface area.

Do not touch `CWin`/`CWinEx` themselves — they're already fully retired as a base class; the only
remaining work there is the widget-family consolidation above, not anything structural.

## Candidate Classes

| Class / Module | Current Role | Issues | Recommended Role | Action |
|---|---|---|---|---|
| `mu::ui::window::CObject`/`CManager` | Lifecycle/dispatch base for ~88 windows | No geometry/hit-test | Canonical window base | Keep, extend (additive geometry mixin) |
| `UI::Scaling::UITransform`/`UILayoutPolicy` | Coordinate/scaling policy | None found | Canonical coordinate authority | Keep as-is |
| `CUITextInputBox` | Text field, used everywhere | None found | Canonical text widget | Keep as-is |
| `mu::ui::window::CButton`/`CCheckBox`/`CRadioButton` (`Widgets/Window/`) | Already-dominant new-tier widget family | Tooltip logic baked in ad hoc | Canonical widget family | Keep, extract Tooltip out of it |
| `::CButton : CSprite` (`Widgets/Button.h`) | Legacy sprite button | Competing with the tier's own `CButton` | Superseded | Deprecate, migrate callers, remove |
| `CUIButton : CUIControl` | Inventory-toolkit-era button | Third competing implementation | Superseded | Deprecate, migrate callers, remove |
| `::CRadioButton` (`UIControls.h`, global) | Raw-GL radio button | Name-collides with the tier's own `CRadioButton`, needs explicit `::` qualification | Superseded | Deprecate, migrate callers, remove |
| `CUIManager`/`g_pUIManager` | Open/close mutual exclusion for inventory panels | Own `INTERFACE_*` enum collides by name with the unrelated tier enum | Keep the *function*, rename the enum | Refactor (rename enum values only) |
| `CWin`/`CWinEx` | Historical base class, now composed-only | None as a composed widget; confusing only as a base-class *concept* still referenced in docs/comments | Retired base, live widget | Keep as composed widget, no base-class role |
| `UI::Skills::Tooltip`, `CButton::ChangeToolTipText`, ad hoc `RenderTooltip()` methods, RmlUi `.tooltip` CSS | Four independent tooltip mechanisms | No shared primitive; duplicated hazard-workarounds | One canonical Tooltip primitive | Consolidate |
| `RmlModelBinder<T>`, `RmlTheme.cpp` | Data-binding/theming for RmlUi-ported windows | None found | Canonical presentation layer for ported windows | Keep as-is |
| `UI::RmlBridge::MakeDraggable()` | Generic RmlUi drag helper | Zero live callers, one open scaling gap noted in its own header | Candidate canonical drag mechanism, unproven | Keep, but resolve before first real caller rather than after |
| `CInventoryActionController`/`IInventoryActionContext` | Item-slot drag-drop | Domain-specific, not a generic UI mechanism — correctly separate from window-drag | Keep as a distinct, non-UI-framework concern | Keep as-is |

## Confidence / Evidence Gaps

- **RmlUi coverage is much smaller than the widget-family problem** — this assessment traced 5
  mechanisms deep into the C++ widget layer but did not exhaustively catalog every `.rcss` file
  for tooltip/style duplication; the two checked (`mu_helper_bar.rcss`, `main_frame.rcss`) both
  showed the same duplicated pattern, but the exact count across all ~26 `.rcss` files would need a
  dedicated pass.
- **Not every one of the ~88 `CObject` windows' actual widget composition was traced** — confirmed
  on `CTrade`/`CMyInventory` and cross-checked class declarations broadly, but a minority could
  still be using the `CUIControl` family or a bespoke approach not sampled here.
- **`CUIManager`'s full call-site graph** (`Open()`/`Close()` callers across the inventory-family
  windows) wasn't exhaustively traced — confirmed its role and the enum collision, not every
  window that participates in its mutual-exclusion rules.
- **No test coverage evidence was examined** — this assessment is architecture-only; whether
  `tests/ui/` exercises any of the widget families discussed (which would affect how safe
  consolidation is to attempt) is unchecked.
