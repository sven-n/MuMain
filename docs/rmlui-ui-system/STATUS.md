# Status Against the Architecture Principles

Living document — update this, not `architecture-principles.md`, when status changes. See
[`architecture-principles.md`](architecture-principles.md) first if you haven't read it; section
numbers below refer to it. [`legacy-theme-modernization.md`](legacy-theme-modernization.md) amends
it on one specific question — when legacy-theme C++ behavior should move into RML/RCSS versus
genuinely stay in C++ — worth reading before auditing any legacy-theme code against this document.

## What's migrated

- **Login/character-select scene (`CWin` tier)** — fully migrated, no remaining
  legacy-`CWin`-rendered chrome: `CLoginWin`, `CLoginMainWin`, `CSysMenuWin`,
  `RememberPasswordPrompt`, `CCharSelMainWin`, `CCharMakeWin`, `CCharInfoBalloonMng`, `CMsgWin`.
  `COptionWin` (the `CWin`-tier options window) was confirmed unreachable in live play — its
  RmlUi port was never wired up, and the class was later deleted outright as confirmed-dead code
  (see `README.md`'s Coexistence patterns); `CSysMenuWin`'s Option button opens
  `mu::ui::window::COptionWindow` instead — now a 6-tab settings window (grown well past a flat
  options list, see `migration-ledger.md`'s own row for the full history), built and verified live
  against a real server, both themes.
- **In-game HUD (`mu::ui::window::CObject` tier)** — `CMuHelperBar` (map/position readout + MU Helper bot
  control bar) and `CBuffStrip` (active-buff icon strip, the `data-for`/dynamic-array pilot at
  this tier) are fully done. `CMainFrameWindow`'s 3-phase HUD-frame port is **done**: Phase 1
  (HP/MP/AG/SD/EXP bars + 5 corner buttons), Phase 2 (`CSkillList` — compact hotkey row
  click/hover/cooldown, expanded skill grid, pet-command row, and skill tooltips for both themes,
  replacing the old hand-rolled `EVENT_STATE` hover/click machine entirely), and Phase 3
  (`CItemHotKey` — potion-slot hover-highlight border, stack-count text, and right-click-to-use,
  all via RmlUi; the potion icon itself stays native — see the correction below). **Icon/box-frame
  art for the skill grid and pet row stays legacy 2D**, a deliberate Phase 2 scope cut — see the
  pilots-to-revisit table below. **Correction, 2026-09-06**: Phase 3's icons are not a sprite-atlas
  porting gap — traced to `RenderItem3D()`/`RenderObjectScreen(MODEL_...)`
  (`ZzzInventory.cpp`), they're genuine live 3D model renders, the same permanent, no-RmlUi-
  equivalent category as `CCharMakeWin`'s character-preview panel (`ui-target-architecture.md`
  Section E) — no future pattern will port the icon itself. Phase 3 accordingly only moved the
  slot chrome (hover-highlight border, stack-count text, right-click-to-use — the last of these the
  first `data-event-mouseup`/right-click RmlUi binding in the codebase, see
  `newui-tier-adapter.md`'s own former "still unproven" entry) to RmlUi as an overlay around the
  still-native icon, the same split Phase 2 already proved for skill icons, not a new pattern. All
  landed pilots (including Phase 3) built and verified against a real server, both themes. The rest
  of this tier — ~88 other `mu::ui::window::CObject` windows, drag-and-drop, and 3D-camera-space
  rendering generally — is not
  yet migrated.
- **`CMyInventory`** — Stage 1 (frame/title/gold/4 corner buttons, retiring `CButton` for this
  window entirely) and Stage 3 (Set/Socket option header labels + their shared hover tooltip,
  replacing the legacy `TextList`/`RenderTipTextList` mechanism — see
  `UI/Inventory/ItemOptionTooltipModel.h`, the same `BuildModel`-then-bind-or-legacy-render
  pattern `CMainFrameWindow`'s skill tooltip already proved) are **done**, both themes, built and
  verified against a real server. **Stage 2 (equipment paperdoll chrome — background sprite,
  durability tint, drag-compatibility highlight) was deliberately skipped**: those three visuals
  render *behind* the equipped item's live 3D icon today (native paint order, `RenderEquippedItem()`
  at a lower `GetLayerDepth()` than `Render3D()`'s camera); porting them to RmlUi's main context
  (always last in the frame) would flip that to *in front of*, a real regression, and preserving
  the current look needs the same background-context mechanism that already produced two real
  bugs during Stage 1 (position-transform, once-per-frame double-render) for no functional gain —
  see the pilots-to-revisit table below. The equipment grid (`CInventoryCtrl`) and both paperdoll/
  grid live-3D icons stay permanently native either way (Section E, same category as
  `CItemHotKey`'s potion icon). **Drag-by-title-bar** (`#title`, `UI::RmlBridge::MakeDraggable()`'s
  first real caller) with a **persisted, override-aware position** (`GameConfig::GetWindowPosition`/
  `SetWindowPosition`, `RestoreDefaultOrUserPosition()`) is also done — see the "Known gaps" entry
  below for the full mechanism, built generically so the next draggable window reuses the same two
  pieces rather than inventing its own.
- **Rest of the inventory family** (`CTrade`, `CStorageInventory`, `CStorageInventoryExt`,
  `CMixInventory`, `CNPCShop`, `CMyShopInventory`, `CPurchaseShopInventory`,
  `CInventoryExtension`, `CLuckyItemWnd`) — **done, both themes (2026-09-13)**: frame/header-rail/
  title/wallet/action-buttons ported per window, following `CMyInventory`'s background-context
  pattern; each window's own `CInventoryCtrl` grid(s) (and, for `CTrade`, both grids) stay fully
  native, same permanent no-RmlUi-equivalent category as `CMyInventory`'s own grid. Shared
  primitives factored out along the way instead of copy-pasted nine times:
  `UI::RmlBridge::SyncRootTransform()`/`CreateBackgroundDocument()` (`RmlRootTransform.h`,
  `RmlTheme.h`/`.cpp`) replace the hand-rolled root-transform math and background-document
  boilerplate `CMyInventory` had; `base.rcss`'s `.modern-header-rail-px`/`.modern-wallet-px`/
  `.modern-icon-btn-px` families give every window in this tier the same header/wallet/button
  chrome instead of a one-off per window. This is also the trigger condition the "Known gaps"
  entry below names for generalizing `RenderBackgroundLayer()` — see that entry for what changed.
  `CMyShopInventory`/`CPurchaseShopInventory` keep their native `CUITextInputBox` subject-line field
  exactly where `building-new-ui.md` says a Type-2 companion widget belongs — only its decorative
  background sprite moved to RmlUi (and, after a visual bug, from the fg document to the bg one, so
  it doesn't cover the real input box). **Update, 2026-09-23**: the remaining ordinary native text
  blocks in this tier are also ported now — `CMixInventory`'s recipe/success-rate/source/advice/
  per-mixtype description text, `CMyShopInventory`'s static instructional lines (former
  `RenderTextInfo()`), `CLuckyItemWnd`'s result/description text (former `Render_Frame()`'s
  `m_sText[]`/`AddText()` loop), and `CTrade`'s "Warning" item-overlay badge (former
  `RenderWarningArrow()`'s text half) all moved to RmlUi, each window's own C++ still owning the
  semantic strings/colors/line lists. Each window's `CInventoryCtrl` grid(s) stay native as noted
  above; `CLuckyItemWnd`'s mix-completion sparkle effect and `CTrade`'s animated warning-arrow
  glyph (a texture-atlas crop with a GL_CLAMP UV-overflow tint trick) also stay native as genuine
  rendering techniques, not chrome.
- **`CCharacterInfoWindow`** — **done, both themes (2026-09-13)**: fully ported, no permanently-
  native content at all (unlike the inventory family above, this window has no `CInventoryCtrl`
  grid, no live-3D icon, and no `CUITextInputBox` — it's a plain `CManager`-tier window, not
  `C3DRenderMng`-backed, so it needed none of the background-context/`RenderBackgroundLayer()`
  machinery). Frame/name/class-server crossfade/summary box/all five attribute rows (STR/AGI/VIT/
  ENE, plus CMD for Dark Lord)/level-up "+" buttons/Exit/Quest/Pet/Master-Level all move to
  RmlUi; every stat line `RenderAttribute()` used to draw natively (attack/defense/attack-speed/
  mana/magic-and-curse-damage/class-specific bonus lines, buff-conditional colors) is now computed
  in C++ exactly as before and pushed into a small `StatLine{text,color}` list per attribute box,
  rendered via normal block flow (`display:block` stacking, 13px line-height) instead of
  per-line C++-computed `top` offsets — reproduces the original's variable per-class/per-buff line
  count for free. `LoadImages()`/`UnloadImages()` are kept even though this window no longer
  renders through the legacy bitmap-atlas system: `CGensRanking` and `CUIMuHelper`'s own hunt/
  pick-range "+" buttons alias their own `IMAGE_LIST` entries onto these same texture slots (same
  reason `CMyQuestInfoWindow` keeps its own `LoadImages()`). One deliberate simplification: the
  summary box's original 8-piece pixel-tiled frame (4 corner sprites + a 1px sprite tiled across
  each edge, nested per-pixel `for` loops) is reproduced as the 4 corner sprites plus a flat
  translucent fill rather than a literal repeating-tile port — this build has no established,
  verified pattern for a real 1px-tile repeat (see "Findings" below), and the corners alone already
  read as "framed." **Modern theme, corrected same day**: the first pass gave this window its own
  independent `.modern-frame`/`.modern-frame-crimson` redesign (matching `CMyQuestInfoWindow`, the
  nearest *technical-tier* sibling — no `C3DRenderMng`, no live-3D icon, so nothing forced a native
  frame). That was consistent with `CMyQuestInfoWindow` in isolation but visually broke from
  `CMyInventory`, which this window actually docks beside on screen (same `PanelColumnX()` row) and
  shares frame art with (`IMAGE_CHAINFO_TOP` etc. literally alias `CMyInventory`'s own texture
  slots) — picking a technical-tier precedent without checking on-screen dock-neighbors produced a
  window that didn't read as part of the same family. Fixed to use `CMyInventory`'s own
  forged-dialog recipe instead (shell-edge/groove/gradient panel fill, rail-with-rivet header,
  `.modern-icon-btn-px` bordered square action buttons) — copied from `my_inventory_bg.rcss`'s own
  px literals. Unlike `CMyInventory`, this window has no live-3D icon to paint behind, so the whole
  look lives directly in one foreground document instead of a separate background context — that
  split exists purely to solve `CMyInventory`'s paint-order problem, not because the look itself
  requires two documents. See "Known gaps" below for the general lesson this surfaced (check
  on-screen dock-neighbors, not just the nearest technical-tier sibling, before designing a new
  port's theme treatment). Built and verified against a real build (Debug, `windows-x64`); in-game
  verification against a live server still pending for both themes.
- **`CGenericConfirmDialog`** — **done, both themes (2026-09-13)**: the reusable confirm-dialog
  primitive `CommonMessageBox.h`/`CustomMessageBox.h`'s ~140-class native `TMsgBoxLayout<T>` family
  lacked. See `component-catalog.md`'s "Dialog" section for the full shape (`GenericDialogConfig`'s
  field reference lives in the struct's own comments now, not narrated here). Proven on 3 real
  dialogs first, replacing their native call sites end-to-end and deleting the native classes —
  `migration-ledger.md`'s Dialog family table has the current count and what's left.
- **`CGenericMenuDialog`** — **done, both themes (2026-09-15)**: sibling primitive for
  `CustomMessageBox.h`'s "multi-option menu" shape (an arbitrary list of N labeled action buttons,
  not two fixed OK/Cancel slots) — see `component-catalog.md`'s "Dialog" section for the shape and
  the reentrant-`Show()`-during-click chaining pattern several consumers use. Proven on 13 real
  dialogs, replacing their native call sites end-to-end and deleting all of them —
  `migration-ledger.md`'s Dialog family table has the current list. Testing surfaced and fixed real
  bugs along the way, most notably `CManager::CompareKeyEventOrder`'s descending-sort issue (Esc
  closing the wrong window when several were open — see that entry below) and a nested-`data-for`
  requirement (`MenuButton::lines`, for native consumers that interleave body text per-button
  rather than once above the whole list — confirmed architecturally sound from
  `DataViewFor::Update()`'s own `SetInnerRML()` call, no prior precedent for nesting `data-for` in
  this codebase before this). `GenericMenuConfig::columns`/`.gmd-btn.cols-N` (added for a jewel-type
  grid whose item names didn't fit the existing 64dp `compact` button width) is the generalized
  fix if a future consumer needs an N-per-row button grid — legacy's `.cols-2` needed a
  `ninepatch()` decorator instead of `.btn`'s plain `image()` at the new width (see "Findings"
  below); modern's stays a flat fill/border since its `.btn` has no fixed-pixel sprite to 9-slice.

