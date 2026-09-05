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
  - `CSysMenuWin` — done, builds clean (RelWithDebInfo); not yet visually verified against a live
    server. Same shape as `CMsgWin` (RmlUi's `#backdrop`/`#panel` own 100% of the visuals, legacy
    `CWinEx`/`CButton`s kept purely as click-detection redundancy behind RmlUi's own primary
    bindings, `UpdateMouseEvent()` unconditionally claims while shown) but smaller: no
    `RmlModelBinder` complexity beyond simple label/visibility sync, and ESC was never this
    window's own concern even before migration (`UpdateWhileActive()`'s ESC branch was already a
    no-op comment — `CUIMng::Update()`'s dedicated ESC-toggle block opens/closes it directly), so
    none of `CMsgWin`'s three fixes needed re-deriving here. One real, if narrow, coexistence case
    handled deliberately: `ExitGame()` pops up a `MESSAGE_GAME_END_COUNTDOWN` `CMsgWin` *without*
    hiding itself first (unlike `SelectServer()`/`OpenOptions()`/`Close()`, which all do) — given
    depth 40.0f, below `CMsgWin`'s 50.0f, so the actionable countdown dialog wins input priority
    over the superseded menu backdrop during that brief overlap. `g_SysMenuWin` replaces
    `CUIMng::m_SysMenuWin`, same convention as `g_CreditWin` — note this needed adding an explicit
    `#include "UI/Windows/SysMenuWin.h"` to its 3 external call sites (`CharSelMainWin.cpp`,
    `LoginMainWin.cpp`, `CharacterScene.cpp`): removing the `m_SysMenuWin` member also removed
    `UIMng.h`'s own `#include "UI/Windows/SysMenuWin.h"` (no longer needed for the member
    declaration), which those three files had been relying on transitively without an include of
    their own. Worth checking for on every future migration, not just this one.
    **Found via user testing: the menu painted behind the character-info balloon** in
    `CHARACTER_SCENE` — `CCharInfoBalloonMng::Render()`'s `shouldHide` check (added when the
    balloon's own RmlUi port first needed to explicitly restore z-order against `CCharMakeWin`/
    `CMsgWin`, since "RmlUi renders last" flattened their previous legacy-2D-draw-order
    relationship) never accounted for `CSysMenuWin` at all — an omission the balloon's own author
    couldn't have anticipated since `CSysMenuWin` wasn't RmlUi-hybrid-*and*-`CHARACTER_SCENE`-
    relevant from `CCharInfoBalloonMng`'s perspective until this migration made it a genuine peer
    of `CCharMakeWin`/`CMsgWin` in that check. Fixed by adding `g_SysMenuWin.IsVisible()` to the
    `shouldHide` condition, same treatment as the other two. **Any future
    `CHARACTER_SCENE`-relevant migration needs the same addition here** — this is a third instance
    of the same category of gap as `IsCursorOnUI()`'s fold-in (`CMsgWin`'s entry above): a
    balloon-adjacent modal/overlay window needs explicit registration in more than one place
    (`IsCursorOnUI()`'s fold-in is now generic via the manager; this `shouldHide` check is not, and
    would need its own per-window addition until/unless it's generalized the same way).

    **Also reported, not fixed in this session — a pre-existing gap, not caused by this
    migration**: while `CCharMakeWin` (character creation) is open, `CCharSelMainWin`'s own "Menu"
    button remains clickable and still opens `CSysMenuWin` (which then paints in front of, not
    behind, `CCharMakeWin` — expected once the balloon-style fix above is generalized, but the
    underlying question is whether it should be reachable at all while a modal dialog is open).
    Root cause, from static inspection: unlike `CMsgWin`/`CSysMenuWin` (which each get a
    C++-side, unconditional-while-shown `UpdateMouseEvent()` full-screen claim) or `sys_menu.rml`
    (which has its own `#backdrop` `pointer-events: auto` element), **`char_make.rml`/`.rcss` has
    no full-screen input-blocking element at all** — grepped for `#backdrop` across every `.rml`
    document; only `sys_menu.rml` has one. `CCharMakeWin` relies entirely on the legacy `CWin`
    `m_bActive` gate (`CCharSelMainWin::UpdateWhileActive()`'s own menu-button check is correctly
    gated by it) to block background interactions, and that gate is the same one already
    documented elsewhere (`CLoginMainWin`/`CSysMenuWin`'s "act immediately" `RmlClick*()` methods,
    added specifically because `m_bActive` "doesn't reliably grant on a timely basis") as
    unreliable for RmlUi-sourced clicks specifically. Likely resolves naturally once
    `CCharSelMainWin`/`CCharMakeWin` migrate (Phase 2's next two windows) onto the same
    deterministic `UpdateMouseEvent()`-claims-while-shown pattern already proven three times over —
    not patched here to avoid guessing at a fix for a system about to be replaced anyway.
  - `CCharSelMainWin`/`CCharMakeWin` — done together, verified against a real server (the Menu
    button correctly inert while character creation is open; character creation itself including
    the 3D preview, job selection, and name input; character deletion; connect via double-click
    and Enter). Migrated as a pair specifically to land the real fix for the bug above:
    **`CCharSelMainWin::Update()` now explicitly checks
    `g_CharMakeWin.IsVisible() || g_MsgWin.IsVisible() || g_SysMenuWin.IsVisible()` and skips all
    button click-consequence processing while any is true** (buttons still `Update()` regardless —
    `CButton` self-gates on its own `Show()` flag, matching `CServerSelWin`'s established pattern —
    just their `IsClick()`/`RmlClick*()` results go unconsumed). This is deliberate, explicit
    C++-side gating, not a side effect of the manager's `UpdateMouseEvent()` dispatch: that
    dispatch only decides who claims a given *click* for the legacy-walk-skip and
    `IsCursorOnUI()`'s fold-in, and is entirely separate from `Update()`'s own independent
    per-object dispatch pass (gated only by `IsEnabled()`, which nothing here touches) — so a
    higher-depth modal claiming the mouse does **not**, by itself, stop a lower-depth window's own
    `Update()` from still processing button state it already gathered. Same reasoning applied
    defensively to `CCharMakeWin::Update()` itself (skips its own job/OK/Cancel/Enter/Escape
    handling while `g_MsgWin` — its own validation-error dialog — is shown, see its
    `GetLayerDepth()` comment for why the two can coexist).
    - `CCharSelMainWin` is **not modal** — `UpdateMouseEvent()` claims only within its own
      bounding rect (`CalculateFixedAnchorLayout()`'s math, unchanged), exactly `CServerSelWin`'s
      pattern, not `CMsgWin`'s full-screen one — the world behind this bottom bar must stay
      clickable/rotatable.
    - `CCharMakeWin` genuinely **is** modal, and unlike every other Phase 2 window its own
      `CWin::Create()` call used the *default* `nTexID=-1`, not `-2` — a real, visible full-screen
      dimming sprite (`CWin::m_psprBg`), not RmlUi bookkeeping-only content. Ported as an explicit
      `m_sprBg` member, rendered first in `Render()` (same convention as `CCreditWin`'s own
      `m_sprBg`) — dropping it would have been a real, visible regression, not a redundant-
      bookkeeping cleanup.
    - **Caught proactively before any user report**: `RenderCreateCharacter()`'s live 3D preview
      viewport calls `BeginOpengl(m_winBack.GetXPos() / g_fScreenRate_x, ...)` — the exact same
      "pre-divide, rely on a later same-transform multiply to cancel it out" hazard as `CMsgWin`'s
      resident-password gotcha (`BeginOpengl()` → `ConvertPositionX/Y`, same helpers
      `RenderColorQuadARGB`/`CUITextInputBox::Render()` use). This call moves from an unscoped
      context (`CWin::Render()`'s old per-window walk) into this window's own `LayoutMode::Legacy`
      (identity) `ScopedActiveTransform` scope, so the division would have shrunk and mispositioned
      the character preview. Fixed by passing real pixels directly (no division) before ever
      building or shipping this migration — checking every `g_fScreenRate_x`/`g_fScreenRate_y`
      division in a window before migrating it, not just the ones already known to be `CWin`/
      `CUIControl`-family widgets, is now standard practice for the remaining migrations too.
    `g_CharSelMainWin`/`g_CharMakeWin` replace `CUIMng::m_CharSelMainWin`/`m_CharMakeWin`, same
    convention as `g_CreditWin` — 6 external call sites updated across `MsgWin.cpp`,
    `WSclient.cpp`, `Winmain.cpp`, `CharacterScene.cpp`, `SysMenuWin.cpp`,
    `CharInfoBalloonMng.cpp` (several needed the same "add the explicit include `UIMng.h` no
    longer provides transitively" fix `CSysMenuWin`'s migration already found).
  - `CLoginMainWin` — done, verified against a real server (Menu/Credits buttons both confirmed
    working). The simplest of the five RmlUi-hybrid windows: no `RmlModelBinder` at all (its two
    buttons are pure image buttons with no dynamic state, wired via a plain
    `Rml::Element::AddEventListener` self-owning listener, `UI::RmlBridge::RmlDraggable.cpp`'s
    idiom), and its `RmlClickMenu()`/`RmlClickCredit()` already acted immediately rather than
    deferring through the flaky legacy activation gate (a fix already landed here before this
    merger even started, 2026-09-03, after a real reproduction). `UpdateMouseEvent()` claims only
    within its own small bounding rect, `CServerSelWin`'s non-modal pattern, not `CMsgWin`'s
    full-screen one. This migration also permanently retires the last live consequence of a
    documented `m_LoginWin`/`CLoginMainWin` hit-test-overlap bug (`CreateLoginScene()`'s own
    comment): the overlap could previously starve the legacy `CButton` companion's click path by
    letting `m_LoginWin` (checked first in `m_WinList`) win the click instead, but now that this
    window's dispatch runs before any legacy `m_WinList` walk at all, that starvation is no longer
    reachable regardless of overlap — `m_LoginWin` itself can still be thrown off by the same
    drift until its own Phase 3 migration. `g_LoginMainWin` replaces `CUIMng::m_LoginMainWin`, same
    convention as `g_CreditWin` — 2 external call sites updated in `WSclient.cpp`.
  - **Phase 2 complete.** All of `CUIMng`'s still-`CWin` windows are migrated except `CLoginWin`
    (Phase 3, deliberately last — see below) and `CCharInfoBalloonMng` (never a `CWin`, its own
    Phase 3 item).
- **Phase 3 (`CLoginWin` + `CCharInfoBalloonMng`)** — done. `CLoginWin` migrated onto
  `SEASON3B::CNewUIObj`, `g_LoginWin` replaces `CUIMng::m_LoginWin` (same convention as
  `g_CreditWin`) -- 8 external call sites updated across `UIMng.cpp`, `MsgWin.cpp`, `WSclient.cpp`,
  `ReconnectManager.cpp`, `Winmain.cpp`. The one window in this whole series that genuinely needed
  the shown/active split added in Phase 0 (`IsActive()`/`SetActive()`,
  `UpdateWhileShown()`/`UpdateWhileActive()`) -- every other migrated window collapses straight to
  a single `Update()` override instead, leaving that split unused until now.

  **Found via user testing, fixed ahead of the migration itself (both are `CLoginWin`-side gaps,
  same root cause category as `CCharInfoBalloonMng`'s own `shouldHide` fix from Phase 2)**: once
  `CSysMenuWin`/`CCreditWin` had real content behind `login.rml`'s still-un-migrated dialog, two
  z-order bugs surfaced when opening either while the login dialog was up:
  - **Credits opened on top of the login dialog's own panel, not behind it.** `CCreditWin`'s
    visuals are plain `CSprite`/`g_pRenderText` content (Phase 1), drawn in a C++ pass strictly
    *before* RmlUi's own frame-final document render (`docs/rmlui-ui-system/README.md`'s "RmlUi
    renders last") — `login.rml`'s own panel therefore always painted over it regardless of which
    was opened more recently, a cross-render-phase case no RmlUi-side z-index could fix (the two
    are sequential passes, not comparable layers). Fixed the same way as
    `CCharInfoBalloonMng::Render()`'s own `shouldHide` check: `CLoginWin::RenderControls()` now
    toggles `login.rml`'s document `Show()`/`Hide()` every frame based on `g_CreditWin.IsVisible()`
    alone (`CSysMenuWin`'s own RmlUi panel already stacks correctly against `login.rml`'s — both
    are same-phase RmlUi documents — so it's deliberately not part of this condition).
  - **The raw username/password text (`CUITextInputBox::Render()`, not RmlUi content at all)
    painted over both the system menu and the credits screen.** This runs from two call sites --
    `CLoginWin::RenderControls()` itself (the legacy-theme-only "draw order doesn't matter since
    the panel's transparent" shortcut, still true) and Winmain.cpp's post-RmlUi
    `SetPostRmlUiCallback` (the theme-independent, eventually-canonical call site, per
    `RenderTextOnTop()`'s own header comment) -- and neither previously checked whether anything
    was currently covering the login dialog. Since this content isn't an RmlUi document at all,
    hiding `login.rml` doesn't touch it; fixed by gating both call sites on
    `!g_CreditWin.IsVisible() && !g_SysMenuWin.IsVisible()` directly.
  - **Correction, found while actually doing the migration**: both gates turned out to be
    *permanent*, not a stopgap the migration itself retires. `GetLayerDepth()`'s sort only orders
    `CNewUIManager`'s own dispatch of its registered objects' `Update()`/`Render()` calls -- it has
    no effect at all on RmlUi's own, completely separate compositor pass, which still renders every
    document last regardless of which `CNewUIObj` "rendered" (i.e., ran `SyncRmlModel()`) most
    recently. `CLoginWin::Render()` still carries both checks unchanged after migrating -- same
    permanent-gap category as `CCharInfoBalloonMng`'s own `shouldHide`, not a temporary one.
  - Ported unchanged into `CLoginWin::Render()`/`RenderTextOnTop()` once the real migration landed
    (see below) rather than re-derived.

  **The shown/active split, applied for real**: `CLoginWin::UpdateWhileShown()`'s first statement
  computes and pushes `SetActive()` from *current* (pre-`Tick()`) state --
  `!(g_CreditWin.IsVisible() || g_MsgWin.IsVisible() || g_SysMenuWin.IsVisible() || RememberPasswordChoiceState() == Pending)`
  -- then the rest of the body (both `DoAction()` calls, `UI::Login::Tick()`,
  `ApplyRememberPasswordChoice()`, `RevokeSavedCredentialsIfEdited()`) runs unconditionally, same as
  before. `CNewUIObj::Update()`'s own dispatch (`UpdateWhileShown()` always; `UpdateWhileActive()`
  only if the just-set `m_bActive` is true) then gates the OK/Cancel/Enter/Esc handling
  automatically -- no need to re-check `RememberPasswordChoiceState()` a second time inside
  `UpdateWhileActive()` itself the way the old `CWin`-based code had to (that redundant check, and
  the `m_bRememberPasswordPromptWasPending` snapshot member it needed, both dropped: setting
  `SetActive()` from pre-`Tick()` state at the *top* of `UpdateWhileShown()` reproduces the same
  "snapshot before `Tick()` can resolve it" ordering with no separate field). This single mechanism
  also incidentally fixed a real, not-yet-reported bug found via code review while designing the
  migration: `CLoginWin` polled `VK_RETURN`/`VK_ESCAPE` directly with no check for whether
  `CMsgWin`/`CSysMenuWin`/`CCreditWin` was covering it -- the same "a higher-depth modal claiming
  `UpdateMouseEvent()` doesn't stop a lower window's own `Update()`" gap already fixed once for
  `CCharSelMainWin`/`CCharMakeWin` (Phase 2) -- folded into the same `SetActive()` computation
  rather than a second, separate check. **Any future window needing this split should look at
  whether it also needs this same modal-coexistence gate folded into the same `SetActive()` call**,
  not just whatever originally motivated adding the split.

  `CLoginWin`'s own `g_fScreenRate_x`/`g_fScreenRate_y` divide in `SetPosition()` (storing the
  input boxes' position pre-divided, relying on `Render()`'s later `ConvertPositionX/Y` multiply to
  cancel it back out) got the same proactive fix `CCharMakeWin`'s `BeginOpengl()` call already
  needed: store real pixels directly, and wrap `RenderTextOnTop()` in its own explicit
  `LayoutMode::Legacy` `ScopedActiveTransform` so both ends agree on identity unconditionally
  (same fix as `CMsgWin`'s resident-password gotcha).

  `CCharInfoBalloonMng` also now derives from `SEASON3B::CNewUIObj` and registers with
  `GetNewStyleMng()`, promoted to a global `g_CharInfoBalloonMng` (same convention). Most of the
  `INewUIBase` surface is thin/inert for it -- no interaction (`UpdateMouseEvent()`/
  `UpdateKeyEvent()` never consume), no shown-vs-active distinction to make (`Update()` is a bare
  `return true;` -- nothing drives a per-frame update distinct from `UpdateDisplay()`'s
  event-driven refresh), `IsVisible()` overridden to mean `m_isInitialized` rather than the base
  `m_bRender` flag (nothing meaningfully toggles "shown" for this manager beyond init/release).
  `Render()`'s `shouldHide` check ports unchanged -- registering with the manager doesn't replace
  it, same permanent cross-render-phase reasoning as `CLoginWin`'s own credits/sysmenu gates above.
  `CUIMng::Render()`'s old direct, unconditional `m_CharInfoBalloonMng.Render()` call is gone --
  `m_NewStyleMng.Render()`'s generic per-object walk now calls it instead, which also moves *when
  in the frame* it renders (previously before the legacy `CWin*` walk, which no longer exists in
  `CHARACTER_SCENE` regardless since Phase 2 finished) -- not expected to matter, since the
  `shouldHide` toggle rather than draw-order governs its stacking, but flagged for the live-server
  verification pass. This was about uniformity for Phase 4 (zero hardcoded per-window calls left in
  `CUIMng`), not new behavior -- `CCharInfoBalloonMng` had no shown/active or interaction need of
  its own to speak of.

  **Two regressions found via live testing right after landing, both fixed in the same commit**:
  - **ESC no longer opened/closed the system menu, in both `LOG_IN_SCENE` and `CHARACTER_SCENE`.**
    `CUIMng::Update()` had `if (m_WinList.IsEmpty()) { m_NewStyleMng.Update(); return; }` guarding
    the ESC-toggle-system-menu block below it -- harmless before this migration (`g_LoginWin` kept
    the list non-empty throughout `LOG_IN_SCENE`), but `g_LoginWin` was the *last* window ever added
    to `m_WinList`, so once it migrated the list is permanently empty and that early return fires
    every frame, silently skipping the ESC block (and the rest of the legacy per-window walk) for
    good. Symptom the user could actually see: pressing ESC over the system menu no longer closed
    it. Fixed by moving the ESC-toggle block (and the `m_NewStyleMng.Update()` call) to run
    unconditionally, and moving the `m_WinList.IsEmpty()` check down to guard only the genuinely
    `m_WinList`-specific bookkeeping below it (`m_bWinActive`, the click walk, docking -- all
    correctly dead code now, kept only in case a `CWin` subclass ever reappears before Phase 4).
    **Originally suspected to also explain a second symptom, a mysterious "hang" on ESC at idle --
    turned out not to be `CreateSocket()` being slow at all.** Confirmed by later live testing: once
    the actual race below (closing the system menu via ESC also canceling the login form) was fixed,
    the hang was gone entirely, with or without any dialog open. So the hang was the race itself,
    not `CreateSocket()`'s connect cost surfacing more often as speculated here originally -- left
    this paragraph in place, corrected, as a reminder not to accept a plausible-sounding "that's just
    pre-existing cost, not a new bug" explanation without confirming it against a retest.
  - **Character-select balloons only positioned correctly at exactly 640x480.**
    `INTERFACE_CHAR_INFO_BALLOON` was given `LayoutMode::Legacy` (identity) in
    `UILayoutPolicy.cpp` on the assumption that, like every other window in this migration series,
    it computes real screen pixels itself. It doesn't: `CCharInfoBalloon::Render()`
    (`CharInfoBalloon.cpp`) computes `nPosX`/`nPosY` via `CameraProjection::WorldToScreen()` (into
    the 640x480 reference space) and then does `nPosX * g_fScreenRate_x` -- the exact "multiply by
    whatever transform is ambient when this runs" contract `INTERFACE_NAME_WINDOW`/
    `INTERFACE_ITEM_TOOLTIP` already use `LayoutMode::WorldOverlay` for. `LayoutMode::Legacy` made
    `g_fScreenRate_x/y` always `1.0` regardless of window size, so the multiply became a no-op --
    correct by coincidence only at 640x480 (where the "real" scale also happens to be 1.0), visibly
    wrong everywhere else. Fixed by moving `INTERFACE_CHAR_INFO_BALLOON` to `LayoutMode::WorldOverlay`
    (which resolves to `ScreenOverlayTransform()`, the same transform `UI::Scaling::LegacyUiTransform()`
    used to set manually before this adapter existed). **Worth checking for on any future adapter
    for legacy 2D content**: `LayoutMode::Legacy` is only correct for a window that computes real
    screen pixels itself; one that scales up from the 640x480 reference space (anything reading
    `g_fScreenRate_x/y` as a multiplier, as opposed to the CUIControl-family "divide then rely on a
    later multiply" hazard documented separately below) needs `WorldOverlay`/`Hud` instead.

  **A second live-testing pass, right after the first fix landed, found two more bugs**:
  - **Closing the system menu via ESC also canceled the login form behind it** (visible as the
    same multi-second `CreateSocket()` hang the first pass's ESC-hang note already flagged as
    pre-existing -- this occurrence, though, is a real, new-to-this-migration bug, not just that
    pre-existing cost surfacing). Root cause: `CUIMng::Update()`'s ESC-toggle block runs and closes
    `g_SysMenuWin` *entirely before* `m_NewStyleMng.Update()` -- not as part of its depth-sorted
    dispatch at all. Every other pair of Escape consumers in this manager gets ordering protection
    for free from that dispatch (e.g. `g_LoginWin`'s depth 20 < `g_CreditWin`'s depth 100 means
    `g_LoginWin::Update()` always runs first, seeing `g_CreditWin` pre-close), but `g_SysMenuWin`'s
    own ESC close isn't part of that dispatch at all, so `g_LoginWin`'s `SetActive()` gate (which
    includes `g_SysMenuWin.IsVisible()`) read the POST-close value in the very same frame the menu
    closed, saw "not covered", and fired `SubmitCancel()` off the same keypress. Fixed with a new
    `CUIMng::m_bSysMenuToggledByEscThisFrame` flag, set whenever that block acts (either direction)
    and reset every `Update()`, which `CLoginWin::UpdateWhileShown()`'s `SetActive()` computation
    now also ORs in alongside the live visibility check. **General lesson for any future window
    whose own Escape (or similar directly-polled key) gate depends on another window's visibility**:
    check whether that other window's own show/hide-on-that-key path runs through the shared
    depth-sorted dispatch (ordering protects it for free) or through a special-cased, out-of-band
    call site like this one (it needs an explicit same-frame flag like this instead).
  - **Tab-cycling between `CLoginWin`'s username/password fields visually highlighted both at
    once** (pre-existing in `CUITextInputBox`/`UIControls.cpp`, not something this migration
    touched or introduced, but real and now fixed alongside it since it was blocking verification).
    `RenderPortableSingleLine()`/`RenderPortableMultiline()` drew the selection-highlight rectangle
    whenever `HasSelection()` (`m_iSelAnchor != m_iCaret`) was true, with no check that the field
    was actually the focused one (`s_pFocusedPortable == this`) -- only the caret blink had that
    guard. `GiveFocus(TRUE)` (what Tab calls on the destination field) selects all of *its* text but
    never collapses the field being tabbed *away from*'s own now-stale selection range, so
    tabbing back and forth left both fields satisfying `HasSelection()` simultaneously, each
    rendering its own highlight regardless of which actually had keyboard focus. Fixed by requiring
    `bFocused` too in both rendering paths -- a real, previously-undiagnosed widget bug, not
    specific to `CLoginWin` (anything using two `CUITextInputBox`es with `SetTabTarget()` between
    them would have shown the same symptom).

  **Phase 3 complete and fully verified against a live server** (as of 2026-09-05), all four
  regressions/bugs above fixed, including the `CHARACTER_SCENE` ESC concern -- confirmed no longer
  reproducing (the fixed same-frame race above was the actual cause; not a separate crash). `CUIMng`
  now has zero remaining `CWin`-list members (`m_WinList` is permanently empty) and zero remaining
  direct hardcoded per-window calls -- every window it drives goes through `m_NewStyleMng`
  generically. Clear to start Phase 4.
- **Phase 4 (delete `CUIMng`)** — done. Three rounds of research (a full inventory of every member/
  method, a complete grep-based blast-radius map of every external call site, and a trace of how
  the three scene files drive it) found `CUIMng` had become a grab-bag of three unrelated things:
  confirmed-dead `CWin`/`m_WinList` plumbing, the title-screen loading-bar sprites (never actually
  related to the window-system merger, just historically bundled in), and its one real remaining
  job (scene-transition orchestration + forwarding to its own `CNewUIManager` instance). Landed as
  three independently-built-and-verified steps:
  1. Stripped the dead `CWin`/`m_WinList` machinery outright (`ShowWin`/`HideWin`/`SetActiveWin`/
     `CheckDockWin`/`SetDockWinPosition`/`RemoveWinList`, `Update()`'s/`Render()`'s dead per-window
     walks) — all confirmed genuinely unreachable, not just unused. Also caught and removed three
     more pieces of dead state found during this pass: `m_bSysMenuWinShow`/`SetSysMenuWinShow()`/
     `IsSysMenuWinShow()` (write-only — the setter had 6 external callers, the getter zero),
     `m_bBlockCharMove` (written twice, read nowhere), and `m_pLoadingScene` (a public member set
     to `NULL` in the constructor and never touched again anywhere).
  2. Extracted `CreateTitleSceneUI()`/`ReleaseTitleSceneUI()`/`RenderTitleSceneUI()` (confirmed zero
     references to `m_WinList`/`CWin`/`m_NewStyleMng` in their bodies) into a new
     `TitleSceneUI::CreateSceneUI()`/`ReleaseSceneUI()`/`RenderSceneUI()` free-function trio
     (`src/source/Scenes/TitleSceneUI.h`/`.cpp`) — used from only 2 files
     (`WebzenScene.cpp`, `ZzzOpenData.cpp`'s `OpenBasicData()`), too small a surface for a singleton
     class of its own.
  3. Renamed what was left — `CUIMng` → `CSceneUICoordinator`, `UIMng.h`/`.cpp` →
     `SceneUICoordinator.h`/`.cpp` (same `UI/Legacy/` folder at the time; that folder itself no
     longer exists as of the directory restructure below) — a pure rename, not a redesign:
     every method name/signature/body unchanged, so all ~50 call sites across ~30 files changed
     only their type name (`CUIMng::Instance()` → `CSceneUICoordinator::Instance()`), verified by a
     full rebuild (the real check for a rename this size — anything missed would be a compile
     error, not a silent bug). Three dead transitive `#include "UI/Legacy/UIMng.h"` lines
     (`ZzzBMD.cpp`, `LoadingScene.cpp`, `SceneCore.cpp`) dropped outright; `NewUIHotKey.cpp`'s real
     but transitive `CNewUIManager` dependency given its own direct include.

  **`CUIMng` no longer exists as a class or a file.** `CWin` itself (the base class every migrated
  window used to derive from) is untouched — it's still composed by `CWinEx`/`CButton`/
  `CUITextInputBox`-family widgets various migrated windows keep as their own private members
  (e.g. `CServerSelWin`'s `m_winDescription`), so it isn't part of this deletion and has no reason
  to be.

  **Deliberately not done, a documented follow-up**: `m_nScene`'s `UIM_SCENE_*` values (`NONE`/
  `TITLE`/`LOGIN`/`LOADING`/`CHARACTER`/`MAIN` = 0-5) are numerically and semantically identical to
  the already-existing, already-global `EGameScene`/`SceneFlag` (`Core/Globals/_define.h`) —
  `m_nScene` looks like a redundant shadow copy that predates `SceneFlag`, and every `m_nScene ==`
  check in `CSceneUICoordinator` could probably read `SceneFlag ==` directly instead, dropping the
  variable (and its own `UIM_SCENE_*` macros) entirely. Confirming that's safe needs verifying
  `SceneFlag`'s exact set-before-read ordering across all three scene files' per-frame dispatch,
  which nothing in this phase's research nailed down precisely enough to act on — a real
  simplification opportunity for whoever picks it up next, not attempted here.
- **Phase 5 (rename cleanup)** — not started; now unblocked, Phases 1-4 all complete and the split
  fully retired. Drop the "New"/`NewUI*` naming (`CNewUIObj`, `CNewUIManager`, `NewUIBase.h`, the
  `INTERFACE_*` prefix, etc.) — it only ever meant "new relative to `CUIMng`", which stopped being
  a meaningful distinction once `CUIMng` was deleted in Phase 4. Also sweep code comments (this
  doc's own included) that reference the old "New UI"/"Legacy UI" framing now that it's no longer
  accurate.
- **Directory restructure (done, 2026-09-05)** — `src/source/UI/Legacy/` and `src/source/UI/NewUI/`
  no longer exist. Neither folder name meant anything real anymore: `Legacy/` was a grab-bag (a
  widget toolkit, five unrelated base-less game-feature state classes, a second self-contained
  mini window-manager) and `NewUI/` was just "everything else," 193 files deep, already split into
  sensible per-feature subfolders that happened to duplicate names top-level folders already used
  (`UI/Combat/`, `UI/Chat/` predate the whole split). Moved everything to topic-based folders
  directly under `UI/`: a new `UI/Core/` holds the base-class/orchestration layer (`NewUIBase.h`,
  `NewUIManager`, `NewUIGroup`, `NewUISystem`, `NewUICommon`, `NewUI3DRenderMng`, `UILayoutPolicy`,
  `UIManager`, `SceneUICoordinator`); `UI/Widgets/` absorbed the old `UIControls`/`TextSearch`
  toolkit and all of `NewUI/Widgets/`; `UI/Character/`, `UI/HUD/`, `UI/Options/`, `UI/Quests/` are
  straight promotions of their `NewUI/` counterparts (`HUD/` also picked up the stray `UIMapName`);
  `UI/Combat/`, `UI/Dialogs/`, `UI/Events/`, `UI/Inventory/`, `UI/NPCs/` are each their `NewUI/`
  counterpart merged with one `Legacy/` game-feature class that matched it thematically
  (`UISenatus`, `UIPopup`, `UIGuardsMan`, `UIJewelHarmony`, `UIGateKeeper` respectively); `UI/Party/`
  is `NewUI/Party/` plus `UIWindows.h/.cpp` (the friend/mail/chat-room mini window-manager —
  confirmed still live, not dead, moved as-is; whether it's superseded by `NewUIFriendWindow` is a
  separate, not-yet-done follow-up). `UIDefaultBase.h/.cpp` deleted outright (fully inert,
  `#ifdef UIDEFAULTBASE`-gated, the guard macro was never defined anywhere). Pure file-move +
  include-path rewrite, no class renamed and no logic touched — done in independently-built chunks
  (`Core` first, since it has the widest fan-in, then `Widgets`, then the no-merge promotions, then
  the five thematic merges, then `Party`, then cleanup), each verified by a full rebuild plus
  running the `tests/ui/` binaries whose hardcoded source paths this touched. One naming collision
  flagged but deliberately not resolved here: `UI/Widgets/Button.h`'s `CButton` and the former
  `NewUIButton.h`'s `CNewUIButton` family are unrelated implementations that will collide if Phase
  5 mechanically strips `New*` off the latter — left for Phase 5 to resolve per-case, not decided
  speculatively now.

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
- **A window using `CNewUIObj`'s shown/active split should compute `SetActive()` from *every*
  condition that ought to suspend its "active" handling, not just whatever originally motivated
  adding the split.** `CLoginWin` (Phase 3, the first real user of this split) needed it purely for
  its own "Remember Password" sub-dialog, but its `UpdateWhileActive()` also independently polled
  `VK_RETURN`/`VK_ESCAPE` with no check for whether `CMsgWin`/`CSysMenuWin`/`CCreditWin` was
  currently covering it — the same "a higher-depth modal claiming `UpdateMouseEvent()` doesn't
  stop a lower window's own `Update()`" gap `CCharSelMainWin`/`CCharMakeWin` needed an explicit
  `modalOpen` check for in Phase 2. Rather than adding a second, separate check, both conditions
  fold into the same `SetActive(...)` call at the top of `UpdateWhileShown()` (`!(g_CreditWin.IsVisible() || g_MsgWin.IsVisible() || g_SysMenuWin.IsVisible() || RememberPasswordChoiceState() == Pending)`)
  — `CNewUIObj::Update()`'s own dispatch then gates `UpdateWhileActive()` on the result
  automatically, so there's exactly one place this logic lives instead of one check embedded in
  the split and a second, separately-invented one for modal coexistence. Also worth noting since it
  generalizes beyond the split itself: setting `SetActive()` (or any per-frame gating flag) from
  state read *before* a call that might resolve/consume that same state later in the same function
  (here, `UpdateWhileShown()` computing `SetActive()` before calling `UI::Login::Tick()`, which can
  resolve a Pending prompt) reproduces a "snapshot before it changes" ordering for free, without a
  dedicated snapshot member — `CLoginWin` dropped its old `m_bRememberPasswordPromptWasPending`
  field this way once it moved onto `CNewUIObj`.
