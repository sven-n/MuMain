# The `mu::ui::window` Tier Adapter Pattern

How to port an in-game HUD/`mu::ui::window::CObject`-tier window to RmlUi. This is a *different* tier from every
other document in `rmlui-ui-system/` — `mu::ui::window::CObject`/`mu::ui::window::CManager`, not `CWin`/`CSceneUICoordinator` — with its
own base class, its own lifecycle, and its own input-gating mechanism. Read this before touching
any window on that tier (as of `newui-legacy-merger.md`'s directory restructure, these live
in topic folders directly under `UI/` — `UI/HUD/`, `UI/Inventory/`, etc. — not one `UI/NewUI/`
folder); the `CWin`-tier docs (`README.md`, `theming-and-modding.md`,
`layout-and-scaling.md`) still apply for RML/RCSS content itself, just not for how the C++ side
plugs in.

Proven on the first two pilots, `CMuHelperBar` (`UI/HUD/MuHelperBar.h/.cpp` →
`Data/Interface/RmlUi/mu_helper_bar.rml`, renamed at port time from the legacy `CNewUIHeroPositionInfo`)
and `CBuffStrip` (`UI/HUD/BuffStrip.h/.cpp` → `Data/Interface/RmlUi/buff_strip.rml`, renamed
from `CNewUIBuffWindow` — the active-buff icon strip, and the first `data-for`/dynamic-array pilot
at this tier), and since firmed up by `CMainFrameWindow`'s full 3-phase port (`migration-ledger.md`)
— including this tier's first `data-event-mouseup`/right-click binding (Phase 3). This doc is the
pattern extracted from all three; check `migration-ledger.md` for the current full list of
`mu::ui::window::CObject`-tier ports before assuming this doc's own examples are the only ones.

## Why this tier needed its own pass

