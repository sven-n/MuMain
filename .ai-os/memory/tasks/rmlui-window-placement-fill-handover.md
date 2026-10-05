# Window placement / theme-sized fill: handover (2026-10-05)

Copy the prompt below to the next agent. The governing status is in
`rmlui-window-placement-rollout.md`; the design and phase table are in
`docs/rmlui-ui-system/window-placement.md`.

```text
Continue the RmlUi window-placement and theme-sized window rollout in
C:\Users\benit\MU\client\MuMain, branch dev/rmlui-ui-system.

Read AGENTS.md, docs/CODING_RULES.md, docs/rmlui-ui-system/architecture-principles.md,
docs/rmlui-ui-system/building-new-ui.md, docs/rmlui-ui-system/window-placement.md, and
.ai-os/memory/tasks/rmlui-window-placement-rollout.md before editing. Check git status and
recent commits first: another session may have changed this branch. Stage only your own files.

Goal and decision: themes control window placement and size using workspace RML/RCSS, including
full-height or partial-width layouts. The user reopened Phase 4 (stretch/fill). Player drag-to-dock
is out of scope; preserve current drag behavior. Keep legacy and modern in step; do not add a
third theme. Keep shipped workspaces visually unchanged unless the user requests otherwise.

State at handover: Phases 0-3, LayoutMode::Slot, and theme-sized content work are committed.
Phase 4 now has its first opt-in: character info supports a slot with data-fit="fill". The
placement service reads the resolved slot rectangle and gives that size to a fill-capable window;
unsupported/zero-size slots fall back to content sizing. Character info applies it to #panel,
its theme CSS stretches the summary/stat areas and pins actions to the lower edge, and its
top-right close target is a RmlUi click element that follows the panel edge. Both shipped
workspaces still use content fit. A headless test exercises a 35%-width, full-safe-height slot in
both themes (2 cases, 54 assertions). The full RelWithDebInfo build and RML/RCSS checks passed.
In-game, a temporary legacy-theme runtime layout at 1024x768/UI scale 90% showed the stretched
panel and closing from its top-right target. Runtime layout copies were restored. Modern-theme
fill appearance, more resolutions and detailed interaction remain unverified. Do not claim all
windows support fill; each needs a separate content and hit-area audit.

Next work:
1. Inspect the current character fill commit and validate both themes at 100% and a non-100%
   scale, with a narrower and taller slot. Exercise action buttons, stat controls, tooltips,
   close, and switching between filled and content-sized theme layouts. Fix any mismatch.
2. Choose the next Rml-only window from the fill-capability audit in window-placement.md.
   For each opt-in, ensure every drawn background, text group, button, pointer target and any
   native part follows the theme-sized panel. Keep the service generic and the sizing recipe in
   theme RML/RCSS. Add a focused headless layout case and test in game when possible.
3. Do not treat native grids or live 3D as stretchable frames: use existing .native-anchor,
   SyncPanelWidth() and FollowAnchor() patterns, and separately design any render-target resize.
4. Finish the remaining placement items from the task file: Gens ranking needs its HUD scale,
   centered HUD widgets need to account for a left dock, and unslotted window families need
   slots/content-size recipes. The earlier legacy text centering, lucky-item size, catapult hit
   box and siege tab/pick click-target code is done; its in-game checks remain pending.
5. Ask the user for the pending hand checks as a consolidated list: trade, vault, NPC shop,
   Chaos Machine, browsed shop, unlocked inventory extension, castle/guard/gatekeeper controls,
   inventory drag and item moves. The user asked to transfer the work, so do not block other
   independent implementation on these checks. Do not pursue socket world-picking unless asked.

Implementation rules: Read each affected window fully. Watch the LayoutMode::Slot regression:
m_Pos is (0,0) in slot space, so native parts positioned only at creation can be left behind.
Use exact-match Python patch scripts in .ai-os/scratch/ and preserve BOM/CRLF. Run the full
RelWithDebInfo build after each significant batch; it runs syntax, theme drift and bound-geometry
checks. Build command (PowerShell):
cmd /c '"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 && cmake --build C:\Users\benit\MU\client\MuMain\out\build\windows-x64 --config RelWithDebInfo'

For local play checks, see the task file and reference_ingame_testing.md. This local CMake cache
has ENABLE_CONTROL_SOCKET=ON; never commit that setting. The scratch scripts
.ai-os/scratch/placement_fill_runtime.py and win_unix_control.py support temporary runtime-copy
layout experiments; always run placement_fill_runtime.py off and quit the test client afterward.
Before building, confirm the game is closed to avoid LNK1104/LNK1168. Commit focused batches,
stage only your files, and use the configured author without a Co-Authored-By trailer. Update
the design phase table and task status as each batch lands. End with one consolidated set of
remaining in-game checks.
```
