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
| `CCharacterInfoWindow` | `CObject`-tier | Done | RmlUi-only 2D | `STATUS.md` "What's migrated" |
| `CNameWindow` | `CObject`-tier | Not started | TBD | |
| `CPetInfoWindow` | `CObject`-tier | Not started | TBD | Docks alongside `CMyInventory`/`CCharacterInfoWindow` (`PanelColumnX`) — check on-screen neighbors before picking a modern-theme treatment, per `STATUS.md`'s "no documented cross-check" gap |

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
| `CPartyListWindow` | `CObject`-tier | Not started | TBD | |
| `CPartyInfoWindow` | `CObject`-tier | Not started | TBD | Docks with `CMyInventory`/`CCharacterInfoWindow` — same dock-neighbor check as `CPetInfoWindow` above |
| `CFriendWindow` | `CObject`-tier | Stays native (transitional stopgap) | Native-only (stopgap) | `building-new-ui.md`'s own reference-shape table and "`UIWindows.cpp`/`CFriendWindow` pattern" section — thin adapter over a live legacy subsystem (friend/mail/chat-room), not excluded from a future port, just not scheduled |
| `CGuildMakeWindow` | `CObject`-tier | Not started | TBD | |
| `CGuildInfoWindow` | `CObject`-tier | Not started | TBD | Already a `CGenericConfirmDialog` caller for its alliance-master-can't-leave notice — the window shell itself is still fully native |

### Quests

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CMyQuestInfoWindow` | `CObject`-tier | Done | RmlUi-only 2D | `component-catalog.md`'s "List / repeated rows" — the `data-for` reference for `CUITextListBox<T>` retirement |
| `CQuestProgress` | `CObject`-tier | Not started | TBD | |
| `CQuestProgressByEtc` | `CObject`-tier | Not started | TBD | |
| `CNPCQuest` | `CObject`-tier + live-3D (`I3DRenderObj`) | Not started | TBD (likely Hybrid) | |

### NPCs

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CNPCShop` | `CObject`-tier + live-3D | Done | Hybrid RmlUi/native 3D | Listed under Inventory/Shop above |
| `CNPCDialogue` | `CObject`-tier | Not started | TBD | |
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
| `COptionWindow` | `CObject`-tier | Not started | TBD | **Flagged by this audit**: `STATUS.md` says `CSysMenuWin`'s Option button "opens `mu::ui::window::COptionWindow` instead" of the deleted `CWin`-tier `COptionWin`, which could read as "already replaced." It has zero RmlUi call sites — the live in-game Options window today is fully native. Worth a one-line correction in `STATUS.md` if this reads as more done than it is. |
| `CServerMsgWin` | `CObject`-tier | Not started | TBD | **Flagged by this audit**: sibling of the already-done `CMsgWin` (same folder, same era) but has zero RmlUi call sites — not previously called out anywhere as a separate, still-native class. |
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
and their native classes deleted outright (see `dialog-migration-plan.md` for the full list and
per-dialog history: 3 on `CGenericConfirmDialog`, 13 on `CGenericMenuDialog`). What's left, by
current grep of the two headers:

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
