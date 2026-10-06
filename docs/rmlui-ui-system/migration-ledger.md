# Legacy UI Component Migration Ledger

One row per legacy UI class, so "is `X` done?" has a single place to check. Update the row in the
same commit as any port — a stale row reads as a confident false negative. Shapes are
`building-new-ui.md`'s reference shapes (RmlUi-only 2D, Hybrid RmlUi/native 3D, World-overlay).

Statuses: `Done` / `Partial` / `Not started` / `Stays native` (a deliberate permanent decision) /
`Deleted` (dead code removed). **`Done` means ported, not audited**: `tracked-deferrals.md`'s
"audit where ports steered away from the original UI" lists known divergences — read it before
treating a row as settled.

## Login and character select

All `Done`, both themes. These were the `CWin` toolkit, now deleted; each is a `CObject` owned by
`CSceneUICoordinator`.

| Component | Note |
|---|---|
| `CLoginWin`, `CLoginMainWin`, `CSysMenuWin`, `CServerSelWin`, `CCharSelMainWin`, `CMsgWin` | RmlUi-only 2D; `CMsgWin` is the reference screen |
| `RememberPasswordPrompt` | Free-function module (`UI::Login`); reference for a window with no reusable state |
| `CCharMakeWin` | Hybrid: the character preview stays live 3D |
| `CCharInfoBalloonMng` | World-overlay reference |
| `CCreditWin` | RmlUi-only 2D (`credit_win.rml`); illustrations as `@spritesheet` decorators |
| `COptionWin` | `Deleted` — unreachable; the live options window is `COptionWindow` |

## In-game windows

### HUD

| Component | Status | Shape | Note |
|---|---|---|---|
| `CMainFrameWindow` (+ `CSkillList`, `CItemHotKey`) | Done | Hybrid | One shared `main_frame.rml` in `dp`, its parts placed by each theme's RCSS; one workspace slot (`window-placement.md`). Skill icons are sprites (`ResolveSkillIcon()`, `skill_icons.rcss`); potions are native 3D in render targets their slots show |
| `CBuffStrip` | Done | RmlUi-only 2D | The `data-for` pilot. Right-clicking Infinity Arrow or Swell of Magic Power asks to cancel it, as native did. Legacy has native's per-line-coloured tooltip; modern keeps a plain one by choice |
| `CMuHelperBar` | Done | RmlUi-only 2D | Header slot |
| `CHotKey` | Nothing to port | — | Handles hotkeys, draws nothing. Not `CItemHotKey` |
| `CGensRanking` | Done | RmlUi-only 2D | Reward text keeps native wrapping and measured row pitch; RmlUi owns scrolling and the scrollbar (drag and scale checked in game) |
| `CCommandWindow` | Done | RmlUi-only 2D | Right-docked; the twelve `CButton`s are gone. C++ keeps the armed command, its right-click run and the corner-close hit test |
| `CQuickCommandWindow` | Done | RmlUi-only 2D | Render-only: placement, hover index and clicks stay native (the control socket reads the hover index) |
| `CMoveCommandWindow` | Done | RmlUi-only 2D | Left-docked `/move` list; `.scroll-pane` replaced its hand-rolled scrollbar. `UI::MoveCommand::CalculateLayout()` keeps the dock-derived height. Counter-scaled pane reference (`component-catalog.md`) |
| `CChatLogWindow` | Done | RmlUi-only 2D | All 200 lines in the DOM, `.scroll-pane`, `data-attr-class` per line. C++ keeps messages, filters, the 3-line resize and the pointed-line hit test; `AddText()` callers unchanged |
| `CSystemLogWindow` | Done | RmlUi-only 2D | Grows downward from a static origin, two colours, no interaction (`pointer-events: none`); shares `ChatLogLineEntry` |
| `CChatInputBox` | Done | RmlUi-only 2D | Bar art, ten buttons, tooltip and both fields (`<input>` + `.text-field`, document Tab navigation). C++ keeps history, sending and keys |
| `CMiniMap` | Done | RmlUi-only 2D | The full-screen map: 45°-turned quads as CSS `matrix()` (`UI/HUD/MiniMapLayout`), clipped around the `main_hud` slot |
| `CMasterLevel` | Done | RmlUi-only 2D | `MasterSkillTreeLayout`, `master_skill_icons.rcss`; its learn confirm is `CGenericConfirmDialog` |
| `CMuHelperConfigWindow`, `CMuHelperDetailWindow`, `CMuHelperSkillPicker` (were `CUIMuHelper`, `CMuHelperExt`, `CMuHelperSkillList`) | Done | RmlUi-only 2D | `UI/MuHelper/`; docked config and detail on the `character_info` recipe, a borderless picker whose fan-out stays in C++. Class-specific controls from `UI::MuHelper::ResolveClassFeatures()` bound as flags. Detail thresholds are level gauges (`component-catalog.md`). Native bugs fixed are listed in the divergence audit |
| `CHelpWindow` | Done | RmlUi-only 2D | Shown unfocused, in front (`SyncDocumentVisibilityInFront()`) |
| `CWindowMenu` | Done | RmlUi-only 2D | `LayoutMode::Hud`; row clicks queued and run from `Update()` |
| `CPartyListWindow` | Done | RmlUi-only 2D | The HUD mini list (not `CPartyInfoWindow`); C++ keeps the hovered card `Selection.cpp` reads |
| `CItemEnduranceInfo` | Done | RmlUi-only 2D | Durability icons (edge-following), pet HP frame, arrow/summon lines; hit tests in C++ |

