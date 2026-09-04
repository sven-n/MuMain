# Retiring the CUIMng/CNewUIManager split

Living document — update this, not the plan file, as phases land. See the architecture discussion
that led here for the full reasoning; this doc tracks execution status and the gotchas found doing
it, so the next phase doesn't rediscover them the hard way.

## Why

The "Legacy" (`CWin`/`CUIMng`) vs "NewUI" (`CNewUIObj`/`CNewUIManager`) framing is a historical
artifact of *when* each was written (`CNewUIManager` is Webzen's own `SEASON3B`-era content
generation, built because `CUIMng`'s fixed-member design couldn't scale to that era's flood of new
panels), not a live distinction — both serve the same current game. The goal is one window-object
system, not two, with `CUIMng`'s windows migrating onto `CNewUIObj`/`CNewUIManager` one at a
time, matching the incremental, independently-verified discipline the RmlUi migration
(`docs/rmlui-ui-system/`) already established. Of the 11 windows `CUIMng` originally owned, one
(`COptionWin`) turned out to be dead code rather than a migration candidate — see Phase 2 below.

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
  - **`COptionWin` deleted, not migrated** — confirmed genuinely unreachable in live play (a
    finding this branch's own pre-merger RmlUi-porting work had already made once, independently
    reconfirmed here before starting its migration): the only call site that ever showed it
    (`CUIMng::RepositionSceneUI()`'s restore-if-was-shown path) never had anything to restore,
    because nothing else ever called `ShowWin(&m_OptionWin)` in the first place — `CSysMenuWin`'s
    own "Option" button opens `g_pNewUISystem->Show(INTERFACE_OPTION)` (`CNewUIOptionWindow`)
    instead, and has since before this merger started. Migrating dead code onto the new manager
    would have been pure wasted effort, so it was deleted outright instead: removed from `CUIMng`
    (member, `Create*Scene()` calls, `WindowName()` lookup, `RepositionSceneUI()`'s restore
    logic) and its two `IsShow()` click-passthrough gate references (`UIMng.cpp`'s ESC-toggle
    gate, `CharacterScene.cpp`'s Enter-key gate) simply dropped their `!m_OptionWin.IsShow()`
    clause — behavior-preserving, since that flag was always `false` (never shown) and so never
    actually filtered anything. `OptionWin.h/.cpp` deleted (not left as untracked dead code this
    time, unlike the earlier RmlUi-porting decision to skip-but-keep it — this merger's Phase 4
    goal is zero `CUIMng`-owned windows, and there was no remaining reason to keep it around).
  - `CServerSelWin` — done, verified against a real server (server selection and the
    login-main bar's credits/exit buttons both confirmed still working). The most interactive
    window migrated so far (37 real `CButton`s across two
    arrays, `CGaugeBar`s, decorative sprites, a `CWinEx`-based description panel) — unlike
    `CCreditWin`/`CServerMsgWin` it needed real click-handling ported: `UpdateMouseEvent()` claims
    (consumes) any click within its own bounding box, ported directly from
    `CWin::CursorInWin(WA_ALL)`'s rect math, coexisting correctly with `CCreditWin`'s depth-100
    override (sort-by-depth dispatch means `CCreditWin` still always wins while shown, regardless
    of this window's own depth). `m_winDescription` stays a composed `CWinEx` member as before —
    `CWinEx` itself isn't retiring, only `CServerSelWin`'s own base class is, so nothing needed
    porting there. **Found and deliberately preserved (not fixed) a pre-existing render gap**:
    `m_aBtnDeco`/`m_aArrowDeco`/`m_winDescription` are positioned and `Show()`-toggled but were
    never actually rendered in the original `CWin`-based `RenderControls()` either — ported as-is
    for parity rather than silently drawing content that wasn't drawn before; worth a separate,
    deliberate look if the decorative art/description panel is supposed to be visible.
    `g_ServerSelWin` replaces `CUIMng::m_ServerSelWin`, same convention as `g_CreditWin`.
  - `CMsgWin` — done, verified against a real server (login flows, server-full/bad-password
    messages, character creation/deletion including the resident-password step, ESC on dialogs).
    The first RmlUi-hybrid window migrated (its own `RmlModelBinder`/`Rml::ElementDocument`
    lifecycle, ~40 `PopUp()` message codes) — bigger than the three still-legacy windows above in
    line count, but a smaller external blast radius than `CServerSelWin` (only 3 read-only
    external `IsShow()`/`RenderTextOnTop()` call sites). `RenderControls()`/`Render()` needed no
    porting at all: RmlUi's `#panel` already owns 100% of this dialog's visuals, and the legacy
    `CSprite`/`CButton` objects were already pure click-detection bookkeeping, never rendered.
    `UpdateMouseEvent()` unconditionally claims the click while shown (no rect check needed, same
    as `CCreditWin`) — replaces what `CWin::Create()`'s full-screen bounding rect used to do for
    `CUIMng::IsCursorOnUI()`. Three things found and fixed as part of this migration, all explained
    in the gotchas section below since they generalize to any future scene-relevant migration, not
    just this window: **`IsCursorOnUI()` didn't fold in new-style windows at all** (a real, if
    previously-dormant, gap — `CCreditWin`/`CServerMsgWin`/`CServerSelWin` never needed it fixed
    since `IsCursorOnUI()` is only consulted in `CharacterScene.cpp`, and none of those three are
    ever shown mid-gameplay the way a `CMsgWin` delete-character confirmation is); **a same-
    frame ESC double-trigger risk** from `m_NewStyleMng.Update()` running before the legacy
    ESC-toggle-menu check instead of after it; and (found via user testing, after the build-clean
    state above was first reported) **the resident-password input box rendering far from the
    dialog** — a transform-scoping inconsistency between where its position was set (now
    sometimes inside `CMsgWin::Update()`'s own `LayoutMode::Legacy` scope) and where it was
    rendered (unscoped, ambient transform), causing its stored position to be scaled twice. `g_MsgWin`
    replaces `CUIMng::m_MsgWin`, same convention as `g_CreditWin`.
  - Still to do: `CSysMenuWin`, `CLoginMainWin`, `CCharSelMainWin`, `CCharMakeWin`.
- **Phase 3 (`CLoginWin` + `CCharInfoBalloonMng`)** — not started. `CLoginWin` is the one window
  that genuinely needs the new shown/active split (keeps ticking its text inputs and "Remember
  Password" dialog while inactive, per its own `UpdateWhileShow()`/`UpdateWhileActive()` split);
  `CCharInfoBalloonMng` was never a `CWin` at all (`CUIMng` already drives it by a direct call,
  outside `m_WinList`) and needs its own thin adapter.
- **Phase 4 (delete `CUIMng`)** — not started; blocked on Phases 2-3. `CWin` has a closed,
  fully-enumerated subclass set (confirmed by a full-tree grep) — no hidden subclass elsewhere to
  worry about once the remaining 9 (`COptionWin` deleted, not counted) are gone.
- **Phase 5 (rename cleanup)** — not started; deliberately deferred until Phases 1-4 are complete
  and the split is fully retired. Once there is only one window-object system left, drop the
  "New"/`NewUI*` naming (`CNewUIObj`, `CNewUIManager`, `NewUIBase.h`, the `INTERFACE_*` prefix,
  etc.) — it only ever meant "new relative to `CUIMng`", which stops being a meaningful
  distinction once `CUIMng` is gone. Also sweep code comments (this doc's own included) that
  reference the old "New UI"/"Legacy UI" framing once it's no longer accurate.

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
- **`CUIMng::IsCursorOnUI()` didn't fold in new-style windows at all until `CMsgWin`'s migration**
  — it's computed purely from walking `m_WinList` (`CursorInWin(WA_ALL)`/`GetState()`), so a
  migrated window contributed nothing to it, silently. Every migration before `CMsgWin`
  (`CCreditWin`/`CServerMsgWin`/`CServerSelWin`) got away with this because `IsCursorOnUI()` is
  only ever consulted in `CharacterScene.cpp` (world-click/rotation gating), and none of those
  three are ever shown mid-`CHARACTER_SCENE`-gameplay the way `CMsgWin`'s delete-character
  confirmation is — but a *modal* window relevant to `CHARACTER_SCENE` absolutely needs this,
  since otherwise a click on the 3D world behind it (missing all its buttons) would fall straight
  through to world/character-selection logic once it migrates off `m_WinList`. Fixed generally,
  not just for `CMsgWin`: `CUIMng::Update()` now folds `bNewStyleConsumedClick` (already computed
  from `m_NewStyleMng.UpdateMouseEvent()`'s per-frame hover/claim result, not just click edges)
  into `m_bCursorOnUI` right alongside the pre-existing legacy-`m_WinList` contribution, computed
  *before* the `m_WinList.IsEmpty()` early-return (future-proofing for Phase 4, when that list
  really will be empty). Any future migration of a `CHARACTER_SCENE`-relevant window that needs to
  block world clicks gets this for free — no per-window special-casing needed, just implement
  `UpdateMouseEvent()` to actually claim (`return false`) when appropriate.
- **A migrated window's own per-frame `Update()` can race the legacy ESC-toggle-menu check in the
  same frame.** `CUIMng::Update()` used to run `m_NewStyleMng.Update()` (which invokes every
  migrated window's own `Update()`, including any internal ESC handling — e.g. `CMsgWin` closing
  itself on ESC) *before* the legacy block that checks `!g_MsgWin.IsVisible() && ...` to decide
  whether to open the system menu. That let a single ESC press both close `CMsgWin` *and* open the
  system menu in the same frame, since the toggle check saw the already-updated (just-hidden)
  state instead of the frame's starting state. Fixed by moving `m_NewStyleMng.Update()` to run
  *after* the ESC-toggle block instead of before it — matching the relative ordering the legacy
  per-window `m_WinList` `Update()` walk further down already had with that same check.
  `UpdateMouseEvent()`/`UpdateKeyEvent()` still run early (their results — `bNewStyleConsumedClick`
  and the cursor-on-UI fold-in above — are needed before the emptiness check), only the full
  `Update()` call moved. Worth rechecking whenever a future migrated window's `Update()` reads or
  changes state something earlier in the same function also depends on.
- **A legacy widget that reads `UI::Scaling::GetActiveTransform()` *at render time* (not just once
  at position-set time) can end up double-scaled once its owning window's `Update()` runs inside a
  `ScopedActiveTransform` scope.** Found on `CMsgWin`'s resident-password input
  (`g_pSinglePasswdInputBox`, a `CUITextInputBox`): unlike `CSprite` (which never consults the
  transform system at all, per the `CCreditWin` gotcha above) or `CButton`/`CSprite`-based widgets
  this migration series had ported so far, `CUITextInputBox::Render()` calls
  `RenderColorQuadARGB()`/`g_pRenderText->RenderText()`, both of which rescale the position it was
  given via `ConvertPositionX/Y` using *whatever transform is active when `Render()` runs* — a
  fundamentally different contract than `CSprite`'s "store real pixels, ignore the transform
  entirely". The original code exploited this deliberately: it pre-divided a real-pixel position
  by the ambient `g_fScreenRate_x`/`g_fScreenRate_y` before calling `SetPosition()`, relying on
  `Render()`'s later `ConvertPositionX/Y` call multiplying by that *same* ambient value to cancel
  the division out — correct as long as both the divide and the later multiply see the same
  transform, true unconditionally back when this only ever ran unscoped (`CWin` days). After
  migration, the call chain that sets this position
  (`ManageOKClick()`→`PopUp()`→`SetMsg()`→`SetCtrlPosition()`) can now run *inside*
  `CMsgWin::Update()`, which `CNewUIManager::Update()` wraps in a `LayoutMode::Legacy` (identity)
  `ScopedActiveTransform` — dividing by identity is a no-op, so a real-pixel value got stored
  un-descaled, then `RenderTextOnTop()` (still unscoped, ambient non-identity transform) multiplied
  it back up a *second* time, landing it far off from the actual dialog. Fixed by breaking the
  "divide now, rely on multiply later to cancel it out" coupling entirely: store real pixels
  directly (no division), and wrap the one consuming `Render()` call in its own explicit
  `LayoutMode::Legacy` `ScopedActiveTransform` so `ConvertPositionX/Y` also treats them as real
  pixels — both sides now agree unconditionally, regardless of which context triggered the write.
  **Any future migrated window driving a `CUIControl`-family widget (`CUITextInputBox` and
  siblings, `UIControls.h`) needs this same "identity at both ends" treatment** — it's a different
  hazard from the `CSprite`-based `LayoutMode::Legacy` gotcha above, not a duplicate of it.