Every window migrated before this was `CWin`-tier: `CWin::Create(w, h, -2)` (opaque-nothing
sentinel), `CSceneUICoordinator` (then still named `CUIMng`) owned the window list, `Rml::Context::IsMouseInteracting()`'s absence from
its `IsCursorOnUI()` didn't need a *new* mechanism added the way this tier did — that check
already existed via `CursorInWin()`, it just needed its rect kept in sync with the RmlUi visuals
(see `layout-and-scaling.md`'s `CalculateFixedAnchorLayout()` story). **Don't read "already
existed" as "already reliable"**: `CursorInWin()`'s rect going stale was the mechanism behind
several confirmed bugs in this same `CWin` tier (`engine-findings.md`'s click-activation finding) —
the comparison here (this tier needed a new flag, `CWin`-tier didn't) still holds. Those specific
bugs are fixed and the list-driven activation dispatch that caused them was later deleted outright
(`engine-findings.md`'s "Update, 2026-09-05" note); `CalculateFixedAnchorLayout()`'s own hand-duplicated
rect-sync is the one piece of this that's still a real, open reliability concern, not a solved
detail.
The `mu::ui::window::CObject` tier has none of that: a different base class (`mu::ui::window::CObject`, not `CWin`), a different manager
(`mu::ui::window::CManager`, not `CSceneUICoordinator`), and — critically — it's the tier that renders during `MAIN_SCENE`
(actual gameplay), which had never run any RmlUi content through its input-gating path before this
pilot (see "The `MAIN_SCENE` prerequisites" below).

## The adapter shape

`mu::ui::window::CObject`'s real interface is small (`UI/Core/WindowObject.h`, 64 lines):
`Render()`/`Update()`/`UpdateMouseEvent()`/`UpdateKeyEvent()`/`GetLayerDepth()`/`IsVisible()`/
`IsEnabled()`, plus non-virtual per-subclass `Create()`/`Release()` (signature varies per window)
and `Show()`/`Enable()` (already implemented on the base, not overridden). Porting a window means:

- **`Render()` → no-op.** RmlUi renders it via the existing `SetPreSubmitCallback` seam, same as
  every `CWin`-tier window. Nothing left for this method to do.
- **`UpdateMouseEvent()`/`UpdateKeyEvent()` → always return `true`** (RmlUi's own
  "not consumed" convention here, matching `RmlUiRuntime::ProcessSdlEvent()`'s contract).
  RmlUi's context does hit-testing now; this method existing at all is only to satisfy
  `mu::ui::window::CManager`'s consume-and-stop loop over every registered object.
- **`Create()`/`Release()`/`GetLayerDepth()`/`Show()`/`Enable()`/`IsVisible()` stay real.**
  `mu::ui::window::CManager` still owns z-order (`GetLayerDepth()`-sorted render loop),
  registration/lookup (`AddUIObj(INTERFACE_KEY, this)`, keyed by the window's `INTERFACE_*` id),
  and visibility toggling through these exactly as before — nothing about *that* bookkeeping
  changes. `Update()` keeps doing whatever real per-frame work it did before (reading live game
  state), just feeding an `RmlModelBinder` now instead of a member `Render()` later reads.
- **The RmlUi doc/model creation block goes in `Create()`**, guarded by `if (!m_pRmlDoc &&
  RmlUiRuntime::Instance().IsCreated())` — the exact same guard every `CWin`-tier window uses, for
  the exact same reason (re-entrant `Create()` calls on resolution change must not recreate the
  document).
- **Remove dead legacy widget objects — don't keep them "for redundancy."** The `CWin` tier keeps
  a legacy `CButton` alive alongside its RmlUi replacement for redundant hit-testing (see
  `layout-and-scaling.md`). That doesn't transfer here: once `UpdateMouseEvent()` is a permanent
  "not consumed," a legacy `mu::ui::window::CButton`/`SetButtonInfo()`/tooltip-position helper can never
  detect a click or do anything else again, regardless of its own internal state. Keeping it
  around is dead weight, not real redundancy — delete it. (`CNewUIHeroPositionInfo` had
  `m_BtnConfig`/`m_BtnStart`/`m_BtnStop` plus their `SetButtonInfo()`/`MoveTextTipPos()`/
  `BtnProcess()`/`LoadImages()`/`UnloadImages()` support code; all removed outright.)

## The `MAIN_SCENE` prerequisites (already fixed, applies to every future port here)

Three things must work before *any* `mu::ui::window::CObject`-tier RmlUi content behaves correctly — all three
matter beyond just this tier's own pilots, since `CSysMenuWin` (`CWin`-tier RmlUi) is
also reachable from gameplay via the in-game ESC menu:

1. **The cursor now renders on top of RmlUi content during `MAIN_SCENE`.**
   `Winmain.cpp`'s `SetPostRmlUiCallback` — the seam that draws the cursor *after* RmlUi's own
   render pass — used to gate on `SceneFlag == LOG_IN_SCENE || CHARACTER_SCENE` only.
   `MainScene.cpp`'s own inline `RenderCursor()` call ran during the normal legacy 2D pass instead
   (too early — before RmlUi's frame-final pass), so the cursor was left underneath any RmlUi
   panel open during gameplay. Fixed: widened the callback's gate to include `MAIN_SCENE`, removed
   the now-redundant inline call in `MainScene.cpp`.
2. **World click-to-select/move and right-click-attack now know about RmlUi content.**
   `Input/Selection.cpp`'s `SelectObjects()` and `Engine/Object/ZzzInterface.cpp`'s `Attack()`
   each independently gate on 3 flags (`MouseOnWindow`, `mouseOnHud`,
   `g_pNewUISystem->CheckMouseUse()`) before letting a click reach the 3D world — none of which
   know about RmlUi. Fixed by adding `RmlUiRuntime::IsMouseOverUI()` (wraps
   `Rml::Context::IsMouseInteracting()` — the method already existed, already correctly named and
   commented for exactly this, just was never called anywhere) as a 4th flag in both gates. Any
   new `mu::ui::window::CObject`-tier RmlUi window is automatically covered by this — nothing per-window to add.

If a future port finds a *third* `MAIN_SCENE` call site gating on the same
`MouseOnWindow`/`mouseOnHud`/`CheckMouseUse()` trio without `IsMouseOverUI()`, add it there too —
these two were the only ones found by an explicit grep-based audit at the time, but a new gameplay
system could plausibly add another.

3. **A persistent RmlUi document needs its own scene-visibility gate — `IsVisible()`/`Show()`
   alone doesn't provide one at this tier.** `mu::ui::window::CSystem` (the `mu::ui::window::CObject`-tier manager) is a
   single app-lifetime singleton, created once at startup and never released until the process
   exits — unlike `CWin`-tier windows, which are explicitly `Show()`/`Hide()`'d by app logic at
   real scene-transition points (each `g_*Win` global's own `Show()`, driven by
   `CSceneUICoordinator::CreateLoginScene()`/`CreateCharacterScene()`/`CreateMainScene()`/
   `Release()` — the generic `ShowWin()`/`HideWin()` pair this used to go through on `CUIMng`'s own
   window list was confirmed dead and deleted outright in `newui-legacy-merger.md`'s Phase 4,
   once every window it would have applied to had migrated off that list). Its `Update()`/`Render()` are
   themselves only ever *called* during `MAIN_SCENE` (`MainScene.cpp`) — before any `mu::ui::window::CObject`
   window owned an RmlUi document, that call-site gate was already a complete visibility gate on
   its own, since nothing drew otherwise. It stops being one the moment a window's visuals move to
   a persistent RmlUi document: that document, once shown, keeps rendering every frame regardless
   of scene (`RmlUiRuntime::RenderFrame()` is a true per-frame choke point, scene-agnostic by
   design) — found via `CMuHelperBar`/`CBuffStrip` rendering on the login/character-select
   screens. `mu::ui::window::CObject::Show(bool)` (`WindowObject.h`) only toggles a `m_bRender` flag consumed by the
   now-dead `Render()`; it was never wired to the RmlUi document at all. Fixed generically, not
   per-window: each pilot exposes `SyncDocVisibility(bool sceneAllowsShow)` (computes
   `IsVisible() && sceneAllowsShow`, calls `m_pRmlDoc->Show()`/`Hide()`), and
   `CSystem::SyncMainSceneHudVisibility()` calls it on every such window every frame,
   regardless of scene — wired into `Winmain.cpp`'s `SetPostRmlUiCallback` lambda (already the
   per-frame, scene-agnostic call site above), outside its own `SceneFlag ==` gate so it isn't
   itself scene-restricted. **Any new `mu::ui::window::CObject`-tier window whose visuals move to a persistent
   RmlUi document needs the same `SyncDocVisibility()` method added to
   `SyncMainSceneHudVisibility()`** — this doesn't happen automatically the way the click-gating
   fix above does.

## Naming: match what the widget *is*, not necessarily the legacy class name

The legacy `CNewUIHeroPositionInfo`'s own tooltips call it "Official MU Helper" — it's the MU
Helper bot's mini control bar (Config/Start/Stop) with a position readout riding along, not a
generic "show my coordinates" HUD element. The RmlUi-facing assets are named `mu_helper_bar`, not
`hero_position_info`, to reflect that. When a legacy class name doesn't match what a widget
actually presents as, don't force the RmlUi assets to inherit the mismatch just because the C++
identifiers do — and, per the next paragraph, don't leave the C++ identifiers mismatched either.

Ported as **one** RmlUi component matching the legacy class's own boundary (one `mu::ui::window::CObject`, one
window, one screen position, one `INTERFACE_*` registration) rather than split along a
conceptual line (position display vs. bot control) that doesn't correspond to any real seam in
the code. Split only if the legacy code already has two independent lifecycles to split along —
don't invent a new ownership boundary as part of a straight RmlUi port.

**The C++ adapter class itself is renamed at port time, dropping tier-historical names** —
`CNewUIHeroPositionInfo` → `CMuHelperBar`, and every `INTERFACE_*`/`CSystem`
member/accessor/macro that referenced it, updated to match (`INTERFACE_MU_HELPER_BAR`,
`GetUI_MuHelperBar()`, `g_pMuHelperBar`, etc.) — required by
[`architecture-principles.md`](architecture-principles.md)'s §12. **What's still deferred**: the
`mu::ui::window::CObject` base class/tier boundary itself — that's structural (touches the ~88 other
still-unported `mu::ui::window::CObject` windows' shared machinery), not a per-class naming choice, and stays
premature with only 2 pilots. (The physical file location piece of this deferral was resolved
separately by the `UI/` directory restructure — `newui-legacy-merger.md`, 2026-09-05 — these
windows now live in `UI/HUD/` etc., not `UI/NewUI/HUD/`.) See
[`tracked-deferrals.md`](tracked-deferrals.md)'s "Tracked deferral" section for the full reasoning
on the base-class/tier piece that remains.

**A one-time naming exception here (deferring `CMainFrameWindow`'s own rename past its partial
port) was overtaken by a later blanket mechanical rename and never mattered in practice** — see
`tracked-deferrals.md`'s "`CMainFrameWindow`'s own class rename" entry for what's actually still
open (a file-organization question, not naming). Don't generalize this: a clean one-class-one-file
port still renames at port time, no exception — the carve-out only ever applied to a legacy file
welding multiple classes together with a partial port.

## Proven by `CBuffStrip` (the second pilot)

- **Data-driven lists whose SIZE changes every frame** (`data-for`/array binding over a
  `std::vector` that's `clear()`+`reserve()`+rebuilt from scratch every `Update()`, not just a
  fixed-size array whose members toggle) — works. `RegisterStruct`+`RegisterArray` and an
  unconditional `MarkDirty()` correctly re-render the list as it grows/shrinks with real buffs
  gained/expiring, not only in the synthetic single-frame injection used to first exercise it.
- **Icon atlases: use generated named `@spritesheet` rects, not a clipped-oversized-image
  trick.** An early attempt tried to avoid hand-enumerating ~160 icon rects by nesting a
  full-atlas-sized `decorator: image()` inside a small `overflow:hidden` container, offsetting it
  per-icon via `data-style-left/top` — confirmed by direct in-game testing **not to clip at all**
  in this RmlUi build: `ContainerBox::Close()` (`Source/Core/Layout/BlockContainer.cpp`) submits a
  box's scrollable-overflow rect — which `ElementUtilities::GetClippingRegion()`'s
  `has_clipping_content` check reads to decide whether `overflow:hidden`/`clip:always` produce an
  actual scissor — *before* `ClosePositionedElements()` ever places an absolutely-positioned
  child, so a container with only an oversized absolutely-positioned child inside it always
  computes "nothing to clip," regardless of its own `overflow`/`clip` properties. Symptom: the
  full, uncropped atlas rendered (visible as the entire icon sheet, including its unused/
  placeholder tiles). Fixed by switching to generated `@spritesheet` rects (one 20x28 tile per
  rect, mechanically generated, not hand-authored) and a single `data-style-decorator` binding
  per icon — the same mechanism every other migrated window's icons already use successfully. See
  `BuffStrip.h`'s `BuffEntry::decorator` comment and `themes/legacy/buff_strip.rcss`'s header for
  the full derivation, including why generation is capped below `eBuffState`'s full numeric range.
- **`dp` and `px` are NOT interchangeable in position bindings, even though both "work."**
  `char_make.rml`'s job buttons prove `data-style-left="expr + 'px'"`, but that's only valid
  because `char_make.rcss` itself sizes everything in `px`. `dp` is scaled by
  `Context::SetDensityIndependentPixelRatio()` (the user-configurable UI scale,
  `RmlUiRuntime.cpp`'s `ApplyUIScale()`) while `px` is always literal/unscaled
  (`ComputeProperty.cpp`) — a `px`-bound offset drifts out of step with `dp`-sized siblings at any
  UI scale other than 100%. Confirmed by direct testing: `buff_strip.rcss` sizes its grid in `dp`,
  and a `+'px'` binding compressed the intended gap between icons away entirely at a non-100% UI
  scale. Rule of thumb: bind position offsets in whichever unit the sibling static CSS in the
  *same file* actually uses — don't assume `+'px'` universally, check.
- **An absolutely-positioned, `display:block`, multi-line (`white-space:pre-line`) box needs an
  explicit `width`.** Left to shrink-to-fit (no declared `width`), this build's width computation
  undersized badly — the background/padding ended up sized to only the single longest *word*
  across all lines, not the longest *line*, so text wrapped down to one word per line even inside
  a real block box. A fixed `width` (`buff_strip.rcss`'s `.tooltip`) sidesteps the computation
  entirely; this is a width-computation issue specifically, not "background doesn't follow
  multi-line content" (a `display:block` box's background already correctly covers its own full
  computed box regardless of how many lines of text it contains).

## What's still unproven at this tier

- **Drag-and-drop** (`UI::Items::Drag`, `CPickedItem`) — used throughout `Inventory/*`, not
  touched by any pilot yet. No RmlUi pattern exists for this in this codebase at all.
- **`I3DRenderObj`/3D-camera-space rendering** — a second, parallel compositing path
  (`mu::ui::window::C3DRenderMng`) used for item icons (permanently native, see
  `ui-target-architecture.md` Section E), the dragged-item ghost, and some tooltips. The icons
  themselves stay untouched; `CItemHotKey`'s own slot chrome (hover-highlight border, stack-count
  text) now sits in front of them as an RmlUi overlay, same split as `CSkillList`'s Phase 2 skill
  icons (Phase 3, below).
- **The three separate, non-unified tooltip mechanisms** found in the initial `UI/NewUI` audit
  (generic button tooltip, item tooltip, skill tooltip) — `CMuHelperBar`/`CBuffStrip` each added a
  *fourth*, simple CSS-only hover-tooltip pattern rather than trying to unify with any of them
  (`CBuffStrip`'s is also plain-text, not the original's per-line-colored rich tooltip — a
  separate, deliberately deferred simplification). Consolidating is explicitly deferred
  ([`architecture-principles.md`](architecture-principles.md) §12) until more of the tier is
  actually ported.
- ~~Right-click-distinct-from-left-click in a `data-event-click` binding~~ **Proven**
  (`CMainFrameWindow`'s Phase 3, `CItemHotKey`'s `#item_slots`): RmlUi's `Context` only ever
  dispatches `EventId::Click` for the left mouse button, but `ProcessMouseButtonUp()`
  (`RmlUi/Source/Core/Context.cpp`) dispatches `EventId::Mouseup` for any button, carrying a
  `"button"` event parameter (`1` == right) — bound via `data-event-mouseup`, not
  `data-event-click`. The original `CNewUIBuffWindow`'s right-click-to-cancel special case
  (Infinity Arrow / Swell of Magic Power) still isn't reproduced by `CBuffStrip` — that's a
  separate, still-open simplification, not blocked by this any more.

## Source map

| Subsystem | Key files |
|---|---|
| `mu::ui::window::CObject` base / manager | [`UI/Core/WindowObject.h`](../../src/source/UI/Core/WindowObject.h), [`UI/Core/WindowManager.h/.cpp`](../../src/source/UI/Core/WindowManager.h) |
| Mouse-gating extension | [`Render/RmlUi/RmlUiRuntime.h`](../../src/source/Render/RmlUi/RmlUiRuntime.h) (`IsMouseOverUI()`), [`Input/Selection.cpp`](../../src/source/Input/Selection.cpp), [`Engine/Object/ZzzInterface.cpp`](../../src/source/Engine/Object/ZzzInterface.cpp) (`Attack()`) |
| Cursor-on-top seam | [`App/Platform/Windows/Winmain.cpp`](../../src/source/App/Platform/Windows/Winmain.cpp) (`SetPostRmlUiCallback`), [`Scenes/MainScene.cpp`](../../src/source/Scenes/MainScene.cpp) |
| Pilots | [`UI/HUD/MuHelperBar.h/.cpp`](../../src/source/UI/HUD/MuHelperBar.h), `Data/Interface/RmlUi/mu_helper_bar.rml` + `themes/{legacy,modern}/mu_helper_bar.rcss`; [`UI/HUD/BuffStrip.h/.cpp`](../../src/source/UI/HUD/BuffStrip.h), `Data/Interface/RmlUi/buff_strip.rml` + `themes/{legacy,modern}/buff_strip.rcss` |

Everything else this tier reuses from the `CWin`-tier docs unchanged: `RmlModelBinder<T>`
(`UI/RmlBridge/RmlModelBinder.h`), the theme mechanism (`UI/RmlBridge/RmlTheme.h/.cpp`), the
`dp`/anchor-class layout policy (`layout-and-scaling.md`), and the general gotchas
(`README.md`'s Gotchas section).
