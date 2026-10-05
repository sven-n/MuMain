# Window placement / HUD in the workspace: handover (2026-10-05)

Copy the prompt below to the next agent. Status lives in `rmlui-window-placement-rollout.md`;
the design, including the new "HUD in the workspace" section, is in
`docs/rmlui-ui-system/window-placement.md`.

```text
Continue the RmlUi window-placement rollout in C:\Users\benit\MU\client\MuMain, branch
dev/rmlui-ui-system. The next piece of work is bringing the HUD into the workspace's layout model.

Read first: AGENTS.md, docs/CODING_RULES.md, docs/rmlui-ui-system/architecture-principles.md,
docs/rmlui-ui-system/building-new-ui.md, docs/rmlui-ui-system/window-placement.md (whole file;
the governing section is "HUD in the workspace") and
.ai-os/memory/tasks/rmlui-window-placement-rollout.md (decisions, commits, open work, how to
build and test). Check git status and recent commits first; stage only your own files.

Where things stand: every window the game placed itself (docks, columns, the panel centre) now
takes its place from the theme's workspace.rml/workspace.rcss (right dock, left dock for the move
map, a panel-stage region for the centred NPC panels, a first-position slot for the friend list).
Fill is the theme's decision for 24 RmlUi windows (CObject::GetFillDocument()); native grids and
live 3D stay content-sized. The user's hand checks passed. The HUD still lays itself out; the
workspace only reads the main strip's rectangle (CMainFrameWindow::GetStripRect()) to keep
#safe_area clear of it.

Decision (user, 2026-10-05): HUD components participate in the workspace's layout model. The
workspace is the one place a theme arranges headers, footers, side panels and the content area.
A HUD component keeps its own document, controller, input handling, z-order and scale setting;
taking part means it receives a position and available size from a slot, the same contract
windows have. Nothing is merged into one RML document. The theme chooses per region how it
participates: reserve (takes space the content area avoids) or overlay (placed, takes no space).

Do, in order, following the phase table H1-H4 in that design section:
H1. Settle the section's open points (how the main frame applies a received box with #hud_strip
    and #exp in one document; registration for HUD documents that are not CObjects) and write the
    answers into the design doc. Then: shell regions (header, footer, left, right, content) in
    both themes' workspaces, #safe_area becoming the content region; the participation attribute;
    the main strip as a content-sized reserve footer slot; HudReserve() and
    CMiniMap::SyncClips() reading the slot instead of GetStripRect(); UncoveredWorld*() becoming
    the content region's edges. Both shipped themes must look exactly as today. Extend
    tests/ui/test_window_placement_layout.cpp for the shell.
H2. One HUD component per batch: top bar (main_frame_top.rml), chat log and input, minimap, buff
    row, party list, MU Helper bar, item endurance. The HUD widgets' UncoveredWorld*In() code
    becomes their slots.
H3. Event HUDs as overlay slots.
H4. Prove it with runtime-copy theme recipes (side HUD, header plus footer, split HUD, minimap in
    a corner windows avoid) and record them under "Theme recipes".

Rules: re-arrange only on change (resize, UI scale, theme, a slot opening/closing, a
content-sized component changing size), never unconditionally each frame. Keep legacy and modern
in step; no third theme. Watch the LayoutMode::Slot regression: a placed object's m_Pos is (0, 0)
in its slot space, so native parts positioned once at creation are left behind.

Testing: the local CMake cache has ENABLE_CONTROL_SOCKET=ON (never commit it). Launch with
.ai-os/scratch/launch_fill.ps1 (socket C:/tmp/mu_fill.sock) and drive with .ai-os/scratch/ctl.sh
or win_unix_control.py: login ancient/ancient, select-char ancientDl (Dark Lord), hotkeys,
click-ui, screenshot. Open any registered window with chat: hotkey enter, then
type "$win <name> full" with enter: true. Runtime layout experiments: edit
out/build/windows-x64/src/RelWithDebInfo/Data/Interface/RmlUi/ copies (fill_layout.py helps;
the next build re-mirrors from src/bin) and restore them; back up and restore config.ini when
changing RmlTheme or UIScalePercent. Close the client before building (LNK1168); if a Main.exe
you did not start is running, ask the user to close it.

Build (PowerShell; runs the RML/RCSS syntax, drift and bound-geometry checks):
cmd /c '"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 && cmake --build C:\Users\benit\MU\client\MuMain\out\build\windows-x64 --config RelWithDebInfo'
Tests: reconfigure with -DBUILD_TESTING=ON, build window_placement_layout_tests and
ui_scaling_tests, run, reconfigure back to OFF.

Work style: read each affected file fully; patch with exact-match Python scripts in
.ai-os/scratch/ that abort on a count mismatch, preserving BOM/CRLF; build after each batch; test
in game at 100 % and a non-100 % UI scale in both themes; update the design doc's phase table and
the task file as each batch lands; commit focused batches with the configured author and no
Co-Authored-By trailer. Work autonomously; ask only for decisions that are the user's. End with
one consolidated list of what the user should check by hand.
```
