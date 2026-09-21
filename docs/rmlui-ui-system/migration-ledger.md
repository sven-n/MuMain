# Legacy UI Component Migration Ledger

An inventory of every legacy UI class in scope for this port, one row each, so "is `X` done?" has a
single place to check instead of requiring a grep. `STATUS.md` narrates *why* decisions were made
and carries the running "known gaps" list; this file is the flat index those decisions land in.
Read `architecture-principles.md` and `STATUS.md` first — this ledger assumes their vocabulary
(tiers, "Hybrid"/"World-overlay"/etc. target shapes from `building-new-ui.md`'s Reference-screens
table) without re-explaining it.

**Compiled 2026-09-16** by grepping the tree for every class deriving from the window base classes
below, cross-checked against `LoadThemedDocument`/`RmlModelBinder`/`BuildRmlUi` call sites (signals
"has RmlUi involvement") and `I3DRenderObj`/`RenderItem3D`/`RenderObjectScreen` call sites (signals
"has live-3D content, likely Hybrid shape"). This is a snapshot, not a promise — update the row for
any window touched by a port, per the "Checklist for every new port" step this entry adds to
`STATUS.md`. A stale row is worse than no row; if you port something and don't update its line here,
the next person trusts a false "Not started."

## Columns

| Column | Meaning |
|---|---|
| Component | Native C++ class name |
| Category | Which legacy universe it comes from — determines the porting mechanism needed: `CWin`-tier (oldest, fully retired), `CObject`-tier (current `mu::ui::window::CManager`-dispatched base), `CObject`-tier + live-3D (needs the background-context/`RenderBackgroundLayer()` mechanism), Dialog family (`CommonMessageBox.h`/`CustomMessageBox.h`), or `CUIControl` list family (`UIControls.h`) |
| Status | `Done` / `Partial` / `Not started` / `Stays native` (a deliberate permanent decision, not merely unscheduled) / `Deleted` (removed as confirmed-dead code) |
| Target shape / primitive | For `CObject`-tier screens, one of `building-new-ui.md`'s 4 shapes (`RmlUi-only 2D`, `Hybrid RmlUi/native 3D`, `World-overlay`, `Native-only (stopgap)`); for dialogs, `CGenericConfirmDialog` / `CGenericMenuDialog` / bespoke-native; for list widgets, `data-for`. `TBD` where no port has scoped it yet — a guess in parentheses reflects a strong signal (e.g. live-3D content found), not a decision; the real answer is decided at port time per §27, not speculated here. |
| Detail pointer | Where the full story (rationale, what stays native and why, build history) actually lives |

## CWin-tier (retired — closed historical set)

Zero live subclasses of `CWin`/`CWinEx` remain anywhere in the tree. All done, both themes,
verified against a real server.

| Component | Status | Detail pointer |
|---|---|---|
| `CLoginWin` | Done | `STATUS.md` "What's migrated" |
| `CLoginMainWin` | Done | `STATUS.md` "What's migrated" |
| `CSysMenuWin` | Done | `STATUS.md` "What's migrated" |
| `RememberPasswordPrompt` | Done | `STATUS.md` "What's migrated"; `building-new-ui.md`'s pure-RmlUi reference |
| `CCharSelMainWin` | Done | `STATUS.md` "What's migrated" |
| `CCharMakeWin` | Done | `STATUS.md` "What's migrated"; character-preview panel is the canonical permanent-native-3D example (`ui-target-architecture.md` Section E) |
| `CCharInfoBalloonMng` | Done | `STATUS.md` "What's migrated"; `building-new-ui.md`'s World-overlay reference |
| `CMsgWin` | Done | `STATUS.md` "What's migrated" |
| `CServerSelWin` | Done | `README.md` Coexistence patterns; two-column RML/RCSS rework |
| `COptionWin` | Deleted | Confirmed unreachable dead code, deleted outright — see `README.md`'s Coexistence patterns |

## CObject-tier / C3DRenderMng-tier windows

The `mu::ui::window::CManager`-dispatched surface. Grouped by subsystem folder; "Not started" rows
have had no investigation beyond confirming no RmlUi call sites exist, not a shape decision.