### Character and social

| Component | Status | Shape | Note |
|---|---|---|---|
| `CCharacterInfoWindow` | Done | RmlUi-only 2D | `docked_panel_frame.rcss` family; fill-capable |
| `CPetInfoWindow` | Done | RmlUi-only 2D | `docked_panel_frame.rcss`; opens with character info. Fill-capable |
| `CPartyInfoWindow` | Done | RmlUi-only 2D | `docked_panel_frame.rcss`; member rows `data-for` over `Party[]` |
| `CNameWindow` | Done | World-overlay | `world_labels.rml` (`UI::Character::WorldLabelLayer`): pooled elements in the background context, filled from the native label code under an `Overlay2DRecordScope` |
| `CFriendWindow` | Done | Hybrid | `UI/Social/`: shell (`friend_shell.rml`), one document per chat room and letter; the letter portrait is live 3D in a render target, driven by `UI::Social::PhotoViewerControl`. `CUIWindowMgr` still arranges them (`building-new-ui.md`). The original's F5 menu of open windows is retired: the shell's Window List tab lists them |
| `CGuildMakeWindow` | Done | RmlUi-only 2D | |
| `CGuildInfoWindow` | Done | RmlUi-only 2D | Three RmlUi-scrolled lists; the tab highlight is placed by RCSS. Checked in game: the tabs and the members list |
| `CServerMsgWin` | Done | RmlUi-only 2D | Real pixels (`LayoutMode::Legacy`), fixed face (Cousine) |

### Inventory, shops, trade

| Component | Status | Shape | Note |
|---|---|---|---|
| `CMyInventory` | Done | Hybrid | Frame in the background context; grids, equipped items and the paperdoll chrome stay native (`tracked-deferrals.md`). Native grids follow RCSS anchors |
| `CTrade`, `CStorageInventory`, `CStorageInventoryExt`, `CMixInventory`, `CNPCShop`, `CMyShopInventory`, `CPurchaseShopInventory`, `CInventoryExtension`, `CLuckyItemWnd` | Done | Hybrid | Same pattern. Stays native: the grids, the lucky-item sparkle, the trade warning-arrow glyph |
| `CItemExplanationWindow`, `CSetItemExplanation` | Done | RmlUi-only 2D | `UI/Inventory/TipTextListView` |
| `CUnitedMarketPlaceWindow` | Done | RmlUi-only 2D | Its 3D hook draws nothing |
| `CInGameShop` | Partial | — | **Unscheduled, not permanently native.** OpenMU has no cash shop, which gates verification, not the port. Done: the backdrop (`in_game_shop_bg.rml`), the storage/gift list and two `MsgBoxIGS*` list dialogs. Still native: frame, 11 `CButton`s, 3 `CRadioGroupButton` columns, texts, banner, paging, and `MsgBoxIGSSendGift`'s two text fields. Its sub-dialogs are the last native consumers of the `mu::ui::window` widgets |

