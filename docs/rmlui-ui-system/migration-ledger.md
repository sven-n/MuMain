# Legacy UI Component Migration Ledger

One row per legacy UI class, so "is `X` done?" has a single place to check. Update the row in the
same commit as any port — a stale row reads as a confident false negative. Shapes are
`building-new-ui.md`'s reference shapes (RmlUi-only 2D, Hybrid RmlUi/native 3D, World-overlay).

Statuses: `Done` / `Partial` / `Not started` / `Stays native` (a deliberate permanent decision) /
`Deleted` (dead code removed). **`Done` means ported and signed off**: `legacy` follows the
original as closely as was practical, and further parity tuning is left to contributors rather
than tracked.

## Contract validation

2026-10-10: rollout 0 from [tracked-deferrals.md](tracked-deferrals.md) completed. The drift guard
now checks 111 documents / 222 theme variants / 144 files and rejects zero coverage. Its 28
regression fixtures exercise discovery, independent theme contracts, template fallback,
alternative readouts and explicit exceptions. The fixtures are registered with CTest as
`rml_contract_guard_tests`.

Missing modern markup hooks were restored in `npc_dialogue`, `npc_quest`, `my_quest_info`,
`quest_progress` and `quest_progress_etc`, using their existing styles. Unused model fields,
bindings and synchronization were removed; workspace dimensions remain C++ view state.
Deliberately omitted controls have reasons the guard can review and check for staleness. Syntax,
bound-geometry and active-transform checks passed, and RelWithDebInfo x64 rebuilt successfully
after the cleanup. Runtime dragging/maximize and the repaired NPC geometry and reward-tooltip
hooks have not been checked in a running client.

For running/reviewing the guard and its limits, see [theming-and-modding.md](theming-and-modding.md).

## Party and trade layout follow-up

2026-10-10: rollout 1 from [tracked-deferrals.md](tracked-deferrals.md) completed after the user
confirmed the fixes working in game. The party HUD in both themes clears the reserved header and
keeps all five cards above the bottom HUD while following open docks. Legacy trade's warning
and notice form one wrapping paragraph around the partner's confirmation. Its divider is thinner
and sits above the lower nickname strip, giving translations more room; grid and confirmation
positions are preserved.

Headless RmlUi tests load the real theme assets and bundled font. Party bounds and hover targets
pass at 800x600, 1280x720, 1920x1080 and 3440x1440 at 75/100/125/150%, with docks open and
closed. Legacy trade fits English, German, Spanish, Polish and Russian notices at 1024x768 at
the same scales, with no text intersecting the partner's confirmation. These checks use 1x OS
display scale. The user's in-game confirmation did not include per-configuration results, so it
does not establish a full resolution, localization or OS display-scale sweep. RelWithDebInfo x64 rebuilt
successfully with runtime assets staged; syntax, contract-drift, bound-geometry and active-transform
guards passed, along with 28 contract fixtures and both layout test cases (2,905 assertions).

## Small-scale text baseline

2026-10-10: rollout 2, step 1 from [tracked-deferrals.md](tracked-deferrals.md) completed.
The diagnostic loads the real RML/RCSS and bundled fonts, supplies text from the current C++
model assignments and resource strings, and measures projected glyph meshes. Its 570 scenarios
cover both themes, English/German/Spanish/Polish/Russian, and 1x/1.5x/2x OS display scale at
1024x768 / 75% UI scale. MU Helper covers all three tabs for Dark Knight, Dark Wizard, Elf,
Dark Lord and Summoner, using the game's class-feature rules. Missing translated resource keys
use the same English fallback as the generated game resources.

| Window | Legacy scenarios with defects | Modern scenarios with defects |
|---|---:|---:|
| Blood Castle entry | 15/15 | 15/15 |
| Devil Square entry | 15/15 | 15/15 |
| Seller personal shop | 15/15 | 12/15 |
| Buyer personal shop | 15/15 | 12/15 |
| MU Helper config | 225/225 | 225/225 |

The event titles and several descriptions/level labels exceed their assigned boxes. Blood
Castle descriptions can intersect the first level button; Devil Square's fixed description
rows can intersect one another. Legacy shop notices are clipped horizontally, and fixed notice
rows in both themes fail with longer strings. MU Helper reproduces label/field and label/label
collisions, including Original Position/Distance, Distance/numeric entry and adjacent skill
headers. These describe the initial baseline before the event-entry follow-up below.

At this configuration, normal native text is 12/16.5/22 physical pixels for 1x/1.5x/2x OS scale.
Its existing minimum is 11/16.5/22 respectively, checked against the unchanged 11–16 point policy.
The Blood Castle character split retained the complete descriptions in these five locales;
the observed problems are geometry rather than lost source text. Other languages are not covered.

To rerun in a configured Windows developer shell:

```powershell
cmake -S . -B out/build/windows-x64 -DBUILD_TESTING=ON
cmake --build out/build/windows-x64 --config RelWithDebInfo --target rml_text_layout_tests
ctest --test-dir out/build/windows-x64 -C RelWithDebInfo -R rml_text_layout_baseline --output-on-failure
```

