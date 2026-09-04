# Retiring the CUIMng/CNewUIManager split

Living document — update this, not the plan file, as phases land. See the architecture discussion
that led here for the full reasoning; this doc tracks execution status and the gotchas found doing
it, so the next phase doesn't rediscover them the hard way.

## Why

The "Legacy" (`CWin`/`CUIMng`) vs "NewUI" (`CNewUIObj`/`CNewUIManager`) framing is a historical
artifact of *when* each was written (`CNewUIManager` is Webzen's own `SEASON3B`-era content
generation, built because `CUIMng`'s fixed-member design couldn't scale to that era's flood of new
panels), not a live distinction — both serve the same current game. The goal is one window-object
system, not two, with `CUIMng`'s 11 windows migrating onto `CNewUIObj`/`CNewUIManager` one at a
time, matching the incremental, independently-verified discipline the RmlUi migration
(`docs/rmlui-ui-system/`) already established.

## Design

- Reused as-is: `CNewUIManager`'s dynamic registry (`AddUIObj`/`FindUIObj`), its single
  consume-and-stop mouse/key dispatch pass, `GetLayerDepth()`/`GetKeyEventOrder()` as independent
  sortable axes.
- Added to `CNewUIObj` (`NewUIBase.h`): a shown-vs-active split (`IsActive()`/virtual
  `SetActive()`, `UpdateWhileShown()`/`UpdateWhileActive()` hooks mirroring `CWin`'s own three-state
  lifecycle) and a virtual `Show()` — both purely additive, inert for every pre-existing
  `CNewUIObj` subclass (none override `Update()` via the new hooks, none override `Show()`).
- **`CUIMng` owns its own `SEASON3B::CNewUIManager` instance** (`GetNewStyleMng()`), not the shared
  `g_pNewUIMng` that `MAIN_SCENE`'s ~79 windows use. `CNewUIManager`'s dispatch is only ever driven
  from `MainScene.cpp` today — a window that only exists during `LOG_IN_SCENE`/`CHARACTER_SCENE`
  would never update/render if registered with the shared instance. `CUIMng::Update()`/`Render()`
  (already called unconditionally every frame, any scene) forward to this new instance first; the
  legacy `m_WinList` click-walk is skipped for the frame if the new instance already consumed the
  click, reproducing the "full-screen exclusive window blocks everything behind it" behavior a
  window like `CCreditWin` had as `m_WinList`'s own head entry.
- A window migrating off `CWin` keeps its position/rendering as its own private `CSprite`-based
  logic (no shared rect/hit-test facility exists on the `CNewUIObj` side, matching how every
  pre-existing `CNewUIObj` window already works) — `CursorInWin()`/`SetMovable()` overrides port
  into the migrated window's own `UpdateMouseEvent()` body instead.

## Status

- **Phase 0 (base-class additions)** — done. `NewUIBase.h`'s shown/active split, virtual `Show()`.
- **Phase 1 pilot: `CCreditWin`** — done, verified across multiple resolutions (640x480, 800x600,
  1024x768) against a real server. First window fully off `CWin`, onto `CNewUIObj`/`CUIMng`'s new
  scene-scoped manager instance. `g_CreditWin` replaces `CUIMng::m_CreditWin` (same convention as
  `g_pSkillList` — a raw global, not routed through `FindUIObj()` for its few external call sites).
- **Phase 2 (remaining `CUIMng` windows)** — in progress.
  - `CServerMsgWin` — done, verified against a real server (character-select scene, server
    message posted and rendered correctly). Even simpler than `CCreditWin`: purely passive (its old `CWinEx::CursorInWin(WA_ALL)` override
    always returned `false`, so it could never become `CUIMng`'s "active" window -- confirmed no
    drag/resize behavior was ever reachable in practice), so `UpdateMouseEvent()` just always
    returns `true` (never consumes) with no click-handling logic to port at all. Ported `CWinEx`'s
    5-sprite composite background (`WE_BG_*`) directly into the window's own private sprite array
    instead of reusing `CWinEx` (still used by not-yet-migrated `COptionWin`/`CServerSelWin`/
    `SysMenuWin`) -- matches the "no shared rect/hit-test facility" design principle below.
    `g_ServerMsgWin` replaces `CUIMng::m_ServerMsgWin`, same convention as `g_CreditWin`.
  - Still to do: `COptionWin`, `CServerSelWin` (no RmlUi entanglement, same low-complexity shape),
    then the RmlUi-hybrid ones (`CMsgWin`, `CSysMenuWin`, `CLoginMainWin`, `CCharSelMainWin`,
    `CCharMakeWin`).