### Quests and NPCs

| Component | Status | Shape | Note |
|---|---|---|---|
| `CMyQuestInfoWindow` | Done | RmlUi-only 2D | `data-for` list reference; tab reference |
| `CQuestProgress`, `CQuestProgressByEtc` | Done | RmlUi-only 2D | Share `quest_progress.rcss` (`#panel.qp-etc` modifier) and `UI::Quests::RewardModel`; 7-line pagination kept; reward preview is click, not hover |
| `CNPCQuest` | Done | Hybrid | Frame in `npc_quest_bg.rml` (background context) under the live 3D condition items; content in `npc_quest.rml`. Message/answer tops stay in the model (`tracked-deferrals.md`) |
| `CNPCDialogue` | Done | RmlUi-only 2D | Two paged lists in one document; Gens flows native logic synced into the model |
| `CGatemanWindow` | Done | RmlUi-only 2D | Guest, staff and master pages |
| `CEmpireGuardianNPC`, `CDoppelGangerWindow` | Done | Hybrid | `UI/Events/EventItemEntryView`: frame in the background context under the native 3D preview |

### Combat, siege, duel

| Component | Status | Shape | Note |
|---|---|---|---|
| `CCastleWindow` | Done | RmlUi-only 2D | Senatus: gate, statue and tax pages; tab positions in RCSS |
| `CGuardWindow` | Done | RmlUi-only 2D | Guild lists; tab positions in RCSS |
| `CGateSwitchWindow`, `CCatapultWindow` | Done | RmlUi-only 2D | Right-docked; catapult not checked in game |
| `CSiegeWarfare` | Done | RmlUi-only 2D | Background context (drawn under every panel); buttons still hit-test natively |
| `CDuelWindow`, `CBattleSoccerScore` | Done | RmlUi-only 2D | Background context; event-HUD slots, drawn at the HUD's scale (`LayoutMode::HudFrame`) |
| `CDuelWatchWindow` | Done | RmlUi-only 2D | Right-docked |
| `CDuelWatchUserListWindow` | Done | RmlUi-only 2D | Event-HUD slot (bottom-left corner, grows upward), `LayoutMode::HudFrame` |
| `CDuelWatchMainFrameWindow` | Done | RmlUi-only 2D | Replaces the main frame while spectating; catch-up bars stepped in `Update()` |

### Events

| Component | Status | Note |
|---|---|---|
| `CEnterBloodCastle`, `CEnterDevilSquare` | Done | `UI/Events/EventEntryView` (`event_entry.rcss`), rows by `:nth-child` |
| `CBloodCastle`, `CChaosCastleTime`, `CEmpireGuardianTimer` | Done | `UI/Events/EventTimerView`; event-HUD slots, `LayoutMode::HudFrame` |
| `CDoppelGangerFrame`, `CKanturuInfoWindow` | Done | Event-HUD slots, `LayoutMode::HudFrame` |
| `CCursedTempleEnter`, `CCursedTempleResult`, `CCursedTempleSystem` | Done | Panel stage (enter, result); the Illusion Temple HUD places itself |
| `CCryWolf` | Done; not seen | Renders only in the event (`tracked-deferrals.md`) |
| `CGoldBowmanWindow`, `CGoldBowmanLena`, `CExchangeLuckyCoin`, `CRegistrationLuckyCoin` | Done | `UI/Events/EventItemEntryView`; the registration coin stays native 3D |
| `CKanturu2ndEnterNpc` | Done | Panel stage |

### Options

| Component | Status | Note |
|---|---|---|
| `COptionWindow` | Done | `window_shell`, six tabs, custom `data-for` dropdowns (`engine-findings.md`: no native `<select>`), level gauges for sound, music and effect limit, close button built in C++. Drags (`window-placement.md`) |
| `CChatCommandWindow` | Done | Its value field is an `<input>` claiming RmlUi's text-input identity while focused |