### HUD

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CMainFrameWindow` (+ nested `CSkillList`) | `CObject`-tier + live-3D | Done | Hybrid RmlUi/native 3D | `STATUS.md` "What's migrated" — 3-phase port |
| `CBuffStrip` | `CObject`-tier | Done | RmlUi-only 2D | `STATUS.md` "What's migrated"; the `data-for` pilot |
| `CMuHelperBar` | `CObject`-tier | Done | RmlUi-only 2D | `STATUS.md` "What's migrated" |
| `CHotKey` | `CObject`-tier | Not started | TBD | Not the same class as `CItemHotKey` (a nested type inside `CMainFrameWindow`, already ported as part of Phase 3) — verify relationship before scoping |
| `CGensRanking` | `CObject`-tier | Not started | TBD | Aliases `CCharacterInfoWindow`'s texture slots (`STATUS.md`) — check for coupling before porting either independently |
| `CCommandWindow` | `CObject`-tier | Not started | TBD | |
| `CQuickCommandWindow` | `CObject`-tier | Not started | TBD | |
| `CMoveCommandWindow` | `CObject`-tier | Not started | TBD | |
| `CChatLogWindow` / `CSystemLogWindow` | `CObject`-tier | Not started | TBD | Same header, two classes |
| `CMiniMap` | `CObject`-tier | Not started | TBD | |
| `CMasterLevel` | `CObject`-tier | Not started | TBD | Already a `CGenericConfirmDialog` *caller* for one confirm popup — the window's own chrome is still fully native |
| `CUIMuHelper` (+ `CMuHelperSkillList`, `CMuHelperExt`) | `CObject`-tier | Not started | TBD | The MU Helper bot *configuration* window — distinct from the always-visible `CMuHelperBar`, which is done |

### Character

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CCharacterInfoWindow` | `CObject`-tier | Done | RmlUi-only 2D | `STATUS.md` "What's migrated"; frame/shell shared with `CMyQuestInfoWindow`/`CPetInfoWindow`/`CPartyInfoWindow` via `docked_panel_frame.rcss` (2026-09-19, see `STATUS.md`'s dock-neighbor gap note) |
| `CNameWindow` | `CObject`-tier | Not started | TBD | |
| `CPetInfoWindow` | `CObject`-tier | Done (2026-09-19) | RmlUi-only 2D | No live-3D content (pure text/icon/bar), same shape as `CCharacterInfoWindow`. The docking coupling this row used to flag as unverified turned out to be real but harmless: `WindowSystem.cpp`'s existing native show/hide choreography (`INTERFACE_PET` forces `INTERFACE_CHARACTER` open and vice versa on close) needed zero changes — it only ever reads/writes `m_Pos`/`IsVisible()`, same as before the port. `CCharacterInfoWindow::RmlClickPet()` already called `Toggle(INTERFACE_PET)` before this port landed (a contract waiting to be fulfilled). Group boxes (`RenderGroupBox()`'s 9-slice tiling) reproduced as corner-bracket sprites + flat fills in legacy, a single flat `token(surface-panel)` rect in modern — same simplification `character_info.rcss`'s own summary box already established, not a literal 1px-tile port. Its modern theme initially shipped flatter than `CCharacterInfoWindow`'s forged-dialog look (a conscious effort tradeoff), then unified with it same-day once the shared `docked_panel_frame.rcss` partial existed — see `STATUS.md`'s dock-neighbor gap note. |

