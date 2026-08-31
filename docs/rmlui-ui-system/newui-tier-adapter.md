# The `CNewUIObj`-Tier Adapter Pattern

How to port an in-game HUD/`UI/NewUI` window to RmlUi. This is a *different* tier from every
other document in `rmlui-ui-system/` — `CNewUIObj`/`CNewUIManager`, not `CWin`/`CUIMng` — with its
own base class, its own lifecycle, and its own input-gating mechanism. Read this before touching
anything under `UI/NewUI/`; the `CWin`-tier docs (`README.md`, `theming-and-modding.md`,
`layout-and-scaling.md`) still apply for RML/RCSS content itself, just not for how the C++ side
plugs in.

Proven once so far, on `CNewUIHeroPositionInfo` (`UI/NewUI/HUD/NewUIHeroPositionInfo.h/.cpp` →
`Data/Interface/RmlUi/mu_helper_bar.rml`) — 2026-08-31. This doc is the pattern extracted from
that pilot; expect it to keep firming up as more `CNewUIObj` windows are ported.

## Why this tier needed its own pass

Every window migrated before this was `CWin`-tier: `CWin::Create(w, h, -2)` (opaque-nothing
sentinel), `CUIMng` owns the window list, `Rml::Context::IsMouseInteracting()`'s absence from
`CUIMng::IsCursorOnUI()` was never an issue because that check already existed and just needed the
right rect to stay in sync (see `layout-and-scaling.md`'s `CalculateFixedAnchorLayout()` story).
`UI/NewUI` has none of that: a different base class (`CNewUIObj`, not `CWin`), a different manager
(`CNewUIManager`, not `CUIMng`), and — critically — it's the tier that renders during `MAIN_SCENE`
(actual gameplay), which had never run any RmlUi content through its input-gating path before this
pilot (see "The `MAIN_SCENE` prerequisites" below).

## The adapter shape

`CNewUIObj`'s real interface is small (`UI/NewUI/NewUIBase.h`, 64 lines):
`Render()`/`Update()`/`UpdateMouseEvent()`/`UpdateKeyEvent()`/`GetLayerDepth()`/`IsVisible()`/
`IsEnabled()`, plus non-virtual per-subclass `Create()`/`Release()` (signature varies per window)
and `Show()`/`Enable()` (already implemented on the base, not overridden). Porting a window means:

- **`Render()` → no-op.** RmlUi renders it via the existing `SetPreSubmitCallback` seam, same as
  every `CWin`-tier window. Nothing left for this method to do.
- **`UpdateMouseEvent()`/`UpdateKeyEvent()` → always return `true`** (RmlUi's own
  "not consumed" convention here, matching `RmlUiRuntime::ProcessSdlEvent()`'s contract).
  RmlUi's context does hit-testing now; this method existing at all is only to satisfy
  `CNewUIManager`'s consume-and-stop loop over every registered object.
- **`Create()`/`Release()`/`GetLayerDepth()`/`Show()`/`Enable()`/`IsVisible()` stay real.**
  `CNewUIManager` still owns z-order (`GetLayerDepth()`-sorted render loop),
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
  "not consumed," a legacy `CNewUIButton`/`SetButtonInfo()`/tooltip-position helper can never
  detect a click or do anything else again, regardless of its own internal state. Keeping it
  around is dead weight, not real redundancy — delete it. (`CNewUIHeroPositionInfo` had
  `m_BtnConfig`/`m_BtnStart`/`m_BtnStop` plus their `SetButtonInfo()`/`MoveTextTipPos()`/
  `BtnProcess()`/`LoadImages()`/`UnloadImages()` support code; all removed outright.)

## The `MAIN_SCENE` prerequisites (already fixed, applies to every future port here)

Two things had to work before *any* `CNewUIObj`-tier RmlUi content could behave correctly, fixed
alongside the first pilot — both matter beyond that one window, since `CSysMenuWin`/`COptionWin`
(already `CWin`-tier RmlUi) are reachable from gameplay via the in-game ESC menu, so RmlUi content
was already live during `MAIN_SCENE` before these fixes, just not correctly composited/gated:

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
   new `CNewUIObj`-tier RmlUi window is automatically covered by this — nothing per-window to add.

If a future port finds a *third* `MAIN_SCENE` call site gating on the same
`MouseOnWindow`/`mouseOnHud`/`CheckMouseUse()` trio without `IsMouseOverUI()`, add it there too —
these two were the only ones found by an explicit grep-based audit at the time, but a new gameplay
system could plausibly add another.

## Naming: match what the widget *is*, not necessarily the legacy class name

`CNewUIHeroPositionInfo`'s own tooltips call it "Official MU Helper" — it's the MU Helper bot's
mini control bar (Config/Start/Stop) with a position readout riding along, not a generic
"show my coordinates" HUD element. The RmlUi-facing assets are named `mu_helper_bar`, not
`hero_position_info`, to reflect that — while the C++ class/file/enum
(`CNewUIHeroPositionInfo`/`NewUIHeroPositionInfo.h/.cpp`/`INTERFACE_HERO_POSITION_INFO`/
`CNewUISystem::m_pNewHeroPositionInfo`/`g_pHeroPositionInfo`) stayed as-is — renaming *those* has
a blast radius well beyond one window's RmlUi port and is a separate decision. When a legacy class
name doesn't match what a widget actually presents as, don't force the RmlUi assets to inherit
the mismatch just because the C++ identifiers do.

Ported as **one** RmlUi component matching the legacy class's own boundary (one `CNewUIObj`, one
window, one screen position, one `INTERFACE_*` registration) rather than split along a
conceptual line (position display vs. bot control) that doesn't correspond to any real seam in
the code. Split only if the legacy code already has two independent lifecycles to split along —
don't invent a new ownership boundary as part of a straight RmlUi port.

## What's still unproven at this tier

- **Data-driven lists** (`data-for`/array binding) — proven at the `CWin` tier
  (`CCharMakeWin`'s job buttons, `CCharInfoBalloonMng`'s per-character balloons) but not yet
  through this adapter shape. `NewUIBuffWindow` (variable-length active-buff icon strip, one
  tooltip per icon) is the planned next pilot for exactly this.
- **Drag-and-drop** (`UI::Items::Drag`, `CNewUIPickedItem`) — used throughout `Inventory/*`, not
  touched by any pilot yet. No RmlUi pattern exists for this in this codebase at all.
- **`INewUI3DRenderObj`/3D-camera-space rendering** — a second, parallel compositing path
  (`CNewUI3DRenderMng`) used for item icons, the dragged-item ghost, and some tooltips. Also
  untouched.
- **The three separate, non-unified tooltip mechanisms** found in the initial `UI/NewUI` audit
  (generic button tooltip, item tooltip, skill tooltip) — this pilot added a *fourth*, simple
  CSS-only hover-tooltip pattern rather than trying to unify with any of them. Consolidating is
  explicitly deferred (architecture amendment item 6) until more of the tier is actually ported.

## Source map

| Subsystem | Key files |
|---|---|
| `CNewUIObj` base / manager | [`UI/NewUI/NewUIBase.h`](../../src/source/UI/NewUI/NewUIBase.h), [`UI/NewUI/NewUIManager.h/.cpp`](../../src/source/UI/NewUI/NewUIManager.h) |
| Mouse-gating extension | [`Render/RmlUi/RmlUiRuntime.h`](../../src/source/Render/RmlUi/RmlUiRuntime.h) (`IsMouseOverUI()`), [`Input/Selection.cpp`](../../src/source/Input/Selection.cpp), [`Engine/Object/ZzzInterface.cpp`](../../src/source/Engine/Object/ZzzInterface.cpp) (`Attack()`) |
| Cursor-on-top seam | [`App/Platform/Windows/Winmain.cpp`](../../src/source/App/Platform/Windows/Winmain.cpp) (`SetPostRmlUiCallback`), [`Scenes/MainScene.cpp`](../../src/source/Scenes/MainScene.cpp) |
| Pilot | [`UI/NewUI/HUD/NewUIHeroPositionInfo.h/.cpp`](../../src/source/UI/NewUI/HUD/NewUIHeroPositionInfo.h), `Data/Interface/RmlUi/mu_helper_bar.rml` + `themes/{legacy,modern}/mu_helper_bar.rcss` |

Everything else this tier reuses from the `CWin`-tier docs unchanged: `RmlModelBinder<T>`
(`UI/RmlBridge/RmlModelBinder.h`), the theme mechanism (`UI/RmlBridge/RmlTheme.h/.cpp`), the
`dp`/anchor-class layout policy (`layout-and-scaling.md`), and the general gotchas
(`README.md`'s Gotchas section).
