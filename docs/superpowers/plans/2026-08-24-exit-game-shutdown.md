# Exit Game Shutdown Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the in-game Exit Game button terminate the client instead of triggering auto-reconnect.

**Architecture:** Preserve the existing server logout request. Immediately route shutdown through the existing cross-platform `PostMessage(..., WM_CLOSE, ...)` shim, which becomes `SDL_EVENT_QUIT` outside Windows and follows SDL's native close path on Windows.

**Tech Stack:** C++20, SDL3, CMake/CTest

---

### Task 1: Exit Game shutdown contract

**Files:**
- Create: `tests/ui/test_exit_game_shutdown.cmake`
- Modify: `tests/ui/CMakeLists.txt`
- Modify: `src/source/UI/NewUI/Dialogs/NewUICustomMessageBox.cpp:2394-2398`

- [ ] **Step 1: Write the failing test**

Read `NewUICustomMessageBox.cpp`. Require
`SocketClient->ToGameServer()->SendLogOut(LogOutType::CloseGame);`, then require
`PostMessage(g_hWnd, WM_CLOSE, 0, 0);` at a later source position.

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake -DSOURCE_DIR="$PWD" -P tests/ui/test_exit_game_shutdown.cmake`

Expected: `FATAL_ERROR` stating Exit Game does not request local shutdown.

- [ ] **Step 3: Write minimal implementation**

After the existing logout send, add:

```cpp
PostMessage(g_hWnd, WM_CLOSE, 0, 0);
```

- [ ] **Step 4: Verify**

Run:

```bash
cmake -DSOURCE_DIR="$PWD" -P tests/ui/test_exit_game_shutdown.cmake
cmake --build --preset macos-arm64-release
ctest --test-dir out/build/macos-arm64 -C Release -R 'exit_game_shutdown|platform|chat_command_window_focus' --output-on-failure
git diff --check
```

Expected: contract passes; Release build succeeds; selected tests pass; no whitespace errors.
