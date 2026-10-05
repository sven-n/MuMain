# RmlUi window placement and theme-sized windows rollout

Design (tracked, governing): `docs/rmlui-ui-system/window-placement.md`. Read it first; it holds
the design, phase table, rule classification, theme recipes and the "Theme-sized windows" audit.
Current handover prompt for a new agent: `rmlui-window-placement-hud-handover.md` (this folder).

## User decisions

- Layout is controlled by RML/RCSS per theme: themes may undock windows, put them on either side,
  pair or separate them, change their size. Plan for modder layouts beyond these.
- Workspace document with regions and slots declared in RML; the C++ placement service reading back
  slot rectangles is the one readback exception (alongside `GetStripRect()`).
- Window sizes are the theme's choice: a sprite frame stretches; contents are laid out by the theme
  (fixed coordinates, grid or flex).
- Phase 4 (fill: full/half-screen windows) reopened by the user; character info is the first
  opt-in. Player drag-to-dock remains out of scope; themes choose the dock.
- No RmlUi fork needed (no `calc()`); the user offered one if ever required.
- Keep `legacy` and `modern` in step; only those two themes. No third theme, not even for tests.
- Player drag-to-dock is not needed now. Preserve the existing saved-position behavior; any change
  to drag persistence needs a separate user decision.
- Fill is the theme's decision for every window that can take it (`CObject::GetFillDocument()`).
  A shared window `<template>` was judged not worth it (the shared frame already lives in
  `docked_panel_frame.rcss`); revisit only if a theme author needs one place for frame structure.
- The main HUD is one workspace unit for now (`#hud_layout`, user, 2026-10-05); a slot per part
  waits until a layout needs it.
- HUD joins the workspace's layout model (user's direction, 2026-10-05): HUD components receive a
  position and available size from workspace slots, keep their own documents, controllers and
  scale settings; the theme chooses per region whether it reserves space or overlays. Design and
  phases H1-H4: `window-placement.md`, "HUD in the workspace".

## Commits (all built; full RelWithDebInfo build passes)

| Commit | What |
|---|---|
| `eba8b64d9` | Design doc and rule classification |
| `666ecafc1` | Phase 1: placement service, `workspace.rml`, right dock for 39 windows |
| `3d5e7a2f3` | Headless test: workspaces place windows on the original columns |
| `2562bfc64` | Phase 2: HUD reserve from the HUD strip; `GetScreenWidth()` from open slots |
| `3473c8616` | Phase 3: `data-closes` space rules in the theme; `HideGroupBeforeOpenInterface()` removed |
| `a0c71f3f7` | Uncovered world narrowed from both sides (found by the left-dock experiment) |
| `9391c9a0e` | Phase 4 deferred |
| `0680b7d7c` | `LayoutMode::Slot`, `CObject::GetLayoutTransform()`, slots sized from each window's `#panel`, region `data-scale`, unknown-name logging |
| `e96d40285` | Theme-sized windows audit |
| `78a274aa5` | Tier 1: docked frame and modern bottom buttons pinned to `#panel` edges; `panel_width` (`SyncPanelWidth()`) |
| `6da99a0ec` | Tier 2: hit boxes from `#panel`; `panel_width` in more windows; shared-view windows register documents |
| `c83041308` | MU Helper detail gauges hit-test where the theme draws them |
| `b3f45d9d8` | Inventory equipment slots, grid and option tooltip from `.native-anchor` boxes |
| `f140742cb` | Every inventory-family grid follows an anchor (`CInventoryCtrl::FollowAnchor()`); fixes the slot-placement grid regression |
| `ff0431c6f` | Castle and guard tab hit areas follow the window each frame (same regression) |
| `eb53c7777` | Phase 4: character info fills a `data-fit=fill` slot |
| `2d0262f8f` | Fill slots never smaller than the content size; character info pinned to `#panel` edges |
| `5a62826ac` | Modern character info hides its + buttons without points |
| `e708a3343` | Seven docked windows stay open across a theme switch |
| `05d54637d` | HUD widgets follow both uncovered edges in their own units (`GetScreenLeft()`) |
| `f9d2b0406` | Pet info fills; `FillPlacementSize` helper; shared `#frame_corner_close` |
| `e2ba923b3` | Drop guide when dragging an item between windows |
| `e907e54d7` | Gens ranking slot |
| `01e45526b` | Control socket: typed Enter submits a focused field (`$win <name> full` works) |
| `389c2ad8d` | Move map left-dock slot; fill gives its rows' height and its width |
| `7cbc0d0bd` | Friend list's first position from a `friends` slot (`InitialPosition()`) |
| `ab5efa0e1` | Centred NPC panels on a `panel-stage` region |
| `6a2d08718` | Generic fill (`GetFillDocument()`), 24 windows; native corner close follows panel width |
| `a1f677709` | HUD in the workspace H1: flex shell, `PlacementParticipant`, main HUD as one footer unit, minimap clip from the slot |
| `c1049e7ef` | Header slots for the MU Helper bar and top bar; modern caps its docks at the content area (windows drawn smaller to fit) |
| (H2 chat) | Chat log and input in a `chat-stack` region; adapter options (measure, placedWhileHidden, placed) |

Regression lesson: since `LayoutMode::Slot`, a placed window's `m_Pos` is (0, 0) in its slot space.
Any native part positioned once at creation (not moved by `SetPos()` or re-placed each frame) ends
up in the wrong place. All 39 slotted windows were audited for this; trade, browsed shop, vault
extension, inventory extension, lucky item (grids) and castle/guard (tabs) were the cases, all fixed.