**Not ledgered**: `C3DCamera`, `CGroup` (`CManager` plumbing); `CTextBox`, `CSlideWindow`,
`CScrollBar` (native widgets that retire with their hosts); `CMessageBoxMng` (the dialog
family's manager).

## Dialog family (`UI/Dialogs/CommonMessageBox.h` / `CustomMessageBox.h`)

16 native dialog classes were replaced and deleted — 3 by `CGenericConfirmDialog`, 13 by
`CGenericMenuDialog` (`component-catalog.md`'s Dialog). Both primitives are also called from many
other places for single confirmations. What remains of the native family:

| Component | Status | Note |
|---|---|---|
| `CGuild_ToPerson_Position` | Done | RmlUi through `UI/Dialogs/MessageBoxView`; its buttons are placed by the theme (the box's kind) |
| `CGemIntegrationDisjointMsgBox` | Done; not checked in game | `MessageBoxView` with a scrolling jewel list; stale items rejected again at confirmation |
| `CBloodCastleResultMsgBoxLayout`, `CDevilSquareRankMsgBoxLayout`, `CChaosCastleResultMsgBoxLayout` | Done | `MessageBoxView`; texts from each match's `CollectMatchResult()` |
| `CProgressMsgBox` layouts (crown switch, seal register, crown defence) and `CCursedTempleProgressMsgBox` layouts | Done | `MessageBoxView` with frame and progress bar |
| `CQuestCountLimitMsgBoxLayout` | Compiled out | Its only creator is under `ASG_ADD_TIME_LIMIT_QUEST`, not defined |
| `C3DItemCommonMsgBox` | With `CInGameShop` | Only user: the cash shop's `MsgBoxIGSStorageItemInfo` |

Native message boxes are not draggable (`window-placement.md` section 8).

## `CUIControl` list family (deleted)

Retired. `CUITextListBox<T>` and every instantiation (quest, guild, union, notice, socket, unmix,
siege guild, chat, letter, window and friend lists, the MU Helper's extra-item list, the cash
shop's three lists, the unused move-command list), `CUITextInputBox`, `CUIButton` and the rest are
deleted; their hosts bind `data-for` lists in `.scroll-pane`s. `CUIControl` itself is gone too:
`CUIBaseWindow` (`UI/Social/SocialWindowBase.h`) carries what the friend/mail/chat windows use, and
`CUIPhotoViewer` stands on its own. Not checked in game
since the change: the socket list (mix), the guild info lists, Lahap's unmix list, the guard's two
siege lists, the cash shop lists.

## Native surfaces outside the window classes

| Surface | Native code | Status |
|---|---|---|
| Centre-screen notices | `UI::Notices::Render()` | Done: `notices.rml`, above every document but the tooltip |
| Map name banner | `CUIMapName::Render()` | Done: `map_name.rml`, background context |
| Party HP bars over heads, Kanturu result banner, siege crown switch lines and build-time bars, Hellas object labels | `RenderPartyHP()`, `M39Kanturu3rd::RenderKanturu3rdinterface()`, `RenderSwichState()`, `battleCastle::RenderBuildTimes()`, `RenderObjectDescription()` | Done: recorded by the world-label layer |
| Reconnect dialog | `UI::Reconnect::RenderDialog()` | Done: `reconnect_dialog.rml`, above every document |
| Login scene logo and lines | `NewRenderLogInScene()` | Done: `login_scene.rml` (`Scenes::LoginOverlay`) |
| Loading screen art | `LoadingScene()` | Done: `loading.rml` |
| Photo viewer help text | — | Done: the shared tooltip |
| Tournament countdown | `RenderTournamentInterface()` | Left native: OpenMU never sends its packets |
| Mouse cursor | `RenderCursor()` | Stays native: drawn after RmlUi |
| FPS counter, debug info, GL stats, IME/whisper debug text | `SceneManager.cpp`, `ImeInput.cpp`, `Whisper.cpp` | Stays native: developer overlays |
