# Legacy UI Component Migration Ledger

One row per legacy UI class, so "is `X` done?" has a single place to check. Update the row in the
same commit as any port — a stale row reads as a confident false negative. Shapes are
`building-new-ui.md`'s reference shapes (RmlUi-only 2D, Hybrid RmlUi/native 3D, World-overlay).

Statuses: `Done` / `Partial` / `Not started` / `Stays native` (a deliberate permanent decision) /
`Deleted` (dead code removed). **`Done` means ported and signed off**: `legacy` follows the
original as closely as was practical, and further parity tuning is left to contributors rather
than tracked.

## Small-scale text and event validation

2026-10-10, signed off by the user on 2026-10-11. One focused commit per concern.

**Contract guard.** `tools/check_rml_rcss_drift.py` had stopped discovering anything. It now reads
each window's literal document declarations, reusable views and per-instance models, checks every
theme's variant for the ids and callbacks C++ needs (111 documents, 222 theme variants), and fails
on zero discovery; 28 fixtures (`tests/ui/test_rml_contracts.py`) cover it. Missing modern NPC and
quest hooks were restored and unread model fields removed. Running it and its exception markers:
[theming-and-modding.md](theming-and-modding.md).

**Party and trade.** The party list clears the reserved header and keeps all five cards above the
bottom HUD in both themes; the placement service publishes the workspace's top and bottom for it.
Legacy trade's warning and notice flow as one paragraph around the partner's confirmation. Covered
by `tests/ui/test_rml_party_trade_layout.cpp`; confirmed in game.

**Small-scale text.** Normal native text has a minimum (11 points times the OS display scale), so at
1024x768 / 75% fixed rows ran past small docked windows. The minimum is unchanged; each window
instead lays its text out to fit:

- *Blood Castle / Devil Square entry*: the description is one wrapping paragraph in a scroll pane,
  level labels wrap and centre on their bands, the level list scrolls when needed; modern gained a
  footer exit button.
- *Personal shops* (seller and buyer): native's notice rows wrap in their three groups inside one
  bounded pane between the grid and the buttons (`shop_notice.rcss` per theme); legacy's titles sit
  between the frame's corners.
- *MU Helper config*: legacy keeps native's positions, each label one line in the room native left
  before the next control; modern flows its groups in one scrolling column, 240 units wide.