## Verified in game by the agent (control socket, 1024x768, UI scale 90 %)

Both themes: docking and pairing of character info, inventory, extension, quest log; every
`data-closes` rule; MU Helper config + detail; party and guild windows; layout experiments (split
docks, centre stage, composed row); widened character info (260); moved inventory grid and helm
slot; MU Helper gauge click; personal store grid.

Phase 4 character-info fill (runtime-copy layouts, 1024x768): 35%, 30% and 22% slots in both
themes at UI scale 80, 90 and 100 %. The frame, summary, stat rows and action row follow the
filled panel; the 22% slot (below the content width) keeps the content size; Quest/close buttons,
hints and both close targets work; live theme switches between a filled and a content-sized
workspace resize the open panel. Headless test: 2 cases, 58 assertions. Synthetic world clicks
do not move the character, so click-through on the enlarged area is a hand check.

## Hand checks (user, 2026-10-05)

All passed: trade, vault and extension, NPC shop, Chaos Machine, browsed shop, inventory
extension, castle/guard/gatekeeper, catapult, lucky item, inventory moves/drag/relog, filled
windows' click-through. One regression found: dragging an item from one window's grid to another's
(inventory to vault) showed no blue drop guide. Cause: since LayoutMode::Slot each placed window has
its own space, and the guide compared the target grid with the picked item's position in its
owner window's space. Fixed (`e2ba923b3`) by taking the item's box from the pointer in the target
grid's space; re-checked by the user.

Trade attempt (stopped by the user): accounts `ancient`/`ancient` and `test400`/`test400`, both in
Noria, adjacent. Socket `say` sends chat to the server, so the client's `/trade` handling
(`ZzzInterface.cpp` ~2041, needs adjacent + facing or `SelectedCharacter`) never runs. Command
window Trade + scripted right-click on the other character sent nothing: world picking did not
select the character. Next step would be `Core/Input/SyntheticInput.cpp` (sets `MouseX/MouseY`,
`MouseRButtonPush`) versus how `SelectedCharacter` is picked. Resume only if the user asks.

## Open work, in suggested order

1. HUD in the workspace: H1 and H2's header batch are done. Next H2 batches, one component each:
   buff row, party list, item endurance need the user's call first (window-placement.md,
   "Findings for the rest of H2"; chat done, minimap needs no slot); then H3 and H4
   (`window-placement.md`, "HUD in the workspace"). Hand checks pending: the minimap's clip
   around the HUD; in modern at a capped scale, dragging items between windows and item
   tooltips; the chat log's resize handle and F4/F5 (socket hotkeys did not reach the client).
2. Not verified in game: the Cursed Temple result panel (needs a finished event); Gens ranking,
   move map, friends and the NPC panels in the modern theme; filled windows at resolutions other
   than 1024x768.
3. Optional polish: fluid content RCSS for the 22 fillable windows other than character and pet
   (today they get a larger frame with content at the top-left); Gens ranking's description box
   should compute its visible rows (it clips the last one at 90 %); `HeroX` still uses only the
   right edge (gameplay call).
4. User decisions pending: dragging a window out of its slot; per-theme saved positions.
5. Separate project: native grids and live 3D that resize (render-target design).

## How to work on this

- Build (PowerShell; Git Bash quoting swallows its output):
  `cmd /c '"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 && cmake --build C:\Users\benit\MU\client\MuMain\out\build\windows-x64 --config RelWithDebInfo'`
  The build runs `tools/check_rml_rcss_syntax.py`, `check_rml_rcss_drift.py` and
  `check_rml_bound_geometry.py`. The drift check pools `GetElementById("x")` literals per file for
  documents with no data model; read other documents' boxes through `RmlPanelGeometry` helpers.
- Headless layout test: `tests/ui/test_window_placement_layout.cpp` (reconfigure with
  `-DBUILD_TESTING=ON`, build `window_placement_layout_tests`, run, reconfigure back to OFF).
- In-game testing: auto-memory `reference_ingame_testing.md`. The local cache keeps
  `ENABLE_CONTROL_SOCKET=ON` (never commit it enabled). Launch `Main.exe` from
  `out/build/windows-x64/src/RelWithDebInfo` with `MU_CONTROL_SOCKET=<path>`; Windows Python has no
  `AF_UNIX`, so use a ctypes/Winsock client (`AF_UNIX=1`). Commands: `login`, `select-char`,
  `hotkey`, `click-ui` (RmlUi and native UI; not world clicks), `screenshot`, `move`, `state`,
  `quit`. Open any registered window with chat: `hotkey enter`, then `type` `$win <name> full`
  with `enter: true` (`$win list` names them; without `full` it skips CSystem::Show() and so the
  placement service); switch themes via `config.ini`
  (`RmlTheme`) and restore it. If a client stops answering, it may be stuck at login: kill and retry.
- Layout experiments: edit the runtime copies under
  `out/build/windows-x64/src/RelWithDebInfo/Data/Interface/RmlUi/` (the next build re-mirrors them
  from `src/bin`), restart the client, restore the files afterwards.
- Patch with exact-match Python scripts in `.ai-os/scratch/` that abort on a count mismatch; large
  bash heredocs fail in this shell (use the Write tool for long scripts). Preserve CRLF and BOM.
- Commits: build first; stage only your files (other `.ai-os` task files are dirty from other
  work); no Co-Authored-By trailer.
