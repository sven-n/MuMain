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
| `CMainFrameWindow` (+ nested `CSkillList`) | `CObject`-tier + live-3D | Done | Hybrid RmlUi/native 3D | `STATUS.md` "What's migrated" — 3-phase port; skill icons/boxes RmlUi since 2026-09-27 (`ResolveSkillIcon()`, `skill_icons.rcss`), only the item-hotkey potions stay native 3D |
| `CBuffStrip` | `CObject`-tier | Done | RmlUi-only 2D | `STATUS.md` "What's migrated"; the `data-for` pilot |
| `CMuHelperBar` | `CObject`-tier | Done | RmlUi-only 2D | `STATUS.md` "What's migrated" |
| `CHotKey` | `CObject`-tier | Nothing to port | — | `CHotKey::Render()` only returns true: the window handles hotkeys and draws nothing (verified 2026-09-28). Not the same class as `CItemHotKey`, a nested type inside `CMainFrameWindow`, already ported with it |
| `CGensRanking` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `gens_ranking.rml` + both themes. |
| `CCommandWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `command_window.rml` + both themes (modern overrides the markup for its forged shell). Right-docked; `docked_panel_frame.rcss`'s frame at this window's own 432 height. The twelve `CButton`s are gone: RmlUi draws up/over/down frames and the armed command pressed and bold; C++ keeps the armed command, the right-click that runs it, the cursor state and the corner-close hit test. The target box at the pointer is part of the same document. Title shrunk to its 72-unit box via `UI::Scaling::NativeTextPixelSizeInBox()`. |
| `CQuickCommandWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `quick_command.rml` + both themes. Render-only port: placement at the pointer, the hover index and the clicks stay native (the control socket's quick-peer observation reads the same index), the document mirrors them and takes no pointer events. |
| `CMoveCommandWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `move_command.rml` + both themes. The left-docked warp list (`/move`), and the **second consumer of `base.rcss`'s `.scroll-pane`** — the one that actually retires a hand-rolled scrollbar: `ThumbYForScrollOffset`/`ScrollOffsetForThumbY`/`UpdateDragState`/`MaximumScrollOffset`/`ClampScrollOffset` and the three-state `MOVECOMMAND_MOUSE_EVENT` drag machine are all deleted, along with their five unit tests and this window's whole `IMAGE_LIST` (it aliased `CChatLogWindow::IMAGE_SCROLL_*`, which is why that enum was kept two commits earlier; it loaded its own `LoadBitmap` copies, so the coupling was compile-time only). `UI::MoveCommand::CalculateLayout()` stays — the window genuinely derives its height from the dock column, that is real intent, not scroll bookkeeping. **The only `LayoutMode::DockLeft` window in the game**, so it has no dock neighbours to match and links no shared frame partial; modern borrows `docked_panel_frame.rcss`'s forged vocabulary at rail scale rather than its 190x429 dialog chrome. First `.scroll-pane` consumer inside a `transform: scale(root_scale)` panel — see `component-catalog.md`'s new counter-scale note for the technique and what it costs. |
| `CChatLogWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `chat_log.rml` + both themes. First consumer of `base.rcss`'s `.scroll-pane`, and the first list in this codebase bound with `data-attr-class` (a per-line class composed in the model, instead of nine `data-class-*` attributes). RmlUi owns the fill, the lines, the wheel and the scrollbar; C++ keeps the message vectors, the filters, the 3-line-step resize, and the pointed-line hit test. `AddText()`'s 333 call sites are untouched -- it was always a data API. See `STATUS.md` for the DOM-scroll decision and what it cost. |
| `CSystemLogWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `system_log.rml` + both themes. The top-left system/error overlay, ported right after the `CChatLogWindow` it shares a file with, and reusing its `ChatLogLineEntry` line shape. Simpler in three ways that are easy to get wrong by copying its file-mate: it grows DOWNWARD from a fully static origin (`Create()` at (0,80), `m_WndSize.cy` never recomputed, `SetPosition()` never called), it has only two colours (system-blue vs error-red for everything else, not a per-type map), and its row pitch is font-derived (`MeasureText(L"Q").cy * 1.2`) so RCSS's own `line-height` default of 1.2 reproduces it by doing nothing. No scrolling and no interaction at all, so the whole panel is `pointer-events: none`. |
| `CMiniMap` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `mini_map.rml` + both themes. The map texture and its markers were quads turned 45 degrees in physical pixels (`RenderBitRotate`/`RenderPointRotate`); `UI/HUD/MiniMapLayout` rebuilds them and places each element with a CSS `matrix()`. Pulled in front of the HUD documents, the main frame's document pulled back in front of it; the map paints only outside the native HUD band (clip regions with `clip: always`, a copy of the picture each). |
| `CMasterLevel` | `CObject`-tier | Done | RmlUi-only 2D | `STATUS.md` "What's migrated" (2026-09-27): `master_level.rml`, `MasterSkillTreeLayout`, `master_skill_icons.rcss`; its learn confirm draws above the tree since plain dialog chrome paints from the main context (`a7798a62`) |
| `CMuHelperConfigWindow` (was `CUIMuHelper`) + `CMuHelperDetailWindow` (was `CMuHelperExt`) + `CMuHelperSkillPicker` (was `CMuHelperSkillList`) | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `mu_helper_config.rml`, `mu_helper_detail.rml`, `mu_helper_skill_picker.rml` + both themes; shared pieces in `themes/*/mu_helper_common.rcss`. The bot's *configuration* windows (hotkey `Z`, and the `CMuHelperBar` config button), now in `UI/MuHelper/`. Config and detail are docked `PanelColumnX(1)`/`(2)` panels on the `character_info` recipe; the picker is a borderless flyout whose fan-out layout stays in C++. Every control has one fixed position per id, so the config window is static RML positioned by RCSS, and which class sees which control is one tested table, `UI::MuHelper::ResolveClassFeatures()`, bound as flags — never RCSS. The detail window's fill gauges keep native C++ input (click, wheel, drag) over RmlUi art. Deleted: `UI/Core/WindowMuHelper.h/.cpp` (~3,400 lines), `CUIExtraItemListBox`, 44 leaked `CButton`/`CCheckBox`, the dead `RenderSkillInfo` tooltip chain. Native bugs fixed on the way (see `tracked-deferrals.md`'s divergence audit): pick-all/pick-selected now truly exclusive, a skill condition page opened with no radio set now gets a default, the potion gauge's hit test no longer runs on every page, the buff interval is no longer saved as 0 from other pages, right-clicking a number plate no longer writes past the slot array. Native `CUIMuHelper::Show()` released every `CUITextInputBox`'s focus as a side effect, which hid the dead startup box `g_pMercenaryInputBox` holding focus from launch; the port removed that box (and the equally dead `CUILoginInputBox`) rather than keep the side effect. |

### Character

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CCharacterInfoWindow` | `CObject`-tier | Done | RmlUi-only 2D | `STATUS.md` "What's migrated"; frame/shell shared with `CMyQuestInfoWindow`/`CPetInfoWindow`/`CPartyInfoWindow` via `docked_panel_frame.rcss` (2026-09-19, see `STATUS.md`'s dock-neighbor gap note) |
| `CNameWindow` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `world_labels.rml` + both themes (`UI::Character::WorldLabelLayer`): pooled label elements in the background context, behind its other documents, filled in `PrepareBackgroundLayer()` from what the unchanged native label code draws under an `Overlay2DRecordScope` (names, balloons, shop titles, Gens marks, health bars, item names, macro bar, event times); glow-blended quads through `additive-fill(...)`. |
| `CPetInfoWindow` | `CObject`-tier | Done (2026-09-19) | RmlUi-only 2D | No live-3D content (pure text/icon/bar), same shape as `CCharacterInfoWindow`. The docking coupling this row used to flag as unverified turned out to be real but harmless: `WindowSystem.cpp`'s existing native show/hide choreography (`INTERFACE_PET` forces `INTERFACE_CHARACTER` open and vice versa on close) needed zero changes — it only ever reads/writes `m_Pos`/`IsVisible()`, same as before the port. `CCharacterInfoWindow::RmlClickPet()` already called `Toggle(INTERFACE_PET)` before this port landed (a contract waiting to be fulfilled). Group boxes (`RenderGroupBox()`'s 9-slice tiling) reproduced as corner-bracket sprites + flat fills in legacy, a single flat `token(surface-panel)` rect in modern — same simplification `character_info.rcss`'s own summary box already established, not a literal 1px-tile port. Its modern theme initially shipped flatter than `CCharacterInfoWindow`'s forged-dialog look (a conscious effort tradeoff), then unified with it same-day once the shared `docked_panel_frame.rcss` partial existed — see `STATUS.md`'s dock-neighbor gap note. |

### Inventory / Shop / Trade

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CMyInventory` | `CObject`-tier + live-3D | Done | Hybrid RmlUi/native 3D | `STATUS.md` "What's migrated" |
| `CTrade`, `CStorageInventory`, `CStorageInventoryExt`, `CMixInventory`, `CNPCShop`, `CMyShopInventory`, `CPurchaseShopInventory`, `CInventoryExtension`, `CLuckyItemWnd` | `CObject`-tier + live-3D | Done (2026-09-13) | Hybrid RmlUi/native 3D | `STATUS.md` "Rest of the inventory family" |
| `CItemExplanationWindow` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `item_explanation.rml` through `UI/Inventory/TipTextListView`. Never showed in the original (block-scope externs read `ItemHelp` through a same-named reference, and widths other than 640/800/1024/1280 divided by zero); both fixed. |
| `CItemEnduranceInfo` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `item_endurance.rml` + both themes: the durability icons (dock right), the pet HP frame and the arrow / summon lines (screen overlay). Hit tests stay in C++; main context behind its other documents. |
| `CSetItemExplanation` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `set_item_explanation.rml` through `UI/Inventory/TipTextListView`. |
| `CUnitedMarketPlaceWindow` | `CObject`-tier + live-3D (`I3DRenderObj`) | Done (2026-09-27): `united_market_place.rml` + both themes; RmlUi-only 2D (the 3D hook draws nothing) | TBD (likely Hybrid) | `component-catalog.md`: "not in this family [inventory `C3DRenderMng` group] — no `CInventoryCtrl`/item grid... still native, but not blocked by anything here" |
| `CInGameShop` | `CObject`-tier, has live-3D render calls | Partial | — | Unscheduled, **not** permanently native (corrected 2026-10-02 — this row used to say "Stays native"). OpenMU has no cash shop server side, so its transaction paths can't be exercised against a live server; that gates *verification*, not the port, and the catalog/storage/paging/selection half needs no server at all. Migrate when there is a reason to. It and its `MsgBoxIGS*` sub-dialogs are the last consumers in the client that still draw `UIControls.h` widgets natively — every other surviving call site is a guarded fallback — so the toolkit header cannot be deleted while they stand. Several sub-dialogs already call `CGenericConfirmDialog` for individual confirms. **Partially ported 2026-10-02** by the `CUIControl` retirement, which only ever reached the toolkit lists: the 640x429 backdrop is `in_game_shop_bg.rml` in the background context (the 3x3 package grid's live 3D items draw above it, as in the inventory family), the storage/gift list is `in_game_shop.rml`, and the two `MsgBoxIGS*` list dialogs are `igs_buy_package.rml` and `igs_buy_select.rml`. Everything else is still native and **is not this rollout's work**: the frame decorations, 11 `CButton`s, 3 `CRadioGroupButton` columns, all texts, the banner and paging are `mu::ui::window` widgets, which the retirement's scope boundary excludes. No task schedules them; per `ui-target-architecture.md` that family is "a transitional bridge for the still-native population only" that shrinks as windows are ported, so they go when `CInGameShop` is ported as a window -- still unscheduled. `MsgBoxIGSSendGift`'s two `CUITextInputBox` fields are the one toolkit consumer left here, deferred behind CUI-05 where the multiline/IME input story is settled once |

### Party / Guild

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CPartyListWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `party_list.rml` + both themes. Different class from `CPartyInfoWindow` below — the always-on HUD mini list. Cards, tints, flag, health bar and leave buttons are RmlUi; C++ keeps `Party[]`, the hovered card (the skill target `Selection.cpp` reads) and the leave request. Names use `NativeTextPixelSizeInBox()` for their 48/58-unit boxes; the health bar crops its sprite with `scale-none` (overflow clipping does not work under a transformed panel). No longer aliases `CPartyInfoWindow`'s image slots. |
| `CPartyInfoWindow` | `CObject`-tier | Done (2026-09-19) | RmlUi-only 2D | No live-3D content, same shape as `CCharacterInfoWindow`. Looser docking than `CPetInfoWindow`: opens standalone (just closes Character/Inventory first via `HideAllGroupA()`), no forced pairing — `WindowSystem.cpp` needed zero changes. Member rows (`RenderMemberStatue`'s `iIndex * 71` pixel math) ported to a `data-for` list stacked via normal block flow (`height: 71px` per row) instead of computed offsets — first repeated-row list in this codebase built directly against a live global array (`Party[]`/`PartyNumber`) each frame rather than a cached snapshot, since the row count is small (`MAX_PARTYS` = 5). `IMAGE_LIST` enum and `LoadImages()`/`UnloadImages()` kept despite the window no longer rendering through the legacy bitmap-atlas system, same reason `CCharacterInfoWindow` kept its own — `CPartyListWindow` aliases two of this window's texture slot IDs. Frame/shell now shared with `CCharacterInfoWindow`/`CMyQuestInfoWindow`/`CPetInfoWindow` via `docked_panel_frame.rcss` (both themes) — see `STATUS.md`'s dock-neighbor gap note for why this exists. |
| `CFriendWindow` | `CObject`-tier + live-3D | Done (2026-10-04) | Shared RmlUi dialogs; shell, chat rooms and both letter windows are semantic documents | Add-friend entry, confirmations and notices use the shared RmlUi dialog in both themes; the native prompt classes are removed. The tabbed shell is `friend_shell.rml` + both themes (`UI/Party/FriendShell`): three tabs, the friends, letter and open-window lists, sorting, checked mail, the button rows, dragging, resizing, maximize and keyboard row movement are RmlUi; C++ keeps the lists, the selected identities and the network calls. Chat rooms are `chat_room.rml` + both themes (`UI/Party/ChatRoom`), one document and data model per open room so a closing room cannot take another's focus or scroll with it. Enter goes through the native key path, as every other RmlUi field here does. The letter windows are `letter_read.rml`/`letter_write.rml` + both themes, one document each: their sender portrait is live 3D that composites **after** the main context, through `UI::RmlBridge::OverlayRender` (see `component-catalog.md`), so the panel paints its full width and no hole is cut for it. Only the window in front of the family draws its portrait -- that seam sits above the whole context rather than at one window's depth, and the front window is the one case where those agree. RCSS owns where the portrait sits and C++ moves the native viewer onto that box; `UI::Party::PhotoViewerControl` drives its drag/reset/help from the document, because RmlUi consumes the press before the legacy mouse globals ever see it. The transcription layer is deleted: `FriendWindowRmlBuilder`, `FriendWindowView`, `friend_window.rml` and its allowlist entry are gone, and with them the last `CUITextListBox<T>` subclass and the last `CUIButton`. Retired `CUIWindowListBox`, `CUILetterListBox`, `CUISimpleChatListBox`, `CUIChatPalListBox` and `CUILetterTextListBox`. `UI/Party/` holds no native widget beyond `CUIControl` and `CUIPhotoViewer` itself. Fixed in passing: three crashes on the chat-room receive path (`HandlePacketS` dereferenced `find()` without an `end()` check, the handle->window map outlived the window, and `ReceiveChatRoomUserList` dereferenced the window unchecked). |
| `CGuildMakeWindow` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `guild_make.rml` + both themes. |
| `CGuildInfoWindow` | `CObject`-tier | Implemented; runtime pending | RmlUi-only 2D | All three lists use RmlUi scrolling and plain data; the alliance-master notice remains `CGenericConfirmDialog`. |

### Quests

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CMyQuestInfoWindow` | `CObject`-tier | Done | RmlUi-only 2D | `component-catalog.md`'s "List / repeated rows" — the `data-for` reference for `CUITextListBox<T>` retirement. Modern theme moved off its own `.modern-frame`/`.modern-frame-crimson` onto the shared forged-dialog recipe in `docked_panel_frame.rcss` (2026-09-19) — it was the visual outlier `STATUS.md`'s dock-neighbor gap note flags; now matches `CCharacterInfoWindow`/`CPetInfoWindow`/`CPartyInfoWindow`. |
| `CQuestProgress` | `CObject`-tier | Done | RmlUi-only 2D | `component-catalog.md`'s "List / repeated rows" (`UI::Quests::RewardModel`, shared with `CMyQuestInfoWindow`). Frame/chrome shared with `CCharacterInfoWindow`/`CMyQuestInfoWindow`/`CPetInfoWindow`/`CPartyInfoWindow` via `docked_panel_frame.rcss`. Retires its own `CUIQuestContentsListBox`/3 `CButton`s for a `data-for` reward list + RmlUi buttons; NPC-dialogue pagination (7 lines/page, L/R buttons revealing the answer list on the last page) is deliberately preserved as real intent, not replaced with a scrollbar. Reward-item hover-preview became click-to-preview, matching `CMyQuestInfoWindow`'s own already-shipped precedent for the same list class. Selected reward item's info popup goes through `::RenderItemInfo()`, the same path `CMyQuestInfoWindow` uses; that function now draws through the shared RmlUi tooltip (`component-catalog.md`'s Tooltip section), so no native-drawn popup remains. Build verified (`RelWithDebInfo`) and confirmed working in-game. |
| `CQuestProgressByEtc` | `CObject`-tier | Done | RmlUi-only 2D | Same port as `CQuestProgress` above, sharing `quest_progress.rcss` (one stylesheet, two separate `.rml`/data-model documents — RmlUi binds one model name per document, so a literal shared `.rml` isn't possible) and the same `QuestProgressRmlModel`/`RmlModelBinder<T>` struct shape. Structurally simpler than `CQuestProgress`: no NPC-name/player-name preamble at all (omitted from its own `.rml` entirely, not a data-driven toggle) — the handful of remaining pixel deltas (NPC-line/button top offsets, answers-block top) are handled by a `#panel.qp-etc` modifier-class override in the shared `.rcss`. `WindowSystem.cpp`'s docking/mutual-exclusion dispatch (column-swap with `CMyQuestInfoWindow`/`CCharacterInfoWindow`, no `HideAllGroupA()` sweep) and the `ProcessClosing()`-time `g_QuestMng.DelQuestIndexByEtcList()` call are both untouched. Build verified (`RelWithDebInfo`) and confirmed working in-game. |
| `CNPCQuest` | `CObject`-tier + live-3D (`I3DRenderObj`) | Done | Hybrid RmlUi/native 3D, split fg/bg documents | The third and last member of the NPC-dialogue family, driven by the older `g_csQuest`/`CSQuest`/`DialogStructure` quest system (not `CQuestMng`, the system `CNPCDialogue`/`CQuestProgress` use). Two mutually-exclusive upper panels (quest-condition checklist or a Zen-cost banner) plus a shared dialogue/numbered-answers block, whose top offset is a real per-instance computed value (native's own vertical-centering formula) rather than a static CSS number. The live quest-condition item preview (`Render3D()`/`RenderItem3D()`) stays a permanent native hybrid boundary — genuine rotating-3D-model content, the same class of decision as `CQuestProgress`'s own `::RenderItemInfo()` boundary, just for multiple always-visible rows instead of one on-hover popup; its condition-evaluation math (`FindQuestItemsInInven`/`GetKillMobCount`, including the exact color/completion mapping) was split from its old inline-draw call into a data-row builder, same class of split `UI::Quests::RewardModel::BuildRows()` already established. Unlike every other window in this family, its frame chrome couldn't stay in the same document as its content: RmlUi's main context always renders last in the frame, so an opaque `#panel` there would paint over the live-3D preview drawn earlier the same frame. Split into two documents instead, same mechanism `CNPCShop`/`CMyInventory` already use for their own live-3D icon grids — `npc_quest_bg.rml` (frame chrome, `docked_panel_frame.rcss`'s `#panel`/`.frame-piece`/shell-edge-groove-header-rail) loads into `RmlUiRuntime::GetBackgroundContext()` (rendered explicitly before the 3D pass); `npc_quest.rml` (all actual content) stays on the main context, whose own root (`#content_root`) is a deliberately unpainted positioning anchor — including its own explicit `font-family`, since that no longer comes for free from `docked_panel_frame.rcss`'s `#panel` rule the way every sibling's font does. Build verified (`RelWithDebInfo`) and confirmed working in-game. |

### NPCs

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CNPCShop` | `CObject`-tier + live-3D | Done | Hybrid RmlUi/native 3D | Listed under Inventory/Shop above |
| `CNPCDialogue` | `CObject`-tier | Done | RmlUi-only 2D | The window a quest-giving NPC actually opens first (`ProcessOpening()`/`ReceiveQuestByNPCEPList`/`ReceiveQuestByEtcEPList`) — was the source of a modern-theme "looks legacy" report, since it had zero RmlUi/theme participation before this port; only after picking a step from it does the server push a `PMSG_QUEST_STEP_INFO` that opens the already-ported `CQuestProgress`/`CQuestProgressByEtc` above. Own `npc_dialogue.rml`/`.rcss` (not shared with `quest_progress.rcss` despite reusing the same `Quest_bt_L/R.tga` sprite asset — the layout isn't a near-identical sibling: two independently paginated lists in one document, an NPC-word pager (top) and a sel-text pager (bottom) whose rows come from either `GetNPCDlgAnswer()` or quest-list-mode's `SetQuestListText()` quest subjects, both reduced to the same `{text, index}` row shape via `NPCDialogueRmlModel`). The "page to the end reveals the list" gate is preserved exactly (now a same-click side effect, matching the native single-button double-duty). Retires all 5 `CButton`s and every native `Render*()` call — no hybrid native-draw boundary left at all, unlike `CQuestProgress`'s reward-item popup. Gens contribute-point banner (NPC 543/544) and the Gens join/secede/reward flows (`ASG_ADD_GENS_SYSTEM`/`PBG_ADD_GENSRANKING`, unconditionally on in this build) are untouched native logic, just reactively synced into the model. Fixed a fragile-transitive-include situation this same session's earlier `QuestProgress.h` cleanup had patched around: `NPCDialogue.h` no longer needs `MessageBox.h`/`MyInventory.h`/`MyQuestInfoWindow.h`/`QuestProgress.h`/`Button.h` at all (those were only for texture-slot enum aliasing and the now-removed `CButton` members). Build verified (`RelWithDebInfo`) and confirmed working in-game. |
| `CGatemanWindow` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `gateman.rml` + both themes, right-docked; guest, staff and master pages. |
| `CEmpireGuardianTimer` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `empire_guardian_timer.rml` through `UI/Events/EventTimerView` (`event_timer.rcss`, shared with the Blood Castle and Chaos Castle timers). |
| `CEmpireGuardianNPC` | `CObject`-tier + live-3D (`I3DRenderObj`) | Done (2026-09-27) | Hybrid | `empire_guardian_enter.rml` (texts, buttons) + `empire_guardian_enter_bg.rml` (frame, background context) through `UI/Events/EventItemEntryView`, shared with `CDoppelGangerWindow`; Gaion's Order stays a native 3D preview over the frame. |

### Combat / Duel

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `CCastleWindow` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `castle_window.rml` + both themes (Senatus: gate, statue and tax pages); its buy/repair confirmations stay `CGenericConfirmDialog`. The treasury no longer draws twice from a `%I64d` format. |
| `CGuardWindow` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `guard_window.rml` + both themes with its guild lists; the owner line and the period's tab labels show (the original lost the first to an unterminated buffer and froze the second). |
| `CDuelWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `duel_window.rml` + both themes, in the **background context** behind its other documents: the original drew it under every panel (layer 1.1). |
| `CDuelWatchWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `duel_watch.rml` + both themes; right-docked, `docked_panel_frame.rcss`. |
| `CDuelWatchUserListWindow` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `duel_watch_spectators.rml` + both themes: a `Pk_box` per spectator above the spectator frame's right end; takes no pointer events (C++ keeps the pointer over the boxes from the world). The boxes' fill is the colour the original's `RenderColor()` left current (measured). |
| `CDuelWatchMainFrameWindow` | `CObject`-tier + live-3D (`I3DRenderObj`) | Done (2026-09-28) | RmlUi-only 2D (the 3D hook draws nothing) | `duel_watch_frame.rml` + both themes, in the main frame's place while the duel-watch buff is on: frame, health/shield gauges with their catch-up effect bars (the catch-up stays in C++, `Update()`), score marks, names, the exit button (RmlUi click queued, C++ keeps its tooltip and the channel quit request). Fixed: the right shield bar's catch-up speed was measured against the left fighter's shield. |
| `CSiegeWarfare` | `CObject`-tier | Done (2026-09-28) | RmlUi-only 2D | `siege_warfare.rml` + both themes, background context behind its other documents (layer depth 1.6); observer, soldier and commander fill one model (`SiegeWarfareRmlModel`), buttons still hit-test natively. |

### Events (small, self-contained per-event status/timer windows)

| Component | Category | Notes |
|---|---|---|
| `CCatapultWindow` | `CObject`-tier | Done (2026-09-28): `catapult.rml` + both themes, right-docked |
| `CEnterBloodCastle` | `CObject`-tier | Done (2026-09-27): `blood_castle_enter.rml` through `UI/Events/EventEntryView` (shared with `CEnterDevilSquare`, `event_entry.rcss`) |
| `CBattleSoccerScore` | `CObject`-tier | Done (2026-09-27): `battle_soccer_score.rml`, background context like `CDuelWindow`; guild marks as 8 x 8 cells (`Guild::MarkPalette`) |
| `CBloodCastle` | `CObject`-tier | Done (2026-09-28): `blood_castle_time.rml` through `UI/Events/EventTimerView` |
| `CChaosCastleTime` | `CObject`-tier | Done (2026-09-28): `chaos_castle_time.rml` through `UI/Events/EventTimerView` |
| `CCursedTempleResult` | `CObject`-tier | Done (2026-09-27): `cursed_temple_result.rml` + both themes; the fading banner blends from the start, where the native alpha test (0.25) hid its first moments |
| `CCryWolf` | `CObject`-tier | Done (2026-09-28): `crywolf.rml` + both themes (battle panel, result banner, rank/exp digits, ready notice; main context, pulled to the front); the in-render yes/no box only the unused `MoveMvp_Interface()` opened is not ported |
| `CCursedTempleSystem` | `CObject`-tier | Done (2026-09-28): `cursed_temple_system.rml` + both themes: the Illusion Temple HUD (time, mini map and markers, skill panel, score effect, tutorial) in RmlUi |
| `CCursedTempleEnter` | `CObject`-tier | Done (2026-09-27): `cursed_temple_enter.rml` + both themes |
| `CDoppelGangerFrame` | `CObject`-tier | Done (2026-09-28): `doppelganger_frame.rml` + both themes, background context; gauge and markers still step 0.01 a frame in C++ |
| `CDoppelGangerWindow` | `CObject`-tier + live-3D (`I3DRenderObj`) | Done (2026-09-27), Hybrid: `doppelganger_enter.rml` + `doppelganger_enter_bg.rml` (frame in the background context, under the native 3D preview) through `UI/Events/EventItemEntryView` |
| `CEnterDevilSquare` | `CObject`-tier | Done (2026-09-27): `devil_square_enter.rml` through `UI/Events/EventEntryView` |
| `CGateSwitchWindow` | `CObject`-tier | Done (2026-09-28): `gate_switch.rml` + both themes, right-docked |
| `CGoldBowmanLena` | `CObject`-tier, incidental 3D render call | Done (2026-09-28): `gold_bowman_lena.rml` + `gold_bowman_lena_bg.rml` through `UI/Events/EventItemEntryView` |
| `CGoldBowmanWindow` | `CObject`-tier | Done (2026-09-28): `gold_bowman.rml` + `gold_bowman_bg.rml` through `UI/Events/EventItemEntryView`; the number field is an RmlUi text input |
| `CExchangeLuckyCoin` | `CObject`-tier | Done (2026-09-28): `lucky_coin_exchange.rml` + `_bg` through `UI/Events/EventItemEntryView` |
| `CRegistrationLuckyCoin` | `CObject`-tier, incidental 3D render call | Done (2026-09-28): `lucky_coin_registration.rml` + `_bg` through `UI/Events/EventItemEntryView`; the coin stays a native 3D model |
| `CKanturu2ndEnterNpc` | `CObject`-tier | Done (2026-09-28): `kanturu_enter.rml` + both themes |
| `CKanturuInfoWindow` | `CObject`-tier | Done (2026-09-28): `kanturu_info.rml` + both themes (boss-battle HUD) |

### Options / System

| Component | Category | Status | Target shape / primitive | Detail pointer |
|---|---|---|---|---|
| `COptionWindow` | `CObject`-tier | Done | RmlUi-only 2D, via `window_shell`, 6-tab layout (Gameplay/Audio/Video/Graphics/UI/General) | Built and verified live against a real server, both themes (2026-09-19). Grew substantially past the original flat 10-row layout: tabbed to make room for settings expansion (5 checkboxes, 3 sliders, 5 dropdowns across the 6 tabs), and surfaced several previously console-only/unreachable `GameConfig` settings and `MainScene.h` DXP-23 effect-cost toggles as real, persisted UI (VSync, FPS cap, disable-effects/particles/skill-effect-models/boids/wing-shadow, show-FPS-counter/show-debug-info, live UI-theme switch, and the global UI scale — a 6th dropdown on the UI tab over a fixed
50–200 % ladder, re-applied live by resizing the window to its current size, which is also this
window's first hover tooltip: a plain `.option-row-tip` sibling shown by an RCSS `:hover` rule, no
C++ hover callback, see `layout-and-scaling.md`'s "Global UI scale" section). The close button is built directly in C++ into `window_shell`'s own `#window_shell_footer` anchor (a direct `#panel` child, sibling of `#content`, matching `generic_confirm_dialog`'s own button placement) rather than authored as `{{}}`-bound RML — see `engine-findings.md`'s new "moving an already-parsed element doesn't preserve a `{{}}` text binding" entry for why. All 5 dropdowns ended up as a custom `data-for`/`data-class`/`data-event-click` control (`.option-dropdown` family) instead of RmlUi's native `<select>` — see `engine-findings.md`'s new native-`<select>` entry; three separate rounds of fixing the native widget's missing sub-element styling never got clicking an option to reliably commit. Also fixed along the way: two scenes (`LoginScene.cpp`/`CharacterScene.cpp`) that manually pump this window while it's shown outside `MAIN_SCENE` were missing an `Update()` call, leaving every bound label blank the first time the window opened before ever visiting the main scene (`engine-findings.md`'s new persistent-document-manual-pump entry); a native, unrelated bug in `CServerMsgWin::Release()` not resetting its own visibility flag, which produced a one-frame flicker at its last position when a server notice was showing at the exact moment of the character-select→main-scene transition. |
| `CServerMsgWin` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `server_msg.rml` + both themes, real pixels like the original (`LayoutMode::Legacy`), the lines in the native fixed face (Cousine, now registered with RmlUi). Earlier note: **Bug fixed in passing (2026-09-19)**, unrelated to porting: `Release()` never reset its own `CObject` visibility flag (every sibling window torn down in the same `CSceneUICoordinator::CreateMainScene()` sweep does), so a notice visible at the exact moment of the character-select→main-scene transition kept rendering for a frame against already-released sprites — a stray upper-left flicker. Fixed by resetting the flag directly in `Release()`. |
| `CChatCommandWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `chat_command.rml` + both themes. Its parameter value field is an RmlUi `<input>` (was a `CUITextInputBox`): the window claims RmlUi's text-input identity through `SetRelatedWnd()` while it is focused, as `CChatInputBox` does, and filters digits for numeric parameters. |
| `CHelpWindow` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `help_window.rml` + both themes (legacy overrides the markup to bind the native `RenderTipTextList()` metrics). Pages built by `UI::Help::BuildPage()`; shown unfocused and pulled to the front (`SyncDocumentVisibilityInFront()`), so the HUD documents no longer cover it. The two unreachable pages (clock, event times) are dropped. |
| `CWindowMenu` | `CObject`-tier | Done (2026-09-27) | RmlUi-only 2D | `window_menu.rml` + both themes. A `LayoutMode::Hud` window: the document carries the W/640 x H/480 stretch; hover and arrows are `:hover`, row clicks are queued and run from `Update()`. |
| `CCreditWin` | `CObject`-tier | Done | RmlUi-only 2D | Flagged by the 2026-09-16 audit as an apparently-shipped port that had never been logged; **confirmed and recorded in `STATUS.md` 2026-09-26** — `credit_win.rml` plus both themes' `.rcss`, `LoadThemedDocument` + theme-reload registration, and no native `RenderImage`/`RenderText`/`CSprite` calls left. The port was real; only the record was missing. |

### Excluded from this ledger (infrastructure / reusable widgets, not standalone "components")

`C3DCamera` and `CGroup` (`UI/Core/`) are `CManager` plumbing, not ported windows. `CTextBox`,
`CSlideWindow`, `CScrollBar`, `CChatInputBox` (`UI/Widgets/Window/`) are low-level composable
widgets used *by* several windows above, not top-level components with their own migration status —
they retire implicitly as their host windows port. **`CChatInputBox` was ported directly
(2026-09-27)** rather than waiting for a host, since it *is* the chat bar: `chat_input.rml` + both
themes, with the bar art, all ten buttons, the tooltip and both text fields now RmlUi. Its two
`CUITextInputBox` fields became stock `<input>`s (`.text-field`), and RmlUi's built-in document Tab
navigation replaced `SetTabTarget()`. C++ keeps the history, the send logic and the keyboard
handling. See `STATUS.md` for the focus/dispatch detail that makes the keyboard half work at all. `CMessageBoxMng` (`UI/Dialogs/MessageBox.h`) is
the manager/plumbing class underlying the whole `TMsgBoxLayout<T>` mechanism the Dialog family below
uses, not a dialog itself.

## Dialog family (`UI/Dialogs/CommonMessageBox.h` / `CustomMessageBox.h`)

The native `TMsgBoxLayout<T>` family is far smaller today than `STATUS.md`'s historical "~140
classes" figure — that count predates this branch's migration work. 16 native dialog *classes*
have been ported and deleted outright (3 replaced by `CGenericConfirmDialog`, 13 by
`CGenericMenuDialog` — see `component-catalog.md`'s Dialog section for how the two primitives
work; per-dialog porting history lives in git log, not a doc, once a dialog is done). That 3/13
figure is classes replaced, not call sites — the two shared primitives are now called from far
more places than that: 123 `g_pGenericConfirmDialog->Show()` call sites across 28 files, and 14
`g_pGenericMenuDialog->Show()` call sites in 1 file (`CustomMessageBox.cpp`), by direct grep.
Most of those call sites live inside still-fully-native, not-yet-ported windows reusing the shared
dialog primitive for one sub-confirmation (see `CMasterLevel`/`CInGameShop`/`CGuildInfoWindow`/
`CCastleWindow` above) — a high call-site count is not evidence that 123 or 14 dialogs have each
been individually ported. What's left, by current grep of the two headers:

| Component | Status | Target primitive | Detail pointer |
|---|---|---|---|
| `CGuild_ToPerson_Position` | Done (2026-09-28) | RmlUi (shared `UI/Dialogs/MessageBoxView`) | `component-catalog.md`'s Dialog section |
| `CGemIntegrationDisjointMsgBox` | Implemented; runtime pending | RmlUi (`MessageBoxView` with a scrolling jewel list) | Plain item selection; stale items rejected again at confirmation |
| `CQuestCountLimitMsgBoxLayout` (`CCommonMessageBox`) | Compiled out | — | Its only creator, `ReceiveQuestLimitResult()`, is under `ASG_ADD_TIME_LIMIT_QUEST`, which this build does not define |
| `CBloodCastleResultMsgBoxLayout` | Done (2026-09-28) | RmlUi (`MessageBoxView`) | Texts from the match object's `CollectMatchResult()`, which the native drawing uses too |
| `CDevilSquareRankMsgBoxLayout` | Done (2026-09-28) | RmlUi (`MessageBoxView`) | As above |
| `CChaosCastleResultMsgBoxLayout` | Done (2026-09-28) | RmlUi (`MessageBoxView`) | As above |
| `CCrownSwitchPopLayout`, `CCrownSwitchPushLayout`, `CCrownSwitchOtherPushLayout` (`CProgressMsgBox`) | Done (2026-09-28) | RmlUi (`MessageBoxView` with frame and progress bar) | Share one underlying shape class |
| `CSealRegisterStartLayout`, `CSealRegisterSuccessLayout`, `CSealRegisterFailLayout`, `CSealRegisterOtherLayout`, `CSealRegisterOtherCampLayout` (`CProgressMsgBox`) | Done (2026-09-28) | RmlUi (`MessageBoxView`) | Share one underlying shape class |
| `CCrownDefenseRemoveLayout`, `CCrownDefenseCreateLayout` (`CProgressMsgBox`) | Done (2026-09-28) | RmlUi (`MessageBoxView`) | Share one underlying shape class |
| `CCursedTempleHolicItemGetLayout`, `CCursedTempleHolicItemSaveLayout` (`CCursedTempleProgressMsgBox`) | Done (2026-09-28) | RmlUi (`MessageBoxView`, the `CProgressMsgBox` view) | Share one underlying shape class |

Also present: `C3DItemCommonMsgBox` (`CommonMessageBox.h`) — a `CMessageBoxBase` shape class with
live-3D item content; its only user is the cash shop's `MsgBoxIGSStorageItemInfo`, so it stays with
`CInGameShop` (verified 2026-09-28).

## `CUIControl` list family (`UI/Widgets/UIControls.h`)

The `data-for` binding pattern is proven (`component-catalog.md`'s "List / repeated rows"); the list
widgets retire with their host windows. Eight list widgets remain: five in Friend/Mail/Chat-room
and three in the cash shop. Earlier Done entries describe rendering ports; only Removed entries
mean the native widget is gone. New list migrations still require runtime acceptance.

| Component | Row type | Status | Detail pointer |
|---|---|---|---|
| `CUICurQuestListBox` | `SCurQuestItem` | Removed | Quest hosts use RmlUi |
| `CUIQuestContentsListBox` | `SQuestContents` | Removed | Quest hosts use RmlUi |
| `CUIGuildListBox` | `GUILDLIST_TEXT` | Removed | No live consumer; shared record remains for Friends |
| `CUISimpleChatListBox` | `WHISPER_TEXT` | Done (2026-09-28) | `CFriendWindow` port (`UI/Party/FriendWindowRmlParts.cpp`) |
| `CUILetterTextListBox` | `LETTER_TEXT` | Done (2026-09-28) | `CFriendWindow` port (`UI/Party/FriendWindowRmlParts.cpp`) |
| `CUIChatPalListBox` | `GUILDLIST_TEXT` | Done (2026-09-28) | `CFriendWindow` port (`UI/Party/FriendWindowRmlParts.cpp`) |
| `CUIWindowListBox` | `WINDOWLIST_TEXT` | Done (2026-09-28) | `CFriendWindow` port (`UI/Party/FriendWindowRmlParts.cpp`) |
| `CUILetterListBox` | `LETTERLIST_TEXT` | Done (2026-09-28) | `CFriendWindow` port (`UI/Party/FriendWindowRmlParts.cpp`) |
| `CUISocketListBox` | `SOCKETLIST_TEXT` | Removed; runtime pending | `mix_inventory.rml`; plain socket state, RmlUi scrolling and selection |
| `CUIGuildNoticeListBox` | `GUILDLOG_TEXT` | Removed; runtime pending | `guild_info.rml`; announcement flow pane |
| `CUINewGuildMemberListBox` | `GUILDLIST_TEXT` | Removed; runtime pending | `guild_info.rml`; selection by member name; shared record remains for Friends |
| `CUIUnionGuildListBox` | `UNIONGUILD_TEXT` | Removed; runtime pending | `guild_info.rml`; selection by allied guild name |
| `CUIExtraItemListBox` | `FILTERLIST_TEXT` | Deleted | Its only user, the MU Helper config window, now binds a `data-for` list in a `.scroll-pane` |
| `CUIUnmixgemList` | `UNMIX_TEXT` | Removed; runtime pending | Lahap uses plain item identity and a RmlUi scrolling list in `MessageBoxView` |
| `CUIBCDeclareGuildListBox` | `BCDECLAREGUILD_TEXT` | Removed; runtime pending | Guard declaration rows; RmlUi selection and scrolling |
| `CUIBCGuildListBox` | `BCGUILD_TEXT` | Removed; runtime pending | Guard siege rows and score footer; RmlUi selection and scrolling |
| `CUIMoveCommandListBox` | `MOVECOMMAND_TEXT` | Unused | No user left; `CMoveCommandWindow` binds its own `data-for` list |
| `CUIInGameShopListBox` | `IGS_StorageItem` | Not started | `tracked-deferrals.md` |
| `CUIBuyingListBox` | `IGS_BuyList` | Not started | `tracked-deferrals.md` |
| `CUIPackCheckBuyingListBox` | `IGS_SelectBuyItem` | Not started | `tracked-deferrals.md` |

## Native surfaces outside the window classes

UI drawn by free functions or scene code rather than by one of the classes above (audited
2026-09-28 from the remaining `RenderBitmap`/`RenderImage`/`RenderText`/`RenderColor` callers).

| Surface | Native code | Status |
|---|---|---|
| Centre-screen notice lines | `UI::Notices::Render()` | Done (2026-09-28): `notices.rml`, main context above every document but the tooltip |
| Map name banner | `CUIMapName::Render()` | Done (2026-09-28): `map_name.rml`, background context |
| Party members' HP bars over their heads | `RenderPartyHP()` | Done (2026-09-28): recorded by the world-label layer (`CNameWindow::PrepareBackgroundLayer()`) |
| Tournament countdown | `RenderTournamentInterface()` | Left native: unreachable with OpenMU (its F3 22 / F3 23 packets are defined but never sent) |
| Kanturu final result banner | `M39Kanturu3rd::RenderKanturu3rdinterface()` | Done (2026-09-28): recorded by the world-label layer |
| Siege crown switch lines, build-time bars | `RenderSwichState()`, `battleCastle::RenderBuildTimes()` | Done (2026-09-28): recorded by the world-label layer |
| Hellas object labels | `RenderObjectDescription()` | Done (2026-09-28): recorded by the world-label layer |
| Reconnect dialog | `UI::Reconnect::RenderDialog()` | Done (2026-09-28): `reconnect_dialog.rml`, main context above every document |
| Login scene logo, copyright and version lines | `NewRenderLogInScene()` | Done (2026-09-28): `login_scene.rml` (`Scenes::LoginOverlay`), main context behind its other documents |
| Loading screen art | `LoadingScene()` → `loading.rml` | Done (upstream Phase 1 pilot; art paths and texel rects fixed 2026-09-28). `CLoadingScene` is never instantiated |
| Photo viewer help text | `CUIPhotoViewer::RenderHelpText()` | Done (2026-09-28): the shared RmlUi tooltip |
| Mouse cursor | `RenderCursor()` | Stays native: drawn after RmlUi's pass so the pointer stays on top |
| FPS counter, debug info, GL stats, `ImeInput`/`Whisper` debug text | `SceneManager.cpp`, `ImeInput.cpp`, `Whisper.cpp` | Stays native: developer overlays, not player UI |

## Using this ledger

Before starting a new port: find the component's row, read its Detail pointer for the real story,
and check the "Checklist for every new port" in `STATUS.md`. After landing a port: update the row's
Status/Target columns here in the same commit — this file drifting out of sync with reality is worse
than it not existing, since a stale "Not started" reads as a confident false negative.

**`Done` means ported, not audited.** Every port makes judgement calls that diverge slightly from
the original, and nothing revisits them once the row flips. `tracked-deferrals.md`'s "audit where
ports steered away from the original UI" entry is the standing pass for that, with the known
instances already seeded — read it before treating a `Done` row as settled.