- Where legacy keeps native positions (MU Helper, shop titles, the Illusion Temple result table), a
  longer label is a **marquee**: it ends in `..` and scrolls on hover
  ([component-catalog.md](component-catalog.md#counter-scaled-text)).

The headless audit (`tests/ui/test_rml_text_layout.cpp`) loads the real documents, fonts and five
translations and measures every drawn line for clipping and overlap: the event and shop windows
across 1024x768 / 75% and 1280x720 / 1920x1080 at 100-150%, the MU Helper at 1024x768 / 75% for
five classes and three tabs, each at 1x / 1.5x / 2x OS scale. Everything passes except legacy's MU
Helper, whose native row spacing still meets taller text at 1.5x / 2x; that is reported, by choice.
Re-run it in a configured developer shell:

```powershell
cmake -S . -B out/build/windows-x64 -DBUILD_TESTING=ON
cmake --build out/build/windows-x64 --config RelWithDebInfo --target rml_text_layout_tests
ctest --test-dir out/build/windows-x64 -C RelWithDebInfo -R rml_text_layout_baseline --output-on-failure
```

Results land in `out/build/windows-x64/text-layout-baseline/` (`scenarios.csv`, `text-lines.csv`).

**Event validation, through `$preview`.** Illusion Temple: the three skill-panel tooltips show in both
themes and follow skill changes; one hovered when the HUD hid stayed on screen, and now goes with
it. The legacy result table overlapped even in English; it keeps native's columns with a heading per
column and marquee cells. Siege: the preview seeds markers inside, on the edges of and outside the
hero's zoom-1 crop; measured at both zooms in both themes, they land where expected. A live siege
and temple event remain unchecked.

## Login and character select

All `Done`, both themes. These were the `CWin` toolkit, now deleted; each is a `CObject` owned by
`CSceneUICoordinator`.

| Component | Note |
|---|---|
| `CLoginWin`, `CLoginMainWin`, `CSysMenuWin`, `CServerSelWin`, `CCharSelMainWin`, `CMsgWin` | RmlUi-only 2D; `CMsgWin` is the reference screen |
| `RememberPasswordPrompt` | Free-function module (`UI::Login`); reference for a window with no reusable state |
| `CCharMakeWin` | RmlUi; the live 3D character preview is a `RenderTarget` image, so the dialog and its dimming backdrop cover the character info balloons |
| `CCharInfoBalloonMng` | World-overlay reference |
| `CCreditWin` | RmlUi-only 2D (`credit_win.rml`); illustrations as `@spritesheet` decorators |
| `COptionWin` | `Deleted` — unreachable; the live options window is `COptionWindow` |

## In-game windows

### HUD

| Component | Status | Shape | Note |
|---|---|---|---|
| `CMainFrameWindow` (+ `CSkillList`, `CItemHotKey`) | Done | Hybrid | One shared `main_frame.rml` in `dp`, its parts placed by each theme's RCSS; one workspace slot (`window-placement.md`). Skill icons are sprites (`ResolveSkillIcon()`, `skill_icons.rcss`); potions are native 3D in render targets their slots show |
| `CBuffStrip` | Done | RmlUi-only 2D | The `data-for` pilot. Right-clicking Infinity Arrow or Swell of Magic Power asks to cancel it, as native did. The hover tooltip is the shared one, name, description and duration in native's colours (`ElementTooltip`) |
| `CMuHelperBar` | Done | RmlUi-only 2D | Header slot; its buttons' hints are `data-hint` |
| `CHotKey` | Nothing to port | — | Handles hotkeys, draws nothing. Not `CItemHotKey` |
| `CGensRanking` | Done | RmlUi-only 2D | Reward text keeps native wrapping and measured row pitch; RmlUi owns scrolling and the scrollbar (drag and scale checked in game) |
| `CCommandWindow` | Done | RmlUi-only 2D | Right-docked; the twelve `CButton`s are gone. C++ keeps the armed command, its right-click run and the corner-close hit test |
| `CQuickCommandWindow` | Done | RmlUi-only 2D | Render-only: placement, hover index and clicks stay native (the control socket reads the hover index) |
| `CMoveCommandWindow` | Done | RmlUi-only 2D | Left-docked `/move` list; `.scroll-pane` replaced its hand-rolled scrollbar. The theme sizes the panel and lays the list out between the header and the close bar; a row is its text's height. Counter-scaled pane reference (`component-catalog.md`) |
| `CChatLogWindow` | Done | RmlUi-only 2D | All 200 lines in the DOM, `.scroll-pane`, `data-attr-class` per line. C++ keeps messages, filters, the 3-line resize and the pointed-line hit test; `AddText()` callers unchanged |
| `CSystemLogWindow` | Done | RmlUi-only 2D | Grows downward from a static origin, two colours, no interaction (`pointer-events: none`); shares `ChatLogLineEntry` |
| `CChatInputBox` | Done | RmlUi-only 2D | Bar art, ten buttons, tooltip and both fields (`<input>` + `.text-field`, document Tab navigation). C++ keeps history, sending and keys |
| `CMiniMap` | Done | RmlUi-only 2D | The full-screen map: 45°-turned quads as CSS `matrix()` (`UI/HUD/MiniMapLayout`), clipped around the `main_hud` slot |
| `CMasterLevel` | Done | RmlUi-only 2D | `MasterSkillTreeLayout`, `master_skill_icons.rcss`; its learn confirm is `CGenericConfirmDialog` |
| `CMuHelperConfigWindow`, `CMuHelperDetailWindow`, `CMuHelperSkillPicker` (were `CUIMuHelper`, `CMuHelperExt`, `CMuHelperSkillList`) | Done | RmlUi-only 2D | `UI/MuHelper/`; docked config and detail on the `character_info` recipe, a borderless picker whose fan-out stays in C++. Class-specific controls from `UI::MuHelper::ResolveClassFeatures()` bound as flags. Detail thresholds are level gauges (`component-catalog.md`). Deliberate behaviour changes: pick-all and pick-selected exclude each other, ticking a skill's Condition fills an empty radio group, Esc closes from a focused field, the extra-item list is always sorted. Legacy keeps native positions with marquee labels; modern flows in a 240-unit window |
| `CHelpWindow` | Done | RmlUi-only 2D | Shown unfocused, in front (`SyncDocumentVisibilityInFront()`) |
| `CWindowMenu` | Done | RmlUi-only 2D | On the `.hud-board`; row clicks queued and run from `Update()` |
| `CPartyListWindow` | Done | RmlUi-only 2D | The HUD mini list (not `CPartyInfoWindow`); C++ keeps the hovered card `Selection.cpp` reads |
| `CItemEnduranceInfo` | Done | RmlUi-only 2D | Durability icons (edge-following), pet HP frame, arrow/summon lines; hit tests in C++ |

### Character and social

| Component | Status | Shape | Note |
|---|---|---|---|
| `CCharacterInfoWindow` | Done | RmlUi-only 2D | `docked_panel_frame.rcss` family; fill-capable |
| `CPetInfoWindow` | Done | RmlUi-only 2D | `docked_panel_frame.rcss`; opens with character info. Fill-capable |
| `CPartyInfoWindow` | Done | RmlUi-only 2D | `docked_panel_frame.rcss`; member rows `data-for` over `Party[]` |
| `CNameWindow` | Done | World-overlay | `world_labels.rml` (`UI::Character::WorldLabelLayer`): pooled elements behind every other document, filled from the native label code under an `Overlay2DRecordScope` |
| `CFriendWindow` | Done | Hybrid | `UI/Social/`: shell (`friend_shell.rml`), one document per chat room and letter; the letter portrait is live 3D in a render target, driven by `UI::Social::PhotoViewerControl`. `CUIWindowMgr` still arranges them (`building-new-ui.md`). The original's F5 menu of open windows is retired: the shell's Window List tab lists them |
| `CGuildMakeWindow` | Done | RmlUi-only 2D | |
| `CGuildInfoWindow` | Done | RmlUi-only 2D | Three RmlUi-scrolled lists; the tab highlight is placed by RCSS. Checked in game: the tabs and the members list |
| `CServerMsgWin` | Done | RmlUi-only 2D | Real pixels, text at the login scene's size (`LegacyUiTransform()`), fixed face (Cousine) |

### Inventory, shops, trade

| Component | Status | Shape | Note |
|---|---|---|---|
| `CMyInventory` | Done | Hybrid | One document: frame, equipment slots (`slot_states`), the item grid (`item_grid.rcss`), the equipped and grid items in a render target, stack counts. Hit tests stay native, on the theme's grid and slot geometry |
| `CTrade`, `CStorageInventory`, `CStorageInventoryExt`, `CMixInventory`, `CNPCShop`, `CMyShopInventory`, `CPurchaseShopInventory`, `CInventoryExtension`, `CLuckyItemWnd` | Done | Hybrid | Same pattern, one document each. Their native 2D (trade's guild mark and warning arrows, the mix and lucky-item sparkles, the extension's locked-page art) draws into the window's render target with its items |
| `CItemExplanationWindow`, `CSetItemExplanation` | Done | RmlUi-only 2D | `UI/Inventory/ItemHelpView`: lines and the levels table as data, laid out by `item_help.rcss` |
| `CUnitedMarketPlaceWindow` | Done | RmlUi-only 2D | Its 3D hook draws nothing |
| `CInGameShop` | Done | Hybrid | `in_game_shop.rml`: zones, categories, the package page with its live 3D items (a render target at the original's 2° field of view), balances, banner, storage and gift boxes. Its dialogs (`igs_buy_package`, `igs_buy_select`, `igs_send_gift`) are documents too, over a dimmed screen. `$preview igs` fills it from the shipped script and banner without a server; buying, gifting and using storage items are unchecked until a server supports the cash shop, and their requests are unchanged |

### Quests and NPCs

| Component | Status | Shape | Note |
|---|---|---|---|
| `CMyQuestInfoWindow` | Done | RmlUi-only 2D | `data-for` list reference; tab reference |
| `CQuestProgress`, `CQuestProgressByEtc` | Done | RmlUi-only 2D | Share `quest_progress.rcss` (`#panel.qp-etc` modifier) and `UI::Quests::RewardModel`; 7-line pagination kept; reward preview is click, not hover |
| `CNPCQuest` | Done | Hybrid | `npc_quest.rml`: the frame, the live 3D condition items in a render target (`#nq_item`), the content. The dialogue flows in one `.sharp-flow`, centred by the theme |
| `CNPCDialogue` | Done | RmlUi-only 2D | Two paged lists in one document; Gens flows native logic synced into the model |
| `CGatemanWindow` | Done | RmlUi-only 2D | Guest, staff and master pages |
| `CEmpireGuardianNPC`, `CDoppelGangerWindow` | Done | Hybrid | `UI/Events/EventItemEntryView`: one document, the 3D preview in a render target (`#entry_item`) |

### Combat, siege, duel

| Component | Status | Shape | Note |
|---|---|---|---|
| `CCastleWindow` | Done | RmlUi-only 2D | Senatus: gate, statue and tax pages; tab positions in RCSS |
| `CGuardWindow` | Done | RmlUi-only 2D | Guild lists; tab positions in RCSS |
| `CGateSwitchWindow`, `CCatapultWindow` | Done | RmlUi-only 2D | Right-docked; catapult not checked in game |
| `CSiegeWarfare` | Done | RmlUi-only 2D | Drawn under every panel (its stacking depth); the theme lays out the team and command buttons |
| `CDuelWindow`, `CBattleSoccerScore` | Done | RmlUi-only 2D | Under every panel; event-HUD slots, drawn at the HUD's scale |
| `CDuelWatchWindow` | Done | RmlUi-only 2D | Right-docked |
| `CDuelWatchUserListWindow` | Done | RmlUi-only 2D | Event-HUD slot (bottom-left corner, grows upward), at the HUD's scale |
| `CDuelWatchMainFrameWindow` | Done | RmlUi-only 2D | Replaces the main frame while spectating; catch-up bars stepped in `Update()` |

### Events

| Component | Status | Note |
|---|---|---|
| `CEnterBloodCastle`, `CEnterDevilSquare` | Done | `UI/Events/EventEntryView` (`event_entry.rcss`); descriptions and level labels wrap in bounded scroll regions |
| `CBloodCastle`, `CChaosCastleTime`, `CEmpireGuardianTimer` | Done | `UI/Events/EventTimerView`; event-HUD slots at the HUD's scale |
| `CDoppelGangerFrame`, `CKanturuInfoWindow` | Done | Event-HUD slots at the HUD's scale |
| `CCursedTempleEnter`, `CCursedTempleResult`, `CCursedTempleSystem` | Done | Panel stage (enter, result); the Illusion Temple HUD places itself |
| `CCryWolf` | Done | Renders only in the event; seen through `$preview crywolf`/`crywolfresult` (`tracked-deferrals.md`) |
| `CGoldBowmanWindow`, `CGoldBowmanLena`, `CExchangeLuckyCoin`, `CRegistrationLuckyCoin` | Done | `UI/Events/EventItemEntryView`; the Rena and the registration coin draw into its render target. The Rena now follow the panel (the original drew them at a fixed screen x) |
| `CKanturu2ndEnterNpc` | Done | Panel stage |

### Options

| Component | Status | Note |
|---|---|---|
| `COptionWindow` | Done | Legacy: `window_shell`, six tabs, custom `data-for` dropdowns (`engine-findings.md`: no native `<select>`), level gauges for sound, music and effect limit; drags (`window-placement.md`). Modern: its own full-screen markup on the same model, a category rail and a frosted glass pane (`backdrop-filter`), toggle switches and sliders, a compact layout on a short or narrow screen (`@media` in `dp`); above the HUD (stacking depth 10.67), not draggable. The in-game system menu opens it from a full-screen menu in the same style (`CGenericMenuDialog`'s system menu) |
| `CChatCommandWindow` | Done | Its value field is an `<input>` claiming RmlUi's text-input identity while focused |

**Not ledgered**: `CGroup` (`CManager` plumbing); `CTextBox`, `CSlideWindow`
(native widgets that retire with their hosts); `CMessageBoxMng` (the dialog
family's manager).

## Dialog family (`UI/Dialogs/CommonMessageBox.h` / `CustomMessageBox.h`)

16 native dialog classes were replaced and deleted — 3 by `CGenericConfirmDialog`, 13 by
`CGenericMenuDialog` (`component-catalog.md`'s Dialog). Both primitives are also called from many
other places for single confirmations. What remains of the native family:

| Component | Status | Note |
|---|---|---|
| `CGuild_ToPerson_Position` | Done | RmlUi through `UI/Dialogs/MessageBoxView`; its buttons are placed by the theme (the box's kind) |
| `CGemIntegrationDisjointMsgBox` | Done; not checked in game | `MessageBoxView` with a scrolling jewel list; stale items rejected again at confirmation |
| `CBloodCastleResultMsgBoxLayout`, `CDevilSquareRankMsgBoxLayout`, `CChaosCastleResultMsgBoxLayout` | Done | `MessageBoxView`; texts from each match's `CollectMatchResult()`, from the box's top-left; `$preview bcresult`, `ccresult`, `dsrank` |
| `CProgressMsgBox` layouts (crown switch, seal register, crown defence) and `CCursedTempleProgressMsgBox` layouts | Done | `MessageBoxView` with frame and progress bar; `$preview switchbox` |
| `CQuestCountLimitMsgBoxLayout` | Compiled out | Its only creator is under `ASG_ADD_TIME_LIMIT_QUEST`, not defined |
| `C3DItemCommonMsgBox` | Deleted | No users left; item dialogs are `CGenericConfirmDialog` with `item3D` |

Native message boxes are not draggable (`window-placement.md` section 8). Each box's theme places it
on the `.stage` by its kind (`message_box_view.rcss`, the cash shop's `igs_*.rcss`); C++ gives only its
size and what is inside it, from its top-left.

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
| Map name banner | `CUIMapName::Render()` | Done: `map_name.rml`, behind every window |
| Party HP bars over heads, Kanturu result banner, siege build-time bars, Hellas object labels | `RenderPartyHP()`, `M39Kanturu3rd::RenderKanturu3rdinterface()`, `battleCastle::RenderBuildTimes()`, `RenderObjectDescription()` | Done: recorded by the world-label layer |
| Siege crown switch lines, macro cooldown, event entry countdown | `RenderSwichState()`, `RenderTimes()`, `CSBaseMatch::RenderTime()` | Done: `hud_status.rml` on the HUD board (`UI::Hud::StatusTexts`); `$preview status` |
| Reconnect dialog | `UI::Reconnect::RenderDialog()` | Done: `reconnect_dialog.rml`, above every document |
| Login scene logo and lines | `NewRenderLogInScene()` | Done: `login_scene.rml` (`Scenes::LoginOverlay`) |
| Loading screen art | `LoadingScene()` | Done: `loading.rml` |
| Photo viewer help text | — | Done: the shared tooltip |
| Guild war / battle soccer time and result | `RenderTournamentInterface()` | Done: `match_status.rml` (`UI::Hud::MatchStatus`), the countdown on the HUD board and the result on the `.stage`; OpenMU never sends its packets, so it is seen through `$preview guildwar` |
| Mouse cursor | `RenderCursor()` | Stays native: drawn after RmlUi |
| FPS counter, debug info, GL stats, IME/whisper debug text | `SceneManager.cpp`, `ImeInput.cpp`, `Whisper.cpp` | Stays native: developer overlays |