- **Phase 3 (`CLoginWin` + `CCharInfoBalloonMng`)** — not started. `CLoginWin` is the one window
  that genuinely needs the new shown/active split (keeps ticking its text inputs and "Remember
  Password" dialog while inactive, per its own `UpdateWhileShow()`/`UpdateWhileActive()` split);
  `CCharInfoBalloonMng` was never a `CWin` at all (`CUIMng` already drives it by a direct call,
  outside `m_WinList`) and needs its own thin adapter.
- **Phase 4 (delete `CUIMng`)** — not started; blocked on Phases 2-3. `CWin` has a closed,
  fully-enumerated subclass set (confirmed by a full-tree grep) — no hidden subclass elsewhere to
  worry about once these 11 are gone.

## Gotchas worth knowing before the next migration

- **`CNewUIManager::AddUIObj(dwKey, obj)` silently overwrites the object's `LayoutMode` the first
  time it registers** — it calls `obj->SetLayoutMode(UI::Layout::ForInterface(dwKey))`
  unconditionally on first insertion, clobbering whatever the window's own constructor set.
  **`UI::Layout::ForInterface()`'s policy table (`UILayoutPolicy.cpp`), not the migrated window's
  own constructor, is the actual authority for its layout mode** — every `INTERFACE_*` key used by
  a migrated window needs its own explicit `case` there (a new key with no case falls through to
  `default: return LayoutMode::Dialog`, a real, resolution-scaled transform, not a safe default).
  Found the hard way on `CCreditWin`: a `SetLayoutMode()` call in its constructor compiled fine and
  appeared to work, but had zero actual effect.
- **A window with its own real-pixel rendering (not reference-space, dp-style) needs
  `UI::Scaling::LayoutMode::Legacy`** (added for this), which resolves to a genuine identity
  transform. Every other `LayoutMode` rescales against `UI::Scaling`'s own 640x480 reference
  resolution via `TransformForLayout()`. This matters for two independent reasons, both real bugs
  found on `CCreditWin`:
  - `CNewUIManager::UpdateMouseEvent()`'s per-object dispatch wraps the call in
    `ScopedActiveTransform(transform, transformMouse=true)`, which remaps the *global*
    `MouseX`/`MouseY` through whatever transform is active for that object — a window whose own
    click hit-testing (e.g. `CButton::IsClick()`) expects real, untransformed mouse coordinates
    gets its clicks checked in the wrong coordinate space entirely.
  - Text rendering (`CUIRenderTextSDLTtf.cpp`) explicitly consults
    `UI::Scaling::GetActiveTransform()`; `CSprite::Render()` does NOT consult the transform's scale
    at all (`m_fScaleX`/`m_fScaleY` are per-instance, set once at `Create()` time from whatever
    `fScaleX`/`fScaleY` was passed in) but DOES read its *offset* (`g_fScreenOffset_x/y`, set
    process-wide by `UI::Scaling::SetActiveTransform()`) — a genuinely mixed, partial dependency,
    not an all-or-nothing one. A wrong active transform therefore corrupts text scale/position and
    sprite offset, but never sprite *scale*.
- **Check whether a window's `CursorInWin(WA_ALL)` override is hardcoded to always return `false`
  before assuming it needs real click-handling ported.** `CServerMsgWin` had one (it could never
  become `CUIMng`'s "active" window as a result, which in turn meant its inherited
  `CWinEx::CheckAdditionalState()` drag-resize behavior was reachable in the code but never
  actually triggerable) -- its migrated `UpdateMouseEvent()` is just `return true;`, no porting
  needed. Worth checking explicitly per window rather than assuming interactivity from the
  presence of buttons/sliders in its member list.
- **`g_pTimer` (the global `CTimer*` `SceneManager.cpp` reads for `CUIMng::Update(dDeltaTick)`) is
  never reset anywhere in this codebase** (`ResetTimer()` has zero callers on it) — its
  `GetTimeElapsed()` is total process uptime, not a per-frame delta. The existing
  `dDeltaTick = MIN(g_pTimer->GetTimeElapsed(), 200.0 * FPS_ANIMATION_FACTOR)` only behaves like a
  delta because the clamp dominates almost immediately and forever after — in steady state it's
  just `200.0 * FPS_ANIMATION_FACTOR`. A migrated window's own `Update()` (which receives no
  parameters, unlike `CWin::UpdateWhileActive(double dDeltaTick)`) needing a real per-frame delta
  should read that same clamped expression directly, not `g_pTimer` itself.