The output directory is `out/build/windows-x64/text-layout-baseline/`. `scenarios.csv` records
the configuration, native size/minimum, source completeness and defect count. `text-lines.csv`
records every measured line, its glyph bounds, element box, available width, clipping/overflow/
collision reason and a colliding neighbour's bounds. Prepared RML files preserve the supplied
strings for inspection. Auto-sized, overflow-visible spans are checked against neighbours and
the panel rather than being falsely rejected for having a zero-width element box.

The initial baseline test required defects in all five windows and checked fixture integrity
and the font floor. The current runner requires the fixed event windows to pass containment,
source-completeness and scrolling checks while continuing to report the shop/helper defects.
Event font inputs now use the unchanged native sizing policy directly; the replaced SDL_ttf
measurement fixture was removed. Custom font selections, live model synchronization, actual
entry requests and visual readability still require in-game validation.

Contract, syntax, bound-geometry and active-transform guards passed, as did all 28 contract
fixtures, the party/trade regressions and 42 scaling test cases. The diagnostic and client
RelWithDebInfo x64 builds passed; the final client build was incremental because this step changes
test coverage and documentation. The prior `BUILD_TESTING=OFF` configuration was restored after
validation, and all 385 staged RML/RCSS/INI assets match their source files. The deferral remains
open for the remaining implementation and in-game validation.

## Event entry text follow-up

2026-10-10: rollout 2, step 2 implemented; in-game confirmation pending. Blood Castle displays
the complete translated paragraph, and Devil Square displays all six complete fragments in
their original order. Both themes wrap descriptions inside a bounded pane above the level list.
Scrollbars appear when needed, and wheel/drag scrolling makes the remaining prose reachable.
Titles use the header's available width without covering the corner close target.

Level labels wrap at the retained native font size. The list scrolls when taller labels need
more room, keeping every level band reachable and the footer outside the scroll regions.
This intentionally replaces fixed row spacing while preserving the legacy frame/button art,
grey locked bands, enabled-band hover and entry actions. Legacy button art stretches to each
wrapped row. Modern uses its own button styling and now shows a footer exit icon. Exit and
Escape keep their existing close actions. The native readability minimum is unchanged; no new
fitting feature was needed. Replaced per-line fitting fields, text caches and native width/
line-height measurements were removed.

The runner covers 930 scenarios: 420 event cases plus the 510 remaining shop/helper baseline
cases. Event geometry passes in both themes for English, German, Spanish, Polish and Russian:
1024x768 / 75%, and 1280x720 and 1920x1080 / 100, 125 and 150%, each at 1x, 1.5x and 2x OS
display scale. Checks cover complete wrapped text, unchanged native font sizes/minimum,
non-overlapping header/description/list/footer regions, visible scroll affordances when needed,
the end of each scroll region, and hover access to every level button after scrolling. The glyph
audit measures all lines for horizontal clipping and only compares painted content for collisions;
vertical clipping inside an accessible scroll pane is intentional. The rerun command remains the
one above. Headless checks do not establish visual readability or server-side entry behaviour.

In-game checks before signing off this pilot:

- In both themes, open Blood Castle and Devil Square at 1024x768 / 75%; check the full description,
  title, wrapped level labels and separation between descriptions, buttons and exit controls.
- Wheel and drag each scrollbar to both ends. Verify the last description fragment and every
  level band remain reachable, with no clicks passing through to the world.
- Verify locked bands stay grey and cannot enter, the eligible band highlights and requests
  entry, and the footer exit, corner close target and Escape close the window.
- Change theme, resolution and UI scale while shown; check 100/125/150% at 1280x720 and
  1920x1080. Check a longer translation and 1.5x/2x OS scale where available, including legacy
  stretched button art and modern footer icon visibility.

Personal-shop notices are the next rollout step; MU Helper remains after that. Rollout 2 stays
open until its remaining fixes and in-game checks are complete.

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
| `CMuHelperConfigWindow`, `CMuHelperDetailWindow`, `CMuHelperSkillPicker` (were `CUIMuHelper`, `CMuHelperExt`, `CMuHelperSkillList`) | Done | RmlUi-only 2D | `UI/MuHelper/`; docked config and detail on the `character_info` recipe, a borderless picker whose fan-out stays in C++. Class-specific controls from `UI::MuHelper::ResolveClassFeatures()` bound as flags. Detail thresholds are level gauges (`component-catalog.md`). Deliberate behaviour changes: pick-all and pick-selected exclude each other, ticking a skill's Condition fills an empty radio group, Esc closes from a focused field, the extra-item list is always sorted |
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
| `CEnterBloodCastle`, `CEnterDevilSquare` | Done | `UI/Events/EventEntryView` (`event_entry.rcss`); descriptions and level labels wrap in bounded scroll regions; rollout-2 follow-up awaiting in-game confirmation |
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