## Checklist for every new port (principles §27's workflow, condensed to what to actually check)

1. **Layout intent documented and traceable to the original code's actual computed behavior**,
   not its literal default-case numbers (§2–3) — e.g. `CBuffStrip`'s centering was derived from
   solving `SetPos(int iScreenWidth)`'s four hardcoded pairs as `x = (iScreenWidth - 200) / 2`,
   not copied from one of the literal numbers; see `buff_strip.rml`'s own header comment for the
   full derivation, and keep pointing future sessions at it as the worked example.
2. Uses the `dp`-based anchor/stretch/center utility classes (`base.rcss`,
   `layout-and-scaling.md`) instead of C++-pushed `px` rects, unless the position is genuinely
   data-driven per-frame (`CCharInfoBalloon`'s carve-out). When binding a *computed* per-frame
   offset via `data-style-left`/`top`, match whatever unit the sibling static CSS in that same
   file actually uses — `dp` and `px` are **not** interchangeable (see "Findings" below).
3. Deliberate (not defaulted) aspect-ratio/resolution behavior: fixed, edge-anchored, centered, or
   stretch (§7–8).
4. C++ stays limited to state/binding/events/game behavior; RCSS owns layout/sizing/positioning
   (§1, §16).
5. RmlUi-facing asset naming, and the C++ class name itself, reflects what the component actually
   is, not the legacy tier it came from (§12) — e.g. `mu_helper_bar`/`CMuHelperBar`, not
   `hero_position_info`/`CNewUIHeroPositionInfo`. **Renamed at port time, not deferred** — the one
   exception is a legacy file that welds multiple classes together where only some are ported in
   the current pass (`CMainFrameWindow`'s own file — see `tracked-deferrals.md`'s "Tracked
   deferral: `CMainFrameWindow`'s own class rename" entry); don't treat that as a general excuse to
   defer a rename otherwise.
6. Both themes updated in the same pass, never one left behind. A rendering technique (e.g. an
   icon atlas) is verified to actually work at runtime before being trusted — see
   `engine-findings.md` for a case where it didn't.
7. Uses reusable components/primitives where they exist; doesn't invent a new one-off mechanism
   when an existing pattern already covers the need — though see "Known gaps," several of the
   principles' presumed primitives don't exist yet on this branch.

## Findings worth knowing before the next port

Moved to [`engine-findings.md`](engine-findings.md) (2026-09-16) -- empirical, engine-specific
RmlUi build gotchas, reference material rather than status. Check it before assuming a new bug is
novel; several documented findings there have bitten more than one port.

## Known gaps against the principles (honest status, not yet built)

None of these are wrong so far — the principles doc explicitly endorses incremental delivery
(§26–27) — but they're real, currently-unaddressed gaps in the end-state architecture, not yet
even scheduled. Recorded so no future session mistakes "the pilots pass their own verification"
for "the full architecture is in place":

- **Correction, 2026-09-13**: `ui-target-architecture.md`'s Section D table claimed `CManager`'s
  dispatch loop "was never actually broken." A real bug in it was found and fixed this session:
  `CMyInventory::UpdateMouseEvent()`'s ground-drop guard returned `false` to mean "not a ground
  drop, some other window should handle this," but `CManager::UpdateMouseEvent()`
  (`WindowManager.cpp`) treats any `false` as "consumed, stop dispatching to every remaining
  window this frame" — not "defer to the next one." This silently ate every attempt to drop an
  item into `CTrade`'s own offer grid (and, by the same guard, `CStorageInventory`/`CNPCShop`/
  `CMyShopInventory`/`CPurchaseShopInventory`/`CMixInventory`/`CLuckyItemWnd`, all checked in the
  same guard) for as long as that code existed — a longstanding native bug, unrelated to this
  session's RmlUi work, that simply hadn't been exercised end-to-end before. Fixed by returning
  `true` from that branch. The dispatch *design* (topmost-first, consume-and-stop) doesn't need to
  change; the contract just isn't written down anywhere but the loop itself, and a future window
  adding a similar "not for me" guard could make the identical mistake — `UpdateMouseEvent()` must
  return `true` to let dispatch continue to lower-`GetLayerDepth()` windows, `false` only to
  genuinely consume the event and halt the frame's dispatch there.
- **No mod/user-override resource-precedence system** (§18–19). Themes today are exactly two
  hardcoded directories (`themes/legacy/`, `themes/modern/`) selected by `GameConfig`'s theme
  name — no "user override on top of a theme" layer, no documented precedence order, no tooling
  for a third party to ship a partial theme that inherits the rest from a base theme. Not
  speculative — §18/§19 have called for this since the governing doc was written, and arbitrary
  future themes are a real, stated want (see the Custom/Test-theme entry below), not just legacy
  and modern. Correctly *sequenced* behind other work (nobody's shipping a mod today), but it's a
  stated requirement waiting on priority, not an open question about whether to build it.
- ~~No design-token/shared-variable layer for `legacy`; `modern`'s own layer is a naming
  convention, not a real mechanism.~~ **Fixed 2026-09-04 for `modern`.**
  [`modern-theme-visual-direction.md`](modern-theme-visual-direction.md) defines the palette/
  border/typography token table; `UI::RmlBridge::LoadThemedDocument()` (`RmlTheme.cpp`) now
  resolves a `token(name)` marker against `themes/modern/tokens.ini` before RmlUi ever sees the
  RCSS text — no engine changes needed (reuses RmlUi's own inline-`<style>`-block support). All 11
  already-shipped `themes/modern/*.rcss` files were migrated (mechanically, via the one-off
  `tools/migrate_rcss_tokens.py`) from the old "value + comment" convention to real `token(...)`
  references, including a `font-title`/`font-body` split for the previously-repeated
  `font-family: "Liberation Sans"` literal (both stay the same value for now — no distinct
  display font chosen yet, this only names the future split). Caught and fixed one real,
  independent drift as part of the migration: the table said `border-metal` was
  `rgba(140, 146, 152, 140)`, but every actual shipped usage consistently used alpha `130` — the
  table had the typo, not the RCSS; both now say `130`. `legacy` still has no token layer and
  isn't in scope for one (theme-specific by design).
- ~~Three "shared" RML files have theme-specific class names baked into them — `architecture-
  principles.md` §15 violation.~~ **Fixed 2026-09-04.** `login.rml`, `msg_win.rml`, and
  `remember_password_prompt.rml` used to have `modern`-specific classes (`modern-frame`,
  `modern-frame-accent`, `modern-panel`, etc.) hardcoded directly into the file every theme is
  supposed to share equally. Per `theming-and-modding.md`'s Core Principle section: `legacy`'s
  copies are now the canonical, theme-neutral files; `modern` forks its own copy
  (`themes/modern/login.rml`/`msg_win.rml`/`remember_password_prompt.rml`) that keeps the
  modern-specific classes. No C++ changes were needed.
- ~~No drift-check tooling for a forked theme's RML.~~ **Fixed 2026-09-04.**
  `tools/check_rml_rcss_drift.py`, wired into the build next to `check_rml_rcss_syntax.py`: for
  every window whose C++ calls `LoadThemedDocument`, confirms every id/`data-model` field/
  event-callback name it references appears somewhere across the shared file plus every
  `themes/*/` fork of that document (checked against the union of all copies, not each
  individually — a per-copy requirement flags the legitimate "different themes bind different
  precomputed alternatives" pattern, e.g. `main_frame`'s `hp_text` vs `hp_current_text`, as false
  drift).
- ~~Two C++ call sites branch on theme *name*, violating `architecture-principles.md` §30.~~
  **Fixed 2026-09-04.** `MainFrameWindow.cpp`'s background-fill-behind-legacy-icons and
  selected-skill-slot-highlight logic (4 call sites total, not 2 — see `tracked-deferrals.md`'s
  "Pilots to revisit" table) now branch on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`, a declared capability
  read from an optional `themes/<name>/theme.ini` (`[Capabilities] ProvidesOwnIconChrome=1`) —
  missing file/key defaults to false, so only `modern` (the one theme with non-default behavior)
  needed a new file. The underlying paint-order constraint itself is unchanged (see the next
  entry) — this fix is the capability flag the entry below already anticipated, not a removal of
  the conditional.
- ~~RmlUi rendering strictly last in the frame — a real, systemic constraint, not one HUD
  window's quirk.~~ **Phase 1 (the primitive + one proven caller) built 2026-09-04.** Two
  directions:
  - **Legacy content rendering *after* RmlUi** — `MuRenderer.h`'s `SetPostRmlUiCallback` (game
    cursor, `CMsgWin`, `CharMakeWin`, `MuHelperBar`, `NewUISystem`) already provided this. Not a
    gap, never was.
  - **RmlUi content rendering *before* a specific mid-frame point** — the direction
    `MainFrameWindow.cpp`'s icon-chrome conditional needed, and every other window sharing
    `mu::ui::window::C3DRenderMng` will hit too — was genuinely missing infrastructure, now built.
    The whole inventory-family tier (`CMyInventory`, `CTrade`, `CStorageInventory`,
    `CStorageInventoryExt`, `CMixInventory`, `CNPCShop`, `CMyShopInventory`,
    `CPurchaseShopInventory`, `CInventoryExtension`, `CLuckyItemWnd`) is now ported and using this
    mechanism (2026-09-13); several message-box/quest/duel windows on the same
    `C3DRenderMng` tier are not yet ported:
    - **`IMuRenderer::FlushRenderCommands()`** (`MuRenderer.h`/`MuRendererSDLGpu.cpp`) — opens a
      real render pass *mid-recording*, replaying only what's been recorded since the last flush
      (or frame start). Turned out bigger than "add one more callback like
      `SetPostRmlUiCallback`": rendering here is fully deferred (`g_pNewUISystem->Render()` only
      appends to `s_renderCmds`; nothing reaches the GPU until `EndFrame()`'s one pass, which
      unconditionally `CLEAR`s) — a pass opened earlier would just get wiped by that clear. Fixed
      by refactoring `EndFrame()`'s vertex/bone/strip/texture staging (`StageDeferredGpuData()`)
      and its render-pass replay loop (`ReplayCommandRange()`) into helpers callable more than
      once per frame, tracked via `s_replayedCmdCount`/`s_mainColorPassOpenedThisFrame` (reset in
      `BeginFrame()`) so the first pass of the frame — a flush or `EndFrame()`'s own — `CLEAR`s and
      every one after `LOAD`s. The pre-existing screenshot/readback capture path (a different
      target texture than the swapchain) is untouched: it always does a full `CLEAR`+full-replay
      of its own, recovering flushed content from `s_renderCmds` even though the flush itself only
      ever wrote to the swapchain.
    - **`RmlUiRuntime::GetBackgroundContext()`/`RenderBackgroundLayer()`** (`RmlUiRuntime.h`/`.cpp`)
      — a second, background-only `Rml::Context`, driven explicitly (flush, then
      `Update()`+`Render()`) instead of through the single-slot `SetPreSubmitCallback` the "main"
      context uses. Needs no input routing at all (never registered as an `IUiInputConsumer`) —
      every document loaded into it is `pointer-events: none`, same convention as
      `char_sel_main.rml`'s `#panel`.
    - **Proven end-to-end**: `MainFrameWindow.cpp`'s `RenderLeftFrame()`/`RenderCenterFrame()`
      background-fill hack (`RenderColorQuadARGB` + `ThemeProvidesOwnIconChrome()`) is retired —
      replaced by `themes/modern/main_frame_bg.rml`/`.rcss`, a real RmlUi document rendered via
      `RenderBackgroundLayer()`, tracking the same `BottomHudCenterTransform`/anchor-offset values
      `main_frame.rml`'s own `#bars` group already used (`CMainFrameWindow::SyncRmlModel()`,
      a second small `RmlModelBinder`). The skill-list-up highlight overlay in the same function
      stays a legacy quad — never blocked by this constraint, no reason to move it. **Verified
      visually against a real server, modern theme, 2026-09-04**: potions and skill icons still
      render and animate correctly on top of the now-RmlUi-authored background, no regression.
  - **Phase 2 done (2026-09-13)**: the trigger condition below fired nine times over (the whole
    inventory-family port), so the call is now centralized in
    `mu::ui::window::CManager::Render()`'s own z-sorted loop (`WindowManager.cpp`) instead of each
    window wiring its own. Simpler than the `INVENTORY_CAMERA_Z_ORDER`-threshold design sketched
    below: no z-order audit needed — the call sits immediately before `(*vi)->Render()` inside the
    existing `if ((*vi)->IsVisible())` branch, so it still fires exactly once per frame, at the
    exact same point in the sequence the first visible bg-doc-owning window's own call used to
    (`RenderBackgroundLayer()`'s no-op-after-first guard, unchanged, is what makes every later
    iteration's call in the same frame free). One consequence worth knowing: this call now fires
    every frame regardless of which window happens to be first in z-order, not just ones that own
    background content — any window whose own background-context document stays `Show()`n across
    its own hidden state must gate that document's visibility itself (`CMainFrameWindow` needed a
    fix here: `m_pRmlBgDoc` used to rely on `RenderBackgroundLayer()` only running while
    `RenderLeftFrame()` did, i.e. while `CMainFrameWindow` itself was visible — no longer true, so
    `SyncDocVisibility()` now gates `m_pRmlBgDoc` the same `IsVisible() && sceneAllowsShow` way it
    already gated `m_pRmlDoc`). The inventory-family windows never had this problem — each one's
    own `SyncRmlModel()` already explicitly `Show()`/`Hide()`s its bg doc off its own `IsVisible()`,
    independent of who calls `RenderBackgroundLayer()`.
  - **Correction, 2026-09-13**: the first version of this centralization put the call directly in
    `CManager::Render()` unconditionally, which broke login/character-select — an empty background
    panel (no title/buttons, since those live in the foreground context and stay hidden) briefly
    appeared over those scenes. Root cause: `RmlUiRuntime`'s background context is a single
    app-lifetime singleton, but **two** `CManager` instances exist —
    `CSystem::m_pNewUIMng` (app-lifetime, owns the inventory family + `CMainFrameWindow`, `Update()`/
    `Render()` only ever called during `MAIN_SCENE`) and `CSceneUICoordinator::m_NewStyleMng` (a
    second, scene-scoped instance driving login/character-scene windows migrating off `CWin`, e.g.
    `CSysMenuWin`, `CCreditWin`). `CreateBackgroundDocument()` `Show()`s eagerly at `CSystem::Create()`
    time (app startup, before the login scene even renders) and nothing ever `Hide()`s those
    documents until `m_pNewUIMng`'s own `Update()` first runs (`MAIN_SCENE` only) — so putting the
    call unconditionally in `CManager::Render()` made `m_NewStyleMng.Render()` (which *does* run
    during login/character-select) paint `m_pNewUIMng`'s windows' stale, still-default-positioned
    background docs into the wrong scene. Fixed with an explicit opt-in,
    `CManager::SetDrivesBackgroundLayer(bool)`, called `true` only on `m_pNewUIMng`
    (`CSystem::Create()`) — `m_NewStyleMng` defaults to `false` and never fires the call. Any future
    additional `CManager` instance defaults to not driving this layer; opt in explicitly only if its
    own windows actually load documents into the background context.
  - ~~Not yet done (Phase 2, deliberately deferred): generalizing the one proven call site into
    a single insertion point inside `mu::ui::window::CManager::Render()`'s own z-sorted loop (gated on
    crossing `INVENTORY_CAMERA_Z_ORDER`, 5.5 — every `mu::ui::window::C3DCamera` z-order, unlike every
    2D-chrome window's, not yet audited project-wide) so every window on `mu::ui::window::C3DRenderMng`
    benefits automatically instead of each one wiring its own `RenderBackgroundLayer()` call.
    Ship this when the first inventory-family window's own port actually needs it, not
    speculatively ahead of that — `component-catalog.md` §26.~~
  - Also caught and fixed while writing `check_rml_rcss_drift.py`'s test against this change: the
    checker pooled every `.Bind()`/`.BindEventCallback()`/`GetElementById()` call in a `.cpp` file
    into *every* document that file loads — silently correct as long as no file owned more than
    one themed document. `MainFrameWindow.cpp` now owns two (`main_frame`,
    `main_frame_bg`); fixed by scoping each call to whichever `RmlModelBinder::Create()`/document
    pointer it's textually associated with, not the whole file.
- **No Custom/Test theme yet.** §25/§28 want a Custom/Test theme that looks substantially
  different from Legacy, specifically to surface accidental component/presentation coupling.
  **Legacy and Modern exist to validate that the architecture supports arbitrary themes, not as
  the intended ceiling** — a third theme is genuinely wanted eventually; it's simply not scheduled
  ahead of other work today. This is an ordinary sequencing decision, not an open policy conflict
  — check with the user on *timing*, not on whether it should happen at all.
- **No systematic resolution × UI-scale × theme × drag-state validation matrix** (§25).
  Verification so far has been ad hoc per window. No test plan artifact exists that a future
  session could run through mechanically.
- **The existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has not
  been explicitly audited for windows other than `CMyInventory` (below)** — does a dragged position
  survive a UI-scale change sensibly on other windows once they gain dragging? A theme change?
  Not examined beyond the one pilot.
- ~~`UI::RmlBridge::MakeDraggable()` (`RmlDraggable.h/.cpp`) has zero live call sites and still has
  one open §10-11 gap.~~ **Fixed 2026-09-07 — first real caller landed (`CMyInventory`, drag-by-
  title-bar) and both remaining gaps closed as part of it, generically, not as a one-off:**
  - **Persistence**: `GameConfig::GetWindowPosition()`/`SetWindowPosition(windowId, x, y)`
    (`GameConfig.h`/`.cpp`) — an immediate disk write (bypasses the usual member-field+`Save()`
    batching every other setting uses, deliberately: a drag has no "Apply" button, so deferring to
    the general save lifecycle would lose it on a crash or an ordinary Alt+F4), keyed by a short
    caller-chosen `windowId`, reusable by any future draggable window with one call each way.
  - **Theme-default-layout conflict**: `MakeDraggable()` gained an `OnDragEnd` callback (RmlUi's
    `Dragend` event — previously only `Dragstart`/`Drag` were wired) as the "persist now" hook.
    `CMyInventory::RestoreDefaultOrUserPosition()` replaces the 3 `WindowSystem.cpp` call sites that
    reset this window to its idle default column — restores the saved user position instead, if
    one exists. The **other** 3 call sites that shift this window sideways because Character-
    info/Inventory-Ext is *currently* also visible (real collision avoidance, not a "default reset")
    were deliberately left unconditional — skipping those would let a dragged Inventory panel
    visually overlap Character info, trading one real bug for another.
  - Still open, same as before: no audit yet of how a persisted position behaves across a
    resolution/UI-scale/theme change (see the item above) — `CMyInventory` stores it as its own
    reference-space `m_Pos` (the same logical coordinate every other position in this codebase
    scales from), which should behave correctly by construction, but hasn't been tested against a
    real resolution/scale change post-drag yet.
- ~~No reusable-component catalog exists as such~~ (§20) — **addressed 2026-09-04**:
  [`component-catalog.md`](component-catalog.md) inventories what already functions as a reusable
  primitive (`RmlModelBinder<T>`, `UI::RmlBridge` helpers, the anchor/center/stretch RCSS classes,
  Button/Checkbox's real shared contract) versus what genuinely doesn't exist yet (`ItemSlot`,
  `ProgressBar`, a unified `Tooltip`, `Dialog`, etc.) — a documentation deliverable, not new code;
  the underlying gaps it records are still open, just now named and tracked in one place instead
  of undiscoverable.
- **No rollout/phasing plan sequences the remaining still-`mu::ui::window::CObject`-tier windows against
  the full checklist above.** Work has been pilot-by-pilot, each individually verified.
  [`migration-ledger.md`](migration-ledger.md) (2026-09-16) now names every one of them by class —
  the "~88" figure above was always an approximate count, never a list — but a named ledger is still
  just an inventory, not a sequenced plan; nothing there says what order to tackle them in. The
  ledger's own audit also surfaced two components worth a closer look: `mu::ui::window::COptionWindow`
  (the live in-game Options window, opened by `CSysMenuWin`'s Option button per the "Coexistence
  patterns" note in `README.md`) had zero RmlUi call sites at the time despite reading as though it
  might already be replaced — **since ported and shipped (2026-09-19), see `migration-ledger.md`'s
  own row** — and `CServerMsgWin` (sibling of the already-done `CMsgWin`) is likewise still fully
  native and not yet a distinct port target (a real, unrelated visibility bug in it was found and
  fixed along the way, see its own ledger row — not a port). Conversely,
  `CCreditWin` turned out to already be a real, shipped RmlUi port (`credit_win.rml`) that was never
  logged in this file's own "What's migrated" list above — worth adding there if confirmed.
- ~~`MuPlatform::Initialize()`/`CreatePlatformWindow()`/`GetWindow()`/`Shutdown()`/
  `SetFullscreen()`/`SetMouseGrab()`/`GetDisplaySize()`, and the `IPlatformWindow`/`SDLWindow`
  classes they own, show zero external callers.~~ **Investigated and fixed 2026-09-04.** Root
  cause: `MuPlatform` was scaffolding added in `7f06b3af` (Jul 9) alongside unrelated audio-port
  work, never adopted — `Winmain.cpp`'s `WinMain()` (confirmed, via `Linux/main.cpp` and
  `macOS/main.mm`, the one real cross-platform entry point) has always done its own
  `SDL_InitSubSystem`/`SDL_CreateWindow`/`SDL_SetWindowFullscreen`/`SDL_Quit()` directly, in
  parallel to `MuPlatform`, on every platform. The mechanism itself wasn't unsound — the real
  finding was that `mu::platform::InstallSignalHandlers()` (POSIX crash diagnostics, Story 7.1.2)
  was only ever called from inside the dead `MuPlatform::Initialize()`, so it silently never
  installed on Linux/macOS. Fixed by calling `InstallSignalHandlers()` directly from `WinMain()`
  (`#ifndef _WIN32`, right after `SDL_InitSubSystem` succeeds, preserving the documented ordering)
  and deleting `MuPlatform.cpp`/`.h`, `IPlatformWindow.h`, and `sdl3/SDLWindow.cpp`/`.h` — this
  decouples the still-needed feature from the facade that never got used for its actual purpose.
  Verified via a full incremental build.
- **No documented cross-check for on-screen dock-neighbors when designing a new port's modern
  theme.** `component-catalog.md`/`building-new-ui.md`'s guidance for "which composition pattern
  does this window use" is organized entirely by *technical tier* (does it have a live-3D icon or
  `CInventoryCtrl` grid?), not by *which windows appear on screen together*. `PanelColumnX()`
  (`WindowSystem.cpp`) docks ~30 windows into the same handful of screen slots — `CMyInventory`,
  `CInventoryExtension`, `CMyQuestInfoWindow`, `CCharacterInfoWindow`, `CPartyInfoWindow`,
  `CPetInfoWindow`, and more, not yet all ported — and several of them (`CCharacterInfoWindow`
  confirmed) literally alias `CMyInventory`'s own frame texture slots, meaning the original game
  intended them to read as one visual family when open side by side. Found the hard way porting
  `CCharacterInfoWindow`: its nearest *technical-tier* sibling is `CMyQuestInfoWindow` (no
  `C3DRenderMng`, no live-3D icon, so nothing forces a native frame) — but `CMyQuestInfoWindow`'s
  own modern theme is a fully independent flat `.modern-frame`/`.modern-frame-crimson` redesign,
  which does not match `CMyInventory`'s forged-dialog chrome. `CMyInventory` only kept that
  different, native-frame-plus-overlay look because of its own paint-order constraint (its live-3D
  equipped-item icon), not as a deliberate "this is a different visual family" choice — so matching
  the nearest technical-tier precedent produced a window that broke visual continuity with its
  actual screen-neighbor. Fixed for this one window (see the "What's migrated" entry above) by
  copying `CMyInventory`'s own chrome recipe directly (no live-3D content here, so no background-
  context split was needed to reproduce it). Not yet generalized: `building-new-ui.md`/
  `component-catalog.md` don't yet instruct a future port to check its `PanelColumnX` (or
  equivalent) screen-neighbors' current look before picking a modern-theme treatment — that's the
  fix still needed, either as a step in `/port-window` or a note in the catalog, before the next
  port in this same `PanelColumnX` family. **Recurred once (2026-09-19) then fixed for real
  (same day, follow-up pass)**: `CPetInfoWindow`/`CPartyInfoWindow`'s own modern themes initially
  shipped with a flatter, simpler flat-token treatment instead of copying `CMyInventory`'s
  forged-dialog chrome, a conscious effort tradeoff at the time, not an oversight — but it meant
  these two read as a visually distinct sub-family from their own `PanelColumnX` screen-neighbors.
  Resolved by extracting the shared chrome into `docked_panel_frame.rcss` (legacy and modern, one
  per theme) instead of a fourth copy-paste: `character_info`/`my_quest_info`/`pet_info`/
  `party_info` now all link it for `#panel`'s frame/fill (legacy) or forged-dialog gradient/shell-
  edge/groove/header-rail (modern), and each keeps only genuinely window-specific content in its own
  `.rcss`. `CMyQuestInfoWindow`'s modern theme — the original outlier this whole gap note was about
  — was brought in line too (off `.modern-frame`/`.modern-frame-crimson` onto the shared forged
  recipe), so all four now render as one family, not three-plus-an-outlier. `CMyInventory` stays
  deliberately out of this partial (its frame paint lives in a separate `*_bg.rml` context that
  can't link it — a real technical constraint, not oversight). **The underlying gap — no documented
  step to check `PanelColumnX` screen-neighbors before picking a new port's modern-theme
  treatment — is still real** for the next window that joins this dock group; only the two
  recurrences have been fixed, not the process gap that let them happen twice.
- ~~Theme hot-swap (`$theme <name>`, the Options window's theme picker) relies on two separate
  "don't forget" steps, both of which have already failed once.~~ **Fixed 2026-09-22**: both the
  per-window "don't forget to override" step and the "don't forget to sweep every manager/module"
  step were replaced by a single tier-agnostic registry in `UI::RmlBridge` (`RmlTheme.h`/`.cpp`):
  `RegisterForThemeReload(const void* owner, std::function<void()> callback)` /
  `UnregisterForThemeReload(const void* owner)`, keyed by an opaque owner pointer (`this` for a
  window, a private static token's address for a free-function module), plus
  `ReloadAllThemedDocuments()` which calls every registered callback. `$theme` and the Options
  window's theme picker each now collapse their old 4-step sequence (two separate `CManager`
  sweeps plus two separate free-function calls) into one line,
  `UI::RmlBridge::ReloadAllThemedDocuments();`. All 33 window classes (the 16 already named above,
  plus every other themed window) and both free-function modules (`UI::Login::ReloadRmlTheme()`,
  `UI::RmlBridge::Tooltip::ReloadRmlTheme()`) now register once next to their own `BuildRmlUi()`
  call and unregister at the exact point, if any, they already call `RemoveUIObj(this)` in
  `Release()` — 11 of the 33 are app/scene-lifetime singleton windows (`CLoginWin`,
  `CLoginMainWin`, `CSysMenuWin`, `CServerSelWin`, `CMsgWin`, `CCreditWin`, `CCharSelMainWin`,
  `CCharMakeWin`, `CCharInfoBalloonMng`, `CGenericConfirmDialog`, `CGenericMenuDialog`) that never
  unhook from any `CManager` at all, so they never unregister either — their registration lives
  forever, matching that existing lifetime exactly; getting this backwards for even one of them
  would have silently reintroduced the exact bug this fix closes. `IObject::ReloadRmlTheme()`
  (`UI/Core/WindowObject.h`), `CManager::ReloadAllRmlThemes()`, and both old `CManager`-sweep call
  sites are deleted outright — one mechanism instead of two parallel ones, and a future
  window/tier is covered automatically with zero edits to either trigger site. See
  `component-catalog.md`'s "Theming" section for the corrected per-window pattern. Built and
  verified against a real build (`RelWithDebInfo`); in-engine smoke test (`$theme modern`/
  `$theme legacy` at the login/character-select screens, no live server needed) still pending —
  the `MAIN_SCENE` HUD tier additionally needs a live server to exercise.

## Pilots to revisit, and tracked deferrals

Moved to [`tracked-deferrals.md`](tracked-deferrals.md) (2026-09-16) -- the "Pilots to revisit"
table plus the three short tracked-deferral punch-lists (`mu::ui::window::CObject`-tier adapter
naming, `CMainFrameWindow`'s class-rename/file-split, `CUIControl` family retirement). The fourth,
much larger tracked deferral this section used to include -- `CommonMessageBox`/`CustomMessageBox`
-- is now just [`migration-ledger.md`](migration-ledger.md)'s Dialog family table (2026-09-19): the
per-class worklist this used to point at is done for all but a handful of classes, so a dedicated
file for it stopped earning its keep.

## Upstream sync log

**Branch of record changed, 2026-09-19**: local `rmlui-on-sdl-gpu` (`fork`/nitoygo's copy) turned
out to already be merged upstream via `sven-n/MuMain` PR #580, into `sven-n/MuMain`'s
`dev/rmlui-ui-system` branch — which has since picked up further commits of its own (render-capture
fixes, an `RmlModelBinder` per-model `DataTypeRegister` fix). This branch now tracks
`origin/dev/rmlui-ui-system` directly (local `rmlui-on-sdl-gpu`'s upstream repointed there), not
`fork/rmlui-on-sdl-gpu`. The PR #572 relationship logged below predates this and describes an
earlier stage of this branch's history — record here whether that relationship still holds the
next time it becomes relevant, don't assume the entries below are still current without checking.

Log every rebase/sync onto a newer upstream head here — one line per sync, not one row per upstream
commit (this branch's own commits are ours, `git log` already documents them faithfully; a heavier
per-source-commit replay ledger, like the SDL-migration branch's `docs/porting/*-ledger.md`, is
only warranted if a future sync needs real reconciliation — upstream renaming/restructuring a file
this branch also touched — not before).

| Date | Sync | Conflict verdict | Resulting tip |
|---|---|---|---|
| 2026-09-01 | Rebased onto `sven-n/MuMain` PR #572 (`a9739fb2` docs(render): document Windows parity gaps — docs-only, 2 files, zero overlap with anything this branch touches) | Clean — verified in an isolated worktree first | `878f35e4` |
| 2026-09-19 | Rebased local `rmlui-on-sdl-gpu` (1 commit: `CPetInfoWindow`/`CPartyInfoWindow` port + docked-frame unification) onto `origin/dev/rmlui-ui-system` (7 commits ahead); pushed as a fast-forward, `b3bf33d7..233d808b` | Clean — zero conflicts, verified via a full incremental build against the new base | `233d808b` |
