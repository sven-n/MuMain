# RmlUi window placement and theme-sized windows rollout

Design (tracked, governing): `docs/rmlui-ui-system/window-placement.md`. Read it first; it holds
the design, phase table, rule classification, theme recipes and the "Theme-sized windows" audit.
Continuation prompt for a new agent: `rmlui-window-placement-continuation-prompt.md` (this folder).

## User decisions

- Layout is controlled by RML/RCSS per theme: themes may undock windows, put them on either side,
  pair or separate them, change their size. Plan for modder layouts beyond these.
- Workspace document with regions and slots declared in RML; the C++ placement service reading back
  slot rectangles is the one readback exception (alongside `GetStripRect()`).
- Window sizes are the theme's choice: a sprite frame stretches; contents are laid out by the theme
  (fixed coordinates, grid or flex).
- Phase 4 (fill: full/half-screen windows) deferred until a theme wants it.
- No RmlUi fork needed (no `calc()`); the user offered one if ever required.
- Keep `legacy` and `modern` in step; only those two themes. No third theme, not even for tests.
- Undecided (user's call): per-theme saved drag positions, and whether dragging takes a window out
  of its slot instead of today's "saved position only while first in its region".

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

## Not verified (needs an NPC, a second player or mouse dragging)

1. Trade between two clients: both grids inside the window, add/remove items.
2. Vault (+ extension; vault keeper in Noria), NPC shop, Chaos Machine: grids aligned, drop/pick.
3. Browsing another player's shop: grid aligned, buying works.
4. Inventory extension with an unlocked extension: grids aligned.
5. Castle senate and guard: tab clicks switch pages.
6. Inventory equip/unequip/move; drag the inventory, close/reopen, relog (saved position).
7. A widened window does not let clicks through its new area.

Trade attempt (stopped by the user): accounts `ancient`/`ancient` and `test400`/`test400`, both in
Noria, adjacent. Socket `say` sends chat to the server, so the client's `/trade` handling
(`ZzzInterface.cpp` ~2041, needs adjacent + facing or `SelectedCharacter`) never runs. Command
window Trade + scripted right-click on the other character sent nothing: world picking did not
select the character. Next step would be `Core/Input/SyntheticInput.cpp` (sets `MouseX/MouseY`,
`MouseRButtonPush`) versus how `SelectedCharacter` is picked. Resume only if the user asks.

## Open work, in suggested order

1. Hand checks above (or teach the socket world picking so trade/NPC checks can be scripted).
2. Castle, guard, gatekeeper: native tab radio groups, gate/statue picks and the public toggle sit
   over RmlUi-drawn elements. Right fix: RmlUi click events, and tab positions in RCSS (today bound
   in RML as `12 + i * 41`). Needs testing at the siege NPCs.
3. Catapult has no hit box (clicks fall through to the world; predates this work).
4. Done: lucky item's panel and background sizes are in each theme's RCSS. The native hit box reads `#panel`, and legacy counter-scaled text reads `panel_width`. Full RelWithDebInfo build passed; in-game verification is pending.
5. Done: the eight inventory-family legacy-theme RML documents center counter-scaled text
   with `panel_width` read from `#panel`. Each owner binds and syncs it. The modern theme already
   uses panel-relative header layout. Full RelWithDebInfo build and RML checks pass; in-game checks
   are pending alongside the hand checks above.
6. Placement leftovers: per-theme saved positions and the drag rule (user decision); Gens ranking
   slot (needs a region with the `hud` scale or its own space); HUD widgets centring in the free
   width ignore a left dock (`GetScreenWidth()` only uses the right edge); unslotted families (move
   map, friends, centred dialogs, HUD widgets) with per-window content sizes and region scales.
7. Phase 4 fill: deferred by the user.

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
  `quit`. Synthetic Enter does not submit the chat field; switch themes via `config.ini`
  (`RmlTheme`) and restore it. If a client stops answering, it may be stuck at login: kill and retry.
- Layout experiments: edit the runtime copies under
  `out/build/windows-x64/src/RelWithDebInfo/Data/Interface/RmlUi/` (the next build re-mirrors them
  from `src/bin`), restart the client, restore the files afterwards.
- Patch with exact-match Python scripts in `.ai-os/scratch/` that abort on a count mismatch; large
  bash heredocs fail in this shell (use the Write tool for long scripts). Preserve CRLF and BOM.
- Commits: build first; stage only your files (other `.ai-os` task files are dirty from other
  work); no Co-Authored-By trailer.
