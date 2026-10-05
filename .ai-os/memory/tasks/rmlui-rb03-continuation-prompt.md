# RB-03 continuation prompt — 2026-10-05

Superseded: RB-03 implementation finished in `53daa490e` (see the rollout task). Only the consolidated in-game spot checks remain. The prompt below is kept for history.

```text
Continue RB-03 of the RmlUi replaceability-boundary rollout in
C:\Users\benit\MU\client\MuMain on branch dev/rmlui-ui-system.

Read first: AGENTS.md, docs/CODING_RULES.md,
docs/rmlui-ui-system/architecture-principles.md,
docs/rmlui-ui-system/building-new-ui.md,
.ai-os/memory/tasks/rmlui-replaceability-boundary-rollout.md (RB-03 design,
checkpoints and latest pass notes), and docs/rmlui-ui-system/component-catalog.md
("Feature operations for networking and game code").

Goal: Network/Server/WSclient.cpp must stop depending on RmlUi-backed window
headers. Packet handlers should pass decoded values through small synchronous
feature operations in headers with no RmlUi or window classes. Preserve packet
semantics and the exact order of state changes, messages, sounds and sends.
Do not pass packet buffers, protocol structs, CObject*, Rml types, or window
manager pointers across the boundary. Item payloads remain serialized
std::span<const uint8_t>. Keep protocol outcome names in WSclient.cpp; give
window methods semantic enums where they interpret codes. Move visibility and
window selection decisions into UI operations. Do not create a generic UI
command/event layer or duplicate UI state.

Committed and full RelWithDebInfo builds passed:
- 14b503d81: siege, guardsman, gatekeeper, catapult, crown notices.
- e841be8bb: Doppelganger.
- 79e71a7de: Empire Guardian.
- bb0393684: Lucky Coin and CryWolf.
- f7cf64dca: Kanturu, including typed stage/detail/entry result API.
No in-game spot checks have been done for these five passes. Provide one
consolidated spot-check list after the complete RB-03 rollout.

Start with Cursed Temple, the only remaining event family. I began a decoded
API refactor but parked it before it built. The last passing source tree is
intact. The six in-progress files are backed up at
.ai-os/scratch/rb03_cursed_wip/ (SHA256.txt and tracked.patch included).
The draft patch scripts are in .ai-os/scratch/rb03_cursed_*.py. In particular,
rb03_cursed_result_apply.py was written but NOT run; no WSclient.cpp changes
were made for Cursed Temple. Inspect the drafts before reusing them. The
current source files contain none of that unfinished refactor.

Remaining Cursed Temple handlers/calls in WSclient.cpp: ReceiveTalk case 0x14
entry offer; ReceiveCursedTempleEnterInfo, ReceiveCursedTempleInfo,
ReceiveCursedTempMagicResult, ReceiveCursedTempSkillEnd,
ReceiveCursedTempSkillPoint, ReceiveCursedTempleHolyItemRelics (UI method is
currently a no-op), ReceiveCursedTempleGameResult, ReceiveCursedTempleState.
The UI window methods currently parse packet buffers. Decode in WSclient and
pass semantic data/temporary spans to UI::CursedTemple operations. Keep the
g_CursedTemple game-logic calls in their existing order. The drafted
CursedTempleUpdates.h/.cpp and modified enter/system files can be recovered
from the backup, but they are incomplete: result window and WSclient still
need migration, and the backup has not been compiled. Check the original
handlers and UI method bodies fully before editing.

After Cursed Temple, proceed in order:
1. NPCs and quests: NPC dialogue, quest progress, move command, my-quest info.
2. HUD/main frame: g_pMainFrame, options, map name, UIManager, stamina,
   hotkeys, slide help, MU Helper config, chat input/log ResetFilter,
   IsImpossibleDuelInterface, and duel message-box query.
3. Login-scene window includes: CreditWin, ServerSelWin, LoginMainWin,
   LoginWin, SceneUICoordinator.
4. Remove WindowSystem.h and every other UI/window include from WSclient.cpp;
   delete the dead commented-out ReceiveBuy; verify the real compiler include
   tree no longer reaches RmlUi, then build.

For each feature family: use a script that prints line, enclosing handler and
reference text; read each whole handler; use an exact-match Python patch
script in .ai-os/scratch/ that aborts on count mismatch; preserve CRLF where
present; run a full RelWithDebInfo build and fix errors; commit the family
separately, staging only your files. Record each pass in the rollout task.
Another session has unrelated dirty/untracked files and ~246 lines of
uncommitted rollout-task edits. Do not stage those wholesale. Prior passes
staged only their new task-note hunk using a temporary blob in the Git index.
The script .ai-os/scratch/rb03_record_siege.py shows that method.

Build command (use an unsandboxed/escalated invocation if sandboxed child
processes stall):
cmd /c '"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 && cmake --build C:\Users\benit\MU\client\MuMain\out\build\windows-x64 --config RelWithDebInfo'

The local CMake cache has been restored to normal CMAKE_SUPPRESS_REGENERATION=OFF
and WEBP_ENABLE_SIMD=ON; new .cpp files now trigger GLOB_RECURSE regeneration.
LNK1104/LNK1168 usually means the game is running; ask the user to close it.
Commits use the configured identity only, with no Co-Authored-By trailer.

Work autonomously. Do not ask for a go signal between families. Defer only a
decision that genuinely cannot be weighed or would create a new tracked
deferral. If you approach the session limit, update the task and prepare a
new precise continuation prompt. The user's two clients run on one PC: the
first click after switching windows is Windows activation, not a UI bug.
```
