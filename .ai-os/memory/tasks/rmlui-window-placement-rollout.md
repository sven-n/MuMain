# RmlUi window placement and theme-sized windows rollout

Design (tracked, governing): `docs/rmlui-ui-system/window-placement.md`. Read it first; it holds
the design, phase table, rule classification, theme recipes and the "Theme-sized windows" audit.
Current handover prompt for a new agent: `rmlui-window-placement-fill-handover.md` (this folder).

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

1. Hand checks above (or teach the socket world picking so trade/NPC checks can be scripted).
2. Done in code: gatekeeper public toggle, guard and castle tabs, and castle gate/statue
   picks use RmlUi click targets. The themes place the tabs and map icons in RCSS. Full
   RelWithDebInfo build passes; test the toggle, tabs, map picks and resulting requests at the
   siege NPCs before marking the interaction validated in game.
3. Done: catapult consumes pointer input over its measured `#panel`, with 190x429 as the first-layout fallback. Full RelWithDebInfo build passed; a siege NPC in-game check is pending.
4. Done: lucky item's panel and background sizes are in each theme's RCSS. The native hit box reads `#panel`, and legacy counter-scaled text reads `panel_width`. Full RelWithDebInfo build passed; in-game verification is pending.
5. Done: the eight inventory-family legacy-theme RML documents center counter-scaled text
   with `panel_width` read from `#panel`. Each owner binds and syncs it. The modern theme already
   uses panel-relative header layout. Full RelWithDebInfo build and RML checks pass; in-game checks
   are pending alongside the hand checks above.
6. Placement leftovers: per-theme saved positions and the drag rule (user decision); unslotted families (move map,
   friends, centred dialogs) with per-window content sizes and region scales. Done: HUD widgets
   follow both uncovered edges in their own layout each frame (durability icons and party list had
   been off by the dock/stretch scale ratio at any UI scale other than 100 % or a non-4:3 window).
   Verified in game at 80 %: durability icons beside the panel; buff row centred beside a left dock.
7. Phase 4 fill: character info and pet info support `data-fit=fill` (pet verified in game, both themes, 35 %); a fill slot is never smaller than the
   window's content size. Both shipped themes keep their content-sized workspace. The remaining
   windows need individual fill capability and theme content recipes; native grids and live 3D
   need more than a stretched frame.

8. Done: Gens ranking has a right-dock slot after character info; verified in game at 90 and 100 %
   (`$win gensranking full`). Done: the move map has a left-dock
   slot and supports fill (height = its rows' space, width from `#panel`, minimum chrome + 3 rows);
   verified in game in both themes. Done: the friend list's first position comes from a `friends`
   slot (`InitialPosition()`), verified in game. Done: the centred NPC panels (Kanturu entry, Cursed Temple
   entry/result) sit on a `panel-stage` region; verified in game. Slot coverage is complete for
   every window C++ used to place in a dock, a column or the panel centre.

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