### Inventory / Shop / Trade

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CMyInventory` | `CObject`-tier + live-3D | Done | Hybrid RmlUi/native 3D | `STATUS.md` "What's migrated" |
| `CTrade`, `CStorageInventory`, `CStorageInventoryExt`, `CMixInventory`, `CNPCShop`, `CMyShopInventory`, `CPurchaseShopInventory`, `CInventoryExtension`, `CLuckyItemWnd` | `CObject`-tier + live-3D | Done (2026-09-13) | Hybrid RmlUi/native 3D | `STATUS.md` "Rest of the inventory family" |
| `CItemExplanationWindow` | `CObject`-tier | Not started | TBD | |
| `CItemEnduranceInfo` | `CObject`-tier | Not started | TBD | |
| `CSetItemExplanation` | `CObject`-tier | Not started | TBD | |
| `CUnitedMarketPlaceWindow` | `CObject`-tier + live-3D (`I3DRenderObj`) | Not started | TBD (likely Hybrid) | `component-catalog.md`: "not in this family [inventory `C3DRenderMng` group] — no `CInventoryCtrl`/item grid... still native, but not blocked by anything here" |
| `CInGameShop` | `CObject`-tier, has live-3D render calls | Not started | TBD (likely Hybrid) | Several of its sub-dialogs (`MsgBoxIGS*`) already call `CGenericConfirmDialog` for individual confirms — the shop shell itself is still fully native |

### Party / Guild

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CPartyListWindow` | `CObject`-tier | Not started | TBD | Different class from `CPartyInfoWindow` below — the always-on HUD mini list, fixed position, not the full management window. Aliases `IMAGE_PARTY_FLAG`/`IMAGE_PARTY_EXIT`'s numeric slot IDs from `CPartyInfoWindow::IMAGE_LIST` (loads its own copies of the same files independently, doesn't depend on `CPartyInfoWindow::LoadImages()` having run) — a real compile-time coupling to keep in mind if this window is ever trimmed further. |
| `CPartyInfoWindow` | `CObject`-tier | Done (2026-09-19) | RmlUi-only 2D | No live-3D content, same shape as `CCharacterInfoWindow`. Looser docking than `CPetInfoWindow`: opens standalone (just closes Character/Inventory first via `HideAllGroupA()`), no forced pairing — `WindowSystem.cpp` needed zero changes. Member rows (`RenderMemberStatue`'s `iIndex * 71` pixel math) ported to a `data-for` list stacked via normal block flow (`height: 71px` per row) instead of computed offsets — first repeated-row list in this codebase built directly against a live global array (`Party[]`/`PartyNumber`) each frame rather than a cached snapshot, since the row count is small (`MAX_PARTYS` = 5). `IMAGE_LIST` enum and `LoadImages()`/`UnloadImages()` kept despite the window no longer rendering through the legacy bitmap-atlas system, same reason `CCharacterInfoWindow` kept its own — `CPartyListWindow` aliases two of this window's texture slot IDs. Frame/shell now shared with `CCharacterInfoWindow`/`CMyQuestInfoWindow`/`CPetInfoWindow` via `docked_panel_frame.rcss` (both themes) — see `STATUS.md`'s dock-neighbor gap note for why this exists. |
| `CFriendWindow` | `CObject`-tier | Stays native (transitional stopgap) | Native-only (stopgap) | `building-new-ui.md`'s own reference-shape table and "`UIWindows.cpp`/`CFriendWindow` pattern" section — thin adapter over a live legacy subsystem (friend/mail/chat-room), not excluded from a future port, just not scheduled |
| `CGuildMakeWindow` | `CObject`-tier | Not started | TBD | |
| `CGuildInfoWindow` | `CObject`-tier | Not started | TBD | Already a `CGenericConfirmDialog` caller for its alliance-master-can't-leave notice — the window shell itself is still fully native |

### Quests

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CMyQuestInfoWindow` | `CObject`-tier | Done | RmlUi-only 2D | `component-catalog.md`'s "List / repeated rows" — the `data-for` reference for `CUITextListBox<T>` retirement. Modern theme moved off its own `.modern-frame`/`.modern-frame-crimson` onto the shared forged-dialog recipe in `docked_panel_frame.rcss` (2026-09-19) — it was the visual outlier `STATUS.md`'s dock-neighbor gap note flags; now matches `CCharacterInfoWindow`/`CPetInfoWindow`/`CPartyInfoWindow`. |
| `CQuestProgress` | `CObject`-tier | Done | RmlUi-only 2D | `component-catalog.md`'s "List / repeated rows" (`UI::Quests::RewardModel`, shared with `CMyQuestInfoWindow`). Frame/chrome shared with `CCharacterInfoWindow`/`CMyQuestInfoWindow`/`CPetInfoWindow`/`CPartyInfoWindow` via `docked_panel_frame.rcss`. Retires its own `CUIQuestContentsListBox`/3 `CButton`s for a `data-for` reward list + RmlUi buttons; NPC-dialogue pagination (7 lines/page, L/R buttons revealing the answer list on the last page) is deliberately preserved as real intent, not replaced with a scrollbar. Reward-item hover-preview became click-to-preview, matching `CMyQuestInfoWindow`'s own already-shipped precedent for the same list class. Selected reward item's info popup stays the same permanent native `::RenderItemInfo()` hybrid boundary `CMyQuestInfoWindow` already established. Build verified (`RelWithDebInfo`); in-game verification against a live server still pending. |
| `CQuestProgressByEtc` | `CObject`-tier | Done | RmlUi-only 2D | Same port as `CQuestProgress` above, sharing `quest_progress.rcss` (one stylesheet, two separate `.rml`/data-model documents — RmlUi binds one model name per document, so a literal shared `.rml` isn't possible) and the same `QuestProgressRmlModel`/`RmlModelBinder<T>` struct shape. Structurally simpler than `CQuestProgress`: no NPC-name/player-name preamble at all (omitted from its own `.rml` entirely, not a data-driven toggle) — the handful of remaining pixel deltas (NPC-line/button top offsets, answers-block top) are handled by a `#panel.qp-etc` modifier-class override in the shared `.rcss`. `WindowSystem.cpp`'s docking/mutual-exclusion dispatch (column-swap with `CMyQuestInfoWindow`/`CCharacterInfoWindow`, no `HideAllGroupA()` sweep) and the `ProcessClosing()`-time `g_QuestMng.DelQuestIndexByEtcList()` call are both untouched. Build verified (`RelWithDebInfo`); in-game verification against a live server still pending. |
| `CNPCQuest` | `CObject`-tier + live-3D (`I3DRenderObj`) | Done | Hybrid RmlUi/native 3D, split fg/bg documents | The third and last member of the NPC-dialogue family, driven by the older `g_csQuest`/`CSQuest`/`DialogStructure` quest system (not `CQuestMng`, the system `CNPCDialogue`/`CQuestProgress` use). Two mutually-exclusive upper panels (quest-condition checklist or a Zen-cost banner) plus a shared dialogue/numbered-answers block, whose top offset is a real per-instance computed value (native's own vertical-centering formula) rather than a static CSS number. The live quest-condition item preview (`Render3D()`/`RenderItem3D()`) stays a permanent native hybrid boundary — genuine rotating-3D-model content, the same class of decision as `CQuestProgress`'s own `::RenderItemInfo()` boundary, just for multiple always-visible rows instead of one on-hover popup; its condition-evaluation math (`FindQuestItemsInInven`/`GetKillMobCount`, including the exact color/completion mapping) was split from its old inline-draw call into a data-row builder, same class of split `UI::Quests::RewardModel::BuildRows()` already established. Unlike every other window in this family, its frame chrome couldn't stay in the same document as its content: RmlUi's main context always renders last in the frame, so an opaque `#panel` there would paint over the live-3D preview drawn earlier the same frame. Split into two documents instead, same mechanism `CNPCShop`/`CMyInventory` already use for their own live-3D icon grids — `npc_quest_bg.rml` (frame chrome, `docked_panel_frame.rcss`'s `#panel`/`.frame-piece`/shell-edge-groove-header-rail) loads into `RmlUiRuntime::GetBackgroundContext()` (rendered explicitly before the 3D pass); `npc_quest.rml` (all actual content) stays on the main context, whose own root (`#content_root`) is a deliberately unpainted positioning anchor — including its own explicit `font-family`, since that no longer comes for free from `docked_panel_frame.rcss`'s `#panel` rule the way every sibling's font does. Build verified (`RelWithDebInfo`) and confirmed working in-game. |

### NPCs

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CNPCShop` | `CObject`-tier + live-3D | Done | Hybrid RmlUi/native 3D | Listed under Inventory/Shop above |
| `CNPCDialogue` | `CObject`-tier | Done | RmlUi-only 2D | The window a quest-giving NPC actually opens first (`ProcessOpening()`/`ReceiveQuestByNPCEPList`/`ReceiveQuestByEtcEPList`) — was the source of a modern-theme "looks legacy" report, since it had zero RmlUi/theme participation before this port; only after picking a step from it does the server push a `PMSG_QUEST_STEP_INFO` that opens the already-ported `CQuestProgress`/`CQuestProgressByEtc` above. Own `npc_dialogue.rml`/`.rcss` (not shared with `quest_progress.rcss` despite reusing the same `Quest_bt_L/R.tga` sprite asset — the layout isn't a near-identical sibling: two independently paginated lists in one document, an NPC-word pager (top) and a sel-text pager (bottom) whose rows come from either `GetNPCDlgAnswer()` or quest-list-mode's `SetQuestListText()` quest subjects, both reduced to the same `{text, index}` row shape via `NPCDialogueRmlModel`). The "page to the end reveals the list" gate is preserved exactly (now a same-click side effect, matching the native single-button double-duty). Retires all 5 `CButton`s and every native `Render*()` call — no hybrid native-draw boundary left at all, unlike `CQuestProgress`'s reward-item popup. Gens contribute-point banner (NPC 543/544) and the Gens join/secede/reward flows (`ASG_ADD_GENS_SYSTEM`/`PBG_ADD_GENSRANKING`, unconditionally on in this build) are untouched native logic, just reactively synced into the model. Fixed a fragile-transitive-include situation this same session's earlier `QuestProgress.h` cleanup had patched around: `NPCDialogue.h` no longer needs `MessageBox.h`/`MyInventory.h`/`MyQuestInfoWindow.h`/`QuestProgress.h`/`Button.h` at all (those were only for texture-slot enum aliasing and the now-removed `CButton` members). Build verified (`RelWithDebInfo`); in-game verification against a live server still pending. |
| `CGatemanWindow` | `CObject`-tier | Not started | TBD | |
| `CEmpireGuardianTimer` | `CObject`-tier | Not started | TBD | |
| `CEmpireGuardianNPC` | `CObject`-tier + live-3D (`I3DRenderObj`) | Not started | TBD (likely Hybrid) | |

### Combat / Duel

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CCastleWindow` | `CObject`-tier | Not started | TBD | Already a `CGenericConfirmDialog` caller for a sub-dialog — shell itself still native |
| `CGuardWindow` | `CObject`-tier | Not started | TBD | Same as above |
| `CDuelWindow` | `CObject`-tier | Not started | TBD | |
| `CDuelWatchWindow` | `CObject`-tier | Not started | TBD | |
| `CDuelWatchUserListWindow` | `CObject`-tier | Not started | TBD | |
| `CDuelWatchMainFrameWindow` | `CObject`-tier + live-3D (`I3DRenderObj`) | Not started | TBD (likely Hybrid) | |
| `CSiegeWarfare` | `CObject`-tier | Not started | TBD | |

### Events (all not started — small, self-contained per-event status/timer windows)

| Component | Category | Notes |
|---|---|---|
| `CCatapultWindow` | `CObject`-tier | |
| `CEnterBloodCastle` | `CObject`-tier | |
| `CBattleSoccerScore` | `CObject`-tier | |
| `CBloodCastle` | `CObject`-tier | |
| `CChaosCastleTime` | `CObject`-tier | |
| `CCursedTempleResult` | `CObject`-tier | |
| `CCryWolf` | `CObject`-tier | |
| `CCursedTempleSystem` | `CObject`-tier | |
| `CCursedTempleEnter` | `CObject`-tier | |
| `CDoppelGangerFrame` | `CObject`-tier | |
| `CDoppelGangerWindow` | `CObject`-tier + live-3D (`I3DRenderObj`) | TBD (likely Hybrid) |
| `CEnterDevilSquare` | `CObject`-tier | |
| `CGateSwitchWindow` | `CObject`-tier | |
| `CGoldBowmanLena` | `CObject`-tier, incidental 3D render call | Signal is an NPC-model render, not necessarily a Hybrid-shape icon — verify at port time |
| `CGoldBowmanWindow` | `CObject`-tier | |
| `CExchangeLuckyCoin` | `CObject`-tier | |
| `CRegistrationLuckyCoin` | `CObject`-tier, incidental 3D render call | Same caveat as `CGoldBowmanLena` |
| `CKanturu2ndEnterNpc` | `CObject`-tier | |
| `CKanturuInfoWindow` | `CObject`-tier | |

### Options / System

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `COptionWindow` | `CObject`-tier | Done | RmlUi-only 2D, via `window_shell`, 6-tab layout (Gameplay/Audio/Video/Graphics/UI/General) | Built and verified live against a real server, both themes (2026-09-19). Grew substantially past the original flat 10-row layout: tabbed to make room for settings expansion (5 checkboxes, 3 sliders, 5 dropdowns across the 6 tabs), and surfaced several previously console-only/unreachable `GameConfig` settings and `MainScene.h` DXP-23 effect-cost toggles as real, persisted UI (VSync, FPS cap, disable-effects/particles/skill-effect-models/boids/wing-shadow, show-FPS-counter/show-debug-info, live UI-theme switch, and the global UI scale — a 6th dropdown on the UI tab over a fixed
50–200 % ladder, re-applied live by resizing the window to its current size, which is also this
window's first hover tooltip: a plain `.option-row-tip` sibling shown by an RCSS `:hover` rule, no
C++ hover callback, see `layout-and-scaling.md`'s "Global UI scale" section). The close button is built directly in C++ into `window_shell`'s own `#window_shell_footer` anchor (a direct `#panel` child, sibling of `#content`, matching `generic_confirm_dialog`'s own button placement) rather than authored as `{{}}`-bound RML — see `engine-findings.md`'s new "moving an already-parsed element doesn't preserve a `{{}}` text binding" entry for why. All 5 dropdowns ended up as a custom `data-for`/`data-class`/`data-event-click` control (`.option-dropdown` family) instead of RmlUi's native `<select>` — see `engine-findings.md`'s new native-`<select>` entry; three separate rounds of fixing the native widget's missing sub-element styling never got clicking an option to reliably commit. Also fixed along the way: two scenes (`LoginScene.cpp`/`CharacterScene.cpp`) that manually pump this window while it's shown outside `MAIN_SCENE` were missing an `Update()` call, leaving every bound label blank the first time the window opened before ever visiting the main scene (`engine-findings.md`'s new persistent-document-manual-pump entry); a native, unrelated bug in `CServerMsgWin::Release()` not resetting its own visibility flag, which produced a one-frame flicker at its last position when a server notice was showing at the exact moment of the character-select→main-scene transition. |
| `CServerMsgWin` | `CObject`-tier | Not started | TBD | Sibling of the already-done `CMsgWin` (same folder, same era) but has zero RmlUi call sites — still fully native, not yet a distinct port target. **Bug fixed in passing (2026-09-19)**, unrelated to porting: `Release()` never reset its own `CObject` visibility flag (every sibling window torn down in the same `CSceneUICoordinator::CreateMainScene()` sweep does), so a notice visible at the exact moment of the character-select→main-scene transition kept rendering for a frame against already-released sprites — a stray upper-left flicker. Fixed by resetting the flag directly in `Release()`. |
| `CChatCommandWindow` | `CObject`-tier | Not started | TBD | |
| `CHelpWindow` | `CObject`-tier | Not started | TBD | |
| `CWindowMenu` | `CObject`-tier | Not started | TBD | |
| `CCreditWin` | `CObject`-tier | Done | RmlUi-only 2D | **Flagged by this audit**: has a confirmed `LoadThemedDocument` call (`credit_win.rml`) but isn't named in `STATUS.md`'s "What's migrated" list anywhere — appears to be a real, already-shipped port that was never logged. Worth adding to `STATUS.md` if confirmed. |

### Excluded from this ledger (infrastructure / reusable widgets, not standalone "components")

`C3DCamera` and `CGroup` (`UI/Core/`) are `CManager` plumbing, not ported windows. `CTextBox`,
`CSlideWindow`, `CScrollBar`, `CChatInputBox` (`UI/Widgets/Window/`) are low-level composable
widgets used *by* several windows above, not top-level components with their own migration status —
they retire implicitly as their host windows port. `CMessageBoxMng` (`UI/Dialogs/MessageBox.h`) is
the manager/plumbing class underlying the whole `TMsgBoxLayout<T>` mechanism the Dialog family below
uses, not a dialog itself.

## Dialog family (`UI/Dialogs/CommonMessageBox.h` / `CustomMessageBox.h`)

The native `TMsgBoxLayout<T>` family is far smaller today than `STATUS.md`'s historical "~140
classes" figure — that count predates this branch's migration work. 16 dialogs are already ported
and their native classes deleted outright (3 on `CGenericConfirmDialog`, 13 on
`CGenericMenuDialog` — see `component-catalog.md`'s Dialog section for how the two primitives
work; per-dialog porting history lives in git log, not a doc, once a dialog is done). What's left,
by current grep of the two headers:

| Component | Status | Target primitive | Detail pointer |
|---|---|---|---|
| `CGuild_ToPerson_Position` | Stays native | Bespoke (simultaneous radio-select) | `component-catalog.md`'s Dialog section |
| `CGemIntegrationDisjointMsgBox` | Stays native | Bespoke (embedded live inventory list-selection widget) | `component-catalog.md`'s Dialog section |
| `CQuestCountLimitMsgBoxLayout` (`CCommonMessageBox`) | Not started | TBD | |
| `CBloodCastleResultMsgBoxLayout` | Not started | TBD | |
| `CDevilSquareRankMsgBoxLayout` | Not started | TBD | |
| `CChaosCastleResultMsgBoxLayout` | Not started | TBD | |
| `CCrownSwitchPopLayout`, `CCrownSwitchPushLayout`, `CCrownSwitchOtherPushLayout` (`CProgressMsgBox`) | Not started | TBD | Share one underlying shape class |
| `CSealRegisterStartLayout`, `CSealRegisterSuccessLayout`, `CSealRegisterFailLayout`, `CSealRegisterOtherLayout`, `CSealRegisterOtherCampLayout` (`CProgressMsgBox`) | Not started | TBD | Share one underlying shape class |
| `CCrownDefenseRemoveLayout`, `CCrownDefenseCreateLayout` (`CProgressMsgBox`) | Not started | TBD | Share one underlying shape class |
| `CCursedTempleHolicItemGetLayout`, `CCursedTempleHolicItemSaveLayout` (`CCursedTempleProgressMsgBox`) | Not started | TBD | Share one underlying shape class |

Also present: `C3DItemCommonMsgBox` (`CommonMessageBox.h`) — a `CMessageBoxBase` shape class with
live-3D item content, no `...Layout` instantiation found using it directly in the current grep;
verify whether it's still a live shape before scoping a port.

## `CUIControl` list family (`UI/Widgets/UIControls.h`)

The `data-for` binding pattern is proven (`component-catalog.md`'s "List / repeated rows") on 2 of
18 `CUITextListBox<T>` instantiations. The other 16 are the tracked deferral in
`tracked-deferrals.md`.

| Component | Row type | Status | Detail pointer |
|---|---|---|---|
| `CUICurQuestListBox` | `SCurQuestItem` | Done | `component-catalog.md` — `CMyQuestInfoWindow`'s quest list |
| `CUIQuestContentsListBox` | `SQuestContents` | Done | `component-catalog.md` — same window |
| `CUIGuildListBox` | `GUILDLIST_TEXT` | Not started | `tracked-deferrals.md` |
| `CUISimpleChatListBox` | `WHISPER_TEXT` | Not started | `tracked-deferrals.md` |
| `CUILetterTextListBox` | `LETTER_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIChatPalListBox` | `GUILDLIST_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIWindowListBox` | `WINDOWLIST_TEXT` | Not started | `tracked-deferrals.md` |
| `CUILetterListBox` | `LETTERLIST_TEXT` | Not started | `tracked-deferrals.md` |
| `CUISocketListBox` | `SOCKETLIST_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIGuildNoticeListBox` | `GUILDLOG_TEXT` | Not started | `tracked-deferrals.md` |
| `CUINewGuildMemberListBox` | `GUILDLIST_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIUnionGuildListBox` | `UNIONGUILD_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIExtraItemListBox` | `FILTERLIST_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIUnmixgemList` | `UNMIX_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIBCDeclareGuildListBox` | `BCDECLAREGUILD_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIBCGuildListBox` | `BCGUILD_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIMoveCommandListBox` | `MOVECOMMAND_TEXT` | Not started | `tracked-deferrals.md` |
| `CUIInGameShopListBox` | `IGS_StorageItem` | Not started | `tracked-deferrals.md` |
| `CUIBuyingListBox` | `IGS_BuyList` | Not started | `tracked-deferrals.md` |
| `CUIPackCheckBuyingListBox` | `IGS_SelectBuyItem` | Not started | `tracked-deferrals.md` |

## Using this ledger

Before starting a new port: find the component's row, read its Detail pointer for the real story,
and check the "Checklist for every new port" in `STATUS.md`. After landing a port: update the row's
Status/Target columns here in the same commit — this file drifting out of sync with reality is worse
than it not existing, since a stale "Not started" reads as a confident false negative.
